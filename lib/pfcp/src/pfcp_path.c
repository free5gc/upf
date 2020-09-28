#define TRACE_MODULE _pfcp_path

#include <errno.h>

#include "utlt_debug.h"
#include "utlt_3gppTypes.h"
#include "utlt_network.h"
#include "utlt_buff.h"

#include "pfcp_node.h"

#include "pfcp_path.h"

Status PfcpServer(SockNode *snode, SockHandler handler) {
    Status status;

    UTLT_Assert(snode, return STATUS_ERROR, "socket node error");

    // TODO: config - check if snode->ip is already set when parsing config
    snode->sock = UdpServerCreate(AF_INET, snode->ip, 8805);
    status = SockRegister(snode->sock, handler, NULL);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Handler register error");

    UTLT_Trace("PfcpServer() [%s]:%d\n", GetIP(&snode->sock->localAddr), 8805);

    return STATUS_OK;
}

Status PfcpClient(PfcpNode *node) {
    //Status status;

    UTLT_Assert(node, return STATUS_ERROR, "Client Node error");

    node->sock = UdpServerCreate(AF_INET, GetIP(&node->sock->localAddr), 8805);
    //UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "UDP Client Create error");

    UTLT_Trace("PfcpServer() [%s]:%d\n", GetIP(&node->sock->localAddr), 8805);

    return STATUS_OK;
}

Status PfcpConnect(Sock *sockv4, Sock *sockv6, PfcpNode *node) {
    SockAddr *addr;

    UTLT_Assert(sockv4 || sockv6, return STATUS_ERROR, "No Socket of IPv4 & IPv6");
    UTLT_Assert(node, return STATUS_ERROR, "PFCP Node error");
    UTLT_Assert(node->saList, return STATUS_ERROR, "PFCP node SockAddr error");

    addr = node->saList;
    while (addr) {
        Sock *sockId;

        if (addr->_family == AF_INET) {
            sockId = sockv4;
        } else if (addr->_family == AF_INET6) {
            sockId = sockv6;
        } else {
            UTLT_Assert(0, return STATUS_ERROR, "Neither IPv4 nor IPv6");
        }
        if (sockId) {
            if (SockConnect(sockId, addr) == STATUS_OK) {
                UTLT_Trace("pfcp_connect() [%s]:%d\n",
                        GetIP(addr), GetPort(addr));

                node->sock = sockId;
                break;
            }
        }

        addr = addr->next;
    }

    if (addr == NULL) {
        UTLT_Warning("pfcp_connect() [%s]:%d failed(%d:%s)",
               GetIP(node->saList), GetPort(node->saList),
               errno, strerror(errno));
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

Status PfcpServerList(ListHead *list, SockHandler handler, int epfd) {
    Status status;
    SockNode *node, *nextNode = NULL;

    UTLT_Assert(list, return STATUS_ERROR, "Server list error");
    UTLT_Assert(handler, return STATUS_ERROR, "handler error");

    ListForEachSafe(node, nextNode, list) {
        status = PfcpServer(node, handler);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "create server error");
        status = EpollRegisterEvent(epfd, node->sock);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "PFCP Sock Register to epoll error");
    }
    

    return STATUS_OK;
}

Sock *PfcpLocalSockFirst(ListHead *list) {
    SockNode *node, *nextNode = NULL;
    Sock *sock = NULL;

    UTLT_Assert(list, return NULL, "list error");

    ListForEachSafe(node, nextNode, list) {
        sock = node->sock;
        if (sock) {
            return sock;
        }
    }

    return NULL;
}

SockAddr *PfcpLocalAddrFirst(ListHead *list) {
    SockNode *node, *nextNode = NULL;
    SockAddr *addr = NULL;

    UTLT_Assert(list, return NULL, "list error");

    ListForEachSafe(node, nextNode, list) {
        addr = &node->sock->localAddr;
        if (addr) {
            return addr;
        }
    }

    return NULL;
}

Status PfcpReceive(Sock *sock, Bufblk **bufBlk) {
    ssize_t size;

    UTLT_Assert(sock, return STATUS_ERROR, "socket error");

    *bufBlk = BufblkAlloc(0, MAX_SDU_LEN);
    if ((*bufBlk) == NULL) {
        char tmpBuf[MAX_SDU_LEN];

        UTLT_Fatal("Can't allocate buffer");

        /* Read data from socket to exit from select */
        SockRead(sock, tmpBuf, MAX_SDU_LEN);

        return STATUS_ERROR;
    }

    size = SockRecvFrom(sock, (*bufBlk)->buf, (*bufBlk)->len);
    if (size <= 0) {
        BufblkFree(*bufBlk);

        if (errno != EAGAIN) {
            UTLT_Warning("socket Read failed(%d:%s)", errno, strerror(errno));
        }

        return STATUS_ERROR;
    } else {
        (*bufBlk)->len = size;

        return STATUS_OK;
    }
}

Status PfcpReceiveFrom(Sock *sock, Bufblk **bufBlk, SockAddr *from) {
    int size;

    UTLT_Assert(sock, return STATUS_ERROR, "socket error");
    UTLT_Assert(from, return STATUS_ERROR, "Source socket error");

    *bufBlk = BufblkAlloc(1, MAX_SDU_LEN);

    if (*bufBlk == NULL) {
        char tmpBuf[MAX_SDU_LEN];

        UTLT_Fatal("Can't allocate Buffer");

        SockRecvFrom(sock, tmpBuf, MAX_SDU_LEN);
        return STATUS_ERROR;
    }

    size = UdpRecvFrom(sock, (*bufBlk)->buf, (*bufBlk)->size);

    UTLT_Debug("PFCP Receive packet: (local bind) %s:%d, (remote get) %s:%d",
               GetIP(&sock->localAddr), GetPort(&sock->localAddr),
               GetIP(&sock->remoteAddr), GetPort(&sock->remoteAddr));

    if (size <= 0) {
        BufblkFree(*bufBlk);

        if (errno != EAGAIN) {
            UTLT_Warning("SockRecvFromAddr failed(%d:%s)", errno,
                         strerror(errno));
        }

        return STATUS_ERROR;
    } else {
        (*bufBlk)->len = size;
        *from = sock->remoteAddr;

        return STATUS_OK;
    }
}

Status PfcpSend(PfcpNode *node, Bufblk *bufBlk) {
    Sock *sock = NULL;
    SockAddr *addr = NULL;

    UTLT_Assert(node, return STATUS_ERROR, "No PfcpNode");
    UTLT_Assert(bufBlk, return STATUS_ERROR, "No Bufblk");
    sock = node->sock;
    UTLT_Assert(sock, return STATUS_ERROR, "No sock of node");

    /* New Interface */
    addr = &(sock->remoteAddr);
    UTLT_Assert(addr, return STATUS_ERROR, "remote addr error");
    UTLT_Assert(bufBlk, , "buff NULL");
    UTLT_Assert(bufBlk->buf, , "buff buff NULL");

    Status status = SockSendTo(sock, bufBlk->buf, bufBlk->len);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
            "Sent [%s]:%d failed(%d:%s)", GetIP(addr), GetPort(addr), errno, strerror(errno));

    return STATUS_OK;
}


