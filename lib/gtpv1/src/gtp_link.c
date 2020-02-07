#include "gtp_link.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "utlt_network.h"
#include "utlt_pool.h"
#include "libgtpnl/gtpnl.h"
#include "linux/genetlink.h"
#include "linux/gtp.h"
#include "libgtpnl/gtp.h"

#define MAX_NUM_OF_GTPDEV 64

PoolDeclare(gtpv1DevPool, Gtpv1TunDevNode, MAX_NUM_OF_GTPDEV);

Status GtpLinkCreate(Gtpv1TunDevNode *node) {
    UTLT_Assert(node, return STATUS_ERROR, "GTPv1 tunnel node is NULL");
    Status status;

    node->sockPrime = UdpServerCreate(AF_INET, node->ip, GTP_PRIME_PORT);
    UTLT_Assert(node->sockPrime, return STATUS_ERROR,
                "GTP tunnel bind on NIC create fail : IP[%s] port[%d]", node->ip, GTP_PRIME_PORT);
    node->sock1 = UdpServerCreate(AF_INET, node->ip, GTP_V1_PORT);
    UTLT_Assert(node->sock1, return STATUS_ERROR, 
                "GTP tunnel create fail : IP[%s] port[%d]", node->ip, GTP_V1_PORT);

    status = gtp_dev_create(-1, node->ifname, node->sockPrime->fd, node->sock1->fd);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                "GTP device named %s Create fail", node->ifname);
    
    UTLT_Trace("GTP device create success : IP[%s], port[%d], ifname[%s]",
               node->ip, GTP_V1_PORT, node->ifname);

    return STATUS_OK;
}

Status GtpLinkFree(Gtpv1TunDevNode *node) {
    UTLT_Assert(node, return STATUS_ERROR, "GTPv1 tunnel node is NULL");
    Status status = STATUS_OK;

    UTLT_Assert(UdpFree(node->sock1) == STATUS_OK, status |= STATUS_ERROR,
                "GTPv1 tunnel socket free fail : IP[%s]", node->ip);
    UTLT_Assert(UdpFree(node->sockPrime) == STATUS_OK, status |= STATUS_ERROR,
                "GTP Prime tunnel socket free fail : IP[%s]", node->ip);

    UTLT_Assert(gtp_dev_destroy(node->ifname) >= 0, status |= STATUS_ERROR,
                "GTP device destroy fail");
    
    UTLT_Trace("GTP device destroy success");

    return status;
}

#define FreeGtpSockAndTunnel(__nl, __tun) gtp_tunnel_free(__tun); genl_socket_close(__nl)

Status GtpTunnelAdd(const char *ifname, int iteid, int oteid, const char *destIP, const char *tunIP) {
    struct in_addr ms, sgsn;

    struct mnl_socket *nl = genl_socket_open();
    UTLT_Assert(nl, return STATUS_ERROR, "genl_socket_open fail");

    int genl_id = genl_lookup_family(nl, "gtp");
    UTLT_Assert(genl_id >= 0, genl_socket_close(nl); return STATUS_ERROR, 
                "Not found GTP genl family");
   
    struct gtp_tunnel *t = gtp_tunnel_alloc();

    uint32_t gtp_ifidx = if_nametoindex(ifname);
    UTLT_Assert(gtp_ifidx, FreeGtpSockAndTunnel(nl, t); return STATUS_ERROR,
                "Wrong GTP interface %s", ifname);

    gtp_tunnel_set_ifidx(t, gtp_ifidx);
    gtp_tunnel_set_version(t, GTP_V1);
    gtp_tunnel_set_i_tei(t, iteid);
    gtp_tunnel_set_o_tei(t, oteid);

    UTLT_Assert(inet_aton(destIP, &ms) >= 0, FreeGtpSockAndTunnel(nl, t); return STATUS_ERROR,
                "Bad Address for Destination IP");
    gtp_tunnel_set_ms_ip4(t, &ms);

    UTLT_Assert(inet_aton(tunIP, &sgsn) >= 0, FreeGtpSockAndTunnel(nl, t); return STATUS_ERROR,
                "Bad Address for Tunnel IP");
    gtp_tunnel_set_sgsn_ip4(t, &sgsn);

    gtp_add_tunnel(genl_id, nl, t);

    FreeGtpSockAndTunnel(nl, t);

    return STATUS_OK;
}

Status GtpTunnelDel(const char *ifname, int iteid) {
    struct mnl_socket *nl = genl_socket_open();
    UTLT_Assert(nl, return STATUS_ERROR, "genl_socket_open fail");

    int genl_id = genl_lookup_family(nl, "gtp");
    UTLT_Assert(genl_id >= 0, genl_socket_close(nl); return STATUS_ERROR, 
                "Not found GTP genl family");
        
    struct gtp_tunnel *t = gtp_tunnel_alloc();
    
    uint32_t gtp_ifidx = if_nametoindex(ifname);
    UTLT_Assert(gtp_ifidx, FreeGtpSockAndTunnel(nl, t); return STATUS_ERROR, 
                "Wrong GTP interface %s", ifname);
    
    gtp_tunnel_set_ifidx(t, gtp_ifidx);
    gtp_tunnel_set_version(t, GTP_V1);
    gtp_tunnel_set_i_tei(t, iteid);

    gtp_del_tunnel(genl_id, nl, t);
    
    FreeGtpSockAndTunnel(nl, t);

    return STATUS_OK;

}

Status GtpTunnelList() {
    struct mnl_socket *nl = genl_socket_open();
    UTLT_Assert(nl, return STATUS_ERROR, "genl_socket_open fail");

    int genl_id = genl_lookup_family(nl, "gtp");
    UTLT_Assert(genl_id >= 0, genl_socket_close(nl); return STATUS_ERROR, 
                "Not found GTP genl family");
        
    gtp_list_tunnel(genl_id, nl);
    
    genl_socket_close(nl);

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

Gtpv1TunDevNode *Gtpv1DevListAdd(ListNode *list, const char *ip, const char *ifname) {
    UTLT_Assert(list, return NULL, "");
    UTLT_Assert(strlen(ip) < INET6_ADDRSTRLEN, return NULL, "");
    UTLT_Assert(strlen(ifname) < MAX_IFNAME_STRLEN, return NULL, "");

    Gtpv1TunDevNode *node;
    PoolAlloc(&gtpv1DevPool, node);
    UTLT_Assert(node, return NULL, "");
    ListAppend(list, node);

    strcpy(node->ip, ip);
    strcpy(node->ifname, ifname);

    return node;
}

Status Gtpv1DevListFree(ListNode *list) {
    UTLT_Assert(list, return STATUS_ERROR, "");

    Gtpv1TunDevNode *node, *nextNode;

    node = ListFirst(list);
    while (node) {
        nextNode = (Gtpv1TunDevNode *)ListNext(node);
        ListRemove(list, node);
        PoolFree(&gtpv1DevPool, node);
        node = nextNode;
    }

    return STATUS_OK;
}
