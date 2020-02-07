#include "utlt_network.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "utlt_debug.h"

struct epoll_event ev;

int EpollCreate() {
    int epfd = epoll_create(MAX_NUM_OF_EVENT);
    UTLT_Assert(epfd >= 0, return -1, 
                "Epoll Create Error : %s", strerror(errno));
    
    return epfd;
}

Status EpollRegisterEvent(int epfd, Sock *sock) {
    UTLT_Assert(sock, return STATUS_ERROR, "");

    memset(&ev, 0, sizeof(ev));
    ev.events = sock->epollMode;
    ev.data.ptr = sock;

    Status status;
    status = epoll_ctl(epfd, EPOLL_CTL_ADD, sock->fd, &ev);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                "Socket %d register event in epoll error : %s",
                sock->fd, strerror(errno));

    return STATUS_OK;
}

Status EpollModifyEvent(int epfd, Sock *sock) {
    UTLT_Assert(sock, return STATUS_ERROR, "");
    
    memset(&ev, 0, sizeof(ev));
    ev.events = sock->epollMode;
    ev.data.ptr = sock;

    Status status;
    status = epoll_ctl(epfd, EPOLL_CTL_MOD, sock->fd, &ev);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                "Socket %d modify event in epoll error : %s",
                sock->fd, strerror(errno));

    return STATUS_OK;
}

Status EpollDeregisterEvent(int epfd, Sock *sock) {
    UTLT_Assert(sock, return STATUS_ERROR, "");
    
    Status status;
    status = epoll_ctl(epfd, EPOLL_CTL_DEL, sock->fd, NULL);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                "Socket %d delete event in epoll error : %s",
                sock->fd, strerror(errno));

    return STATUS_OK;
}

int EpollWait(int epfd, struct epoll_event *epEvent, int timeout) {
    if (timeout < 0) { 
        timeout = -1;
    }

    int nfds = epoll_wait(epfd, epEvent, MAX_NUM_OF_EVENT, timeout);
    UTLT_Assert(nfds >= 0, return -1, 
                "Epoll Wait Error : %s", strerror(errno));
    
    return nfds;
}
