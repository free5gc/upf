#include "up_match.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <regex.h>
#include <pthread.h>

#include "utlt_debug.h"
#include "utlt_pool.h"
#include "utlt_list.h"
#include "utlt_hash.h"
#include "utlt_3gppTypes.h"
#include "utlt_netheader.h"
#include "pfcp_types.h"
#include "upf_context.h"
#include "up/up_path.h"

#include "updk/rule_pdr.h"

#define MAX_SIZE_OF_PACKET 1600

#define MAX_NUM_OF_MATCH_RULE (MAX_POOL_OF_BEARER * 2)

PoolDeclare(MatchRuleNodePool, MatchRuleNode, MAX_NUM_OF_MATCH_RULE);

#define MAX_NUM_OF_H_LIST       (8192)
Hash *MatchHash; // Only use its key

static inline uint32_t MHash32(uint32_t input) {
    uint32_t rt = MatchHash->seed;
    for (int i = 0; i < sizeof(input); i++, input >>= 8)
        rt = rt * 33 + (input & 0xFF);
    return rt;
}

ListHead IPv4HList[MAX_NUM_OF_H_LIST];
pthread_mutex_t IPv4HListLock;

ListHead TEIDHList[MAX_NUM_OF_H_LIST];
pthread_mutex_t TEIDHListLock;

#define IPv4_HList_Thread_Safe(expr) \
    pthread_mutex_lock(&IPv4HListLock); \
    expr; \
    pthread_mutex_unlock(&IPv4HListLock)

#define TEID_HList_Thread_Safe(expr) \
    pthread_mutex_lock(&TEIDHListLock); \
    expr; \
    pthread_mutex_unlock(&TEIDHListLock)

Status MatchInit() {
    PoolInit(&MatchRuleNodePool, MAX_NUM_OF_MATCH_RULE);

    MatchHash = HashMake();
    UTLT_Assert(MatchHash, return STATUS_ERROR, "Hash used in match rule alloc failed");

    for (int i = 0; i < MAX_NUM_OF_H_LIST; i++) {
        ListHeadInit(&IPv4HList[i]);
        ListHeadInit(&TEIDHList[i]);
    }

    pthread_mutex_init(&IPv4HListLock, 0);
    pthread_mutex_init(&TEIDHListLock, 0);

    return STATUS_OK;
}

Status MatchTerm() {
    PoolTerminate(&MatchRuleNodePool);

    pthread_mutex_destroy(&IPv4HListLock);
    pthread_mutex_destroy(&TEIDHListLock);
    HashDestroy(MatchHash);

    return STATUS_OK;
}

MatchRuleNode *MatchRuleNodeAlloc() {
    MatchRuleNode *rt = NULL;

    rt = calloc(1, sizeof(MatchRuleNode));
    // PoolAlloc(&MatchRuleNodePool, rt);
    UTLT_Assert(rt, return NULL, "MatchRuleNodePool is empty");

    //memset(rt, 0, sizeof(MatchRuleNode));
    ListHeadInit(&rt->node);

    return rt;
}

void MatchRuleDelete(MatchRuleNode *node) {
    if (node->sport_list)
        free(node->sport_list);
    if (node->dport_list)
        free(node->dport_list);

    ListRemove(node);
}

Status MatchRuleNodeFree(MatchRuleNode *node) {
    if (!node)
        return STATUS_OK;
    
    MatchRuleDelete(node);

    free(node);
    // PoolFree(&MatchRuleNodePool, node);

    return STATUS_OK;
}


// Copy from libgtp5gnl
static inline uint32_t *port_list_create(char *port_list) {
    uint32_t *ret = calloc(0xff, sizeof(uint32_t));
    uint32_t port1, port2, cnt = 0;

    char *tok_ptr = strtok(port_list, ","), *chr_ptr;
    while (tok_ptr != NULL)  {
        chr_ptr = strchr(tok_ptr, '-');
        if (chr_ptr) {
            *chr_ptr = '\0'; port1 = atoi(tok_ptr); port2 = atoi(chr_ptr + 1);
            if (port1 <= port2)
                ret[++cnt] = port1 + (port2 << 16);
            else
                ret[++cnt] = port2 + (port1 << 16);
        }
        else {
            port1 = atoi(tok_ptr);
            ret[++cnt] = port1 + (port1 << 16);
        }
        tok_ptr = strtok(NULL, ",");
    }
    ret[0] = cnt;

    return ret;
}

Status MatchRuleCompile(UPDK_PDR *pdr, MatchRuleNode *matchRule) {
    UTLT_Assert(pdr && matchRule, return STATUS_ERROR, "PDR or MatchRuleNode should not be NULL");

    // Clean up MatchRuleNode
    MatchRuleDelete(matchRule);
    memset(matchRule, 0, sizeof(MatchRuleNode));

    matchRule->precedence = pdr->precedence;
    
    if (pdr->flags.pdi) {
        UPDK_PDI *pdi = &pdr->pdi;

        UTLT_Assert(pdi->flags.sourceInterface, return STATUS_ERROR,
            "Need source interface in PDI to represent UL or DL");
        
        uint32_t *ueIP, *ueMask;
        switch(pdi->sourceInterface) {
            case 0: // Access or UL
                ueIP = &matchRule->saddr;
                ueMask = &matchRule->smask;
                break;
            case 1: // Core or DL
            case 2: // SGi-LAN or N6-LAN
                ueIP = &matchRule->daddr;
                ueMask = &matchRule->dmask;
                break;
            case 3:
                UTLT_Error("Source interface does NOT support CP-function yet");
                return STATUS_ERROR;
            default:
                UTLT_Error("%u in Source interfacet is reversed, it cannot use", pdi->sourceInterface);
                return STATUS_ERROR;
        }

        if (pdi->flags.ueIpAddress) {
            UPDK_UEIPAddress *ueIpAddress = &pdi->ueIpAddress;
            
            if (ueIpAddress->flags.v6)
                UTLT_Warning("Do NOT support IPv6 yet");

            if (ueIpAddress->flags.v4) {
                *ueIP = ueIpAddress->ipv4.s_addr;
                *ueMask = UINT_MAX;
            }
        }

        if (pdi->flags.fTeid) {
            UPDK_FTEID *fteid = &pdi->fTeid;

            matchRule->teid = fteid->teid;

            // TODO: Check Outer header
        }

        if (pdi->flags.sdfFilter) {
            UPDK_SDFFilter *sdfFilter = &pdi->sdfFilter;

            if (sdfFilter->flags.fd && sdfFilter->lenOfFlowDescription) {
                char *ruleStr = sdfFilter->flowDescription;
                ruleStr[sdfFilter->lenOfFlowDescription] = 0;

                // Copy from libgtp5gnl
                char reg_act[] = "(permit)";
                char reg_direction[] = "(in|out)";
                char reg_proto[] = "(ip|[0-9]{1,3}})";
                char reg_src_ip_mask[] = "(any|assigned|[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}(/[0-9]{1,5})?)";
                char reg_dest_ip_mask[] = "(any|assigned|[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}(/[0-9]{1,5})?)";
                char reg_port[] = "([ ][0-9]{1,5}([,-][0-9]{1,5})*)?";

                char reg[0xfff];
                sprintf(reg, "^%s %s %s from %s%s to %s%s$", reg_act, reg_direction, reg_proto,
                    reg_src_ip_mask, reg_port,
                    reg_dest_ip_mask, reg_port);

                regex_t preg;
                regmatch_t pmatch[0x10];
                int nmatch = sizeof(pmatch) / sizeof(regmatch_t);
                int cflags = REG_EXTENDED | REG_ICASE;

                UTLT_Assert(!regcomp(&preg, reg, cflags), return STATUS_ERROR,
                    "Regex string for SDF filter description format error");

                UTLT_Assert(!regexec(&preg, ruleStr, nmatch, pmatch, 0), return STATUS_ERROR,
                    "SDF filter description format error");

                int len;
                char buf[0xff];

                // Get Action
                len = pmatch[1].rm_eo - pmatch[1].rm_so;
                strncpy(buf, ruleStr + pmatch[1].rm_so, len); buf[len] = '\0';
                UTLT_Assert(!strcmp(buf, "permit"), return STATUS_ERROR,
                    "SDF filter description action only support 'permit', not support '%s'", buf);
                
                // Get Direction
                len = pmatch[2].rm_eo - pmatch[2].rm_so;
                strncpy(buf, ruleStr + pmatch[2].rm_so, len); buf[len] = '\0';
                if (strcmp(buf, "in") == 0) {
                    // TODO: Handle something here if need
                }
                else if (strcmp(buf, "out") == 0) {
                    // TODO: Handle something here if need
                }
                else {
                    UTLT_Error("SDF filter description direction not support '%s'", buf);
                    return STATUS_ERROR;
                }

                // Get Protocol
                len = pmatch[3].rm_eo - pmatch[3].rm_so;
                strncpy(buf, ruleStr + pmatch[3].rm_so, len); buf[len] = '\0';
                if (strcmp(buf, "ip") == 0)
                    matchRule->proto = 0;
                else {
                    int tmp = atoi(buf);
                    UTLT_Assert(tmp <= 0xff, return STATUS_ERROR,
                        "SDF filter description protocol[%s] not support", buf);

                    // TODO: Need to check if any value in this field
                    matchRule->proto = tmp;
                }

                // Get SRC Mask
                len = pmatch[5].rm_eo - pmatch[5].rm_so;
                if (len) {
                    strncpy(buf, ruleStr + pmatch[5].rm_so + 1, len - 1); buf[len - 1] = '\0';
                    int smask = atoi(buf);
                    UTLT_Assert(smask <= 32 && smask > 0, return STATUS_ERROR,
                        "SDF filter description SRC mask[%s] is invalid", smask);

                    // TODO: Need to check if any value in this field
                    matchRule->smask = UINT_MAX - ((1 << (32 - smask)) - 1);
                }

                // Get SRC IP
                len = pmatch[4].rm_eo - pmatch[4].rm_so - len;
                strncpy(buf, ruleStr + pmatch[4].rm_so, len); buf[len] = '\0';
                // TODO: Need to check if any value in this field
                if (strcmp(buf, "any") == 0)
                    matchRule->saddr = 0;
                else if (strcmp(buf, "assigned") == 0) {
                    UTLT_Error("SDF filter description dest ip do NOT support assigned yet");
                    return STATUS_ERROR;
                }
                else
                    UTLT_Assert(inet_pton(AF_INET, buf, &matchRule->saddr) == 1, return STATUS_ERROR,
                        "SDF filter description src ip[%s] is invalid", buf);

                // Get SRC Port
                len = pmatch[6].rm_eo - pmatch[6].rm_so;
                if (len) {
                    strncpy(buf, ruleStr + pmatch[6].rm_so + 1, len - 1); buf[len - 1] = '\0';
                    matchRule->sport_list = port_list_create(buf);
                }

                // Get Dest Mask
                len = pmatch[9].rm_eo - pmatch[9].rm_so;
                if (len) {
                    strncpy(buf, ruleStr + pmatch[9].rm_so + 1, len - 1); buf[len - 1] = '\0';
                    int dmask = atoi(buf);
                    UTLT_Assert(dmask <= 32 && dmask > 0, return STATUS_ERROR,
		                "SDF filter description Dest mask[%s] is invalid", dmask);
                    
                    // TODO: Need to check if any value in this field
                    matchRule->dmask = UINT_MAX - ((1 << (32 - dmask)) - 1);
                }

                // Get Dest IP
                len = pmatch[8].rm_eo - pmatch[8].rm_so - len;
                strncpy(buf, ruleStr + pmatch[8].rm_so, len); buf[len] = '\0';
                // TODO: Need to check if any value in this field
                if (strcmp(buf, "any") == 0)
                    matchRule->daddr = 0;
                else if (strcmp(buf, "assigned") == 0) {
                    UTLT_Error("SDF filter description dest ip do NOT support assigned yet");
                    return STATUS_ERROR;
                }
                else 
                    UTLT_Assert(inet_pton(AF_INET, buf, &matchRule->daddr) == 1, return STATUS_ERROR,
                        "SDF filter description dest ip[%s] is invalid", buf);

                // Get Dest Port
                len = pmatch[10].rm_eo - pmatch[10].rm_so;
                if (len) {
                    strncpy(buf, ruleStr + pmatch[10].rm_so + 1, len - 1); buf[len - 1] = '\0';
                    matchRule->dport_list = port_list_create(buf);
                }

            }

            // TODO: Add if need other part
        }
    }

    return STATUS_OK;
}

static inline ListHead *GetLastSmallPrecedenceFromList(ListHead *entry, MatchRuleNode *matchRule) {
    ListHead *it = entry, *itNext;
    ListForEachSafe(it, itNext, entry) {
            if (((MatchRuleNode *) itNext)->precedence > matchRule->precedence)
            break;
    }
    return it;
}

static Status MatchRuleRegisterToGTPU(MatchRuleNode *matchRule) {
    TEID_HList_Thread_Safe(
        ListHead *entry = GetLastSmallPrecedenceFromList(&TEIDHList[MHash32(matchRule->teid) % MAX_NUM_OF_H_LIST], matchRule);
        ListInsert(matchRule, entry);
    );
    
    return STATUS_OK;
}

static Status MatchRuleRegisterToIPv4(MatchRuleNode *matchRule) {
    IPv4_HList_Thread_Safe(
        ListHead *entry = GetLastSmallPrecedenceFromList(&IPv4HList[MHash32(matchRule->daddr) % MAX_NUM_OF_H_LIST], matchRule);
        ListInsert(matchRule, entry);
    );
    
    return STATUS_OK;
}

Status MatchRuleRegister(MatchRuleNode *matchRule) {
    UTLT_Assert(matchRule, return STATUS_ERROR, "MatchRuleNode should not be NULL")

    if (matchRule->teid)
        return MatchRuleRegisterToGTPU(matchRule);
    else
        return MatchRuleRegisterToIPv4(matchRule);
}

static Status MatchRuleDeregisterToGTPU(MatchRuleNode *matchRule) {
    TEID_HList_Thread_Safe(
        ListRemove(matchRule);
    );
    return STATUS_OK;
}

static Status MatchRuleDeregisterToIPv4(MatchRuleNode *matchRule) {
    IPv4_HList_Thread_Safe(
        ListRemove(matchRule);
    );
    return STATUS_OK;
}

Status MatchRuleDeregister(MatchRuleNode *matchRule) {
    if (matchRule->teid)
        return MatchRuleDeregisterToGTPU(matchRule);
    else
        return MatchRuleDeregisterToIPv4(matchRule);
}

/**
 * PacketLenIsEnough - Check the length of packet is enough
 * 
 * @need: expected length of packet you want to access
 * @total: actual length of packet
 * @return: 1 or 0 is not enough
 */
static inline int PacketLenIsEnough(uint16_t need, uint16_t total) {
    return need <= total;
}

/**
 * CheckGTPUVersion - Check GTP-U version is GTPv1
 * 
 * @pkt: packet pointer which layer should upper or equal than outer L3 header
 * @pktlen: total length of @pkt
 * @hdrlen: Header length from @pkt to outer L3 header
 * @return: 1 is GTPv1 or 0 is not GTPv1
 */
static inline int CheckGTPUVersion(uint8_t *pkt, uint16_t pktlen, uint16_t hdrlen) {
    if (!PacketLenIsEnough(hdrlen + sizeof(Gtpv1Header), pktlen))
        return 0;

    Gtpv1Header *gtpHdr = (Gtpv1Header *) (pkt + hdrlen);
    return (gtpHdr->flags >> 5) == 1;
}

/**
 * CheckIsGTPU - Check packet is using GTP-U
 * 
 * @pkt: packet pointer which layer should upper or equal than outer L3 header
 * @pktlen: total length of @pkt
 * @hdrlen: Header length from @pkt to outer L3 header
 * @return: 1 is GTP-U, 0 is not GTP-U, -1 is GTP, but header is wrong
 */
static inline int CheckIsGTPU(uint8_t *pkt, uint16_t pktlen, uint16_t hdrlen) {
    if (!PacketLenIsEnough(hdrlen + sizeof(IPv4Header) + sizeof(UDPHeader) + sizeof(Gtpv1Header), pktlen))
        return 0;

    IPv4Header *iph = (IPv4Header *) (pkt + hdrlen);
    UDPHeader *udpHdr = (UDPHeader *) ((uint8_t *) iph + sizeof(IPv4Header));

    return (iph->proto == 17 && udpHdr->dest == htons(2152)) ? 
        (CheckGTPUVersion(pkt, pktlen, hdrlen + sizeof(IPv4Header) + sizeof(UDPHeader)) ? 1 : -1) : 0;
}

/**
 * GTPUHeaderLen - Calculate total length of GTP-U header
 * 
 * @pkt: packet pointer which layer should upper or equal than GTP-U header
 * @pktLen: total length of @pkt
 * @hdrlen: Header length from @pkt to GTP-U header
 */
static int GTPUHeaderLen(uint8_t *pkt, uint16_t pktlen, uint16_t hdrlen) {
    uint8_t *gtp1 = (pkt + hdrlen);
    uint8_t *ext_hdr = NULL;

    uint16_t remain_len = pktlen - hdrlen;
    uint16_t rt_len = 8 + ((*gtp1 & 0x07) ? 4 : 0);

    if (*gtp1 & 0x04) {
        /* ext_hdr will always point to "Next ext hdr type" */
        while (*(ext_hdr = gtp1 + rt_len - 1)) {
            if (!PacketLenIsEnough(rt_len, remain_len))
                return -1;
            rt_len += (*(++ext_hdr)) * 4;
        }
    }

    return rt_len;
}

static inline int ProtocolMatch(uint8_t targetProto, uint8_t matchProto) {
    return targetProto == matchProto;
}

static inline int IPv4Match(uint32_t targetIP, uint32_t matchIP, uint32_t matchMask) {
    return !((targetIP ^ matchIP) & matchMask);
}

static int PortMatch(uint16_t port, uint32_t *matchList, int len) {
    if (!len)
        return 1;
    
    for (int i = 0; i < len; i++) {
        if (PortStart(matchList[i]) <= port && PortEnd(matchList[i]) >= port)
            return 1;
    }

    return 0;
}

static inline int PacketNonGTPUMatch(uint8_t *pkt, uint16_t pktlen, uint32_t hdrlen, MatchRuleNode *matchRule) {
    UTLT_Assert(PacketLenIsEnough(hdrlen + sizeof(IPv4Header), pktlen), return STATUS_ERROR,
        "Packet length is not enough");
    IPv4Header *iph = (IPv4Header *) ((uint8_t *) pkt + hdrlen);

    UDPHeader *udpHdr = (PacketLenIsEnough(hdrlen + sizeof(IPv4Header) + sizeof(UDPHeader), pktlen) ?
        (UDPHeader *) ((uint8_t *) iph + sizeof(IPv4Header)) : NULL);

    if (matchRule->proto)
        if (!ProtocolMatch(iph->proto, matchRule->proto))
            return 0;
    if (matchRule->saddr && matchRule->smask)
        if (!IPv4Match(iph->saddr, matchRule->saddr, matchRule->smask))
            return 0;
    if (matchRule->daddr && matchRule->dmask)
        if (!IPv4Match(iph->daddr, matchRule->daddr, matchRule->dmask))
            return 0;
    if (matchRule->sport_num)
        if (!udpHdr || !PortMatch(ntohs(udpHdr->source), matchRule->sport_list, matchRule->sport_num))
            return 0;
    if (matchRule->dport_num)
        if (!udpHdr || !PortMatch(ntohs(udpHdr->dest), matchRule->dport_list, matchRule->dport_num))
            return 0;
    return 1;
}

Status FindPDRByTEID(uint8_t *pkt, uint16_t pktlen, uint32_t hdrlen, void *pdrBuf) {
    UTLT_Assert(PacketLenIsEnough(hdrlen + sizeof(Gtpv1Header), pktlen), return STATUS_ERROR,
        "Packet length is not enough");
    
    Status status = STATUS_ERROR;

    Gtpv1Header *gtpHdr = (Gtpv1Header *) (pkt + hdrlen);
    uint32_t teid = ntohl(gtpHdr->_teid);

    int gtpuLen = GTPUHeaderLen((uint8_t *) gtpHdr, pktlen - hdrlen, 0);
    UTLT_Assert(gtpuLen >= 0, return STATUS_ERROR, "GTP-U packet format failed");

    MatchRuleNode *matchRule, *nextMatchRule = NULL;
    TEID_HList_Thread_Safe(
        ListHead *entry = &TEIDHList[MHash32(teid) % MAX_NUM_OF_H_LIST];
        ListForEachSafe(matchRule, nextMatchRule, entry) {
            if (matchRule->teid != teid) {
                continue;
            }

            if (!PacketNonGTPUMatch((uint8_t *) gtpHdr, pktlen, gtpuLen, matchRule))
                continue;

            memcpy(pdrBuf, matchRule->pdr, sizeof(UPDK_PDR));
            status = STATUS_OK;
            break;
        }
    );

    return status;
}

Status FindPDRByUEIP(uint8_t *pkt, uint16_t pktlen, uint32_t hdrlen, void *pdrBuf) {
    UTLT_Assert(PacketLenIsEnough(hdrlen + sizeof(IPv4Header), pktlen), return STATUS_ERROR,
        "Packet length is not enough");

    Status status = STATUS_ERROR;

    IPv4Header *iph = (IPv4Header *) ((uint8_t *) pkt + hdrlen);
    MatchRuleNode *matchRule, *nextMatchRule = NULL;
    IPv4_HList_Thread_Safe(
        ListHead *entry = &IPv4HList[MHash32(iph->daddr) % MAX_NUM_OF_H_LIST];
        ListForEachSafe(matchRule, nextMatchRule, entry) {
            if (!PacketNonGTPUMatch(pkt, pktlen, hdrlen, matchRule))
                continue;

            memcpy(pdrBuf, matchRule->pdr, sizeof(UPDK_PDR));
            status = STATUS_OK;
            break;
        }
    );

    return status;
}

static int PacketInGTPUHandle(uint8_t *pkt, uint16_t pktlen, uint16_t hdrlen, uint32_t remoteIP, uint16_t _remotePort, MatchRuleNode *matchRule) {
    UTLT_Assert(PacketLenIsEnough(hdrlen + sizeof(Gtpv1Header), pktlen), return STATUS_ERROR,
        "Packet length is not enough");

    Sock *sock = &Self()->upSock;
    sock->remoteAddr._family= AF_INET;
    sock->remoteAddr.s4.sin_addr.s_addr = remoteIP;
    sock->remoteAddr._port = _remotePort;

    Status status = STATUS_OK;
    Gtpv1Header *gtpHdr = (Gtpv1Header *) (pkt + hdrlen);
    switch (gtpHdr->type) {
        case GTPV1_T_PDU: // Should be the first to speed up UP packet matching
            status = FindPDRByTEID(pkt, pktlen, sizeof(IPv4Header) + sizeof(UDPHeader), matchRule);
            return (status == STATUS_OK ? 0 : -1);
        case GTPV1_ECHO_REQUEST:
            status = GtpHandleEchoRequest(sock, gtpHdr);
            break;
        case GTPV1_ECHO_RESPONSE:
            status = GtpHandleEchoResponse(gtpHdr);
            break;
        case GTPV1_ERROR_INDICATION:
            // TODO: Implement it if we need it
            break;
        case GTPV1_END_MARK:
            // TODO : Need to deal with the UE packet that does not have tunnel yet
            status = GtpHandleEndMark(sock, gtpHdr);
            break;
        default :
            UTLT_Debug("This type[%d] of GTPv1 header does not implement yet", gtpHdr->type);
    }

    return (status == STATUS_OK ? 1 : -1);
}

static int PacketInBufferHandle(uint8_t *pkt, uint16_t pktlen, UPDK_PDR *matchedPDR) {
    Status status;
    uint8_t action;
    uint32_t farId = ((UPDK_PDR *) matchedPDR)->farId;
    
    UTLT_Assert(HowToHandleThisPacket(farId, &action) == STATUS_OK, return -1,
        "FAR[%u] does not existed", farId);

    if (action & PFCP_FAR_APPLY_ACTION_BUFF) {
        uint32_t pdrId = ((UPDK_PDR *) matchedPDR)->pdrId;
        UpfBufPacket *packetStorage = UpfBufPacketFindByPdrId(pdrId);
        UTLT_Assert(packetStorage, return -1, "Cannot find matching PDR ID buffer slot");

        // protect data write with spinlock
        // instead of protect code block with mutex
        UTLT_Assert(!pthread_spin_lock(&Self()->buffLock), return -1,
                    "spin lock buffLock error");

        if (!packetStorage->packetBuffer) {
            // if packetBuffer null, allocate space
            // reuse the pktbuf, so don't free it
            packetStorage->packetBuffer = BufblkAlloc(1, MAX_SIZE_OF_PACKET);
            UTLT_Assert(packetStorage->packetBuffer, return -1, "UpfBufPacket alloc failed");
        }

        // if packetBuffer not null, just add packet followed
        status = BufblkBytes(packetStorage->packetBuffer, (const char *) pkt, pktlen);
        UTLT_Assert(status == STATUS_OK, return -1,
                    "block add behand old buffer error");

        while (pthread_spin_unlock(&Self()->buffLock)) {
            // if unlock failed, keep trying
            UTLT_Error("spin unlock error");
        }

        if (action & PFCP_FAR_APPLY_ACTION_NOCP) {
            // If NOCP, Send event to notify SMF
            uint64_t seid = ((UpfSession*) packetStorage->sessionPtr)->upfSeid;
            UTLT_Debug("buffer NOCP to SMF: SEID: %u, PDRID: %u", seid, pdrId);
            status = EventSend(Self()->eventQ, UPF_EVENT_SESSION_REPORT, 2,
                            seid, pdrId);
            UTLT_Assert(status == STATUS_OK, ,
                        "DL data message event send to N4 failed");
        }

        return 1;
    }

    return 0;
}

int PacketInWithL3(uint8_t *pkt, uint16_t pktlen, void *matchedPDR) {
    UTLT_Assert(pkt && pktlen >= 0, goto MATCHFAILED, "Packet and its length should not be NULL and 0");
    UTLT_Assert(matchedPDR, goto MATCHFAILED, "The space to store UPDK_PDR should not be NULL");

    int status;

    status = CheckIsGTPU(pkt, pktlen, 0);
    UTLT_Level_Assert(LOG_DEBUG, status != -1, goto MATCHFAILED, "Packet GTP version error");

    IPv4Header *iph = (IPv4Header *) (pkt);
    UDPHeader *udpHdr = (UDPHeader *) ((uint8_t *) iph + sizeof(IPv4Header));

    if (status) { // GTP-U Packet
        status = PacketInGTPUHandle(pkt, pktlen, sizeof(IPv4Header) + sizeof(UDPHeader), iph->saddr, udpHdr->source, matchedPDR);
        UTLT_Level_Assert(LOG_DEBUG, status != -1, goto MATCHFAILED, "Packet match with GTP-U header failed");
        if (status) return 1; // Non T-PDU packet
    } else { // General L3 Packet
        status = FindPDRByUEIP(pkt, pktlen, 0, matchedPDR);
        UTLT_Level_Assert(LOG_DEBUG, status == STATUS_OK, goto MATCHFAILED, "Packet match with L3/L4 header failed");
    }

    return PacketInBufferHandle(pkt, pktlen, matchedPDR);

MATCHFAILED:
    return -1;
}

int PacketInWithGTPU(uint8_t *pkt, uint16_t pktlen, uint32_t remoteIP, uint16_t _remotePort, void *matchedPDR) {
    UTLT_Assert(pkt && pktlen >= 0, goto MATCHFAILED, "Packet and its length should not be NULL and 0");
    UTLT_Assert(remoteIP && _remotePort, goto MATCHFAILED, "Remote IP and port should not be 0");
    UTLT_Assert(matchedPDR, goto MATCHFAILED, "The space to store UPDK_PDR should not be NULL");

    UTLT_Level_Assert(LOG_DEBUG, CheckGTPUVersion(pkt, pktlen, 0) == 1, goto MATCHFAILED, "Packet GTP version error");

    int status = PacketInGTPUHandle(pkt, pktlen, 0, remoteIP, _remotePort, matchedPDR);
    UTLT_Level_Assert(LOG_DEBUG, status != -1, goto MATCHFAILED, "Packet match with GTP-U header failed");
    if (status) return 1; // Non T-PDU packet

    return PacketInBufferHandle(pkt, pktlen, matchedPDR);

MATCHFAILED:
    return -1;
}