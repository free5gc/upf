#include "gtp_link.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "utlt_network.h"
#include "utlt_pool.h"
#include "libgtp5gnl/gtp5gnl.h"
#include "linux/genetlink.h"

#define MAX_NUM_OF_GTPDEV 64

PoolDeclare(gtpv1DevPool, Gtpv1TunDevNode, MAX_NUM_OF_GTPDEV);

Status GtpLinkCreate(Gtpv1TunDevNode *node) {
    UTLT_Assert(node, return STATUS_ERROR, "GTPv1 tunnel node is NULL");
    Status status;

    node->sock = UdpServerCreate(AF_INET, node->ip, GTP_V1_PORT);
    UTLT_Assert(node->sock, return STATUS_ERROR, 
                "GTP tunnel create fail : IP[%s] port[%d]", node->ip, GTP_V1_PORT);

    status = gtp_dev_create(-1, node->ifname, node->sock->fd);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                "GTP device named %s Create fail", node->ifname);

    UTLT_Trace("GTP device create success : IP[%s], port[%d], ifname[%s]",
               node->ip, GTP_V1_PORT, node->ifname);

    return STATUS_OK;
}

Status GtpLinkFree(Gtpv1TunDevNode *node) {
    UTLT_Assert(node, return STATUS_ERROR, "GTPv1 tunnel node is NULL");
    Status status = STATUS_OK;

    UTLT_Assert(UdpFree(node->sock) == STATUS_OK, status |= STATUS_ERROR,
                "GTPv1 tunnel socket free fail : IP[%s]", node->ip);

    UTLT_Assert(gtp_dev_destroy(node->ifname) >= 0, status |= STATUS_ERROR,
                "GTP device destroy fail");
    
    UTLT_Trace("GTP device destroy success");

    return status;
}

Status NetlinkSockOpen(NetlinkInfo *info, const char *ifname, const char *family_name) {
    UTLT_Assert(info, return STATUS_ERROR, "NetlinkInfo is NULL");

    info->nl = genl_socket_open();
    UTLT_Assert(info->nl, return STATUS_ERROR, "genl_socket_open fail");

    info->genl_id = genl_lookup_family(info->nl, family_name);
    UTLT_Assert(info->genl_id >= 0, goto err, "Not found gtp5g genl family");

    info->ifidx = if_nametoindex(ifname);
    UTLT_Assert(info->ifidx, goto err, "Wrong 5G GTP interface %s", ifname);

    return STATUS_OK;

err:
    mnl_socket_close(info->nl);
    return STATUS_ERROR;
}

Status NetlinkSockClose(NetlinkInfo *info) {
    mnl_socket_close(info->nl);
    return STATUS_OK;
}

Status Gtpv1DevPoolInit() {
    PoolInit(&gtpv1DevPool, MAX_NUM_OF_GTPDEV);

    return STATUS_OK;
}

Status Gtpv1DevPoolFinal() {
    PoolTerminate(&gtpv1DevPool);

    return STATUS_OK;
}

Gtpv1TunDevNode *Gtpv1DevListAdd(ListHead *list, const char *ip, const char *ifname) {
    UTLT_Assert(list, return NULL, "");
    UTLT_Assert(strlen(ip) < INET6_ADDRSTRLEN, return NULL, "");
    UTLT_Assert(strlen(ifname) < MAX_IFNAME_STRLEN, return NULL, "");

    Gtpv1TunDevNode *node;
    PoolAlloc(&gtpv1DevPool, node);
    UTLT_Assert(node, return NULL, "");
    ListInsert(node, list);

    strcpy(node->ip, ip);
    strcpy(node->ifname, ifname);

    return node;
}

Status Gtpv1DevListFree(ListHead *list) {
    UTLT_Assert(list, return STATUS_ERROR, "");

    Gtpv1TunDevNode *node, *nextNode;
                        
    ListForEachSafe(node, nextNode, list) {
        ListRemove(node);
        PoolFree(&gtpv1DevPool, node);
    }

    return STATUS_OK;
}