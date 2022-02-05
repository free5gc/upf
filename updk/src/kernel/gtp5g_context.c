#include "gtp5g_context.h"

/*
 * These functions shall be customized by kinds of device.
 * You can create a directory and put all customized function
 * in there, like "device.c" under "updk/src/kernel/".
 *
 * This file would be included when the UP part runs with
 * Linux kernel module "gtp5g", otherwise it will not be used.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "utlt_debug.h"
#include "utlt_list.h"
#include "utlt_buff.h"
#include "utlt_network.h"
#include "gtp_link.h"
#include "gtp_path.h"
#include "libgtp5gnl/gtp5gnl.h"

#include "updk/env.h"
#include "updk/rule_pdr.h"
#include "gtp5g_buffer.h"

static Gtp5gDevice gtp5gDevice;

Gtp5gDevice *Gtp5gSelf() {
    return &gtp5gDevice;
}

void UpdkPacketReceiverThread(ThreadID id, void *data) {
    Status status;

    int nfds;
    Sock *sockPtr;
    struct epoll_event events[MAX_NUM_OF_EVENT];

    while (!ThreadStop()) {
        nfds = EpollWait(gtp5gDevice.epfd, events, 2000);
        if (nfds < 0) {
            UTLT_Assert(errno == EINTR, break, "Epoll Wait error : %s (%d)", strerror(errno), errno);
            UTLT_Level_Assert(LOG_INFO, 0, usleep(1000), "Epoll Wait : %s (%d)", strerror(errno), errno);
        }

        for (int i = 0; i < nfds; i++) {
            sockPtr = events[i].data.ptr;
            status = sockPtr->handler(sockPtr, sockPtr->data);
            // TODO : Log may show which socket
            UTLT_Level_Assert(LOG_DEBUG, status == STATUS_OK, , "Error handling UP socket");
        }
    }

    sem_post(((Thread *)id)->semaphore);
    UTLT_Trace("Packet receiver thread terminated");

    return;
}

Status UPDKGtpHandler(Sock *sock, void *data) {
    UTLT_Assert(sock, return STATUS_ERROR, "GTP socket not found");
    Status status = STATUS_OK;

    Bufblk *pktbuf = BufblkAlloc(1, MAX_OF_GTPV1_PACKET_SIZE);
    UTLT_Assert(pktbuf, return STATUS_ERROR, "create buffer error");
    int readNum = GtpRecv(sock, pktbuf);
    UTLT_Assert(readNum >= 0, status = STATUS_ERROR; goto FREEBUFBLK, "GTP receive fail");

    // All rules are set to kernel space, packet should pass by UPF
    UPDK_PDR updkPDR;
    int packetInStatus = Gtp5gSelf()->PacketInGTPU(pktbuf->buf, pktbuf->len, sock->remoteAddr.s4.sin_addr.s_addr, sock->remoteAddr._port, &updkPDR);
    UTLT_Level_Assert(LOG_DEBUG, packetInStatus > 0, status = STATUS_ERROR, "Find Rule for buffering test failed");

FREEBUFBLK:
    UTLT_Assert(BufblkFree(pktbuf) == STATUS_OK, , "Bufblk free fail");

    return status;
}

Status Gtp5gDeviceInit(VirtualDevice *dev) {
    UTLT_Assert(dev, return STATUS_ERROR,
        "VirtualDevice shall not be NULL");

    memset(&gtp5gDevice, 0, sizeof(Gtp5gDevice));

    strcpy(gtp5gDevice.ifname, dev->deviceID);

    gtp5gDevice.PacketInL3 = dev->eventCB.PacketInL3;
    gtp5gDevice.PacketInGTPU = dev->eventCB.PacketInGTPU;
    gtp5gDevice.GetPDRByID = dev->eventCB.getPDR;
    gtp5gDevice.GetFARByID = dev->eventCB.getFAR;
    gtp5gDevice.GetQERByID = dev->eventCB.getQER;

    gtp5gDevice.epfd = EpollCreate();
    UTLT_Assert(gtp5gDevice.epfd >= 0, return STATUS_ERROR, "Epoll for gtp5g device create failed");

    UTLT_Assert(ThreadCreate(&gtp5gDevice.PacketRecvThread, UpdkPacketReceiverThread, NULL) == STATUS_OK,
        goto FREEEPOLL, "UPDK receiver thread create failed");

    return STATUS_OK;

FREEEPOLL:
    close(gtp5gDevice.epfd);

    return STATUS_ERROR;
}

Status Gtp5gDeviceTerm() {
    Status status = STATUS_OK;

    UTLT_Assert(BufferServerTerm() == STATUS_OK, status = STATUS_ERROR, "BufferServerTerm fail");

    UTLT_Assert(EpollDeregisterEvent(gtp5gDevice.epfd, gtp5gDevice.sock) == STATUS_OK, status = STATUS_ERROR,
        "UPDK epoll deregister error");

    UTLT_Assert(gtp_dev_destroy(gtp5gDevice.ifname) >= 0, status = STATUS_ERROR,
                "gtp5g device named %s destroy fail", gtp5gDevice.ifname);

    UTLT_Assert(UdpFree(gtp5gDevice.sock) == STATUS_OK, status = STATUS_ERROR,
                "UDP server socket free fail : IP[%s]", gtp5gDevice.port->ipStr);

    UTLT_Assert(ThreadDelete(gtp5gDevice.PacketRecvThread) == STATUS_OK, status = STATUS_ERROR,
        "UPDK receiver thread delete failed");

    close(gtp5gDevice.epfd);

    free(gtp5gDevice.port);

    UTLT_Trace("gtp5g device destroy success");

    return status;
}

Status Gtp5gDeviceAdd(VirtualDevice *dev, VirtualPort *port) {
    UTLT_Assert(dev && port, return STATUS_ERROR,
        "VirtualDevice or VirtualPort shall not be NULL");

    Status status;

    // Create UDP socket
    gtp5gDevice.sock = UdpServerCreate(AF_INET, port->ipStr, GTP_V1_PORT);
    UTLT_Assert(gtp5gDevice.sock, return STATUS_ERROR, 
                "UDP server create fail for gtp5g: IP[%s] port[%d]", port->ipStr, GTP_V1_PORT);

    // Create gtp5g interface
    status = gtp_dev_create(-1, dev->deviceID, gtp5gDevice.sock->fd);
    UTLT_Assert(status == STATUS_OK, goto FREEUDPSOCK,
                "gtp5g device named %s created fail", dev->deviceID);

    gtp5gDevice.port = port;

    // Set MTU for gtp5g interface
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, dev->deviceID, sizeof(ifr.ifr_name));
    ifr.ifr_mtu = 1500;

    int ioctlSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    status = ioctl(ioctlSock, SIOCSIFMTU, (caddr_t)&ifr);
    close(ioctlSock); 
    UTLT_Assert(status == 0, goto FREEGTP5GINT,
        "Set MTU %d on %s failed", ifr.ifr_mtu, dev->deviceID);

    UTLT_Assert(SockRegister(gtp5gDevice.sock, UPDKGtpHandler, NULL) == STATUS_OK,
        return STATUS_ERROR, "SockRegister failed");

    UTLT_Assert(EpollRegisterEvent(gtp5gDevice.epfd, gtp5gDevice.sock) == STATUS_OK,
        goto FREEGTP5GINT, "UPDK epoll register error");

    UTLT_Assert(BufferServerInit() == STATUS_OK, goto FREEGTP5GINT, "BufferServerInit failed");

    return STATUS_OK;

FREEGTP5GINT:
    gtp_dev_destroy(gtp5gDevice.ifname);
FREEUDPSOCK:
    UdpFree(gtp5gDevice.sock);

    return STATUS_ERROR;
}
