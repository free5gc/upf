#include "utlt_network.h"

#include <unistd.h>

#include "utlt_debug.h"

Sock *UnixSockCreate(int type) {
    Sock *sock = SockCreate(AF_UNIX, type, 0);
    UTLT_Assert(sock, return NULL, "Unix Socket Create fail");

    return sock;
}

Status UnixFree(Sock *sock) {
    if (strlen(sock->localAddr.su.sun_path))
        unlink(sock->localAddr.su.sun_path);
    
    SockFree(sock);
    
    return STATUS_OK;
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

    // Delete the pipe if it is existed
    unlink(path);

    status = SockBind(sock, &sock->localAddr);
    UTLT_Assert(status == STATUS_OK, goto FREESOCK, "Unix SockBind fail");

    return sock;

FREESOCK:
    UnixFree(sock);
    return NULL;
}