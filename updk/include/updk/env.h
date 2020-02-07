#ifndef __UPDK_ENV_H__
#define __UPDK_ENV_H__

#include "linux/list.h"

#include <netinet/in.h>

#define MAX_LEN_OF_DNN_NAME 0x40
#define MAX_LEN_OF_IFNAME 40

/**
 * DNN - Structure for DNN information
 * 
 * @node: node for linked-list
 * @name: name of DNN
 * @ipStr: IP in string format
 * @natifname: name of device which will do nat for packet
 * @subnetPrefix: netmask for ipStr
 */
typedef struct {
    struct list_head node;
    char name[MAX_LEN_OF_DNN_NAME + 1];
    char ipStr[INET6_ADDRSTRLEN];
    char natifname[MAX_LEN_OF_IFNAME + 1];
    uint8_t subnetPrefix;
} DNN;

/**
 * L3PacketInHandlerCB - Callback function when underlayer UP interface cannot handle the UP packet
 * 
 * @pkt: L3 packet pointer receiving from UP interface
 * @pktlen: Total length of @pkt
 * @bufPDR: An allocated space to get the matched UPDK_PDR
 * @return: 1 if packet is passed by UPF, 0 if packet is matched any rule or -1 if packet is mismatched
 */
typedef int (*L3PacketInHandlerCB)(uint8_t *pkt, uint16_t pktlen, void *bufPDR);

/**
 * GTPUPacketInHandlerCB - Callback function when underlayer UP interface cannot handle the UP packet
 * 
 * @pkt: GTP-U packet pointer receiving from UP interface
 * @pktlen: Total length of @pkt
 * @remoteIP: Sender IPv4 address with network type
 * @_remotePort: Sender port with network type 
 * @bufPDR: An allocated space to get the matched UPDK_PDR
 * @return: 1 if packet is passed by UPF, 0 if packet is matched any rule or -1 if packet is mismatched
 */
typedef int (*GTPUPacketInHandlerCB)(uint8_t *pkt, uint16_t pktlen, uint32_t remoteIP, uint16_t _remotePort, void *bufPDR);

/**
 * GetRule16CB - Callback function for getting rule which ID is uint16_t
 * 
 * @id: Rule ID which is uint16_t
 * @ruleBuf: An allocated space to get the designated rule
 * @return: 0 or -1 if UPF cannot find the designated rule
 */
typedef int (*GetRule16CB)(uint16_t id, void *ruleBuf);

/**
 * GetRule32CB - Callback function for getting rule which ID is uint32_t
 * 
 * @id: Rule ID which is uint32_t
 * @ruleBuf: An allocated space to get the designated rule
 * @return: 0 or -1 if UPF cannot find the designated rule
 */
typedef int (*GetRule32CB)(uint32_t id, void *ruleBuf);

/**
 * VirtualPort - Structure for Interface in Device, like switch port
 * 
 * @node: node for linked-list
 * @pciId: device id or NULL if it is not DPDK
 * @ipStr: IP in string format
 */
typedef struct {
    struct list_head node;
    char pciId[MAX_LEN_OF_IFNAME]; // for dpdk
    char ipStr[INET6_ADDRSTRLEN];
} VirtualPort;

/** 
 * VirtualDevice - Structure for Device which has many ports
 *
 * @deviceID: tuntap name or NULL if it is DPDK
 * @virtualPortList: the first node for linked-list
 * @EventCB.packetIn: function pointer when packet in or NULL if do not handle this event
 * @EventCB.getPDR: function pointer uesd to get PDR by ID
 * @EventCB.getFAR: function pointer uesd to get FAR by ID
 * @EventCB.getQER: function pointer uesd to get QER by ID, but not support yet now
 * @EventCB.getBAR: function pointer uesd to get BAR by ID, but not support yet now
 * @EventCB.getURR: function pointer uesd to get URR by ID, but not support yet now
 */
typedef struct {
    char deviceID[MAX_LEN_OF_IFNAME];

    struct list_head virtualPortList;

    struct {
        // TODO: param and return value of the function pointer have not finished yet
        L3PacketInHandlerCB PacketInL3;
        GTPUPacketInHandlerCB PacketInGTPU;
        GetRule16CB getPDR;
        GetRule32CB getFAR;
        GetRule32CB getQER;
        GetRule32CB getBAR;
        GetRule32CB getURR;
    } eventCB;
} VirtualDevice;

/**
 * EnvParams - Structure for Create Device, Routing... information from UPF config
 * 
 * @virtualDevice: VirtualDevice pointer for creating device
 * @dnnList: struct list_head to store all DNN information
 */
typedef struct {
    VirtualDevice *virtualDevice;
    struct list_head dnnList;
} EnvParams;

/**
 * AllocDNN - Alloc a memory for DNN and init it
 * 
 * @return: pointer or NULL if alloc fail
 */
DNN *AllocDNN();

/**
 * FreeDNN - Free the DNN and remove this node from linked-list
 * 
 * @port: DNN pointer which want to free
 */
void FreeDNN(DNN *dnn);

/**
 * AllocVirtualPort - Alloc a memory for VirtualPort and init it
 * 
 * @return: pointer or NULL if alloc fail
 */
VirtualPort *AllocVirtualPort();

/**
 * FreeVirtualPort - Free the VirtualPort and remove this node from linked-list
 * 
 * @port: VirtualPort pointer which want to free
 */
void FreeVirtualPort(VirtualPort *port);

/**
 * AllocVirtualPort - Alloc a memory for VirtualDevice and init it
 * 
 * @return: pointer or NULL if alloc fail
 */
VirtualDevice *AllocVirtualDevice();

/**
 * FreeVirtualDevice - Free the VirtualDevice and clear this virtualPortList
 * 
 * @dev: VirtualDevice pointer which want to free
 */
void FreeVirtualDevice(VirtualDevice *dev);

/**
 * VirtualDeviceAddPort - Append a VirtualPort to VirtualDevice
 * 
 * @dev: VirtualDevice pointer whose virtualPortList will be appended
 * @port: VirtualPort pointer which want to append
 */
void VirtualDeviceAddPort(VirtualDevice *dev, VirtualPort *port);

/**
 * VirtualDeviceForEachVirtualPort - iterate VirtualPort over a VirtualDevice
 * @port:	the &VirtualPort to use as a loop cursor
 * @dev:	the VirtualDevice
 */
#define VirtualDeviceForEachVirtualPort(port, dev) \
    for (port = (VirtualPort *) (dev)->virtualPortList.next; \
        &(port)->node != &(dev)->virtualPortList; port = (VirtualPort *) (port)->node.next)

/**
 * AllocEnvParams - Alloc a memory for EnvParams and init it
 * 
 * @return: pointer or NULL if alloc fail
 */
EnvParams *AllocEnvParams();

/**
 * FreeEnvParams - Free the EnvParams and clear all list nodes in this struct
 * 
 * @dev: EnvParams pointer which want to free
 */
void FreeEnvParams(EnvParams *env);

/**
 * EnvParamsAddDNN - Append a DNN to EnvParams
 * 
 * @env: EnvParams pointer whose dnnList will be appended
 * @dnn: DNN pointer which want to append
 */
void EnvParamsAddDNN(EnvParams *env, DNN *dnn);

/**
 * EnvParamsForEachDNN - iterate DNN over a EnvParams
 * @dnn:	the &DNN to use as a loop cursor
 * @env:	the EnvParams
 */
#define EnvParamsForEachDNN(dnn, env) \
    for (dnn = (DNN *) (env)->dnnList.next; \
        &(dnn)->node != &(env)->dnnList; dnn = (DNN *) (dnn)->node.next)

#endif /* __UPDK_ENV_H__ */