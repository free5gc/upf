#define TRACE_MODULE _pfcp_node

#include "utlt_debug.h"
#include "utlt_pool.h"

#include "pfcp_convert.h"
#include "pfcp_types.h"
#include "pfcp_xact.h"

#include "pfcp_node.h"

#define MAX_PFCP_NODE_POOL_SIZE 512

PoolDeclare(pfcpNodePool, PfcpNode, MAX_PFCP_NODE_POOL_SIZE);

Status PfcpNodeInit() {
    PoolInit(&pfcpNodePool, MAX_PFCP_NODE_POOL_SIZE);

    return STATUS_OK;
}

Status PfcpNodeTerminate() {
    if (PoolUsedCheck(&pfcpNodePool)) {
        UTLT_Error("%d not freed in pfcpNodePool[%d]",
                PoolUsedCheck(&pfcpNodePool), PoolSize(&pfcpNodePool));
    }
    PoolTerminate(&pfcpNodePool);

    return STATUS_OK;
}

Status PfcpAddNode(ListHead *list, PfcpNode **node,
                   const SockAddr *allList, _Bool noIpv4,
                   _Bool noIpv6, _Bool preferIpv4) {
    Status status;
    PfcpNode *newNode = NULL;
    SockAddr *preferredList = NULL;

    UTLT_Assert(list, return STATUS_ERROR, "ListHead error");
    UTLT_Assert(allList, return STATUS_ERROR, "list of socket error");

    status = SockAddrCopy(&preferredList, &allList);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "socket copy failed");

    if (noIpv4) {
        status = SockAddrFilter(&preferredList, AF_INET6);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "no IPv4 error");
    }
    if (noIpv6) {
        status = SockAddrFilter(&preferredList, AF_INET);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "no IPv6 error");
    }
    if (preferIpv4) {
        status = SockAddrSort(&preferredList, AF_INET);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Socket sort error");
    } else {
        status = SockAddrSort(&preferredList, AF_INET6);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Socket sort error");
    }

    if (preferredList) {
        PoolAlloc(&pfcpNodePool, newNode);
        UTLT_Assert(newNode, return STATUS_ERROR, "node allocate error");
        memset(newNode, 0, sizeof(PfcpNode));

        newNode->saList = preferredList;

        ListHeadInit(&newNode->node);
        ListHeadInit(&newNode->localList);
        ListHeadInit(&newNode->remoteList);

        newNode->timeHeartbeat = 0;

        ListInsert(newNode, list);
        newNode->state = PFCP_NODE_ST_NULL;
    }

    *node = newNode;

    return STATUS_OK;
}

PfcpNode *PfcpAddNodeWithSeid(ListHead *list, PfcpFSeid *fSeid,
        uint16_t port, _Bool noIpv4, _Bool noIpv6, _Bool preferIpv4) {
    Status status;
    PfcpNode *node = NULL;
    SockAddr *saList = NULL;

    UTLT_Assert(list, return NULL, "list error");
    UTLT_Assert(fSeid, return NULL, "F-SEID error");
    UTLT_Assert(port, return NULL, "port error");

    status = PfcpFSeidToSockaddr(fSeid, port, &saList);
    UTLT_Assert(status == STATUS_OK, return NULL, "F-SEID to Sock error");

    status = PfcpAddNode(list, &node, saList, noIpv4, noIpv6, preferIpv4);
    UTLT_Assert(status == STATUS_OK, goto err1, "PFCP add to list error");
    UTLT_Assert(node, goto err1, "node can't be allocate");

    status = PfcpFSeidToIp(fSeid, &node->ip);
    UTLT_Assert(status == STATUS_OK, goto err2, "F-SEID to IP error");

    status = SockAddrFillScopeIdInLocal(node->saList);
    UTLT_Assert(status == STATUS_OK, goto err2, "Get local address error");

    SockAddrFreeAll(saList);

    return node;

err2:
    PoolFree(&pfcpNodePool, node);
err1:
    SockAddrFreeAll(saList);
    return NULL;
}

Status PfcpRemoveNode(ListHead *list, PfcpNode *node) {
    UTLT_Assert(node, return STATUS_ERROR, "pfcp node error");

    PfcpNode *it, *nextIt = NULL;
    
    ListForEachSafe(it, nextIt, list) {
        if (it == node) {
            ListRemove(it);
            break;
        }
    }

    if (node->timeHeartbeat) {
        TimerDelete(node->timeHeartbeat);
    }

    PfcpXactDeleteAll(node);

    SockAddrFreeAll(node->saList);
    PoolFree(&pfcpNodePool, node);

    return STATUS_OK;
}

Status PfcpRemoveAllNodes(ListHead *list) {
    PfcpNode *next = NULL, *current = NULL;

    ListForEachSafe(current, next, list) { 
        PfcpRemoveNode(list, current);
    }
    return STATUS_OK;
}

PfcpNode *PfcpFindNode(ListHead *list, PfcpFSeid *fSeid) {
    Status status;
    PfcpNode *current, *nextNode = NULL;
    Ip ip;

    UTLT_Assert(list, return NULL, "Input pfcpList error");
    UTLT_Assert(fSeid, return NULL, "F-SEID error");

    status = PfcpFSeidToIp(fSeid, &ip);
    UTLT_Assert(status == STATUS_OK, return NULL, "F-SEID to IP error");

    current = ListFirst(list);
    if (current == (PfcpNode *) list) {
        return NULL;
    }
    
    ListForEachSafe(current, nextNode, list) { 
        if(!memcmp(&ip, &(current->ip), ip.len)) {
            break;
        }
    }

    return current;
}

// 0: same, 1: different
int SockCmp(SockAddr *a, SockAddr *b) {
    if (a->_family != b->_family) {
        return 1;
    }
    if (a->_family == AF_INET) {
        if (a->s4.sin_addr.s_addr != b->s4.sin_addr.s_addr) {
            return 1;
        } else if (a->next == NULL && b->next == NULL) {
            return 0;
        } else if (a->next != NULL && b->next != NULL) {
            return SockCmp(a->next, b->next);
        } else {
            return 1;
        }
    } else if (a->_family == AF_INET6) {
        if (a->s6.sin6_addr.s6_addr != b->s6.sin6_addr.s6_addr) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 1;
    }
}

PfcpNode *PfcpFindNodeSockAddr(ListHead *list, SockAddr *sock) {
    PfcpNode *current, *nextNode = NULL;

    UTLT_Assert(list, return NULL, "Input pfcpList error");
    UTLT_Assert(sock, return NULL, "SocketAddr error");

    current = ListFirst(list);

    if (current ==  (PfcpNode *) list) {
        return NULL;
    }

    _Bool hitSame = 0;

    ListForEachSafe(current, nextNode, list) {
        SockAddr *addr;
        for (addr = current->saList; addr; addr = addr->next) {
            if(!SockCmp(addr, sock)) {
                hitSame = 1;
                break;
            }
        }
        if (hitSame) {
            break;
        }
    }

    return current;
}
