#include "gtp_tunnel.h"

#include "utlt_debug.h"

#include <net/if.h>
#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include "gtp_link.h"
#include "libgtp5gnl/gtp5g.h"
#include "libgtp5gnl/gtp5gnl.h"

Status GtpTunnelAddQer(const char *ifname, struct gtp5g_qer *qer) {
    Status status;
    NetlinkInfo info;

    UTLT_Assert(qer, return STATUS_ERROR, "QER is NULL");

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    UTLT_Assert(!gtp5g_add_qer(info.genl_id, info.nl, dev, qer), status = STATUS_ERROR,
        "GtpTunnelAddQer Fail: QER id[%u]", *gtp5g_qer_get_id(qer));

    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return status;
}

Status GtpTunnelModQer(const char *ifname, struct gtp5g_qer *qer) {
    Status status;
    NetlinkInfo info;

    UTLT_Assert(qer, return STATUS_ERROR, "QER is NULL");

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    UTLT_Assert(!gtp5g_mod_qer(info.genl_id, info.nl, dev, qer), status = STATUS_ERROR,
        "GtpTunnelModQer Fail: QER id[%u]", *gtp5g_qer_get_id(qer));

    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return status;
}

Status GtpTunnelDelQer(const char *ifname, uint32_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_qer *qer = gtp5g_qer_alloc();
    gtp5g_qer_set_id(qer, id);
    
    UTLT_Assert(!gtp5g_del_qer(info.genl_id, info.nl, dev, qer), status = STATUS_ERROR,
        "GtpTunnelDelqer fail: QER id[%u]", id);

    gtp5g_qer_free(qer);
    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return status;
}

struct gtp5g_qer *GtpTunnelFindQerById(const char *ifname, uint32_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return NULL, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_qer *qer = gtp5g_qer_alloc();
    gtp5g_qer_set_id(qer, id);
    
    struct gtp5g_qer *rt_qer;
    UTLT_Assert((rt_qer = gtp5g_qer_find_by_id(info.genl_id, info.nl, dev, qer)), , 
        "GtpTunnelFindQerById fail: QER id[%u]", id);

    gtp5g_qer_free(qer);
    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return rt_qer;
}

Status GtpTunnelAddPdr(const char *ifname, struct gtp5g_pdr *pdr) {
    Status status;
    NetlinkInfo info;

    UTLT_Assert(pdr, return STATUS_ERROR, "PDR is NULL");

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    UTLT_Assert(!gtp5g_add_pdr(info.genl_id, info.nl, dev, pdr), status = STATUS_ERROR,
        "GtpTunnelAddPdr Fail: PDR id[%u]", *gtp5g_pdr_get_id(pdr));

    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return status;
}

Status GtpTunnelModPdr(const char *ifname, struct gtp5g_pdr *pdr) {
    Status status;
    NetlinkInfo info;

    UTLT_Assert(pdr, return STATUS_ERROR, "PDR is NULL");

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    UTLT_Assert(!gtp5g_mod_pdr(info.genl_id, info.nl, dev, pdr), status = STATUS_ERROR,
        "GtpTunnelModPdr Fail: PDR id[%u]", *gtp5g_pdr_get_id(pdr));

    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return status;
}

Status GtpTunnelDelPdr(const char *ifname, uint16_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_pdr *pdr = gtp5g_pdr_alloc();
    gtp5g_pdr_set_id(pdr, id);
    
    UTLT_Assert(!gtp5g_del_pdr(info.genl_id, info.nl, dev, pdr), status = STATUS_ERROR,
        "GtpTunnelDelPdr fail: PDR id[%u]", id);

    gtp5g_pdr_free(pdr);
    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return status;
}

struct gtp5g_pdr *GtpTunnelFindPdrById(const char *ifname, uint16_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return NULL, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_pdr *pdr = gtp5g_pdr_alloc();
    gtp5g_pdr_set_id(pdr, id);
    
    struct gtp5g_pdr *rt_pdr;
    UTLT_Assert((rt_pdr = gtp5g_pdr_find_by_id(info.genl_id, info.nl, dev, pdr)), , 
        "GtpTunnelFindPdrById fail: PDR id[%u]", id);

    gtp5g_pdr_free(pdr);
    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return rt_pdr;
}

Status GtpTunnelAddFar(const char *ifname, struct gtp5g_far *far) {
    Status status;
    NetlinkInfo info;

    UTLT_Assert(far, return STATUS_ERROR, "FAR is NULL");

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    UTLT_Assert(!gtp5g_add_far(info.genl_id, info.nl, dev, far), status = STATUS_ERROR,
        "GtpTunnelAddFar fail: FAR id[%u]", *gtp5g_far_get_id(far));

    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return status;
}

Status GtpTunnelModFar(const char *ifname, struct gtp5g_far *far) {
    Status status;
    NetlinkInfo info;

    UTLT_Assert(far, return STATUS_ERROR, "FAR is NULL");

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    UTLT_Assert(!gtp5g_mod_far(info.genl_id, info.nl, dev, far), status = STATUS_ERROR,
        "GtpTunnelModFar fail: FAR id[%u]", *gtp5g_far_get_id(far));

    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return status;
}

Status GtpTunnelDelFar(const char *ifname, uint32_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_far *far = gtp5g_far_alloc();
    gtp5g_far_set_id(far, id);

    UTLT_Assert(!gtp5g_del_far(info.genl_id, info.nl, dev, far), status = STATUS_ERROR,
        "GtpTunnelDelFar fail: FAR id[%u]", id);

    gtp5g_far_free(far);
    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return status;
}

struct gtp5g_far *GtpTunnelFindFarById(const char *ifname, uint32_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, return NULL, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_far *far = gtp5g_far_alloc();
    gtp5g_far_set_id(far, id);

    struct gtp5g_far *rt_far;
    UTLT_Assert((rt_far = gtp5g_far_find_by_id(info.genl_id, info.nl, dev, far)), ,
        "GtpTunnelFindFarById fail: FAR id[%u]", id);

    gtp5g_far_free(far);
    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return rt_far;
}
