#include "utlt_network.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "utlt_pool.h"
#include "utlt_debug.h"
#include "utlt_list.h"

#define MAX_NUM_OF_SOCK 1024
#define MAX_NUM_OF_SOCK_NODE 512

PoolDeclare(sockPool, Sock, MAX_NUM_OF_SOCK);
PoolDeclare(sockNodePool, SockNode, MAX_NUM_OF_SOCK_NODE);

Sock *SocketAlloc();

Status SockPoolInit() {
    PoolInit(&sockPool, MAX_NUM_OF_SOCK);
    PoolInit(&sockNodePool, MAX_NUM_OF_SOCK_NODE);

    return STATUS_OK;
}

Status SockPoolFinal() {
    PoolTerminate(&sockPool);
    PoolTerminate(&sockNodePool);

    return STATUS_OK;
}

Sock *SocketAlloc() {
    Sock *sock = NULL;
    PoolAlloc(&sockPool, sock);
    UTLT_Assert(sock, return NULL, "Socket pool is empty");

    memset(sock, 0, sizeof(Sock));
    sock->fd = -1;
    sock->rflag = 0;
    sock->wflag = 0;

    sock->handler = NULL;
    sock->data = NULL;

    sock->epollMode = EPOLLIN;

    return sock;
}

Status SockFree(Sock *sock) {
    if (!sock)
        return STATUS_ERROR;

    if (sock->fd >= 0) {
        close(sock->fd);
    }

    sock->fd = -1;
    PoolFree(&sockPool, sock);

    return STATUS_OK;
}

Status SockListFree(ListHead *list) {
    Status status;
    SockNode *node, *nextNode = NULL;

    UTLT_Assert(list, return STATUS_ERROR, "null list");

    ListForEachSafe(node, nextNode, list) {
        status = SockFree(node->sock);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    }

    return STATUS_OK;
}

SockNode *SockNodeListAdd(ListHead *list, const char *ip) {
    UTLT_Assert(list, return NULL, "");
    UTLT_Assert(strlen(ip) < INET6_ADDRSTRLEN, return NULL, "");

    SockNode *node;
    PoolAlloc(&sockNodePool, node);
    UTLT_Assert(node, return NULL, "");
    ListInsert(node, list);
    strcpy(node->ip, ip);

    return node;
}

Status SockNodeListFree(ListHead *list) {
    UTLT_Assert(list, return STATUS_ERROR, "");

    SockNode *node, *nextNode;

    ListForEachSafe(node, nextNode, list) { 
        ListRemove(node);
    }

    return STATUS_OK;
}

Sock *SockCreate(int domain, int type, int protocol) {
    Sock *sock = SocketAlloc();
    UTLT_Assert(sock, return NULL, "");

    sock->localAddr._family = domain;
    sock->fd = socket(domain, type, protocol); 
    if (sock->fd < 0) {
        SockFree(sock);
        UTLT_Assert(0, return NULL, "Socket create error : %s", strerror(errno));
    }

//    UTLT_Trace("Socket Create : socket(%d, %d, %d)", domain, type, protocol);

    return sock;
}

int SockBind(Sock *sock, SockAddr *sockAddr) {
    Status status;
    UTLT_Assert(sock && sockAddr, return STATUS_ERROR, "")

    switch (sockAddr->_family) {
        case AF_UNIX:
            UTLT_Assert(bind(sock->fd, &(sockAddr->sa), SockAddrLen(sockAddr)) >= 0,
                        return STATUS_ERROR,
                        "Unix socket bind fail : %s", strerror(errno));
            break;
        default:
            status = bind(sock->fd, &(sockAddr->sa), SockAddrLen(sockAddr));
            UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, 
                        "Socket bind fail : %s", strerror(errno));
    }

    memcpy(&sock->localAddr, sockAddr, SockAddrLen(sockAddr));

    return STATUS_OK;
}

Status SockConnect(Sock *sock, SockAddr *sockAddr) {
    Status status;
    UTLT_Assert(sock && sockAddr, return STATUS_ERROR, "");
    
    status = connect(sock->fd, &sockAddr->sa, SockAddrLen(sockAddr));
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, 
                "Connect error : %s", strerror(errno));
    
    memcpy(&sock->remoteAddr, sockAddr, SockAddrLen(sockAddr));
//    UTLT_Trace("Socket Connect : ip = %s, port = %d", GetIP(&sock->localAddr), GetPort(&sock->localAddr));

    return STATUS_OK;
}

Status SockListen(Sock *sock, int queueLen) {
    Status status;
    UTLT_Assert(sock && queueLen, return STATUS_ERROR, "");

    status = listen(sock->fd, queueLen);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, 
                "Listen socket %d fail : %s", sock->fd, strerror(errno));

//    UTLT_Trace("Socket %d is listened", sock->fd);

    return STATUS_OK;
}

Status SockAccept(Sock *lSock, Sock *acSock) {
    UTLT_Assert(lSock && acSock, return STATUS_ERROR, ""); 
    
    int addrLen = SockAddrLen(&lSock->localAddr);
    acSock->fd = accept(lSock->fd, (struct sockaddr *) &acSock->remoteAddr, (socklen_t *) &addrLen);

    UTLT_Assert(acSock->fd, return STATUS_ERROR, 
                "Connection Accept fail : %s", strerror(errno));
//    UTLT_Trace("Socket %d is accepted", acSock->fd);

    return STATUS_OK;
}

// TODO : If need to check size > sizeof(buffer)
int SockRead(Sock *sock, void *buffer, int size) {
    UTLT_Assert(sock && buffer, return STATUS_ERROR, "");

    int readcnt = read(sock->fd, buffer, size);
    UTLT_Assert(readcnt >= 0, return STATUS_ERROR, 
                "Socket Read Error : %s", strerror(errno));
    
//    UTLT_Trace("Socket %d has been read %d byte", sock->fd, size);

    return readcnt;
}

Status SockWrite(Sock *sock, const void *buffer, int size) {
    UTLT_Assert(sock && buffer, return STATUS_ERROR, "");

    Status status;
    int remain = size;
    const char *ptr = buffer;
    
    do {
        status = write(sock->fd, ptr, remain);
        UTLT_Assert(status >= 0, return STATUS_ERROR, "Socket Write Error : %s", strerror(errno));
        ptr += status;
        remain -= status;
    } while(status && remain);

    // TODO : Maybe it can be use UTLT_Assert to print warning log
    if (remain) {
        UTLT_Warning("Socket %d need to write %d byte, but now only write %d byte", sock->fd, size, size - remain);
    } else {
//        UTLT_Trace("Socket %d has been write %d byte", sock->fd, size);
    }

    return STATUS_OK;
}

// TODO : If need to check size > sizeof(buffer)
int SockRecvFrom(Sock *sock, void *buffer, int size) {
    UTLT_Assert(sock && buffer, return STATUS_ERROR, "");

    int addrLen = SockAddrLen(&sock->localAddr);

    int readcnt = recvfrom(sock->fd, buffer, size, sock->rflag, 
                      (struct sockaddr *) &sock->remoteAddr, (socklen_t *) &addrLen);
    UTLT_Assert(readcnt >= 0, return STATUS_ERROR, 
                "Socket Recvfrom Error : %s", strerror(errno));

//    UTLT_Trace("Socket %d has been recvfrom %d byte", sock->fd, size);

    return readcnt;
}

Status SockSendTo(Sock *sock, void *buffer, int size) {
    UTLT_Assert(sock && buffer, return STATUS_ERROR, "");

    Status status;
    int remain = size, addrLen = SockAddrLen(&sock->remoteAddr);
    const char *ptr = buffer;

    do {
        status = sendto(sock->fd, ptr, remain, sock->wflag, (struct sockaddr *) &sock->remoteAddr, addrLen);
        UTLT_Assert(status >= 0, return STATUS_ERROR, "Socket SendTo Error : %s", strerror(errno));
        ptr += status;
        remain -= status;
    } while(status && remain);

    if (remain) {
        UTLT_Warning("Socket %d need to sendto %d byte, but now only sendto %d byte", sock->fd, size, size - remain);
    } else {
//        UTLT_Trace("Socket %d has been sendto %d byte", sock->fd, size);
    }

    return STATUS_OK;
}

Status SockRegister(Sock *sock, SockHandler handler, void *data) {
    UTLT_Assert(sock, return STATUS_ERROR, "Socket pointer is NULL");

    UTLT_Assert(sock->handler == NULL, return STATUS_ERROR,
                "Socket has been register");

    sock->handler = handler;
    sock->data = data;

    return STATUS_OK;
}

Status SockUnregister(Sock *sock) {
    UTLT_Assert(sock, return STATUS_ERROR, "Socket pointer is NULL");

    sock->handler = NULL;
    sock->data = NULL;

    return STATUS_OK;
}

