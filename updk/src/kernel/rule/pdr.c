#include "updk/rule.h"

/**
 * This file is dependent on how UPF to decode PFCP message from SMF.
 * Therefore, it includes "pfcp_message.h" under "upf/lib/pfcp".
 * 
 * Please modify rule type if you use other UPF with different PFCP.
 */

#include <arpa/inet.h>

#include "utlt_debug.h"
#include "libgtp5gnl/gtp5g.h"
#include "libgtp5gnl/gtp5gnl.h"
#include "gtp_tunnel.h"

#include "gtp5g_context.h"
#include "updk/rule_pdr.h"

/*
 * These functions shall be customized by kinds of device.
 * You can create a directory and put all customized function
 * in there, like "device.c" under "updk/src/kernel/include"
 *
 * Note:
 * 1. Parameter in each Gtpv1Tunnel* function may not be the same.
 * Therefore, please do not use memory copy function to handle in different type.
 * 2. Function "Gtpv1Tunnel*" how to work is dependent on the kind of device.
 * It is up to you can set the real rule into your device or just treat it
 * as a notification.
 */

typedef struct gtp5g_pdr     UpfPdr;

enum {
    RULE_PDR_UNSPEC = 0,

    RULE_PDR_ADD,
    RULE_PDR_MOD,
    RULE_PDR_DEL,

    RULE_PDR_MAX,
};

#define _addr4ToStr(addrPtr, ipStr)                         \
    char ipStr[INET_ADDRSTRLEN];                            \
    inet_ntop(AF_INET, addrPtr, ipStr, INET_ADDRSTRLEN);


Status _pushPdrToKernel(struct gtp5g_pdr *pdr, int action) {
    UTLT_Assert(pdr, return -1, "push PDR not found");
    Status status;

    // Only support single device in our UPF
    char *ifname = Gtp5gSelf()->ifname;

    uint16_t pdrId = *(uint16_t*)gtp5g_pdr_get_id(pdr);

    if (gtp5g_pdr_get_precedence(pdr)) {
        UTLT_Debug("gtp5g get precendence: %u", ntohl(*gtp5g_pdr_get_precedence(pdr)));
    }
    if (gtp5g_pdr_get_far_id(pdr)) {
        UTLT_Debug("gtp5g get farId: %u", ntohl(*gtp5g_pdr_get_far_id(pdr)));
    }
    if (gtp5g_pdr_get_outer_header_removal(pdr)) {
        UTLT_Debug("gtp5g get outer header removal: %u",
                   *gtp5g_pdr_get_outer_header_removal(pdr));
    }
    if (gtp5g_pdr_get_ue_addr_ipv4(pdr)) {
        _addr4ToStr(gtp5g_pdr_get_ue_addr_ipv4(pdr), ipStr);
        UTLT_Debug("gtp5g get ue ip: %s", ipStr);
    }
    if (gtp5g_pdr_get_local_f_teid_teid(pdr)) {
        UTLT_Debug("gtp5g get teid: %u", ntohl(*gtp5g_pdr_get_local_f_teid_teid(pdr)));
    }
    if (gtp5g_pdr_get_local_f_teid_gtpu_addr_ipv4(pdr)) {
        _addr4ToStr(gtp5g_pdr_get_local_f_teid_gtpu_addr_ipv4(pdr), ipStr);
        UTLT_Debug("gtp5g get gtpu ip: %s", ipStr);
    }

    switch (action) {
    case RULE_PDR_ADD:
        UTLT_Debug("PDR add to kernel, dev: %s, pdr id: %u",
                   ifname, ntohs(*gtp5g_pdr_get_id(pdr)));
        status = GtpTunnelAddPdr(ifname, pdr);
        UTLT_Assert(status == STATUS_OK, return -1,
                    "Add PDR failed");
        break;
    case RULE_PDR_MOD:
        UTLT_Debug("PDR modify to kernel, dev: %s, pdr id: %u",
                   ifname, ntohs(*gtp5g_pdr_get_id(pdr)));
        status = GtpTunnelModPdr(ifname, pdr);
        UTLT_Assert(status == STATUS_OK, return -1,
                    "Modify PDR failed");
        break;
    case RULE_PDR_DEL:
        UTLT_Debug("PDR delete to kernel, dev: %s, pdr id: %u",
                   ifname, ntohs(*gtp5g_pdr_get_id(pdr)));
        status = GtpTunnelDelPdr(ifname, pdrId);
        UTLT_Assert(status == STATUS_OK, return -1,
                    "Delete PDR failed");
        break;
    default:
        UTLT_Assert(0, return -1,
                    "PDR Action %d not defined", action);
    }

    return STATUS_OK;
}

int _SetGtp5gPdr(UpfPdr *upfPdr, UPDK_PDR *pdr) {

    gtp5g_pdr_set_unix_sock_path(upfPdr, UNIX_SOCK_BUFFERING_PATH);

    gtp5g_pdr_set_id(upfPdr, pdr->pdrId);
    UTLT_Debug("gtp5g set PDR ID: %u", pdr->pdrId);

    if (pdr->flags.precedence) {
        gtp5g_pdr_set_precedence(upfPdr, pdr->precedence);
        UTLT_Debug("gtp5g set PDR Precedence: %u", pdr->precedence);
    }


    if (pdr->flags.pdi) {
        UPDK_PDI *pdi = &pdr->pdi;
        char ipStr[INET6_ADDRSTRLEN];

        /*
        if (pdi->flags.sourceInterface) {
            // TODO: Need to handle with gtp5g and libgtp5gnl
            UTLT_Debug("gtp5g set PDI Source Interface: %u", pdi->sourceInterface);
        }
        */

        if (pdi->flags.fTeid) {
            UPDK_FTEID *fteid = &pdi->fTeid;

            if (fteid->flags.v4 && fteid->flags.v6) {
                // TODO: Dual Stack
                UTLT_Warning("Do NOT support dual stack in F-TEID yet");
            } else if (fteid->flags.v4) {
                gtp5g_pdr_set_local_f_teid(upfPdr, fteid->teid, &(fteid->ipv4));
                inet_ntop(AF_INET, &(fteid->ipv4), ipStr, INET_ADDRSTRLEN);
                UTLT_Debug("gtp5g set PDI F-TEID: %u, %s", fteid->teid, ipStr);
            } else if (fteid->flags.v6) {
                // TODO: Need to handle with gtp5g and libgtp5gnl
                /*
                gtp5g_pdr_set_local_f_teid(upfPdr, fteid->teid, &(fteid->ipv6));
                inet_ntop(AF_INET6, &(fteid->ipv6), ipStr, INET6_ADDRSTRLEN);
                UTLT_Debug("gtp5g set PDI F-TEID: %u, %s", fteid->teid, ipStr);
                */
                UTLT_Warning("Do NOT support IPv6 in F-TEID yet");
            } else {
                // TODO: Handle Choose flag
                UTLT_Warning("Do Not support CHOOSE flag without any IP in F-TEID yet");
            }
        }

        if (pdi->flags.ueIpAddress) {
            UPDK_UEIPAddress *ueIp = &pdi->ueIpAddress;
            if (ueIp->flags.v4 && ueIp->flags.v6) {
                // TODO: Dual Stack
                UTLT_Warning("Do NOT support dual stack in UE IP Address yet");
            } else if (ueIp->flags.v4) {
                gtp5g_pdr_set_ue_addr_ipv4(upfPdr, &(ueIp->ipv4));
                inet_ntop(AF_INET, &(ueIp->ipv4), ipStr, INET_ADDRSTRLEN);
                UTLT_Debug("gtp5g set PDI UE IP Address: %s", ipStr);
            } else if (ueIp->flags.v6) {
                // TODO: Need to handle with gtp5g and libgtp5gnl
                UTLT_Warning("Do NOT support IPv6 in UE IP Address yet");
            } else {
                UTLT_Error("No UE IP in UE IP Address yet");
                return -1;
            }
        }

        if (pdi->flags.sdfFilter) {
            UPDK_SDFFilter *sdfFilter = &pdi->sdfFilter;
            
            if (sdfFilter->flags.fd) {
                gtp5g_pdr_set_sdf_filter_description(upfPdr, sdfFilter->flowDescription);
                UTLT_Debug("gtp5g set SDF Filter Flow Description: %s", sdfFilter->flowDescription);
            }
            
            if (sdfFilter->flags.ttc) {
                //TODO:
                UTLT_Warning("Do NOT support TTC in SDF Filter yet");
            }
            
            if (sdfFilter->flags.spi) {
                //TODO:
                UTLT_Warning("Do NOT support SPI in SDF Filter yet");
            }

            if (sdfFilter->flags.fl) {
                //TODO:
                UTLT_Warning("Do NOT support FL in SDF Filter yet");
            }

            if (sdfFilter->flags.bid) {
                //TODO:
                UTLT_Warning("Do NOT support BID in SDF Filter yet");
            }
        }
    }

    if (pdr->flags.outerHeaderRemoval) {
        gtp5g_pdr_set_outer_header_removal(upfPdr, pdr->outerHeaderRemoval);
        UTLT_Debug("gtp5g set PDR Outer Header Removal: %u", pdr->outerHeaderRemoval);
    }

    if (pdr->flags.farId) {
        gtp5g_pdr_set_far_id(upfPdr, pdr->farId);
        UTLT_Debug("gtp5g set PDR FAR ID: %u", pdr->farId);
    }

    return 0;
}

int Gtpv1TunnelCreatePDR(UPDK_PDR *pdr) {
    UTLT_Assert(pdr, return -1, "UPDK_PDR pointer is NULL");

    UTLT_Assert(pdr->flags.pdrId, return -1, "UPDK_PDR ID is not set");

    Status status = STATUS_OK;

    UpfPdr *tmpPdr = gtp5g_pdr_alloc();
    UTLT_Assert(tmpPdr, return -1, "pdr allocate error");

    status = _SetGtp5gPdr(tmpPdr, pdr);
    UTLT_Assert(status == 0, goto FREEPDR, "Set gtp5g PDR is failed");

    // Send PDR to kernel
    status = _pushPdrToKernel(tmpPdr, RULE_PDR_ADD);
    UTLT_Assert(status == STATUS_OK, goto FREEPDR, "PDR not pushed to kernel");

FREEPDR:
    gtp5g_pdr_free(tmpPdr);
    UTLT_Assert(tmpPdr != NULL, return -1, "Free PDR struct error");

    return status;
}

int Gtpv1TunnelUpdatePDR(UPDK_PDR *pdr) {
    UTLT_Assert(pdr, return -1, "UPDK_PDR pointer is NULL");

    UTLT_Assert(pdr->flags.pdrId, return -1, "UPDK_PDR ID is not set");

    Status status = STATUS_OK;

    UpfPdr *tmpPdr = gtp5g_pdr_alloc();
    UTLT_Assert(tmpPdr, return -1, "pdr allocate error");

    status = _SetGtp5gPdr(tmpPdr, pdr);
    UTLT_Assert(status == 0, goto FREEPDR, "Set gtp5g PDR is failed");

    // update PDR to kernel
    status = _pushPdrToKernel(tmpPdr, RULE_PDR_MOD);
    UTLT_Assert(status == STATUS_OK, goto FREEPDR, "PDR not pushed to kernel");

FREEPDR:
    gtp5g_pdr_free(tmpPdr);
    UTLT_Assert(tmpPdr != NULL, return -1, "Free PDR struct error");

    return 0;
}

int Gtpv1TunnelRemovePDR(UPDK_PDR *pdr) {
    UTLT_Assert(pdr, return -1, "UPDK_PDR pointer is NULL");

    UTLT_Assert(pdr->flags.pdrId, return -1, "UPDK_PDR ID is not set");

    UTLT_Assert(GtpTunnelDelPdr(Gtp5gSelf()->ifname, pdr->pdrId) == STATUS_OK,
        return -1, "PDR[%u] delete failed", pdr->pdrId);

    return 0;
}