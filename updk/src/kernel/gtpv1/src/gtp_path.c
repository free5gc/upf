#include "gtp_path.h"

#include <arpa/inet.h>

#include "utlt_debug.h"
#include "utlt_list.h"
#include "utlt_buff.h"
#include "utlt_network.h"
#include "gtp_link.h"
#include "utlt_netheader.h"

Sock *GtpServerCreate(int domain, const char *addr, int port,
                      SockHandler handler, void *data) {
    Status status;  
    
    Sock *sock = UdpServerCreate(domain, addr, port);
    UTLT_Assert(sock, return NULL, "GtpServerCreate fail");

    status = SockRegister(sock, handler, data);
    UTLT_Assert(status == STATUS_OK, UdpFree(sock); return NULL,
                "SockRegister fail");

    return sock;
}

Status GtpServerFree(Sock *sock) {
    UTLT_Assert(sock, return STATUS_OK, "");
    Status status = STATUS_OK;

    UTLT_Assert(SockUnregister(sock) == STATUS_OK, status |= STATUS_ERROR,
                "GTP socket unregister fail");

    UTLT_Assert(UdpFree(sock) == STATUS_OK, status |= STATUS_ERROR,
                "GTP socket free fail");

    return status;
}

Sock *GtpClientCreate(int domain, const char *addr, int port) {
    return UdpClientCreate(domain, addr, port);
}

Status GtpClientFree(Sock *sock) {
    return UdpFree(sock);
}

Status GtpTunCreate(Gtpv1TunDevNode *node, SockHandler handler, void *data) {
    Status status;

    status = GtpLinkCreate(node);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "GtpLinkCreate fail");

    status = SockRegister(node->sock, handler, data);
    if (status != STATUS_OK) {
        status = GtpLinkFree(node);
        UTLT_Assert(status == STATUS_OK, , "GtpLinkFree fail");
        UTLT_Error("SockRegister fail");
    }

    return STATUS_OK;
}

Status GtpTunFree(Gtpv1TunDevNode *node) {
    UTLT_Assert(node, return STATUS_ERROR,
                "GTPv1 tunnel node is NULL");
    Status status = STATUS_OK;

    UTLT_Assert(SockUnregister(node->sock) == STATUS_OK, status |= STATUS_ERROR,
                "GTP tunnel socket unregister fail");

    UTLT_Assert(GtpLinkFree(node) == STATUS_OK, status |= STATUS_ERROR,
                "GTP tunnel device named %s free fail", node->ifname);

    return status;
}

int GtpRecv(Sock *sock, Bufblk *pktbuf) {
    UTLT_Assert(sock && pktbuf, return -1, "Socket or pktbuf pointer is NULL");

    if (pktbuf->size != MAX_OF_GTPV1_PACKET_SIZE) {
        UTLT_Assert(BufblkResize(pktbuf, 1, MAX_OF_GTPV1_PACKET_SIZE) == STATUS_OK,
                    return -1, "Buffer is not enough");
    }

    int readNum = UdpRecvFrom(sock, pktbuf->buf, MAX_OF_GTPV1_PACKET_SIZE);
    UTLT_Assert(readNum >= 0, return readNum, "GtpRecv fail");

    pktbuf->len = readNum;
    return pktbuf->len;
}

Status GtpSend(Sock *sock, Bufblk *pktbuf) {
    UTLT_Assert(sock && pktbuf, return STATUS_ERROR,
                "Socket or pktbuf pointer is NULL");

    return UdpSendTo(sock, pktbuf->buf, pktbuf->len);
}

Status GtpEpollRegister(int epfd, Sock *sock) {
    return EpollRegisterEvent(epfd, sock);
}

Status GtpEpollDeregister(int epfd, Sock *sock) {
    return EpollDeregisterEvent(epfd, sock);
}

Status GtpServerListCreate(int epfd, int domain, ListHead *sockList, int port, 
                           SockHandler handler, void *data) {
    UTLT_Assert(sockList, return STATUS_ERROR, "Socket List is NULL");
    SockNode *it, *nextIt = NULL; 

    ListForEachSafe(it, nextIt, sockList) {
        it->sock = GtpServerCreate(domain, it->ip, port, handler, data);
        UTLT_Assert(it->sock, return STATUS_ERROR,
                    "GTPv1 socket create fail : IP[%s]", it->ip);
        UTLT_Assert(GtpEpollRegister(epfd, it->sock) == STATUS_OK, return STATUS_ERROR,
                    "GTPv1 socket register to epoll fail : IP[%s]", it->ip);
    }
    
    return STATUS_OK;
}

Status GtpServerListFree(int epfd, ListHead *sockList) {
    UTLT_Assert(sockList, return STATUS_ERROR, "Socket List is NULL");
    Status status = STATUS_OK;
    SockNode *it, *nextIt = NULL;

    ListForEachSafe(it, nextIt, sockList) {
        UTLT_Assert(GtpEpollDeregister(epfd, it->sock) == STATUS_OK, status |= STATUS_ERROR,
                    "GTPv1 socket deregister to epoll fail : IP[%s]", it->ip);
        UTLT_Assert(GtpServerFree(it->sock) == STATUS_OK, status |= STATUS_ERROR,
                    "GTPv1 socket free fail : IP[%s]", it->ip);
    }

    return status;
}

Status GtpDevListCreate(int epfd, int domain, ListHead *sockList,
                        SockHandler handler, void *data) {
    UTLT_Assert(sockList, return STATUS_ERROR, "Socket List is NULL");
    Gtpv1TunDevNode *it, *nextIt = NULL;
    
    ListForEachSafe(it, nextIt, sockList) {
        Status status;
        status = GtpTunCreate(it, handler, data);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                    "GTPv1 tunnel create fail : IP[%s], ifname[%s]",
                    it->ip, it->ifname);

        status = GtpEpollRegister(epfd, it->sock);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                    "GTPv1 tunnel register to epoll fail : IP[%s]", it->ip);
    }

    return STATUS_OK;
}

Status GtpDevListFree(int epfd, ListHead *sockList) {
    UTLT_Assert(sockList, return STATUS_ERROR, "Socket List is NULL");
    Status status = STATUS_OK;
    Gtpv1TunDevNode *it, *nextIt = NULL;
    
    ListForEachSafe(it, nextIt, sockList) {
        status = GtpEpollDeregister(epfd, it->sock);
        UTLT_Assert(status == STATUS_OK, status |= STATUS_ERROR,
                    "GTPv1 tunnel deregister to epoll fail : IP[%s]", it->ip);

        status = GtpTunFree(it);
        UTLT_Assert(status == STATUS_OK, status |= STATUS_ERROR,
                    "GTPv1 tunnel free fail : IP[%s]", it->ip);
    }

    return status;
}

SockNode *GtpFindSockNodeByIp(ListHead *list, Ip *ip) {
    UTLT_Assert(list, return NULL, "Socket node list is NULL");
    UTLT_Assert(ip && (ip->ipv4 || ip->ipv6), return NULL, "Target IP is invalid");

    struct in_addr *targetIpv4;
    struct in6_addr *targetIpv6;
    if (ip->ipv4 && ip->ipv6) {
        targetIpv4 = &ip->dualStack.addr4;
        targetIpv6 = &ip->dualStack.addr6;
    } else {
        targetIpv4 = (ip->ipv4 ? &ip->addr4 : NULL);
        targetIpv6 = (ip->ipv6 ? &ip->addr6 : NULL);
    }

    int check = 0;
    Sock *sockPtr = NULL;

    SockNode *it, *nextIt = NULL;
    
    ListForEachSafe(it, nextIt, list) {
        sockPtr = it->sock;
        UTLT_Assert(sockPtr, return NULL, "Socket node is NULL");

        check = ip->ipv4 + ip->ipv6;
        for (SockAddr *sockAddr = &sockPtr->localAddr; sockAddr; sockAddr = sockAddr->next) {
            if (targetIpv4 && sockAddr->_family == AF_INET &&
                memcmp(targetIpv4, &sockAddr->s4.sin_addr, sizeof(struct in_addr)) == 0) {
                check--;
            }
            if (targetIpv6 && sockAddr->_family == AF_INET6 &&
                memcmp(targetIpv6, &sockAddr->s6.sin6_addr, sizeof(struct in6_addr)) == 0) {
                check--;
            }
        }

        if (check == 0) {
            return it;
        }

    }

    return NULL;
}

// TODO : Need to handle IPv4 and IPv6
SockNode *GtpAddSockNodeWithIp(ListHead *list, Ip *ip, int port) {
    UTLT_Assert(list, return NULL, "Socket node list is NULL");
    UTLT_Assert(ip && (ip->ipv4 || ip->ipv6), return NULL, "IP is invalid");
    UTLT_Assert(port >= 0 && port < 65536, return NULL, "Port is invalid");

    char ipBuf[INET6_ADDRSTRLEN];

    if (ip->ipv4) {
        inet_ntop(AF_INET, &ip->addr4, ipBuf, INET_ADDRSTRLEN);
    } else {
        inet_ntop(AF_INET6, &ip->addr6, ipBuf, INET6_ADDRSTRLEN);
    }
    return SockNodeListAdd(list, ipBuf);
}

Status GtpBuildEchoRequest(Bufblk *pktbuf, int teid, int seq) {
    UTLT_Assert(pktbuf, return STATUS_ERROR, "Packet buffer is NULL");

    Gtpv1Header gtpEchoReqHdr = {
        .flags = 0x32,
        .type = GTPV1_ECHO_REQUEST,
        ._length = htons(GTPV1_OPT_HEADER_LEN),
        ._teid = htonl(teid),
    };
    BufblkBytes(pktbuf, (void *) &gtpEchoReqHdr, GTPV1_HEADER_LEN);

    Gtpv1OptHeader gtpOptHdr = {
        ._seqNum = htons(seq),
    };
    BufblkBytes(pktbuf, (void *) &gtpOptHdr, GTPV1_OPT_HEADER_LEN);

    return STATUS_OK;
}

Status GtpBuildEndMark(Bufblk *pktbuf, int teid) {
    UTLT_Assert(pktbuf, return STATUS_ERROR, "Packet buffer is NULL");

    Gtpv1Header gtpEndMarkHdr = {
        .flags = 0x20,
        .type = GTPV1_END_MARK,
        ._teid = htonl(teid),
    };
    BufblkBytes(pktbuf, (void *) &gtpEndMarkHdr, GTPV1_HEADER_LEN);

    return STATUS_OK;
}
