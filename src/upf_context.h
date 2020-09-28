#ifndef __UPF_CONTEXT_H__
#define __UPF_CONTEXT_H__

#include <stdint.h>
#include <netinet/in.h>
#include <net/if.h>
#include <pthread.h>

#include "utlt_list.h"
#include "utlt_buff.h"
#include "utlt_event.h"
#include "utlt_thread.h"
#include "utlt_network.h"
#include "utlt_hash.h"
#include "utlt_3gppTypes.h"
#include "utlt_timer.h"

#include "pfcp_node.h"
#include "pfcp_message.h"

#include "up/up_match.h"

#include "updk/env.h"
#include "updk/init.h"
#include "updk/rule_pdr.h"
#include "updk/rule_far.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _UpfUeIp      UpfUeIp;
typedef struct _UpfDev       UpfDev;
typedef struct gtp5g_pdr     UpfPdr;
typedef struct gtp5g_far     UpfFar;
typedef struct _UpfBufPacket UpfBufPacket;

// Rule structure dependent on UPDK
typedef UPDK_PDR UpfPDR;
typedef UPDK_FAR UpfFAR;
/*
typedef UPDK_QER UpfQER;
typedef UPDK_BAR UpfBAR;
typedef UPDK_URR UpfURR;
*/

typedef enum _UpfEvent {

    UPF_EVENT_N4_MESSAGE,
    UPF_EVENT_SESSION_REPORT,
    UPF_EVENT_N4_T3_RESPONSE,
    UPF_EVENT_N4_T3_HOLDING,

    UPF_EVENT_TOP,

} UpfEvent;

typedef struct {
    uint8_t         role;                // UpfRole
    const char      *gtpDevNamePrefix;   // Default : "upfgtp"

    ListHead        gtpInterfaceList;    // name of interface (char*)
    // Add context related to GTP-U here
    uint16_t        gtpv1Port;           // Default : GTP_V1_PORT
    EnvParams       *envParams;          // EnvParams parsing from UPF Config
    Sock            upSock;              // User Plane Socket builds from Gtpv1EnvInit()

    // Add context related to PFCP here
    uint16_t        pfcpPort;            // Default : PFCP_PORT
    ListHead        pfcpIPList;          // PFCP IPv4 Server List (SockNode)
    ListHead        pfcpIPv6List;        // PFCP IPv6 Server List (SockNode)
    Sock            *pfcpSock;           // IPv4 Socket
    Sock            *pfcpSock6;          // IPv6 Socket
    SockAddr        *pfcpAddr;           // IPv4 Address
    SockAddr        *pfcpAddr6;          // IPv6 Address

    /* Use Array or Hash for better performance
     * Because max size of the list is 65536 due to the max of PDR ID
     * We can use array for O(1) search instead of O(N) search in list
     * Trade off of speed and memory size
     */
    //ListNode        bufPacketList;       // save pdrId and buffer here

    // DNS
#define MAX_NUM_OF_DNS          2
    const char      *dns[MAX_NUM_OF_DNS];
    const char      *dns6[MAX_NUM_OF_DNS];

    // Add other context here
    ListHead        ranS1uList;         // RAN List connected to UPF
    ListHead        upfN4List;          // UPF PFCP Node List
    ListHead        dnnList;

    // Different list of policy rule
    // TODO: if implementing QER in kernel, remove these list
    ListHead        qerList;
    ListHead        urrList;

    uint32_t        recoveryTime;       // UTC time
    TimerList       timerServiceList;

    // Add some self library structure here
    int             epfd;               // Epoll fd
    EvtQId          eventQ;             // Event queue communicate between UP and CP
    ThreadID        pktRecvThread;      // Receive packet thread

    // Session : hash(IMSI+DNN)
    Hash            *sessionHash;
    // Save buffer packet here
    Hash            *bufPacketHash;
    // Use spin lock to protect data write
    pthread_spinlock_t buffLock;
    // TODO: read from config
    // no reason, just want to bigger than /tmp/free5gc_unix_sock
#define MAX_SOCK_PATH_LEN 64
    char            buffSockPath[MAX_SOCK_PATH_LEN];
    // Buffering socket for recv packet from kernel
    Sock            *buffSock;


    // Config file
    const char      *configFilePath;
} UpfContext;

typedef struct _UpfUeIp {
    union {
        struct in_addr addr4;
        struct in6_addr addr6;
    };
} UpfUeIp;

typedef struct _UpfSession {
    int             index;

    uint64_t        upfSeid;
    uint64_t        smfSeid;

    /* DNN Config */
    Pdn             pdn;
    UpfUeIp         ueIpv4;
    UpfUeIp         ueIpv6;

    /* User location */
    Tai             tai;
    //ECgi          eCgi; // For LTE E-UTRA Cell ID
    //NCgi          nCgi; // For 5GC NR Cell ID

    /* Hashed key: hash(IMSI+DNN) */
    uint8_t         hashKey[MAX_IMSI_LEN+MAX_DNN_LEN];
    int             hashKeylen;

    /* GTP, PFCP context */
    //SockNode        *gtpNode;
    PfcpNode        *pfcpNode;
    ListHead        pdrIdList;

    ListHead        pdrList;
    ListHead        farList;
    ListHead        qerList;
    ListHead        barList;
    ListHead        urrList;

} UpfSession;

// Used for buffering, Index type for each PDR
typedef struct _UpfBufPacket {
    //ListHead        node;
    int             index;

    // If sessionPtr == NULL, this PDR don't exist
    // TS 29.244 5.2.1 shows that PDR won't cross session
    const UpfSession *sessionPtr;
    uint16_t        pdrId;
    Bufblk          *packetBuffer;
} UpfBufPakcet;

typedef struct {
    ListHead node;
    int index;

    UpfPDR pdr;

    MatchRuleNode *matchRule;
} UpfPDRNode;

typedef struct {
    ListHead node;
    int index;

    UpfFAR far;
} UpfFARNode;

typedef struct {
    ListHead node;
    int index;

    // UpfQER qer;
} UpfQERNode;

typedef struct {
    ListHead node;
    int index;

    // UpfBAR bar;
} UpfBARNode;

typedef struct {
    ListHead node;
    int index;

    // UpfURR urr;
} UpfURRNode;

UpfContext *Self();
Status UpfContextInit();
Status UpfContextTerminate();

// Rules
UpfPDRNode *UpfPDRNodeAlloc();
UpfFARNode *UpfFARNodeAlloc();
UpfQERNode *UpfQERNodeAlloc();
UpfBARNode *UpfBARNodeAlloc();
UpfURRNode *UpfURRNodeAlloc();

void UpfPDRNodeFree(UpfPDRNode *node);
void UpfFARNodeFree(UpfFARNode *node);
void UpfQERNodeFree(UpfQERNode *node);
void UpfBARNodeFree(UpfBARNode *node);
void UpfURRNodeFree(UpfURRNode *node);

int UpfPDRFindByID(uint16_t id, void *ruleBuf);
int UpfFARFindByID(uint32_t id, void *ruleBuf);
/*
int UpfQERFindByID(uint32_t id, void *ruleBuf);
int UpfBARFindByID(uint32_t id, void *ruleBuf);
int UpfURRFindByID(uint32_t id, void *ruleBuf);
*/

Status HowToHandleThisPacket(uint32_t farID, uint8_t *action);

void UpfPDRDump();
void UpfFARDump();
/*
void UpfQERDump();
void UpfBARDump();
void UpfURRDump();
*/

UpfPDRNode *UpfPDRRegisterToSession(UpfSession *sess, UpfPDR *rule);
UpfFARNode *UpfFARRegisterToSession(UpfSession *sess, UpfFAR *rule);
/*
UpfQERNode *UpfQERRegisterToSession(UpfSession *sess, UpfQER *rule);
UpfBARNode *UpfBARRegisterToSession(UpfSession *sess, UpfBAR *rule);
UpfURRNode *UpfURRRegisterToSession(UpfSession *sess, UpfURR *rule);
*/

Status UpfPDRDeregisterToSessionByID(UpfSession *sess, uint16_t id);
Status UpfFARDeregisterToSessionByID(UpfSession *sess, uint32_t id);
/*
Status UpfQERDeregisterToSessionByID(UpfSession *sess, uint32_t id);
Status UpfBARDeregisterToSessionByID(UpfSession *sess, uint32_t id);
Status UpfURRDeregisterToSessionByID(UpfSession *sess, uint32_t id);
*/

// BufPacket
HashIndex *UpfBufPacketFirst();
HashIndex *UpfBufPacketNext(HashIndex *hashIdx);
UpfBufPacket *UpfBufPacketThis(HashIndex *hashIdx);
UpfBufPacket *UpfBufPacketFindByPdrId(uint16_t pdrId);
UpfBufPacket *UpfBufPacketAdd(const UpfSession * const session,
                              const uint16_t pdrId);
Status UpfBufPacketRemove(UpfBufPacket *bufPacket);
Status UpfBufPacketRemoveAll();

// Session
HashIndex *UpfSessionFirst();
HashIndex *UpfSessionNext(HashIndex *hashIdx);
UpfSession *UpfSessionThis(HashIndex *hashIdx);
void SessionHashKeygen(uint8_t *out, int *outLen, uint8_t *imsi, int imsiLen, uint8_t *dnn);
UpfSession *UpfSessionAdd(PfcpUeIpAddr *ueIp, uint8_t *dnn, uint8_t pdnType);
Status UpfSessionRemove(UpfSession *session);
Status UpfSessionRemoveAll();
UpfSession *UpfSessionFind(uint32_t idx);
UpfSession *UpfSessionFindBySeid(uint64_t seid);
UpfSession *UpfSessionAddByMessage(PfcpMessage *message);
UpfSession *UpfSessionFindByPdrTeid(uint32_t teid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPF_CONTEXT_H__ */
