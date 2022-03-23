#include "upf_init.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>

#include "utlt_lib.h"
#include "utlt_debug.h"
#include "utlt_buff.h"
#include "utlt_thread.h"
#include "utlt_timer.h"
#include "utlt_network.h"
#include "upf_context.h"
#include "upf_config.h"
#include "up/up_path.h"
#include "n4/n4_pfcp_path.h"
#include "pfcp_xact.h"

#include "updk/env.h"
#include "updk/init.h"

static Status SignalRegister(void *data);

static Status ConfigHandle(void *data);

static Status EpollInit(void *data);
static Status EpollTerm(void *data);

static Status EventQueueInit(void *data);
static Status EventQueueTerm(void *data);

static Status PacketRecvThreadInit(void *data);
static Status PacketRecvThreadTerm(void *data);

static Status Gtpv1Init(void *data);
static Status Gtpv1Term(void *data);

static Status PfcpInit(void *data);
static Status PfcpTerm(void *data);

void PacketReceiverThread(ThreadID id, void *data);

static char configFilePath[MAX_FILE_PATH_STRLEN] = "./config/upfcfg.yaml";
static char nfLogFilePath[MAX_FILE_PATH_STRLEN] = "./log/upf.log";
static char free5gcLogFilePath[MAX_FILE_PATH_STRLEN] = "";

static pthread_mutex_t signalHandlerLock;
static uint8_t signalHandlerFlag;

UpfOps UpfOpsList[] = {
    {
        .name = "Library - Bufblk Pool",
        .init = BufblkPoolInit,
        .initData = NULL,
        .term = BufblkPoolFinal,
        .termData = NULL,
    },
    {
        .name = "Library - Thread",
        .init = ThreadInit,
        .initData = NULL,
        .term = ThreadFinal,
        .termData = NULL,
    },
    {
        .name = "Library - Timer Pool",
        .init = TimerPoolInit,
        .initData = NULL,
        .term = TimerFinal,
        .termData = NULL,
    },
    {
        .name = "Library - Socket Pool",
        .init = SockPoolInit,
        .initData = NULL,
        .term = SockPoolFinal,
        .termData = NULL,
    },
    {
        .name = "UPF - Context",
        .init = UpfContextInit,
        .initData = NULL,
        .term = UpfContextTerminate,
        .termData = NULL,
    },
    {
        .name = "UPF - Signal Registration",
        .init = SignalRegister,
        .initData = NULL,
        .term = NULL,
        .termData = NULL,
    },
    {
        .name = "UPF - Config Handle",
        .init = ConfigHandle,
        .initData = &configFilePath,
        .term = NULL,
        .termData = NULL,
    },
    {
        .name = "UPF - Epoll",
        .init = EpollInit,
        .initData = NULL,
        .term = EpollTerm,
        .termData = NULL,
    },
    {
        .name = "UPF - Event Queue",
        .init = EventQueueInit,
        .initData = NULL,
        .term = EventQueueTerm,
        .termData = NULL,
    },
    {
        .name = "UPF - Thread",
        .init = PacketRecvThreadInit,
        .initData = PacketReceiverThread,
        .term = PacketRecvThreadTerm,
        .termData = NULL,
    },
    {
        .name = "UPF - Environment Init",
        .init = Gtpv1Init,
        .initData = NULL,
        .term = Gtpv1Term,
        .termData = NULL,
    },
    {
        .name = "UPF - PFCP",
        .init = PfcpInit,
        .initData = NULL,
        .term = PfcpTerm,
        .termData = NULL,
    },
    // TODO: This part will be abstract as GtpEnvInit
    /*
    {
        .name = "UPF - Routing Setting",
        .init = UpRouteInit,
        .initData = NULL,
        .term = UpRouteTerminate,
        .termData = NULL,
    },
    {
        .name = "UPF - Buffer Server",
        .init = BufferServerInit,
        .initData = NULL,
        .term = BufferServerTerminate,
        .termData = NULL,
    },
    */
};

Status UpfSetConfigPath(char *path) {
    if (path == NULL) {
        UTLT_Error("Configuration is null point");
        return STATUS_ERROR;
    }

    if (strlen(path) > (sizeof(configFilePath) - 1)) {
        UTLT_Error("Configuration path length (%d) > buffer length (%d)", strlen(path), (sizeof(configFilePath) - 1));
        return STATUS_ERROR;
    }
    strcpy(configFilePath, path);
    return STATUS_OK;
}

Status UpfSetNfLogPath(char *path) {
    if (path == NULL) {
        UTLT_Error("NF path is null point");
        return STATUS_ERROR;
    }

    if (strlen(path) > (sizeof(nfLogFilePath) - 1)) {
        UTLT_Error("NF log path length (%d) > buffer length (%d)", strlen(path), (sizeof(nfLogFilePath) - 1));
        return STATUS_ERROR;
    }
    strcpy(nfLogFilePath, path);
    return STATUS_OK;
}

Status UpfSetFree5gcLogPath(char *path) {
    if (path == NULL) {
        UTLT_Error("Free5gc path is null point");
        return STATUS_ERROR;
    }

    if (strlen(path) > (sizeof(free5gcLogFilePath) - 1)) {
        UTLT_Error("Free5GC log path length (%d) > buffer length (%d)", strlen(path), (sizeof(free5gcLogFilePath) - 1));
        return STATUS_ERROR;
    }
    strcpy(free5gcLogFilePath, path);
    return STATUS_OK;
}

Status UpfInit() {
    Status status = STATUS_OK;

    UTLT_Assert(UTIL_LogFileHook(nfLogFilePath, free5gcLogFilePath) == STATUS_OK,
        return STATUS_ERROR, "Log file hool error, nf path: %s, free5gc path: %s", nfLogFilePath, free5gcLogFilePath);

    if (strlen(free5gcLogFilePath) > 0) {
        UTLT_Assert(GetAbsPath(free5gcLogFilePath) == STATUS_OK,
            return STATUS_ERROR, "Invalid free5gc log path: %s", free5gcLogFilePath);
        UTLT_Info("Free5GC log: %s", free5gcLogFilePath);
    }
    if (strlen(nfLogFilePath) > 0) {
        UTLT_Assert(GetAbsPath(nfLogFilePath) == STATUS_OK,
            return STATUS_ERROR, "Invalid UPF log path: %s", nfLogFilePath);
        UTLT_Info("UPF log: %s", nfLogFilePath);
    }

    UTLT_Assert(GetAbsPath(configFilePath) == STATUS_OK, 
        return STATUS_ERROR, "Invalid config path: %s", configFilePath);
    UTLT_Info("Config: %s", configFilePath);

    for (int i = 0; i < sizeof(UpfOpsList) / sizeof(UpfOps); i++) {
        if (UpfOpsList[i].init) {
            status = UpfOpsList[i].init(UpfOpsList[i].initData);
            UTLT_Assert(status == STATUS_OK, status |= STATUS_ERROR; break,
                "%s error when UPF initializes", UpfOpsList[i].name);

            UTLT_Trace("%s is finished in UPF initialization", UpfOpsList[i].name);
        }
    }
    return status;
}

Status UpfTerm() {
    Status status = STATUS_OK;
    for (int i = (int)(sizeof(UpfOpsList) / sizeof(UpfOps)) - 1; i >= 0 ; i--) {
        if (UpfOpsList[i].term) {
            status = UpfOpsList[i].term(UpfOpsList[i].termData);
            UTLT_Assert(status == STATUS_OK, status |= STATUS_ERROR,
                "%s error when UPF terminates", UpfOpsList[i].name);

            UTLT_Trace("%s is finished in UPF termination", UpfOpsList[i].name);
        }
    }
    
    return status;
}

static void SignalHandler(int sigval) {
    pthread_mutex_lock(&signalHandlerLock);
    if (signalHandlerFlag){
        pthread_mutex_unlock(&signalHandlerLock);
        return;
    }
    signalHandlerFlag = 1;
    pthread_mutex_unlock(&signalHandlerLock);
    switch(sigval) {
        case SIGINT :
            UTLT_Assert(UpfTerm() == STATUS_OK, , "Handle Ctrl-C fail");
            break;
        case SIGTERM :
            UTLT_Assert(UpfTerm() == STATUS_OK, , "Handle Ctrl-C fail");
            break;
        default :
            break;
    }
    exit(0);
}

static Status SignalRegister(void *data) {
    signalHandlerFlag = 0;
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    return STATUS_OK;
}

static Status ConfigHandle(void *data) {
    UTLT_Assert(UpfLoadConfigFile(configFilePath) == STATUS_OK,
        return STATUS_ERROR, "");

    UTLT_Assert(UpfConfigParse() == STATUS_OK,
        return STATUS_ERROR, "");

    return STATUS_OK;
}

static Status EpollInit(void *data) {
    UTLT_Assert((Self()->epfd = EpollCreate()) >= 0,
        return STATUS_ERROR, "");

    return STATUS_OK;
}

static Status EpollTerm(void *data) {
    close(Self()->epfd);

    return STATUS_OK;
}

static Status EventQueueInit(void *data) {
    Self()->eventQ = EventQueueCreate(O_RDWR);
    UTLT_Assert(Self()->eventQ > 0, return STATUS_ERROR, "");

    return STATUS_OK;
}

static Status EventQueueTerm(void *data) {
    UTLT_Assert(EventQueueDelete(Self()->eventQ) == STATUS_OK,
        return STATUS_ERROR, "");

    return STATUS_OK;
}

static Status PacketRecvThreadInit(void *data) {
    ThreadFuncType threadFuncPtr = data;
    
    UTLT_Assert(ThreadCreate(&Self()->pktRecvThread, threadFuncPtr, NULL) == STATUS_OK,
        return STATUS_ERROR, "");

    return STATUS_OK;
}

static Status PacketRecvThreadTerm(void *data) {
    UTLT_Assert(ThreadDelete(Self()->pktRecvThread) == STATUS_OK,
        return STATUS_ERROR, "");

    return STATUS_OK;
}

void PacketReceiverThread(ThreadID id, void *data) {
    Status status;

    int nfds;
    Sock *sockPtr;
    struct epoll_event events[MAX_NUM_OF_EVENT];
    utime_t prev, now; // For timer checking purpose

    prev = TimeNow();

    while (!ThreadStop()) {
        nfds = EpollWait(Self()->epfd, events, 300);
        if (nfds < 0) {
            UTLT_Assert(errno == EINTR, break, "Epoll Wait error : %s (%d)", strerror(errno), errno);
            UTLT_Level_Assert(LOG_INFO, 0, usleep(1000), "Epool Wait : %s (%d)", strerror(errno), errno);
        }

        for (int i = 0; i < nfds; i++) {
            sockPtr = events[i].data.ptr;
            status = sockPtr->handler(sockPtr, sockPtr->data);
            // TODO : Log may show which socket
            UTLT_Assert(status == STATUS_OK, , "Error handling UP socket");
        }

        // Check if timer expired
        now = TimeNow();
        if (now - prev > 10) {
            TimerExpireCheck(
                    &Self()->timerServiceList, Self()->eventQ);

            prev = now;
        }

    }

    sem_post(((Thread *)id)->semaphore);
    UTLT_Trace("Packet receiver thread terminated");

    return;
}

static Status Gtpv1Init(void *data) {
    Self()->upSock.fd = Gtpv1EnvInit(Self()->envParams);
    UTLT_Assert(Self()->upSock.fd != -1, return STATUS_OK, "");

    socklen_t addrlen = 0;
    UTLT_Assert(getsockname(Self()->upSock.fd, &Self()->upSock.localAddr.sa, &addrlen) == 0,
        return STATUS_ERROR, "Get address from fd failed");

    if (addrlen == INET_ADDRSTRLEN) {
        Self()->upSock.localAddr._family = AF_INET;
        Self()->upSock.localAddr.s4.sin_port = ntohs(Self()->gtpv1Port);
    }
    else if (addrlen == INET6_ADDRSTRLEN) {
        Self()->upSock.localAddr._family = AF_INET6;
        Self()->upSock.localAddr.s6.sin6_port = ntohs(Self()->gtpv1Port);
    }
    else
        UTLT_Warning("Do Not Support this protocol in L3");

    return STATUS_OK;
}

static Status Gtpv1Term(void *data) {
    Status status = STATUS_OK;

    UpfSessionRemoveAll();

    UTLT_Assert(Gtpv1EnvTerm(Self()->envParams) == 0,
        status |= STATUS_ERROR, "");

    return status;
}

static Status PfcpInit(void *data) {
    Status status = STATUS_OK;
    UTLT_Assert(PfcpServerInit() == STATUS_OK,
        status |= STATUS_ERROR, "");

    // init pfcp xact context
    UTLT_Assert(PfcpXactInit(&Self()->timerServiceList,
                    UPF_EVENT_N4_T3_RESPONSE, UPF_EVENT_N4_T3_HOLDING) == STATUS_OK,
        status |= STATUS_ERROR, "");

    return status;
}

static Status PfcpTerm(void *data) {
    Status status = STATUS_OK;
    UTLT_Assert(PfcpXactTerminate() == STATUS_OK,
        status |= STATUS_ERROR, "");

    UTLT_Assert(PfcpServerTerminate() == STATUS_OK,
        status |= STATUS_ERROR, "");

    return status;
}
