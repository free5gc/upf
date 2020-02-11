#include "gtp_rule.h"

#include <net/if.h>
#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include "libgtp5gnl/gtp5g.h"
#include "libgtp5gnl/gtp5gnl.h"

typedef struct {
    struct mnl_socket *nl;
    int32_t genl_id;
    uint32_t ifidx;
} NetlinkInfo;

Status NetlinkSockOpen(NetlinkInfo *info, const char *ifname) {
    UTLT_Assert(info, return STATUS_ERROR, "NetlinkInfo is NULL");

    info->nl = genl_socket_open();
    UTLT_Assert(info->nl, return STATUS_ERROR, "genl_socket_open fail");

    info->genl_id = genl_lookup_family(info->nl, "gtp5g");
    UTLT_Assert(info->genl_id >= 0, goto err, "Not found gtp5g genl family");

    info->ifidx = if_nametoindex(ifname);
    UTLT_Assert(info->ifidx, goto err, "Wrong 5G GTP interface %s", ifname);

    return STATUS_OK;

err:
    mnl_socket_close(info->nl);
    return STATUS_ERROR;
}

Status GtpPdrAdd(const char *ifname, struct gtp5g_pdr *pdr) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname);
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    gtp5g_add_pdr(info.genl_id, info.nl, dev, pdr);

    gtp5g_dev_free(dev);
    mnl_socket_close(info.nl);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}
Status GtpPdrMod(const char *ifname, struct gtp5g_pdr *pdr) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname);
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    gtp5g_mod_pdr(info.genl_id, info.nl, dev, pdr);

    gtp5g_dev_free(dev);
    mnl_socket_close(info.nl);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpPdrDel(const char *ifname, uint32_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname);
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_pdr *pdr = gtp5g_pdr_alloc();
    gtp5g_pdr_set_id(pdr, id);
    
    gtp5g_del_pdr(info.genl_id, info.nl, dev, pdr);

    gtp5g_pdr_free(pdr);
    gtp5g_dev_free(dev);
    mnl_socket_close(info.nl);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpFarAdd(const char *ifname, struct gtp5g_far *far) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname);
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    gtp5g_add_far(info.genl_id, info.nl, dev, far);

    gtp5g_dev_free(dev);
    mnl_socket_close(info.nl);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpFarMod(const char *ifname, struct gtp5g_far *far) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname);
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    gtp5g_mod_far(info.genl_id, info.nl, dev, far);

    gtp5g_dev_free(dev);
    mnl_socket_close(info.nl);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}

Status GtpFarDel(const char *ifname, uint32_t id) {
    Status status;
    NetlinkInfo info;

    status = NetlinkSockOpen(&info, ifname);
    UTLT_Assert(status == STATUS_OK, goto err, "NetlinkSockOpen fail");

    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, info.ifidx);

    struct gtp5g_far *far = gtp5g_far_alloc();
    gtp5g_far_set_id(far, id);

    gtp5g_del_far(info.genl_id, info.nl, dev, far);

    gtp5g_far_free(far);
    gtp5g_dev_free(dev);
    mnl_socket_close(info.nl);

    return STATUS_OK;

err:
    return STATUS_ERROR;
}