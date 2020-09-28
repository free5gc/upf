#ifndef __KERNEL_GTP5G_CONTEXT_H__
#define __KERNEL_GTP5G_CONTEXT_H__

/*
 * These functions shall be customized by kinds of device.
 * You can create a directory and put all customized function
 * in there, like "device.c" under "updk/src/kernel/".
 *
 * This file would be included when the UP part runs with
 * Linux kernel module "gtp5g", otherwise it will not be used.
 */

#include "utlt_debug.h"
#include "utlt_lib.h"
#include "utlt_list.h"
#include "utlt_network.h"
#include "utlt_thread.h"

#include "updk/env.h"

#define UNIX_SOCK_BUFFERING_PATH "/tmp/free5gc_unix_sock"

/**
 * Gtp5gDevice - Structure for store info after gtp5g device created
 * 
 * @ifname: Name of gtp5g device
 * @sock: Sock after gtp5g device created
 * @port: A pointer link to associated VirtualPort
 * @PacketInL3: Function pointer to handle L3 packet in getting from VirtualDevice
 * @PacketInGTPU:  Function pointer to handle GTP-U packet in getting from VirtualDevice
 * @GetPDRByID: Function pointer to get PDR by ID getting from VirtualDevice
 * @GetFARByID: Function pointer to get FAR by ID getting from VirtualDevice
 * @PacketRecvThread: Thread for receiving all packet from kernel
 * @epfd: Epoll fd created in @PacketRecvThread 
 * @unixPath: Absolute path for named pipe for buffering
 * @unixSock: Sock for buffering
 */
typedef struct { // TODO: Need to change name to context and split these member into multi-struct
    char ifname[MAX_IFNAME_STRLEN];
    
    // Do NOT support multiple address in kernel module gtp5g
    Sock *sock;
    VirtualPort *port;

    // Method from EnvParams
    L3PacketInHandlerCB PacketInL3;
    GTPUPacketInHandlerCB PacketInGTPU;
    GetRule16CB GetPDRByID;
    GetRule32CB GetFARByID;

    ThreadID PacketRecvThread;
    int epfd;

    // Buffering
    char unixPath[MAX_FILE_PATH_STRLEN];
    Sock *unixSock;
    
} Gtp5gDevice;

/**
 * Gtp5gSelf - Get the real Gtp5gDevice pointer
 * 
 * @return: real Gtp5gDevice pointer in gtp5g_context.c
 */
Gtp5gDevice *Gtp5gSelf();

/**
 * Gtp5gDeviceInit - Initialize the Gtp5gDevice
 * 
 * @dev: VirtualDevice pointer for setting Gtp5gDevice
 * @port: VirtualPort pointer for setting Gtp5gDevice
 * @port: STATUS_OK or STATUS_ERROR if one of initialization part is failed
 */
Status Gtp5gDeviceInit(VirtualDevice *dev, VirtualPort *port);

/**
 * Gtp5gDeviceTerm - Terminate the Gtp5gDevice
 * 
 * @return: STATUS_OK or STATUS_ERROR if one of termination part is failed
 */
Status Gtp5gDeviceTerm();

#endif /* __KERNEL_GTP5G_CONTEXT_H__ */