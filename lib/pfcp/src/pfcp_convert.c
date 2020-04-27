#define TRACE_MODULE _pfcp_convert

#include "utlt_3gppTypes.h"
#include "utlt_debug.h"
#include "utlt_lib.h"
#include "utlt_network.h"
#include "utlt_buff.h"

#include "pfcp_types.h"

#include "pfcp_convert.h"

Status PfcpFSeidToSockaddr(
PfcpFSeid *fSeid, uint16_t port, SockAddr **list) {
    SockAddr *addr = NULL, *addr6 = NULL;

    UTLT_Assert(fSeid, return STATUS_ERROR, "fSeid null");
    UTLT_Assert(list, return STATUS_ERROR, "list null");

    addr = UTLT_Calloc(1, sizeof(SockAddr));
    UTLT_Assert(addr, return STATUS_ERROR, "addr null");
    addr->_family = AF_INET;
    addr->_port = htons(port);

    addr6 = UTLT_Calloc(1, sizeof(SockAddr));
    UTLT_Assert(addr6, return STATUS_ERROR, "addr6 null");
    addr6->_family = AF_INET6;
    addr6->_port = htons(port);

    if (fSeid->v4 && fSeid->v6) {
        addr->next = addr6;

        addr->s4.sin_addr = fSeid->dualStack.addr4;
        addr6->s6.sin6_addr = fSeid->dualStack.addr6;

        *list = addr;
    } else if (fSeid->v4) {
        addr->s4.sin_addr = fSeid->addr4;
        UTLT_Free(addr6);

        *list = addr;
    } else if (fSeid->v6) {
        addr6->s6.sin6_addr = fSeid->addr6;
        UTLT_Free(addr);

        *list = addr6;
    } else {
        UTLT_Free(addr);
        UTLT_Free(addr6);
        UTLT_Assert(0, return STATUS_ERROR, "Not a IPv4 or IPv6 family");
    }

    return STATUS_OK;
}

Status PfcpSockaddrToFSeid(
    SockAddr *addr, SockAddr *addr6, PfcpFSeid *fSeid, int *len) {
    UTLT_Assert(fSeid, return STATUS_ERROR, "fSeid error when convert socket to f-SEID");

    if (addr && addr6) {
        fSeid->v4 = 1;
        fSeid->v6 = 1;
        fSeid->dualStack.addr4 = addr->s4.sin_addr;
        fSeid->dualStack.addr6 = addr6->s6.sin6_addr;
        *len = PFCP_F_SEID_IPV4V6_LEN;
    } else if (addr) {
        fSeid->v4 = 1;
        fSeid->v6 = 0;
        fSeid->addr4 = addr->s4.sin_addr;
        *len = PFCP_F_SEID_IPV4_LEN;
    } else if (addr6) {
        fSeid->v4 = 0;
        fSeid->v6 = 1;
        fSeid->addr6 = addr6->s6.sin6_addr;
        *len = PFCP_F_SEID_IPV6_LEN;
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "socket neither IPv4 nor IPv6");
    }

    return STATUS_OK;
}

Status PfcpFSeidToIp(PfcpFSeid *fSeid, Ip *ip) {
    UTLT_Assert(ip, return STATUS_ERROR, "f-SEID to IP IP error");
    UTLT_Assert(fSeid, return STATUS_ERROR, "f-SEID to IP f-SEID error");

    memset(ip, 0, sizeof(Ip));

    ip->ipv4 = fSeid->v4;
    ip->ipv6 = fSeid->v6;

    if (ip->ipv4 && ip->ipv6) {
        ip->dualStack.addr4 = fSeid->dualStack.addr4;
        ip->dualStack.addr6 = fSeid->dualStack.addr6;
        ip->len = IPV4V6_LEN;
    } else if (ip->ipv4) {
        ip->addr4 = fSeid->addr4;
        ip->len = IPV4_LEN;
    } else if (ip->ipv6) {
        ip->addr6 = fSeid->addr6;
        ip->len = IPV6_LEN;
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "f-SEID neither IPv4 nor IPv6");
    }

    return STATUS_OK;
}

Status PfcpIpToFSeid(Ip *ip, PfcpFSeid *fSeid, int *len) {
    UTLT_Assert(ip, return STATUS_ERROR, "IP to f-SEID IP error");
    UTLT_Assert(fSeid, return STATUS_ERROR, "IP to f-SEID f-SEID error");

    fSeid->v4 = ip->ipv4;
    fSeid->v6 = ip->ipv6;

    if (fSeid->v4 && fSeid->v6) {
        fSeid->dualStack.addr4 = ip->dualStack.addr4;
        fSeid->dualStack.addr6 = ip->dualStack.addr6;
        *len = PFCP_F_SEID_IPV4V6_LEN;
    } else if (fSeid->v4) {
        fSeid->addr4 = ip->addr4;
        *len = PFCP_F_SEID_IPV4_LEN;
    } else if (fSeid->v6) {
        fSeid->addr6 = ip->addr6;
        *len = PFCP_F_SEID_IPV6_LEN;
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "IP neigher IPv4 nor IPv6");
    }

    return STATUS_OK;
}

Status PfcpIpToFTeid(Ip *ip, PfcpFTeid *fTeid, int *len) {
    UTLT_Assert(ip, return STATUS_ERROR, "IP to f-TEID IP error");
    UTLT_Assert(fTeid, return STATUS_ERROR, "IP to f-TEID f-TEID error");

    fTeid->v4 = ip->ipv4;
    fTeid->v6 = ip->ipv6;

    if (fTeid->v4 && fTeid->v6) {
        fTeid->dualStack.addr4 = ip->dualStack.addr4;
        fTeid->dualStack.addr6 = ip->dualStack.addr6;
        *len = PFCP_F_TEID_IPV4V6_LEN;
    } else if (fTeid->v4) {
        fTeid->addr4 = ip->addr4;
        *len = PFCP_F_TEID_IPV4_LEN;
    } else if (fTeid->v6) {
        fTeid->addr6 = ip->addr6;
        *len = PFCP_F_TEID_IPV6_LEN;
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "f-TEID neigher IPv4 nor IPv6");
    }

    return STATUS_OK;
}

Status PfcpIpToUeIpAddr(Ip *ip, PfcpUeIpAddr *ueIp, int *len) {
    UTLT_Assert(ip, return STATUS_ERROR, "IP to UEIP IP error");
    UTLT_Assert(ueIp, return STATUS_ERROR, "IP to UEIP UEIP error");

    ueIp->v4 = ip->ipv4;
    ueIp->v6 = ip->ipv6;

    if (ueIp->v4 && ueIp->v6) {
        ueIp->dualStack.addr4 = ip->dualStack.addr4;
        ueIp->dualStack.addr6 = ip->dualStack.addr6;
        *len = PFCP_UE_IP_ADDR_IPV4V6_LEN;
    } else if (ueIp->v4) {
        ueIp->addr4 = ip->addr4;
        *len = PFCP_UE_IP_ADDR_IPV4_LEN;
    } else if (ueIp->v6) {
        ueIp->addr6 = ip->addr6;
        *len = PFCP_UE_IP_ADDR_IPV6_LEN;
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "UEIP neigher IPv4 nor IPv6");
    }

    return STATUS_OK;
}

Status PfcpOuterHdrToIp(PfcpOuterHdr *outerHdr, Ip *ip) {
    UTLT_Assert(ip, return STATUS_ERROR, "Outer HDR to IP IP error");
    UTLT_Assert(outerHdr, return STATUS_ERROR, "Outer HDR to IP HDR error");

    memset(ip, 0, sizeof(Ip));
    
    ip->ipv4 = outerHdr->gtpuIpv4;
    ip->ipv6 = outerHdr->gtpuIpv6;

    if (outerHdr->gtpuIpv4 && outerHdr->gtpuIpv6) {
        ip->dualStack.addr4 = outerHdr->dualStack.addr4;
        ip->dualStack.addr6 = outerHdr->dualStack.addr6;
        ip->len = IPV4V6_LEN;
    } else if (outerHdr->gtpuIpv4) {
        ip->addr4 = outerHdr->addr4;
        ip->len = IPV4_LEN;
    } else if (outerHdr->gtpuIpv6) {
        ip->addr6 = outerHdr->addr6;
        ip->len = IPV6_LEN;
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "HDR neigher IPv4 nor IPv6");
    }

    return STATUS_OK;
}

Status PfcpSockaddrToFTeid(
    SockAddr *addr, SockAddr *addr6, PfcpFTeid *fTeid, int *len) {
    UTLT_Assert(fTeid, return STATUS_ERROR, "Socket to F-TEID F-TEID error");

    if (addr && addr6) {
        fTeid->v4 = 1;
        fTeid->v6 = 1;
        fTeid->dualStack.addr4 = addr->s4.sin_addr;
        fTeid->dualStack.addr6 = addr6->s6.sin6_addr;
        *len = PFCP_F_TEID_IPV4V6_LEN;
    } else if (addr) {
        fTeid->v4 = 1;
        fTeid->v6 = 0;
        fTeid->addr4 = addr->s4.sin_addr;
        *len = PFCP_F_TEID_IPV4_LEN;
    } else if (addr6) {
        fTeid->v4 = 0;
        fTeid->v6 = 1;
        fTeid->addr6 = addr6->s6.sin6_addr;
        *len = PFCP_F_TEID_IPV6_LEN;
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "Socket neigher IPv4 nor IPv6");
    }

    return STATUS_OK;
}
