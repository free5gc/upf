#define TRACE_MODULE _upf_context

#include "upf_context.h"

#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <net/if.h>

#include "utlt_debug.h"
#include "utlt_pool.h"
#include "utlt_index.h"
#include "utlt_hash.h"
#include "utlt_network.h"
#include "utlt_netheader.h"

#include "pfcp_message.h"
#include "pfcp_types.h"
#include "pfcp_xact.h"

#include "up/up_match.h"

#include "updk/env.h"
#include "updk/init.h"
#include "updk/rule.h"
#include "updk/rule_pdr.h"
#include "updk/rule_far.h"

#define MAX_NUM_OF_SUBNET       16

IndexDeclare(upfSessionPool, UpfSession, MAX_POOL_OF_SESS);

#define MAX_NUM_OF_UPF_PDR_NODE (MAX_POOL_OF_BEARER * 2)
#define MAX_NUM_OF_UPF_FAR_NODE MAX_NUM_OF_UPF_PDR_NODE
#define MAX_NUM_OF_UPF_QER_NODE (MAX_POOL_OF_SESS * 2)
#define MAX_NUM_OF_UPF_BAR_NODE (MAX_POOL_OF_UE)
#define MAX_NUM_OF_UPF_URR_NODE (MAX_POOL_OF_UE)

IndexDeclare(upfPDRNodePool, UpfPDRNode, MAX_NUM_OF_UPF_PDR_NODE);
IndexDeclare(upfFARNodePool, UpfFARNode, MAX_NUM_OF_UPF_FAR_NODE);
IndexDeclare(upfQERNodePool, UpfQERNode, MAX_NUM_OF_UPF_QER_NODE);
IndexDeclare(upfBARNodePool, UpfBARNode, MAX_NUM_OF_UPF_BAR_NODE);
IndexDeclare(upfURRNodePool, UpfURRNode, MAX_NUM_OF_UPF_URR_NODE);

/**
 * PDRHash - Store PDRs with Hash struct
 */
Hash *PDRHash;
pthread_mutex_t PDRHashLock;
Hash *FARHash;
pthread_mutex_t FARHashLock;
Hash *QERHash;
pthread_mutex_t QERHashLock;
Hash *BARHash;
pthread_mutex_t BARHashLock;
Hash *URRHash;
pthread_mutex_t URRHashLock;

#define Rule_Thread_Safe(__ruleType, expr) \
    pthread_mutex_lock(&__ruleType##HashLock); \
    expr; \
    pthread_mutex_unlock(&__ruleType##HashLock)

#define PDR_Thread_Safe(expr) Rule_Thread_Safe(PDR, expr)
#define FAR_Thread_Safe(expr) Rule_Thread_Safe(FAR, expr)
#define QER_Thread_Safe(expr) Rule_Thread_Safe(QER, expr)
#define BAR_Thread_Safe(expr) Rule_Thread_Safe(BAR, expr)
#define URR_Thread_Safe(expr) Rule_Thread_Safe(URR, expr)

static UpfContext self;
static _Bool upfContextInitialized = 0;

UpfContext *Self() {
    return &self;
}

#define RuleInit(__ruleType) do { \
    IndexInit(&upf##__ruleType##NodePool, MAX_NUM_OF_UPF_##__ruleType##_NODE); \
    __ruleType##Hash = HashMake(); \
    pthread_mutex_init(&__ruleType##HashLock, 0); \
} while (0)

Status UpfContextInit() {
    UTLT_Assert(upfContextInitialized == 0, return STATUS_ERROR,
                "UPF context has been initialized!");

    memset(&self, 0, sizeof(UpfContext));

    // TODO : Add GTPv1 init here
    self.envParams = AllocEnvParams();
    UTLT_Assert(self.envParams, return STATUS_ERROR,
        "EnvParams alloc failed");
    self.envParams->virtualDevice->eventCB.PacketInL3 = PacketInWithL3;
    self.envParams->virtualDevice->eventCB.PacketInGTPU = PacketInWithGTPU;
    self.envParams->virtualDevice->eventCB.getPDR = UpfPDRFindByID;
    self.envParams->virtualDevice->eventCB.getFAR = UpfFARFindByID;

    self.upSock.fd = -1;
    SockSetEpollMode(&self.upSock, EPOLLIN);
    

    // TODO : Add PFCP init here
    ListHeadInit(&self.pfcpIPList);
    // ListHeadInit(&self.pfcpIPv6List);

    // TODO : Add by self if context has been updated
    // TODO: check if gtp node need to init?
    // ListHeadInit(&self.pfcpIPList);
    // ListHeadInit(&self.pfcpIPv6List);

    ListHeadInit(&self.ranS1uList);
    ListHeadInit(&self.upfN4List);
    ListHeadInit(&self.dnnList);
    ListHeadInit(&self.qerList);
    ListHeadInit(&self.urrList);

    self.recoveryTime = htonl(time((time_t *)NULL));

    // Set Default Value
    self.gtpDevNamePrefix = "upfgtp";
    // defined in utlt_3gpptypes instead of GTP_V1_PORT defined in GTP_PATH;
    self.gtpv1Port = GTPV1_U_UDP_PORT;
    self.pfcpPort = PFCP_UDP_PORT;
    strcpy(self.envParams->virtualDevice->deviceID, self.gtpDevNamePrefix);

    // Init Resource
    IndexInit(&upfSessionPool, MAX_POOL_OF_SESS);
    RuleInit(PDR);
    RuleInit(FAR);
    RuleInit(QER);
    RuleInit(BAR);
    RuleInit(URR);
    MatchInit();

    PfcpNodeInit(); // init pfcp node for upfN4List (it will used pfcp node)
    TimerListInit(&self.timerServiceList);

    // TODO: Read from config
    strncpy(self.buffSockPath, "/tmp/free5gc_unix_sock", MAX_SOCK_PATH_LEN);
    self.sessionHash = HashMake();
    self.bufPacketHash = HashMake();
    // spin lock protect write data instead of mutex protect code block
    int ret = pthread_spin_init(&self.buffLock, PTHREAD_PROCESS_PRIVATE);
    UTLT_Assert(ret == 0, , "buffLock cannot create: %s", strerror(ret));

    upfContextInitialized = 1;

    return STATUS_OK;
}

#define RuleTerminate(__ruleType) do { \
    pthread_mutex_destroy(&__ruleType##HashLock); \
    IndexTerminate(&upf##__ruleType##NodePool); \
    HashDestroy(__ruleType##Hash); \
} while (0)

// TODO : Need to Remove List Members iterativelyatively
Status UpfContextTerminate() {
    UTLT_Assert(upfContextInitialized == 1, return STATUS_ERROR,
                "UPF context has been terminated!");

    Status status = STATUS_OK;

    int ret = pthread_spin_destroy(&self.buffLock);
    UTLT_Assert(ret == 0, , "buffLock cannot destroy: %s", strerror(ret));
    UTLT_Assert(self.bufPacketHash, , "Buffer Hash Table missing?!");
    HashDestroy(self.bufPacketHash);

    UTLT_Assert(self.sessionHash, , "Session Hash Table missing?!");
    HashDestroy(self.sessionHash);

    // Terminate resource
    MatchTerm();
    IndexTerminate(&upfSessionPool);
    RuleTerminate(PDR);
    RuleTerminate(FAR);
    RuleTerminate(QER);
    RuleTerminate(BAR);
    RuleTerminate(URR);

    PfcpRemoveAllNodes(&self.upfN4List);
    PfcpNodeTerminate();

    // TODO: remove gtpv1TunnelList, ranS1uList, upfN4LIst, dnnList,
    // pdrList, farList, qerList, urrLIist
    SockNodeListFree(&self.pfcpIPList);
    // SockNodeListFree(&self.pfcpIPv6List);
    FreeVirtualDevice(self.envParams->virtualDevice);

    UpfBufPacketRemoveAll();

    upfContextInitialized = 0;

    return status;
}

#define RuleNodeAlloc(__ruleType) \
Upf##__ruleType##Node *Upf##__ruleType##NodeAlloc() { \
    Upf##__ruleType##Node *node = NULL; \
    IndexAlloc(&upf##__ruleType##NodePool, node); \
    return node; \
}

RuleNodeAlloc(PDR);
RuleNodeAlloc(FAR);
RuleNodeAlloc(QER);
RuleNodeAlloc(BAR);
RuleNodeAlloc(URR);

#define RuleNodeFree(__ruleType) \
void Upf##__ruleType##NodeFree(Upf##__ruleType##Node *node) { \
    if (node) IndexFree(&upf##__ruleType##NodePool, node); \
}

RuleNodeFree(PDR);
RuleNodeFree(FAR);
RuleNodeFree(QER);
RuleNodeFree(BAR);
RuleNodeFree(URR);

#define UPF_RULE_ID(__ruleName) __ruleName ## Id

// Do the thread safe to upper layer function
#define RuleNodeHashSet(__ruleType, __id, __ptr) HashSet(__ruleType##Hash, &(__id), sizeof(__id), (__ptr))
#define RuleNodeHashGet(__ruleType, __id) HashGet(__ruleType##Hash, &(__id), sizeof(__id))

#define RuleFindByID(__ruleType, __ruleName, __keyType) \
int Upf##__ruleType##FindByID(__keyType id, void *ruleBuf) { \
    __ruleType##_Thread_Safe( \
        Upf##__ruleType##Node *node = RuleNodeHashGet(__ruleType, id); \
    ); \
    if (!node) return -1; \
    memcpy(ruleBuf, &node->__ruleName, sizeof(Upf##__ruleType)); \
    return 0; \
}

RuleFindByID(PDR, pdr, uint16_t);
RuleFindByID(FAR, far, uint32_t);
/* TODO: Not support yet
RuleFindByID(QER, qer, uint32_t);
RuleFindByID(BAR, bar, uint32_t);
RuleFindByID(URR, urr, uint32_t);
*/

Status HowToHandleThisPacket(uint32_t farID, uint8_t *action) {
    Status status = STATUS_OK;
    FAR_Thread_Safe(
        UpfFARNode *node = RuleNodeHashGet(FAR, farID);
        if (!node)
            status = STATUS_ERROR;
        else
            *action = node->far.applyAction;
    );
    return status;
}

#define RuleDump(__ruleType, __ruleName, __keyType) \
void Upf##__ruleType##Dump() { \
    __ruleType##_Thread_Safe( \
        for (HashIndex *hi = HashFirst(__ruleType ## Hash); hi; hi = HashNext(hi)) { \
            const __keyType *key = HashThisKey(hi); \
            UTLT_Info(#__ruleType" ID[%u] does exist", *key); \
        } \
     ); \
}

RuleDump(PDR, pdr, uint16_t);
RuleDump(FAR, far, uint32_t);
/*
RuleDump(QER, qer, uint32_t);
RuleDump(BAR, bar, uint32_t);
RuleDump(URR, urr, uint32_t);
*/

UpfPDRNode *UpfPDRRegisterToSession(UpfSession *sess, UpfPDR *rule) {
    UTLT_Assert(sess && rule, return NULL, "Session or UpfPDR should not be NULL");
    UTLT_Assert(rule->flags.pdrId, return NULL, "PDR ID should be set");

    MatchRuleNode *newMatchRule = MatchRuleNodeAlloc();
    UTLT_Assert(newMatchRule, return NULL, "MatchRuleNodeAlloc failed");

    UTLT_Assert(MatchRuleCompile(rule, newMatchRule) == STATUS_OK, goto FREEMATCHRULENODE,
        "MatchRuleCompile failed");

    PDR_Thread_Safe(
        UpfPDRNode *ruleNode = RuleNodeHashGet(PDR, rule->pdrId);
        if (!ruleNode) {
            ruleNode = UpfPDRNodeAlloc();
            UTLT_Assert(ruleNode, goto FREEMATCHRULENODE, "UpfPDENodeAlloc failed");

            ListHeadInit(&ruleNode->node);
            ListInsert(ruleNode, &sess->pdrList);
        } else {
            MatchRuleNodeFree(ruleNode->matchRule);
        }
        memcpy(&ruleNode->pdr, rule, sizeof(UpfPDR));
        ruleNode->matchRule = newMatchRule;
        newMatchRule->pdr = &ruleNode->pdr;
        RuleNodeHashSet(PDR, ruleNode->pdr.pdrId, ruleNode);
    );

    MatchRuleRegister(newMatchRule);

    return ruleNode;

FREEMATCHRULENODE:
    MatchRuleNodeFree(newMatchRule);

    return NULL;
}

#define RuleRegisterToSession(__ruleType, __ruleName) \
Upf##__ruleType##Node *Upf##__ruleType##RegisterToSession(UpfSession *sess, Upf##__ruleType *rule) { \
    UTLT_Assert(sess && rule, return NULL, "Session or Upf"#__ruleType" should not be NULL"); \
    UTLT_Assert(rule->flags.UPF_RULE_ID(__ruleName), return NULL, #__ruleType" ID should be set"); \
    __ruleType##_Thread_Safe( \
        Upf##__ruleType##Node *ruleNode = RuleNodeHashGet(__ruleType, rule->UPF_RULE_ID(__ruleName)); \
        if (!ruleNode) { \
            ruleNode = Upf##__ruleType##NodeAlloc(); \
            UTLT_Assert(ruleNode, return NULL, "Upf"#__ruleType"NodeAlloc failed"); \
            ListHeadInit(&ruleNode->node); \
            ListInsert(ruleNode, &sess->__ruleName##List); \
        } \
        memcpy(&ruleNode->__ruleName, rule, sizeof(Upf##__ruleType)); \
        RuleNodeHashSet(__ruleType, ruleNode->__ruleName.UPF_RULE_ID(__ruleName), ruleNode); \
    ); \
    return ruleNode; \
}

RuleRegisterToSession(FAR, far);
/* TODO: Not support yet
RuleRegisterToSession(QER, qer);
RuleRegisterToSession(BAR, bar);
RuleRegisterToSession(URR, urr);
*/

// Do the thread safe to upper layer function
#define RuleDeletionFromSession(__ruleType, __ruleName, __sessPtr, __nodePtr) do { \
    RuleNodeHashSet(__ruleType, (__nodePtr)->__ruleName.UPF_RULE_ID(__ruleName), NULL); \
    ListRemove(__nodePtr); \
} while (0)

Status UpfPDRDeregisterToSessionByID(UpfSession *sess, uint16_t id) {
    UTLT_Assert(sess, return STATUS_ERROR, "Session should not be NULL");

    PDR_Thread_Safe(
        UpfPDRNode *ruleNode = RuleNodeHashGet(PDR, id);
        UTLT_Assert(ruleNode, return STATUS_ERROR, "PDR ID[%u] does NOT exist", id);

        if (ruleNode->matchRule) {
            MatchRuleDeregister(ruleNode->matchRule);
            MatchRuleNodeFree(ruleNode->matchRule);
        }

        RuleDeletionFromSession(PDR, pdr, sess, ruleNode);
    );

    UpfPDRNodeFree(ruleNode);

    return STATUS_OK;
}

#define RuleDeregisterToSessionByID(__ruleType, __ruleName, __keyType) \
Status Upf##__ruleType##DeregisterToSessionByID(UpfSession *sess, __keyType id) { \
    UTLT_Assert(sess, return STATUS_ERROR, "Session should not be NULL"); \
    __ruleType##_Thread_Safe( \
        Upf##__ruleType##Node *ruleNode = RuleNodeHashGet(__ruleType, id); \
        UTLT_Assert(ruleNode, return STATUS_ERROR, #__ruleType" ID[%u] does NOT exist", id); \
        RuleDeletionFromSession(__ruleType, __ruleName, sess, ruleNode); \
    ); \
    Upf##__ruleType##NodeFree(ruleNode); \
    return STATUS_OK; \
}

RuleDeregisterToSessionByID(FAR, far, uint32_t);
/*
RuleDeregisterToSessionByID(QER, qer, uint32_t);
RuleDeregisterToSessionByID(BAR, bar, uint32_t);
RuleDeregisterToSessionByID(URR, urr, uint32_t);
*/

#define UPF_RULE_LIST(__ruleName) __ruleName ## List

// Do the thread safe to upper layer function
#define RuleListDeletionAndFreeWithGTPv1Tunnel(__ruleType, __ruleName, __sessionPtr) do { \
    Upf##__ruleType##Node *ruleNode; \
    Upf##__ruleType##Node *nextNode; \
    ruleNode = ListFirst(&(__sessionPtr)->UPF_RULE_LIST(__ruleName)); \
    while (ruleNode != (Upf##__ruleType##Node *) &(__sessionPtr)->UPF_RULE_LIST(__ruleName)) { \
        nextNode = (Upf##__ruleType##Node *)ListNext(ruleNode); \
        UTLT_Assert(!Gtpv1TunnelRemove##__ruleType(&ruleNode->__ruleName), , \
            "Remove "#__ruleType"[%u] failed", ruleNode->__ruleName.UPF_RULE_ID(__ruleName)); \
        RuleDeletionFromSession(__ruleType, __ruleName, (__sessionPtr), ruleNode); \
        IndexFree(&upf##__ruleType##NodePool, ruleNode); \
        ruleNode = nextNode; \
    }  \
} while (0)

HashIndex *UpfBufPacketFirst() {
    UTLT_Assert(self.bufPacketHash, return NULL, "");
    return HashFirst(self.bufPacketHash);
}

HashIndex *UpfBufPacketNext(HashIndex *hashIdx) {
    UTLT_Assert(hashIdx, return NULL, "");
    return HashNext(hashIdx);
}

UpfBufPacket *UpfBufPacketThis(HashIndex *hashIdx) {
    UTLT_Assert(hashIdx, return NULL, "");
    return (UpfBufPacket *)HashThisVal(hashIdx);
}

UpfBufPacket *UpfBufPacketFindByPdrId(uint16_t pdrId) {
    return (UpfBufPacket*)HashGet(self.bufPacketHash,
                                  &pdrId, sizeof(uint16_t));
}

UpfBufPacket *UpfBufPacketAdd(const UpfSession * const session,
                              const uint16_t pdrId) {
    UTLT_Assert(session, return NULL, "No session");
    UTLT_Assert(pdrId, return NULL, "PDR ID cannot be 0");

    UpfBufPacket *newBufPacket = UTLT_Malloc(sizeof(UpfBufPacket));
    UTLT_Assert(newBufPacket, return NULL, "Allocate new slot error");
    newBufPacket->sessionPtr = session;
    newBufPacket->pdrId = pdrId;
    newBufPacket->packetBuffer = NULL;

    HashSet(self.bufPacketHash, &newBufPacket->pdrId,
            sizeof(uint16_t), newBufPacket);

    //ListAppend(&Self()->bufPacketList, newBufPacket);
    return newBufPacket;
}

Status UpfBufPacketRemove(UpfBufPacket *bufPacket) {
    UTLT_Assert(bufPacket, return STATUS_ERROR,
                "Input bufPacket error");
    Status status;

    bufPacket->sessionPtr = NULL;
    bufPacket->pdrId = 0;
    if (bufPacket->packetBuffer) {
        status = BufblkFree(bufPacket->packetBuffer);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                    "packet in bufPacket free error");
    }

    HashSet(self.bufPacketHash, &bufPacket->pdrId,
            sizeof(uint16_t), NULL);
    //ListRemove(&Self()->bufPacketList, bufPacket);
    status = UTLT_Free(bufPacket);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                "bufPacket free error");

    return STATUS_OK;
}

Status UpfBufPacketRemoveAll() {
    HashIndex *hashIdx = NULL;
    UpfBufPacket *bufPacket = NULL;

    for (hashIdx = UpfBufPacketFirst(); hashIdx;
         hashIdx = UpfBufPacketNext(hashIdx)) {
        bufPacket = UpfBufPacketThis(hashIdx);
        UpfBufPacketRemove(bufPacket);
    }

    return STATUS_OK;
}

HashIndex *UpfSessionFirst() {
    UTLT_Assert(self.sessionHash, return NULL, "");
    return HashFirst(self.sessionHash);
}

HashIndex *UpfSessionNext(HashIndex *hashIdx) {
    UTLT_Assert(hashIdx, return NULL, "");
    return HashNext(hashIdx);
}

UpfSession *UpfSessionThis(HashIndex *hashIdx) {
    UTLT_Assert(hashIdx, return NULL, "");
    return (UpfSession *)HashThisVal(hashIdx);
}

void SessionHashKeygen(uint8_t *out, int *outLen, uint8_t *imsi,
                       int imsiLen, uint8_t *dnn) {
    memcpy(out, imsi, imsiLen);
    strncpy((char *)(out + imsiLen), (char*)dnn, MAX_DNN_LEN + 1);
    *outLen = imsiLen + strlen((char *)(out + imsiLen));

    return;
}

UpfSession *UpfSessionAdd(PfcpUeIpAddr *ueIp, uint8_t *dnn,
                          uint8_t pdnType) {
    UpfSession *session = NULL;

    IndexAlloc(&upfSessionPool, session);
    UTLT_Assert(session, return NULL, "session alloc error");

    //session->gtpNode = NULL;

    if (self.pfcpAddr) {
        session->upfSeid =
          ((uint64_t)self.pfcpAddr->s4.sin_addr.s_addr << 32)
          | session->index;
    } else if (self.pfcpAddr6) {
        uint32_t *ptr =
          (uint32_t *)self.pfcpAddr6->s6.sin6_addr.s6_addr;
        session->upfSeid =
          (((uint64_t)(*ptr)) << 32) | session->index;
        // TODO: check if correct
    }
    session->upfSeid = htobe64(session->upfSeid);
    //UTLT_Info()
    session->upfSeid = 0; // TODO: check why

    /* IMSI DNN Hash */
    /* DNN */
    strncpy((char*)session->pdn.dnn, (char*)dnn, MAX_DNN_LEN + 1);

    ListHeadInit(&session->pdrIdList);
    ListHeadInit(&session->pdrList);
    ListHeadInit(&session->farList);
    ListHeadInit(&session->qerList);
    ListHeadInit(&session->barList);
    ListHeadInit(&session->urrList);

    session->pdn.paa.pdnType = pdnType;
    if (pdnType == PFCP_PDN_TYPE_IPV4) {
        session->ueIpv4.addr4 = ueIp->addr4;
        //session->pdn.paa.addr4 = ueIp->addr4;
    } else if (pdnType == PFCP_PDN_TYPE_IPV6) {
        session->ueIpv6.addr6 = ueIp->addr6;
        //session->pdn.paa.addr6 = ueIp->addr6;
    } else if (pdnType == PFCP_PDN_TYPE_IPV4V6) {
        // TODO
        // session->ueIpv4 = UpfUeIPAlloc(AF_INET, dnn);
        // UTLT_Assert(session->ueIpv4,
        //   UpfSessionRemove(session); return NULL,
        //   "Cannot allocate IPv4");

        // session->ueIpv6 = UpfUeIPAlloc(AF_INET6, dnn);
        // UTLT_Assert(session->ueIpv6,
        //   UpfSessionRemove(session); return NULL,
        //   "Cannot allocate IPv6");

        // session->pdn.paa.dualStack.addr4 = session->ueIpv4->addr4;
        // session->pdn.paa.dualStack.addr6 = session->ueIpv6->addr6;
    } else {
        UTLT_Assert(0, return NULL, "UnSupported PDN Type(%d)", pdnType);
    }

    /* Generate Hash Key: IP + DNN */
    if (pdnType == PFCP_PDN_TYPE_IPV4) {
        SessionHashKeygen(session->hashKey,
                          &session->hashKeylen,
                          (uint8_t *)&session->ueIpv4.addr4, 4, dnn);
    } else {
        SessionHashKeygen(session->hashKey,
                          &session->hashKeylen,
                          (uint8_t *)&session->ueIpv6.addr6,
                          IPV6_LEN, dnn);
    }

    HashSet(self.sessionHash, session->hashKey,
            session->hashKeylen, session);

    return session;
}

Status UpfSessionRemove(UpfSession *session) {
    UTLT_Assert(self.sessionHash, return STATUS_ERROR,
                "sessionHash error");
    UTLT_Assert(session, return STATUS_ERROR, "session error");

    HashSet(self.sessionHash, session->hashKey,
            session->hashKeylen, NULL);

    // if (session->ueIpv4) {
    //     UpfUeIPFree(session->ueIpv4);
    // }
    // if (session->ueIpv6) {
    //     UpfUeIPFree(session->ueIpv6);
    // }

    PDR_Thread_Safe(
        RuleListDeletionAndFreeWithGTPv1Tunnel(PDR, pdr, session);
    );

    FAR_Thread_Safe(
        RuleListDeletionAndFreeWithGTPv1Tunnel(FAR, far, session);
    );


    RuleListDeletionAndFreeWithGTPv1Tunnel(PDR, pdr, session);
    RuleListDeletionAndFreeWithGTPv1Tunnel(FAR, far, session);
    /* TODO: Not support yet
    QER_Thread_Safe(
        RuleListDeletionAndFreeWithGTPv1Tunnel(QER, qer, session);
    );

    BAR_Thread_Safe(
        RuleListDeletionAndFreeWithGTPv1Tunnel(BAR, bar, session);
    );

    URR_Thread_Safe(
        RuleListDeletionAndFreeWithGTPv1Tunnel(URR, urr, session);
    );
    */

    IndexFree(&upfSessionPool, session);

    return STATUS_OK;
}

Status UpfSessionRemoveAll() {
    HashIndex *hashIdx = NULL;
    UpfSession *session = NULL;

    for (hashIdx = UpfSessionFirst(); hashIdx;
         hashIdx = UpfSessionNext(hashIdx)) {
        session = UpfSessionThis(hashIdx);
        UpfSessionRemove(session);
    }

    return STATUS_OK;
}

UpfSession *UpfSessionFind(uint32_t idx) {
    //UTLT_Assert(idx, return NULL, "index error");
    return IndexFind(&upfSessionPool, idx);
}

UpfSession *UpfSessionFindBySeid(uint64_t seid) {
    return UpfSessionFind((seid-1) & 0xFFFFFFFF);
}

UpfSession *UpfSessionAddByMessage(PfcpMessage *message) {
    UpfSession *session;

    PFCPSessionEstablishmentRequest *request =
      &message->pFCPSessionEstablishmentRequest;

    if (!request->nodeID.presence) {
        UTLT_Error("no NodeID");
        return NULL;
    }
    if (!request->cPFSEID.presence) {
        UTLT_Error("No cp F-SEID");
        return NULL;
    }
    if (!request->createPDR[0].presence) {
        UTLT_Error("No PDR");
        return NULL;
    }
    if (!request->createFAR[0].presence) {
        UTLT_Error("No FAR");
        return NULL;
    }
    if (!request->pDNType.presence) {
        UTLT_Error("No PDN Type");
        return NULL;
    }
    if (!request->createPDR[0].pDI.presence) {
        UTLT_Error("PDR PDI error");
        return NULL;
    }
    if (!request->createPDR[0].pDI.uEIPAddress.presence) {
        UTLT_Error("UE IP Address error");
        return NULL;
    }
    if (!request->createPDR[0].pDI.networkInstance.presence) {
        UTLT_Error("Interface error");
        return NULL;
    }

    session = UpfSessionAdd((PfcpUeIpAddr *)
                &request->createPDR[0].pDI.uEIPAddress.value,
                request->createPDR[0].pDI.networkInstance.value,
                ((int8_t *)request->pDNType.value)[0]);
    UTLT_Assert(session, return NULL, "session add error");

    session->smfSeid = *(uint64_t*)request->cPFSEID.value;
    session->upfSeid = session->index+1;
    UTLT_Trace("UPF Establishment UPF SEID: %lu", session->upfSeid);

    return session;
}

