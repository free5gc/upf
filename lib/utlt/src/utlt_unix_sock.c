#include "utlt_network.h"

#include "utlt_debug.h"

Sock *UnixSockCreate(int type) {
    Sock *sock = SockCreate(AF_UNIX, type, 0);
    UTLT_Assert(sock, return NULL, "Unix Socket Create fail");

    return sock;
}

Status UnixSockSetAddr(SockAddr *sockAddr, const char *path) {
    Status status;
    status = SockSetAddr(sockAddr, AF_UNIX, path, 0);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Unix Socket Addr Create fail");

    return STATUS_OK;
}

Sock *UnixServerCreate(int type, const char *path) {
    Status status;

    Sock *sock = UnixSockCreate(type);
    UTLT_Assert(sock, return NULL, "Unix SockCreate fail");

    status = UnixSockSetAddr(&sock->localAddr, path);
    if (status != STATUS_OK)
        goto FREESOCK;

    int opt = SO_REUSEADDR;
    SockSetOpt(sock, SOL_SOCKET, SO_REUSEADDR, &opt);

    status = SockBind(sock, &sock->localAddr);
    UTLT_Assert(status == STATUS_OK, goto FREESOCK, "Unix SockBind fail");

    return sock;

FREESOCK:
    SockFree(sock);
    return NULL;
}