#include "utlt_network.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <errno.h>

#include "utlt_debug.h"
#include "utlt_buff.h"

Status SockSetAddr(SockAddr *sockAddr, int domain, const char *addr, int port) {
    Status status;
    sockAddr->_family = domain;
    sockAddr->_port = htons(port);
    
    switch(sockAddr->_family) {
        case AF_INET:
            if (!addr) {
                sockAddr->s4.sin_addr.s_addr = INADDR_ANY;
                return STATUS_OK;
            }
            status = inet_pton(AF_INET, addr, &(sockAddr->s4.sin_addr));
            UTLT_Assert(status == 1, return STATUS_ERROR, "IPv4 %s translation error", addr);
            break;
        case AF_INET6:
            if (!addr) {
                sockAddr->s6.sin6_addr = in6addr_any;
                return STATUS_OK;
            }
            status = inet_pton(AF_INET6, addr, &(sockAddr->s6.sin6_addr));
            UTLT_Assert(status == 1, return STATUS_ERROR, "IPv6 %s translation error", addr);
            break;
        case AF_UNIX:
            UTLT_Assert(addr, return STATUS_ERROR, "NULL path in unix socket");
            strcpy(sockAddr->su.sun_path, addr);
            break;
        default:
            UTLT_Assert(0, return STATUS_ERROR, "Unknown family : %d", sockAddr->_family);
    }
    return STATUS_OK;
} 

int SockAddrLen(const void *sockAddr) {
    const SockAddr *ptr = sockAddr;
    UTLT_Assert(ptr, return 0, "");

    switch(ptr->_family) {
        case AF_INET:
            return sizeof(struct sockaddr_in);
        case AF_INET6:
            return sizeof(struct sockaddr_in6);
        case AF_UNIX:
            return sizeof(ptr->su.sun_family) + strlen(ptr->su.sun_path);
        default:
            UTLT_Assert(0, return 0, "Unknown family : %d", ptr->_family);
    }
}

const char *UTLT_InetNtop(const void *sockAddr) {
    static char buffer[40];
    const SockAddr *ptr = sockAddr;
    UTLT_Assert(ptr, return 0, "");

    int family = ptr->_family;
    switch(family) {
        case AF_INET:
            return inet_ntop(family, &(ptr->s4.sin_addr), buffer, INET_ADDRSTRLEN);
        case AF_INET6:
            return inet_ntop(family, &(ptr->s6.sin6_addr), buffer, INET6_ADDRSTRLEN);
        default:
            UTLT_Assert(0, return NULL, "Unknown family : %d", family);
    }
}

const int SockAddrPortPrint(const void *sockAddr) {
    const SockAddr *ptr = sockAddr;
    return ntohs(ptr->_port);
}

Status GetAddrFromHost(char *addr, const char *host, size_t maxAddrLen) {
    struct addrinfo hints;
    struct addrinfo *res, *p;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // or AF_INET / AF_INET6
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0)
        return STATUS_ERROR;

    for (p = res; p != NULL; p = p->ai_next) {
        void *sinAddr;

        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            sinAddr = &(ipv4->sin_addr);
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            sinAddr = &(ipv6->sin6_addr);
        }

        UTLT_Assert(inet_ntop(p->ai_family, sinAddr, addr, maxAddrLen),
                    freeaddrinfo(res); return STATUS_ERROR, "");

        // Only return the first address
        freeaddrinfo(res);
        return STATUS_OK;
    }

    freeaddrinfo(res);
    return STATUS_EAGAIN;
}

Status SockAddrFilter(SockAddr **sa_list, int family) {
    SockAddr *addr = NULL, *prev = NULL, *next = NULL;

    UTLT_Assert(sa_list, return STATUS_ERROR, "socket list error");

    prev = NULL;
    addr = *sa_list;

    while(addr) {
        next = addr->next;

        if (addr->_family != family) {
            if (prev) {
                prev->next = addr->next;
            } else {
                *sa_list = addr->next;
            } 
        } else {
            prev = addr;
        }

        addr = next;
    }

    return STATUS_OK;
}

Status SockAddrCopy(SockAddr **dst, const SockAddr **src) {
    SockAddr *d;
    const SockAddr *s;

    for (*dst = d = NULL, s = *src; s; s = s->next) {
        if (!d) {
            d = UTLT_Calloc(1, sizeof(*s));
            *dst = memcpy(d, s, sizeof(*s));
        } else {
            d->next = UTLT_Calloc(1, sizeof(SockAddr));
            d = memcpy(d->next, s, sizeof(*s));
        }
    }

    return STATUS_OK;
}

Status SockAddrSort(SockAddr **saList, int family) {
    SockAddr *head = NULL, *addr = NULL, *new = NULL, *old = NULL;

    UTLT_Assert(saList, return STATUS_ERROR, "socket list error");

    old = *saList;

    while(old) {
        addr = old;
        old = old->next;

        if (head == NULL || addr->_family == family) {
            addr->next = head;
            head = addr;
        } else {
            new = head;
            while(new->next != NULL && new->next->_family != family) {
                new = new->next;
            }
            addr->next = new->next;
            new->next = addr;
        }
    }

    *saList = head;

    return STATUS_OK;
}

Status SockAddrFillScopeIdInLocal(SockAddr *saList) {
    struct ifaddrs *ifList = NULL, *current;
    SockAddr *addr, *ifaddr;

    for (addr = saList; addr != NULL; addr = addr->next) {
        if (addr->_family != AF_INET6) {
            continue;
        } else if (!IN6_IS_ADDR_LINKLOCAL(&addr->s6.sin6_addr)) {
            continue;
        } else if (addr->s6.sin6_scope_id != 0) {
            continue;
        } else if (ifList == NULL) {
            if (getifaddrs(&ifList) != 0) {
                UTLT_Error("getifaddrs failed(%d:%s)", errno, strerror(errno));
                return STATUS_ERROR;
            }
        }

        for (current = ifList; current != NULL; current = current->ifa_next) {
            ifaddr = (SockAddr *)current->ifa_addr;

            if (current->ifa_addr == NULL) {
                continue;
            } else if (current->ifa_addr->sa_family != AF_INET6) {
                continue;
            } else if (!IN6_IS_ADDR_LINKLOCAL(&ifaddr->s6.sin6_addr)) {
                continue;
            } else if (memcmp(&addr->s6.sin6_addr,
                    &ifaddr->s6.sin6_addr, sizeof(struct in6_addr)) == 0) {
                addr->s6.sin6_scope_id = ifaddr->s6.sin6_scope_id;
            }
        }
    }

    if (ifList) {
        freeifaddrs(ifList);
    }

    return STATUS_OK;
}

Status SockAddrFreeAll(SockAddr *saList) {
    SockAddr *next = NULL, *current = NULL;

    current = saList;
    
    while(current) {
        next = current->next;
        UTLT_Free(current);
        current = next;
    }

    return STATUS_OK;
}
