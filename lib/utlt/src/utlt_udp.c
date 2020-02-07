#include "utlt_network.h"

#include "utlt_debug.h"

Sock *UdpSockCreate(int domain) {
    Sock *sock = SockCreate(domain, SOCK_DGRAM, 0);
    UTLT_Assert(sock, return NULL, "UDP Socket Create fail");

//    UTLT_Trace("Create UPD Socket %d", sock->fd);

    return sock;
}

Status UdpSockSetAddr(SockAddr *sockAddr, int domain, const char *addr, int port) {
    Status status;
    status = SockSetAddr(sockAddr, domain, addr, port);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "UDP Socket Addr Create fail");
    
//    UTLT_Trace("UDP Socket Addr : IP[%s], Port[%d]", GetIP(sockAddr), GetPort(sockAddr));

    return STATUS_OK;
}

Sock *UdpServerCreate(int domain, const char *addr, int port) {
    Status status;

    Sock *sock = UdpSockCreate(domain);
    UTLT_Assert(sock, return NULL, "UDP SockCreate fail");

    status = UdpSockSetAddr(&sock->localAddr, domain, addr, port);
    if (status != STATUS_OK) {
        status = SockFree(sock);
        UTLT_Assert(status == STATUS_OK, return NULL, "");
    }

    int opt = SO_REUSEADDR;
    SockSetOpt(sock, SOL_SOCKET, SO_REUSEADDR, &opt);

    status = SockBind(sock, &sock->localAddr);
    if (status != STATUS_OK) {
        status = SockFree(sock);
        UTLT_Assert(status == STATUS_OK, return NULL, "UDP SockBind fail");
    }

//    UTLT_Trace("Create UDP Server : IP[%s], Port[%d]", 
//                GetIP(&sock->localAddr), GetPort(&sock->localAddr));

    return sock;
}


Sock *UdpClientCreate(int domain, const char *addr, int port) {
    Status status;
    
    Sock *sock = UdpSockCreate(domain);
    UTLT_Assert(sock, return NULL, "UDP SockCreate fail");

    status = UdpSockSetAddr(&sock->remoteAddr, domain, addr, port);
    if (status != STATUS_OK) {
        status = SockFree(sock);
        UTLT_Assert(status == STATUS_OK, return NULL, "");
    }

    status = SockConnect(sock, &sock->remoteAddr);
    if (status != STATUS_OK) {
        status = SockFree(sock);
        UTLT_Assert(status == STATUS_OK, return NULL, "UDP SockConnect fail");
    }
    
//    UTLT_Trace("Connect to UDP Server : IP[%s], Port[%d]", 
//                GetIP(&sock->remoteAddr), GetPort(&sock->remoteAddr));

    return sock;
}
