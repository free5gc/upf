#include "knet_route.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <libmnl/libmnl.h>
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

#include "utlt_buff.h"

#define SOCKET_DUMP_SIZE 32768

RouteEntryNode *AllocRouteEntry();
static struct mnl_socket* MnlskOpenAndSend(unsigned int *portid, const struct nlmsghdr *nlh);
static Status AddKnetDelRoute(const char *ifname, const char *dstIP, uint32_t cidrPrefix, 
                              const char *gatewayAddr, uint32_t priority, int op);

/* --------------- For KnetAddRoute() --------------- */
struct RtCbParams {
    int family;
    int table;
    ListHead *routeEntries; // list of RouteEntryNode
};

static int RtDataCallback(const struct nlmsghdr *nlh, void *data);
static int DataAttrCallback(const struct nlattr *attr, void *data);
static int DataAttrCallback6(const struct nlattr *attr, void *data);

static void ShowAttributes(struct nlattr *tb[], RouteEntryNode *rtEntry);
static void ShowAttributes6(struct nlattr *tb[], RouteEntryNode *rtEntry);
// static int dataMetricsAttrCallback(const struct nlattr *attr, void *data);
/* -------------------------------------------------- */

ListHead* KnetGetRoutes(int family, int table) {
    ListHead *routeEntries = UTLT_Malloc(sizeof(ListHead));
    ListHeadInit(routeEntries);

    char buf[SOCKET_DUMP_SIZE];
    unsigned int seq, portid;
    struct mnl_socket *nl;
    struct nlmsghdr *nlh;
    struct rtmsg *rtm;
    struct RtCbParams *callbackParams;
    int ret;

    nlh = mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type = RTM_GETROUTE;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = seq = time(NULL);
    rtm = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtmsg));
    rtm->rtm_family = family;

    nl = MnlskOpenAndSend(&portid, nlh);
    UTLT_Assert(nl, goto err1, "");

    callbackParams = UTLT_Malloc(sizeof(struct RtCbParams));
    UTLT_Assert(callbackParams, mnl_socket_close(nl); goto err1, "");
    callbackParams->family = family;
    callbackParams->table = table;
    callbackParams->routeEntries = routeEntries;

    ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    while (ret > 0) {
        ret = mnl_cb_run(buf, ret, seq, portid, RtDataCallback, callbackParams);
        if (ret <= MNL_CB_STOP)
            break;
        ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    }
    UTLT_Assert(ret != -1, goto err2, "error");

    mnl_socket_close(nl);
    UTLT_Free(callbackParams);
    return routeEntries;

err2:
    UTLT_Free(callbackParams);
err1:
    UTLT_Free(routeEntries);
    return NULL;
}

void KnetRtListFree(ListHead* rtEntries) {
    RouteEntryNode *node, *nextNode;

    ListForEachSafe(node, nextNode, rtEntries) {
        ListRemove(node);
        UTLT_Free(node);
    } 
}

Status KnetAddRoute(const char *ifname, const char *dstIP, uint32_t cidrPrefix, 
                 const char *gatewayAddr, uint32_t priority) {
    return AddKnetDelRoute(ifname, dstIP, cidrPrefix, gatewayAddr, priority, RTM_NEWROUTE);
}

Status KnetDelRoute(const char *ifname, const char *dstIP, uint32_t cidrPrefix, 
                 const char *gatewayAddr, uint32_t priority) {
    return AddKnetDelRoute(ifname, dstIP, cidrPrefix, gatewayAddr, priority, RTM_DELROUTE);
}

static Status AddKnetDelRoute(const char *ifname, const char *dstIP, uint32_t cidrPrefix, 
                     const char *gatewayAddr, uint32_t priority, int op) {
    UTLT_Assert(op == RTM_NEWROUTE || op == RTM_DELROUTE, 
                return STATUS_ERROR, "AddKnetDelRoute op should be RTM_NEWROUTE or RTM_DELROUTE");

    struct mnl_socket *nl;
    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct nlmsghdr *nlh;
    struct rtmsg *rtm;
    uint32_t seq, portid;
    union {
        in_addr_t ip;
        struct in6_addr ip6;
    } dst;
    union {
        in_addr_t ip;
        struct in6_addr ip6;
    } gw;
    int iface, ret, family = AF_INET;

    iface = if_nametoindex(ifname);
    UTLT_Assert(iface, return STATUS_ERROR, "if_nametoindex");

    if (!inet_pton(AF_INET, dstIP, &dst)) {
        if (!inet_pton(AF_INET6, dstIP, &dst)) {
            UTLT_Error("inet_pton");
            return STATUS_ERROR;
        }
        family = AF_INET6;
    }

    if (gatewayAddr && !inet_pton(family, gatewayAddr, &gw)) {
        UTLT_Error("inet_pton");
        return STATUS_ERROR;
    }

    nlh = mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type    = op;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
    nlh->nlmsg_seq = seq = time(NULL);

    rtm = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtmsg));
    rtm->rtm_family = family;
    rtm->rtm_dst_len = cidrPrefix;
    rtm->rtm_src_len = 0;
    rtm->rtm_tos = 0;
    rtm->rtm_protocol = RTPROT_STATIC;
    rtm->rtm_table = RT_TABLE_MAIN;
    if (op != RTM_DELROUTE)
        rtm->rtm_type = RTN_UNICAST;
    /* is there any gateway? */
    rtm->rtm_scope = gatewayAddr ? RT_SCOPE_LINK : RT_SCOPE_UNIVERSE;
    rtm->rtm_flags = 0;

    if (family == AF_INET)
        mnl_attr_put_u32(nlh, RTA_DST, dst.ip);
    else
        mnl_attr_put(nlh, RTA_DST, sizeof(struct in6_addr), &dst);

    mnl_attr_put_u32(nlh, RTA_OIF, iface);
    if (gatewayAddr) {
        if (family == AF_INET)
            mnl_attr_put_u32(nlh, RTA_GATEWAY, gw.ip);
        else {
            mnl_attr_put(nlh, RTA_GATEWAY, sizeof(struct in6_addr),
                    &gw.ip6);
        }
    }

    mnl_attr_put_u32(nlh, RTA_PRIORITY, priority);
    
    nl = MnlskOpenAndSend(&portid, nlh);
    UTLT_Assert(nl, return STATUS_ERROR, "");

    ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    UTLT_Assert(ret >= 0,  mnl_socket_close(nl); return STATUS_ERROR, "mnl_socket_recvfrom");

    ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
    UTLT_Assert(ret >= 0,  mnl_socket_close(nl); return STATUS_ERROR, "mnl_cb_run, run as root?");

    mnl_socket_close(nl);
    return STATUS_OK;
}

Status KnetPrintRoutes(ListHead *routeEntries) {
    const char *rtProtoName[] = {"unspec", "redirect", "kernel", "boot", "static"};
    const char *rtmTypeName[] = {"unspec", "unicast", "local", "broadcast", "anycast", "multicast", 
                                 "blackhole", "unreachable", "prohibit", "throw", "nat", "xresolve"};
    char buf[120] = "";
	char cidrBuf[20] = "";

	// TODO: longer padding for ipv6 address?
    UTLT_Info("%-20s%-15s%-10s%-9s%-10s%-11s", "DstIp", "Gateway", "Iface", "Priority", "RtProto", "Type");
    RouteEntryNode *it, *nextIt = NULL;
    ListForEachSafe(it, nextIt, routeEntries) {
        if (it->family != AF_INET && it->family != AF_INET6) {
            UTLT_Error("Unsupported family");
            return STATUS_ERROR;
        }
        buf[0] = 0;

        if (it->dstIp.available)
            sprintf(cidrBuf, "%s /%d", it->family == AF_INET ? inet_ntoa(it->dstIp.addr.v4) : inet6Ntoa(it->dstIp.addr.v6), it->dstCIDR);
        else
            sprintf(cidrBuf, "%s /%d", "0.0.0.0", it->dstCIDR);
		sprintf(buf + strlen(buf), "%-20s", cidrBuf);

        if (it->gateway.available)
            sprintf(buf + strlen(buf), "%-15s", it->family == AF_INET ? inet_ntoa(it->gateway.addr.v4) : inet6Ntoa(it->gateway.addr.v6));
        else
            sprintf(buf + strlen(buf), "%-15s", "0.0.0.0");

        sprintf(buf + strlen(buf), "%-10s", it->outIfName);
        sprintf(buf + strlen(buf), "%-9d", it->priority);

        if (it->rtProtocol <= 4)
            sprintf(buf + strlen(buf), "%-10s", rtProtoName[it->rtProtocol]);
        else if (it->rtProtocol == 16)
            sprintf(buf + strlen(buf), "%-10s", "dhcp");
        else
            sprintf(buf + strlen(buf), "%-10d", it->rtProtocol);
        
        if (it->type <= 11)
            sprintf(buf + strlen(buf), "%-11s", rtmTypeName[it->type]);
        else
            sprintf(buf + strlen(buf), "%-11d", it->type);

        UTLT_Info("%s", buf);
    }

    return STATUS_OK;
}

/* like inet_ntoa(), not reentrant */
const char *inet6Ntoa(struct in6_addr in6) {
    static char buf[INET6_ADDRSTRLEN];

    return inet_ntop(AF_INET6, &in6.s6_addr, buf, sizeof(buf));
}

RouteEntryNode *AllocRouteEntry() {
    RouteEntryNode *rtEntry = UTLT_Malloc(sizeof(RouteEntryNode));
    memset(rtEntry, 0, sizeof(RouteEntryNode));
    return rtEntry;
}

static struct mnl_socket* MnlskOpenAndSend(unsigned int *portid, const struct nlmsghdr *nlh) {
    int ret;
    
    struct mnl_socket *nl = mnl_socket_open(NETLINK_ROUTE);
    UTLT_Assert(nl, return NULL, "mnl_socket_open");
    
    ret = mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID);
    UTLT_Assert(ret >= 0,  mnl_socket_close(nl); return NULL, "mnl_socket_bind");
    *portid = mnl_socket_get_portid(nl);

    ret = mnl_socket_sendto(nl, nlh, nlh->nlmsg_len);
    UTLT_Assert(ret >= 0,  mnl_socket_close(nl); return NULL, "mnl_socket_sendto");

    return nl;
}

static int RtDataCallback(const struct nlmsghdr *nlh, void *data) {
    struct nlattr *tb[RTA_MAX+1] = {};
    struct rtmsg *rm = mnl_nlmsg_get_payload(nlh);
    struct RtCbParams *callbackParams = (struct RtCbParams *)data;

    if (!(callbackParams->family == rm->rtm_family && 
          (callbackParams->table == RT_TABLE_ANY || callbackParams->table == rm->rtm_table))) {
        return MNL_CB_OK;
    }
    
    ListHead *routeEntries = callbackParams->routeEntries;
    RouteEntryNode *rtEntry = AllocRouteEntry();
    ListInsert(rtEntry, routeEntries);

    rtEntry->tableId = rm->rtm_table;
    rtEntry->family = rm->rtm_family;
    rtEntry->dstCIDR = rm->rtm_dst_len;
    
    rtEntry->tos = rm->rtm_tos;
    rtEntry->type = rm->rtm_type;
    rtEntry->scope = rm->rtm_scope;
    rtEntry->rtProtocol = rm->rtm_protocol;
    rtEntry->rtmFlags = rm->rtm_flags;

    // printf("src_cidr=%u  ", rm->rtm_src_len);

    switch(rm->rtm_family) {
    case AF_INET:
        mnl_attr_parse(nlh, sizeof(*rm), DataAttrCallback, tb);
        ShowAttributes(tb, rtEntry);
        break;
    case AF_INET6:
        mnl_attr_parse(nlh, sizeof(*rm), DataAttrCallback6, tb);
        ShowAttributes6(tb, rtEntry);
        break;
    }

    return MNL_CB_OK;
}

static int DataAttrCallback(const struct nlattr *attr, void *data) {
    const struct nlattr **tb = data;
    int type = mnl_attr_get_type(attr);

    /* skip unsupported attribute in user-space */
    if (mnl_attr_type_valid(attr, RTA_MAX) < 0)
        return MNL_CB_OK;

    switch(type) {
    case RTA_TABLE:
    case RTA_DST:
    case RTA_SRC:
    case RTA_OIF:
    case RTA_FLOW:
    case RTA_PREFSRC:
    case RTA_GATEWAY:
    case RTA_PRIORITY:
        if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
            UTLT_Error("mnl_attr_validate");
            return MNL_CB_ERROR;
        }
        break;
    case RTA_METRICS:
        if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0) {
            UTLT_Error("mnl_attr_validate");
            return MNL_CB_ERROR;
        }
        break;
    }
    tb[type] = attr;
    return MNL_CB_OK;
}

static int DataAttrCallback6(const struct nlattr *attr, void *data) {
    const struct nlattr **tb = data;
    int type = mnl_attr_get_type(attr);

    /* skip unsupported attribute in user-space */
    if (mnl_attr_type_valid(attr, RTA_MAX) < 0)
        return MNL_CB_OK;

    switch(type) {
    case RTA_TABLE:
    case RTA_OIF:
    case RTA_FLOW:
    case RTA_PRIORITY:
        if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
            UTLT_Error("mnl_attr_validate");
            return MNL_CB_ERROR;
        }
        break;
    case RTA_DST:
    case RTA_SRC:
    case RTA_PREFSRC:
    case RTA_GATEWAY:
        if (mnl_attr_validate2(attr, MNL_TYPE_BINARY,
                    sizeof(struct in6_addr)) < 0) {
            UTLT_Error("mnl_attr_validate2");
            return MNL_CB_ERROR;
        }
        break;
    case RTA_METRICS:
        if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0) {
            UTLT_Error("mnl_attr_validate");
            return MNL_CB_ERROR;
        }
        break;
    }
    tb[type] = attr;
    return MNL_CB_OK;
}

static void ShowAttributes(struct nlattr *tb[], RouteEntryNode *rtEntry) {
    if (tb[RTA_DST]) {
        struct in_addr *addr = mnl_attr_get_payload(tb[RTA_DST]);
        rtEntry->dstIp.available = 1;
        rtEntry->dstIp.addr.v4 = *addr;
    }
    if (tb[RTA_OIF]) {
        char ifname[IF_NAMESIZE];
        if (if_indextoname(mnl_attr_get_u32(tb[RTA_OIF]), ifname) == NULL)
            UTLT_Assert(0,, "if_indextoname error");
        strcpy(rtEntry->outIfName, ifname);
    }
    if (tb[RTA_PREFSRC]) {
        struct in_addr *addr = mnl_attr_get_payload(tb[RTA_PREFSRC]);
        rtEntry->perfSrcIp.available = 1;
        rtEntry->perfSrcIp.addr.v4 = *addr;
    }
    if (tb[RTA_GATEWAY]) {
        struct in_addr *addr = mnl_attr_get_payload(tb[RTA_GATEWAY]);
        rtEntry->gateway.available = 1;
        rtEntry->gateway.addr.v4 = *addr;
    }
    if (tb[RTA_PRIORITY]) {
        rtEntry->priority = mnl_attr_get_u32(tb[RTA_PRIORITY]);
    }

    // if (tb[RTA_TABLE]) {
    //     printf("table=%u  ", mnl_attr_get_u32(tb[RTA_TABLE]));
    // }
    // if (tb[RTA_SRC]) {
    //     struct in_addr *addr = mnl_attr_get_payload(tb[RTA_SRC]);
    //     printf("src=%s  ", inet_ntoa(*addr));
    // }
    // if (tb[RTA_FLOW]) {
    //     printf("flow=%u  ", mnl_attr_get_u32(tb[RTA_FLOW]));
    // }

    // if (tb[RTA_METRICS]) {
    //     int i;
    //     struct nlattr *tbx[RTAX_MAX+1] = {};
    //     mnl_attr_parse_nested(tb[RTA_METRICS], dataMetricsAttrCallback, tbx);
    //     for (i=0; i<RTAX_MAX; i++) {
    //         if (tbx[i]) {
    //             printf("metrics[%d]=%u  ",
    //                 i, mnl_attr_get_u32(tbx[i]));
    //         }
    //     }
    // }
}

static void ShowAttributes6(struct nlattr *tb[], RouteEntryNode *rtEntry) {
    if (tb[RTA_DST]) {
        struct in6_addr *addr = mnl_attr_get_payload(tb[RTA_DST]);
        rtEntry->dstIp.available = 1;
        rtEntry->dstIp.addr.v6 = *addr;
    }
    if (tb[RTA_OIF]) {
        char ifname[IF_NAMESIZE];
        if (if_indextoname(mnl_attr_get_u32(tb[RTA_OIF]), ifname) == NULL)
            UTLT_Assert(0,, "if_indextoname error");
        strcpy(rtEntry->outIfName, ifname);
    }
    if (tb[RTA_PREFSRC]) {
        struct in6_addr *addr = mnl_attr_get_payload(tb[RTA_PREFSRC]);
        rtEntry->perfSrcIp.available = 1;
        rtEntry->perfSrcIp.addr.v6 = *addr;
    }
    if (tb[RTA_GATEWAY]) {
        struct in6_addr *addr = mnl_attr_get_payload(tb[RTA_GATEWAY]);
        rtEntry->gateway.available = 1;
        rtEntry->gateway.addr.v6 = *addr;
    }
    if (tb[RTA_PRIORITY]) {
        rtEntry->priority = mnl_attr_get_u32(tb[RTA_PRIORITY]);
    }

    // if (tb[RTA_TABLE]) {
    //     printf("table=%u  ", mnl_attr_get_u32(tb[RTA_TABLE]));
    // }
    // if (tb[RTA_FLOW]) {
    //     printf("flow=%u  ", mnl_attr_get_u32(tb[RTA_FLOW]));
    // }
    // if (tb[RTA_SRC]) {
    //     struct in6_addr *addr = mnl_attr_get_payload(tb[RTA_SRC]);
    //     printf("src=%s  ", inet6Ntoa(*addr));
    // }

    // if (tb[RTA_METRICS]) {
    //     int i;
    //     struct nlattr *tbx[RTAX_MAX+1] = {};
    //     mnl_attr_parse_nested(tb[RTA_METRICS], dataMetricsAttrCallback, tbx);
    //     for (i=0; i<RTAX_MAX; i++) {
    //         if (tbx[i]) {
    //             printf("metrics[%d]=%u  ",
    //                 i, mnl_attr_get_u32(tbx[i]));
    //         }
    //     }
    // }
}

// static int dataMetricsAttrCallback(const struct nlattr *attr, void *data) {
//     const struct nlattr **tb = data;

//     /* skip unsupported attribute in user-space */
//     if (mnl_attr_type_valid(attr, RTAX_MAX) < 0)
//         return MNL_CB_OK;

//     if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
//         UTLT_Error("mnl_attr_validate");
//         return MNL_CB_ERROR;
//     }

//     tb[mnl_attr_get_type(attr)] = attr;
//     return MNL_CB_OK;
// }
