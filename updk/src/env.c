#include "updk/env.h"

#include <stdlib.h>

#include "linux/list.h"

VirtualPort *AllocVirtualPort() {
    VirtualPort *rt = calloc(1, sizeof(VirtualPort));
    if (rt)
        INIT_LIST_HEAD(&rt->node);

    return rt;
}

void FreeVirtualPort(VirtualPort *port) {
    if (!port)
        return;

    list_del(&port->node);
    free(port);
}

VirtualDevice *AllocVirtualDevice() {
    VirtualDevice *rt = calloc(1, sizeof(VirtualDevice));
    if (rt)
        INIT_LIST_HEAD(&rt->virtualPortList);

    return rt;
}

void FreeVirtualDevice(VirtualDevice *dev) {
    if (!dev)
        return;

    VirtualPort *port;
    while ((port = (VirtualPort *) dev->virtualPortList.next) &&
        (struct list_head *) port != &dev->virtualPortList)
        FreeVirtualPort(port);

    free(dev);
}

void VirtualDeviceAddPort(VirtualDevice *dev, VirtualPort *port) {
    list_add_tail((struct list_head *) port, (struct list_head *) &dev->virtualPortList);
}

DNN *AllocDNN() {
    DNN *rt = calloc(1, sizeof(DNN));
    if (rt)
        INIT_LIST_HEAD(&rt->node);

    return rt;
}

void FreeDNN(DNN *dnn) {
    if (!dnn)
        return;

    list_del(&dnn->node);
    free(dnn);
}

EnvParams *AllocEnvParams() {
    EnvParams *rt = calloc(1, sizeof(EnvParams));
    if (rt) {
        if (!(rt->virtualDevice = AllocVirtualDevice())) {
            free(rt);
            return NULL;
        }

        INIT_LIST_HEAD(&rt->dnnList);
    }

    return rt;
}

void FreeEnvParams(EnvParams *env) {
    if (!env)
        return;

    FreeVirtualDevice(env->virtualDevice);

    DNN *dnn;
    while ((dnn = (DNN *) env->dnnList.next) &&
        (struct list_head *) dnn != &env->dnnList)
        FreeDNN(dnn);

    free(env);
}

void EnvParamsAddDNN(EnvParams *env, DNN *dnn) {
    list_add_tail((struct list_head *) dnn, (struct list_head *) &env->dnnList);
}