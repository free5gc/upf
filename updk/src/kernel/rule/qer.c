#include "updk/rule.h"

#include <arpa/inet.h>

#include "utlt_debug.h"
#include "libgtp5gnl/gtp5g.h"
#include "libgtp5gnl/gtp5gnl.h"
#include "gtp_tunnel.h"
#include "pfcp_convert.h"

#include "gtp5g_context.h"
#include "updk/rule_qer.h"

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

typedef struct gtp5g_qer     UpfQer;

enum {
    RULE_QER_UNSPEC = 0,

    RULE_QER_ADD,
    RULE_QER_MOD,
    RULE_QER_DEL,

    RULE_QER_MAX,
};

#define _addr4ToStr(addrPtr, ipStr)                         \
    char ipStr[INET_ADDRSTRLEN];                            \
    inet_ntop(AF_INET, addrPtr, ipStr, INET_ADDRSTRLEN);

Status _pushQerToKernel(struct gtp5g_qer *qer, int action) {
    UTLT_Assert(qer, return -1, "push QER not found");
    Status status;

    // Only support single device in our UPF
    char *ifname = Gtp5gSelf()->ifname;

    uint32_t qerId = *(uint32_t*)gtp5g_qer_get_id(qer);

    switch (action) {
    case RULE_QER_ADD:
        UTLT_Debug("QER add to kernel, dev: %s, qer id: %u",
                   ifname, ntohl(*gtp5g_qer_get_id(qer)));
        status = GtpTunnelAddQer(ifname, qer);
        UTLT_Assert(status == STATUS_OK, return -1,
                    "Add QER failed");
        break;
    case RULE_QER_MOD:
        UTLT_Debug("QER modify to kernel, dev: %s, qer id: %u",
                   ifname, ntohl(*gtp5g_qer_get_id(qer)));
        status = GtpTunnelModQer(ifname, qer);
        UTLT_Assert(status == STATUS_OK, return -1,
                    "Modify QER failed");
        break;
    case RULE_QER_DEL:
        UTLT_Debug("QER delete to kernel, dev: %s, qer id: %u",
                   ifname, ntohl(*gtp5g_qer_get_id(qer)));
        status = GtpTunnelDelQer(ifname, qerId);
        UTLT_Assert(status == STATUS_OK, return -1,
                    "Delete QER failed");
        break;
    default:
        UTLT_Assert(0, return -1,
                    "QER Action %d not defined", action);
    }

    return STATUS_OK;
}

int _SetGtp5gQer(UpfQer *upfQer, UPDK_QER *qer) {
    gtp5g_qer_set_id(upfQer, qer->qerId);
    UTLT_Debug("gtp5g get QER ID: %u", qer->qerId);

    if (qer->flags.qerCorrelationId) {
        gtp5g_qer_set_qer_corr_id(upfQer, qer->qerCorrelationId);
    }
    
    if (qer->flags.gateStatus) {
        gtp5g_qer_set_gate_status(upfQer, qer->gateStatus);
        UTLT_Debug("gtp5g get QER Gate Status: %u", qer->gateStatus);
    }

    if (qer->flags.maximumBitrate) {
        UPDK_Bitrate *mbr = &qer->maximumBitrate;
        gtp5g_qer_set_mbr_uhigh(upfQer, mbr->ul >> 8);
        gtp5g_qer_set_mbr_ulow(upfQer, mbr->ul & 0xff);
        gtp5g_qer_set_mbr_dhigh(upfQer, mbr->dl >> 8);
        gtp5g_qer_set_mbr_dlow(upfQer, mbr->dl & 0xff);
    }

    if (qer->flags.guaranteedBitrate) {
        UPDK_Bitrate *gbr = &qer->guaranteedBitrate;
        gtp5g_qer_set_gbr_uhigh(upfQer, gbr->ul >> 8);
        gtp5g_qer_set_gbr_ulow(upfQer, gbr->ul & 0xff);
        gtp5g_qer_set_gbr_dhigh(upfQer, gbr->dl >> 8);
        gtp5g_qer_set_gbr_dlow(upfQer, gbr->dl & 0xff);
    }

    /*
    if (qer->flags.packetRate) {
        UPDK_PacketRate *pr = &qer->packetRate;
    }

    if (qer->flags.dlFlowLevelMarking) {
        UPDK_DLFlowLevelMarking *m = &qer->dlFlowLevelMarking;
    }
    */

    if (qer->flags.qosFlowIdentifier) {
        gtp5g_qer_set_qfi(upfQer, qer->qosFlowIdentifier);
    }

    if (qer->flags.reflectiveQos) {
        gtp5g_qer_set_rqi(upfQer, qer->reflectiveQos);
    }

    // gtp5g_qer_set_ppi(upfQer, qer->);
    // gtp5g_qer_set_rcsr(upfQer, qer->);

    return 0;
}

int Gtpv1TunnelCreateQER(UPDK_QER *qer) {
    UTLT_Assert(qer, return -1, "UPDK_QER pointer is NULL");

    UTLT_Assert(qer->flags.qerId, return -1, "UPDK_QER ID is not set");

    Status status = STATUS_OK;

    UpfQer *tmpQer = gtp5g_qer_alloc();
    UTLT_Assert(tmpQer, return -1, "QER allocate error");

    status = _SetGtp5gQer(tmpQer, qer);
    UTLT_Assert(status == 0, goto FREEQER, "Set gtp5g QER is failed");

    // Send QER to kernel
    status = _pushQerToKernel(tmpQer, RULE_QER_ADD);
    UTLT_Assert(status == STATUS_OK, goto FREEQER, "QER not pushed to kernel");

FREEQER:
    gtp5g_qer_free(tmpQer);
    UTLT_Assert(tmpQer != NULL, return -1, "Free QER struct error");

    return status;
}

int Gtpv1TunnelUpdateQER(UPDK_QER *qer) {
    UTLT_Assert(qer, return -1, "UPDK_QER pointer is NULL");

    UTLT_Assert(qer->flags.qerId, return -1, "UPDK_QER ID is not set");

    Status status = STATUS_OK;

    UpfQer *tmpQer = gtp5g_qer_alloc();
    UTLT_Assert(tmpQer, return -1, "QER allocate error");

    status = _SetGtp5gQer(tmpQer, qer);
    UTLT_Assert(status == 0, goto FREEQER, "Set gtp5g QER is failed");

    // TODO: update QER to kernel
    status = _pushQerToKernel(tmpQer, RULE_QER_MOD);
    UTLT_Assert(status == STATUS_OK, goto FREEQER, "QER not pushed to kernel");

FREEQER:
    gtp5g_qer_free(tmpQer);
    UTLT_Assert(tmpQer != NULL, return -1, "Free QER struct error");

    return status;
}

int Gtpv1TunnelRemoveQER(UPDK_QER *qer) {
    UTLT_Assert(GtpTunnelDelQer(Gtp5gSelf()->ifname, qer->qerId) == STATUS_OK,
        return -1, "QER[%u] delete failed", qer->qerId);

    return 0;
}
