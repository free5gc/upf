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
#include "pfcp_convert.h"

#include "gtp5g_context.h"
#include "updk/rule_far.h"

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

typedef struct gtp5g_far     UpfFar;

enum {
    RULE_PDR_UNSPEC = 0,

    RULE_FAR_ADD,
    RULE_FAR_MOD,
    RULE_FAR_DEL,

    RULE_FAR_MAX,
};

#define _addr4ToStr(addrPtr, ipStr)                         \
    char ipStr[INET_ADDRSTRLEN];                            \
    inet_ntop(AF_INET, addrPtr, ipStr, INET_ADDRSTRLEN);

Status _pushFarToKernel(struct gtp5g_far *far, int action) {
    UTLT_Assert(far, return -1, "push FAR not found");
    Status status;

    // Only support single device in our UPF
    char *ifname = Gtp5gSelf()->ifname;

    uint32_t farId = *(uint32_t*)gtp5g_far_get_id(far);

    if (gtp5g_far_get_apply_action(far)) {
        UTLT_Debug("gtp5g get apply action: %u", *gtp5g_far_get_apply_action(far));
    }
    if (gtp5g_far_get_outer_header_creation_description(far)) {
        UTLT_Debug("gtp5g get description: %u",
                   *gtp5g_far_get_outer_header_creation_description(far));
        if (gtp5g_far_get_outer_header_creation_peer_addr_ipv4(far)) {
            _addr4ToStr(gtp5g_far_get_outer_header_creation_peer_addr_ipv4(far),
                        ipStr);
            UTLT_Debug("gtp5g get peer ipv4: %s", ipStr);
        }
        if (gtp5g_far_get_outer_header_creation_port(far)) {
            UTLT_Debug("gtp5g get port: %u",
                       ntohs(*gtp5g_far_get_outer_header_creation_port(far)));
        }
    }

    switch (action) {
    case RULE_FAR_ADD:
        UTLT_Debug("FAR add to kernel, dev: %s, far id: %u",
                   ifname, ntohl(*gtp5g_far_get_id(far)));
        status = GtpTunnelAddFar(ifname, far);
        UTLT_Assert(status == STATUS_OK, return -1,
                    "Add FAR failed");
        break;
    case RULE_FAR_MOD:
        UTLT_Debug("FAR modify to kernel, dev: %s, far id: %u",
                   ifname, ntohl(*gtp5g_far_get_id(far)));
        status = GtpTunnelModFar(ifname, far);
        UTLT_Assert(status == STATUS_OK, return -1,
                    "Modify FAR failed");
        break;
    case RULE_FAR_DEL:
        UTLT_Debug("FAR delete to kernel, dev: %s, far id: %u",
                   ifname, ntohl(*gtp5g_far_get_id(far)));
        status = GtpTunnelDelFar(ifname, farId);
        UTLT_Assert(status == STATUS_OK, return -1,
                    "Delete FAR failed");
        break;
    default:
        UTLT_Assert(0, return -1,
                    "FAR Action %d not defined", action);
    }

    return STATUS_OK;
}

int _SetGtp5gFar(UpfFar *upfFar, UPDK_FAR *far) {
    gtp5g_far_set_id(upfFar, far->farId);
    UTLT_Debug("gtp5g get FAR ID: %u", far->farId);
    
    if (far->flags.applyAction) {
        gtp5g_far_set_apply_action(upfFar, far->applyAction);
        UTLT_Debug("gtp5g get FAR Apply Action: %u", far->applyAction);
    }

    if (far->flags.forwardingParameters) {
        UPDK_ForwardingParameters *fwdParam = &far->forwardingParameters;

        if (fwdParam->flags.destinationInterface) {
            // TODO:
            UTLT_Debug("gtp5g get FAR Apply Action: %u", fwdParam->destinationInterface);
        }

        if (fwdParam->flags.outerHeaderCreation) {
            UPDK_OuterHeaderCreation *outerHeaderCreation = &fwdParam->outerHeaderCreation;
            
            char ipStr[INET6_ADDRSTRLEN];
            switch(outerHeaderCreation->description) {
                case UPDK_OUTER_HEADER_CREATION_DESCRIPTION_GTPU_UDP_IPV4:
                    gtp5g_far_set_outer_header_creation(upfFar,
                        outerHeaderCreation->description,
                        outerHeaderCreation->teid,
                        &(outerHeaderCreation->ipv4),
                        2152);
                    inet_ntop(AF_INET, &(outerHeaderCreation->ipv4), ipStr, INET_ADDRSTRLEN);
                    UTLT_Debug("gtp5g get Outer Header Creation: Desp[%u], TEID[%u], IP[%s], Port[%u]",
                        fwdParam->destinationInterface, outerHeaderCreation->teid, ipStr, 2152);
                    break;
                case UPDK_OUTER_HEADER_CREATION_DESCRIPTION_GTPU_UDP_IPV6:
                    // TODO: Need to implement IPv6 with gtp5g and libgtp5gnl
                    /*
                    gtp5g_far_set_outer_header_creation(upfFar,
                        outerHeaderCreation->description,
                        outerHeaderCreation->teid,
                        &(outerHeaderCreation->ipv6),
                        2152);
                    inet_ntop(AF_INET6, &(outerHeaderCreation->ipv6), ipStr, INET6_ADDRSTRLEN);
                    UTLT_Debug("gtp5g get Outer Header Creation: Desp[%u], TEID[%u], IP[%s], Port[%u]",
                        fwdParam->destinationInterface, outerHeaderCreation->teid, ipStr, 2152);
                    */
                    UTLT_Warning("Do NOT implement IPv6 yet");
                    break;
                case UPDK_OUTER_HEADER_CREATION_DESCRIPTION_UDP_IPV4:
                    gtp5g_far_set_outer_header_creation(upfFar,
                        outerHeaderCreation->description,
                        0,
                        &(outerHeaderCreation->ipv4),
                        outerHeaderCreation->port);
                    inet_ntop(AF_INET, &(outerHeaderCreation->ipv4), ipStr, INET_ADDRSTRLEN);
                    UTLT_Debug("Outer Header Creation: Desp[%u], TEID[%u], IP[%s], Port[%u]",
                        fwdParam->destinationInterface, 0, ipStr, outerHeaderCreation->port);
                    break;
                case UPDK_OUTER_HEADER_CREATION_DESCRIPTION_UDP_IPV6:
                    // TODO: Need to implement IPv6 with gtp5g and libgtp5gnl
                    /*
                    gtp5g_far_set_outer_header_creation(upfFar,
                        outerHeaderCreation->description,
                        0,
                        &(outerHeaderCreation->ipv6),
                        outerHeaderCreation->port);
                    inet_ntop(AF_INET6, &(outerHeaderCreation->ipv6), ipStr, INET6_ADDRSTRLEN);
                    UTLT_Debug("gtp5g get Outer Header Creation: Desp[%u], TEID[%u], IP[%s], Port[%u]",
                        fwdParam->destinationInterface, 0, ipStr, outerHeaderCreation->port);
                    */
                    UTLT_Warning("Do NOT implement IPv6 yet");
                    break;
                default:
                    UTLT_Error("Outer Header Creation Description is error: %u (host type)",
                        outerHeaderCreation->description);
                    return -1;
            }
        }

        if (fwdParam->flags.forwardingPolicy) {
            UPDK_ForwardingPolicy *forwardingPolicy = &fwdParam->forwardingPolicy;
            gtp5g_far_set_fwd_policy(upfFar, forwardingPolicy->forwardingPolicyIdentifier);
        }
    }

    return 0;
}

int Gtpv1TunnelCreateFAR(UPDK_FAR *far) {
    UTLT_Assert(far, return -1, "UPDK_FAR pointer is NULL");

    UTLT_Assert(far->flags.farId, return -1, "UPDK_FAR ID is not set");

    Status status = STATUS_OK;

    UpfFar *tmpFar = gtp5g_far_alloc();
    UTLT_Assert(tmpFar, return -1, "FAR allocate error");

    status = _SetGtp5gFar(tmpFar, far);
    UTLT_Assert(status == 0, goto FREEFAR, "Set gtp5g FAR is failed");

    // Send FAR to kernel
    status = _pushFarToKernel(tmpFar, RULE_FAR_ADD);
    UTLT_Assert(status == STATUS_OK, goto FREEFAR, "FAR not pushed to kernel");

FREEFAR:
    gtp5g_far_free(tmpFar);
    UTLT_Assert(tmpFar != NULL, return -1, "Free FAR struct error");

    return status;
}

int Gtpv1TunnelUpdateFAR(UPDK_FAR *far) {
    UTLT_Assert(far, return -1, "UPDK_FAR pointer is NULL");

    UTLT_Assert(far->flags.farId, return -1, "UPDK_FAR ID is not set");

    Status status = STATUS_OK;

    UpfFar *tmpFar = gtp5g_far_alloc();
    UTLT_Assert(tmpFar, return -1, "FAR allocate error");
    
    status = _SetGtp5gFar(tmpFar, far);
    UTLT_Assert(status == 0, goto FREEFAR, "Set gtp5g FAR is failed");

    // TODO: update FAR to kernel
    status = _pushFarToKernel(tmpFar, RULE_FAR_MOD);
    UTLT_Assert(status == STATUS_OK, goto FREEFAR, "FAR not pushed to kernel");
    
FREEFAR:
    gtp5g_far_free(tmpFar);
    UTLT_Assert(tmpFar != NULL, return -1, "Free FAR struct error");

    return status;
}

int Gtpv1TunnelRemoveFAR(UPDK_FAR *far) {
    UTLT_Assert(far, return -1, "UPDK_FAR pointer is NULL");

    UTLT_Assert(far->flags.farId, return -1, "UPDK_FAR ID is not set");

    // TODO: Is this necessary?
    /*
    UpfFar *far = GtpTunnelFindFarById(Gtp5gSelf()->ifname, far->farId);
    UTLT_Assert(far, return STATUS_ERROR, "Cannot find FAR[%u] by FarId", far->farId);

    // Set FarId to 0 if the PDR has this far
    int pdrNum = *(int*)gtp5g_far_get_related_pdr_num(far);
    uint16_t *pdrList = gtp5g_far_get_related_pdr_list(far);

    for (size_t idx = 0; idx < pdrNum; idx++) {
        UpfPdr *tmpPdr = GtpTunnelFindPdrById(gtpIfName, pdrList[idx]);
        gtp5g_pdr_set_far_id(tmpPdr, 0);
        _pushPdrToKernel(tmpPdr, _PDR_MOD);
        gtp5g_pdr_free(tmpPdr);
        UTLT_Assert(tmpPdr != NULL, continue, "Free pdr error");
    }
    */

    UTLT_Assert(GtpTunnelDelFar(Gtp5gSelf()->ifname, far->farId) == STATUS_OK,
        return -1, "FAR[%u] delete failed", far->farId);

    return 0;
}