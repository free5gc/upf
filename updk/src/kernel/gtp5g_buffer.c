#include "gtp5g_buffer.h"

#include "utlt_debug.h"
#include "utlt_network.h"

#include "gtp5g_context.h"
#include "gtp_buffer.h"
#include "updk/rule_pdr.h"
#include "updk/rule_far.h"
#include "updk/rule_qer.h"
#include "rule_tools.h"

// TODO: Need to implement
Status UPDKBufferHandler(Sock *sock, void *data) {
    UTLT_Assert(sock, return STATUS_ERROR, "Unix socket not found");

    uint8_t farAction;
    uint16_t pdrId;

    Bufblk *pktbuf = BufblkAlloc(1, MAX_OF_BUFFER_PACKET_SIZE);

    // BufferRecv return -1 if error
    int readNum = BufferRecv(sock, pktbuf, &pdrId, &farAction);
    UTLT_Assert(readNum >= 0, goto ERROR_AND_FREE, "Buffer receive fail");

    // Buffering packet only, packet should pass by UPF
    UPDK_PDR updkPDR;
    int packetInStatus = Gtp5gSelf()->PacketInL3(pktbuf->buf, pktbuf->len, &updkPDR);
    UTLT_Level_Assert(LOG_DEBUG, packetInStatus > 0, goto ERROR_AND_FREE, "Find Rule for buffering test failed");

    /* TODO: Only for debug or demo, get related FAR and QER from UPF
    UPDK_FAR updkFAR;
    if (!updkPDR.flags.farId || Gtp5gSelf()->GetFARByID(updkPDR.farId, &updkFAR) < 0) {
        UTLT_Error("From GetFARByID, found PDR #%u, but cannot get FAR #%u from UPF", updkPDR.pdrId, updkPDR.farId);
    } else {
        UTLT_Info("From GetFARByID, found PDR #%u, and found FAR #%u in UPF", updkPDR.pdrId, updkPDR.farId);
    }

    UPDK_QER updkQER;
    if (updkPDR.flags.qerId) {
        for (int i = 0; i < sizeof(updkPDR.qerId) / sizeof(uint32_t) && updkPDR.qerId[i]; i++) {
            if (Gtp5gSelf()->GetQERByID(updkPDR.qerId[i], &updkQER) < 0) {
                UTLT_Error("From GetQERByID, found PDR #%u, but cannot get QER #%u from UPF", updkPDR.pdrId, updkPDR.qerId[i]);
            } else {
                UTLT_Info("From GetQERByID, found PDR #%u, and found QER #%u in UPF", updkPDR.pdrId, updkPDR.qerId[i]);
                ShowQERIEsInInfoLog(&updkQER);
            }
        }   
    }
    */

    BufblkFree(pktbuf);

    return STATUS_OK;

ERROR_AND_FREE:
    UTLT_Assert(BufblkFree(pktbuf) == STATUS_OK, , "Bufblk free fail");
    return STATUS_ERROR;
}

Status BufferServerInit() {
    strcpy(Gtp5gSelf()->unixPath, "/tmp/free5gc_unix_sock");
    
    Gtp5gSelf()->unixSock = BufferServerCreate(SOCK_DGRAM, Gtp5gSelf()->unixPath, UPDKBufferHandler, NULL);
    UTLT_Assert(Gtp5gSelf()->unixSock, return STATUS_ERROR, "UnixServerCreate failed");

    UTLT_Assert(EpollRegisterEvent(Gtp5gSelf()->epfd, Gtp5gSelf()->unixSock) == STATUS_OK,
        goto UNIXSOCKFREE, "UPDK epoll register error");

    return STATUS_OK;

UNIXSOCKFREE:
    UnixFree(Gtp5gSelf()->unixSock);

    return STATUS_ERROR;
}

Status BufferServerTerm() {
    Status status = STATUS_OK;

    UTLT_Assert(EpollDeregisterEvent(Gtp5gSelf()->epfd, Gtp5gSelf()->unixSock) == STATUS_OK, status = STATUS_ERROR,
        "UPDK epoll deregister error");

    UTLT_Assert(UnixFree(Gtp5gSelf()->unixSock) == STATUS_OK, status = STATUS_ERROR, "Unix Socket free failed");
    
    return status;
}
