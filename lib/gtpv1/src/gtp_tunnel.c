#include "gtp_tunnel.h"

#include <net/if.h>
#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include "gtp_link.h"
#include "libgtp5gnl/gtp5g.h"
#include "libgtp5gnl/gtp5gnl.h"

Status GtpTunnelAddPdr(const char *ifname, struct gtp5g_pdr *pdr) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    gtp5g_add_pdr(info.genl_id, info.nl, dev, pdr);

    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpTunnelModPdr(const char *ifname, struct gtp5g_pdr *pdr) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    gtp5g_mod_pdr(info.genl_id, info.nl, dev, pdr);

    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpTunnelDelPdr(const char *ifname, uint32_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_pdr *pdr = gtp5g_pdr_alloc();
    gtp5g_pdr_set_id(pdr, id);
    
    gtp5g_del_pdr(info.genl_id, info.nl, dev, pdr);

    gtp5g_pdr_free(pdr);
    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpTunnelFindPdrById(const char *ifname, uint32_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_pdr *pdr = gtp5g_pdr_alloc();
    gtp5g_pdr_set_id(pdr, id);
    
    gtp5g_pdr_find_by_id(info.genl_id, info.nl, dev, pdr);

    gtp5g_pdr_free(pdr);
    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpTunnelAddFar(const char *ifname, struct gtp5g_far *far) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    gtp5g_add_far(info.genl_id, info.nl, dev, far);

    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpTunnelModFar(const char *ifname, struct gtp5g_far *far) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    gtp5g_mod_far(info.genl_id, info.nl, dev, far);

    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpTunnelDelFar(const char *ifname, uint32_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_far *far = gtp5g_far_alloc();
    gtp5g_far_set_id(far, id);

    gtp5g_del_far(info.genl_id, info.nl, dev, far);

    gtp5g_far_free(far);
    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpTunnelFindFarById(const char *ifname, uint32_t id) {
        Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname, "gtp5g");
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_far *far = gtp5g_far_alloc();
    gtp5g_far_set_id(far, id);

    gtp5g_far_find_by_id(info.genl_id, info.nl, dev, far);

    gtp5g_far_free(far);
    gtp5g_dev_free(dev);
    NetlinkSockClose(&info);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}