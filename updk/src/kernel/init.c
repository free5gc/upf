#include "updk/init.h"

/*
 * These functions shall be customized by kinds of device.
 * You can create a directory and put all customized function
 * in there, like "device.c" under "updk/src/kernel/".
 */

#include <arpa/inet.h>

#include "utlt_debug.h"
#include "utlt_list.h"
#include "utlt_buff.h"
#include "knet_route.h"

#include "updk/env.h"
#include "gtp5g_context.h"

int Gtpv1EnvInit(EnvParams *env) {
    UTLT_Assert(env, return -1, "EnvParams is NULL");

    VirtualDevice *dev = env->virtualDevice;
    UTLT_Assert(dev, return -1, "VirtualDevice is NULL");

    char *ifname = dev->deviceID;
    UTLT_Debug("Get VirtualDevice Name: %s", ifname);

    VirtualPort *port;
    // Only get the first one, because not support multiple gtp5g interface
    VirtualDeviceForEachVirtualPort(port, dev) {
        UTLT_Debug("Get VirtualPort Info in VirtualDevice: %s", port->ipStr);
        UTLT_Assert(Gtp5gDeviceInit(dev, port) == STATUS_OK,
            return -1, "Gtp5gDeviceInit failed");
        break;
    }

    // Set Routing to gtp5g interface
    DNN *dnn;
    EnvParamsForEachDNN(dnn, env) {
        UTLT_Assert(KnetAddRoute(ifname, dnn->ipStr, dnn->subnetPrefix, NULL, 0) == STATUS_OK,
            return -1,
            "Add routing rule to device %s failed: %s/%u", ifname, dnn->ipStr, dnn->subnetPrefix);
    }

    // Get routes from main IPv4 routing table and print
    ListHead *routeEntries = KnetGetRoutes(AF_INET, RT_TABLE_MAIN);
    UTLT_Assert(routeEntries, return -1, "");

    UTLT_Info("DNN routes added, main routing table:");
    KnetPrintRoutes(routeEntries);
    KnetRtListFree(routeEntries);

    UTLT_Free(routeEntries);

    return Gtp5gSelf()->sock->fd;
}

int Gtpv1EnvTerm(EnvParams *env) {
    UTLT_Assert(env, return -1, "EnvParams is NULL");

    VirtualDevice *dev = env->virtualDevice;
    UTLT_Assert(dev, return -1, "VirtualDevice is NULL");

    char *ifname = dev->deviceID;
    UTLT_Debug("Get VirtualDevice Name: %s", ifname);

    UTLT_Info("Removing DNN routes");

    DNN *dnn;
    Status status = STATUS_OK;
    EnvParamsForEachDNN(dnn, env) {
        UTLT_Assert(KnetDelRoute(ifname, dnn->ipStr, dnn->subnetPrefix, NULL, 0) == STATUS_OK,
            status = STATUS_ERROR,
            "Delete routing rule to device %s failed: %s/%u", ifname, dnn->ipStr, dnn->subnetPrefix);
    }

    status |= Gtp5gDeviceTerm();

    return (status == STATUS_OK ? 0 : -1);
}