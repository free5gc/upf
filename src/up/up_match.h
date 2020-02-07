#ifndef __UP_MATCH_H__
#define __UP_MATCH_H__

#include <stdint.h>

#include "utlt_debug.h"
#include "utlt_list.h"
#include "updk/rule_pdr.h"

typedef struct {
    ListHead node;

    // GTP-U
    uint32_t precedence;
    uint32_t teid;

    // L3 header
    uint8_t proto;
    uint32_t saddr, smask;
    uint32_t daddr, dmask;

    // L4 header
    int sport_num;
    uint32_t *sport_list;
    int dport_num;
    uint32_t *dport_list;

    // Result Only pointer, no any alloc
    UPDK_PDR *pdr;
} MatchRuleNode;

#define PortStart(__u32) ((__u32) >> 16)
#define PortEnd(__u32) ((__u32) & 0xFFFF)

Status MatchInit();

Status MatchTerm();

MatchRuleNode *MatchRuleNodeAlloc();

Status MatchRuleNodeFree(MatchRuleNode *node);

Status MatchRuleCompile(UPDK_PDR *pdr, MatchRuleNode *matchRule);

Status MatchRuleRegister(MatchRuleNode *matchRule);

Status MatchRuleDeregister(MatchRuleNode *matchRule);

Status FindPDRByTEID(uint8_t *pkt, uint16_t pktlen, uint32_t hdrlen, void *pdrBuf);

Status FindPDRByUEIP(uint8_t *pkt, uint16_t pktlen, uint32_t hdrlen, void *pdrBuf);

/**
 * PacketInWithL3 - Find the matched rule, handle the L3 packet and return the rule
 * 
 * @pkt: L3 packet pointer 
 * @pktlen: Total length of @pkt
 * @matchedPDR: A allocated space used to store matched rule
 * @return: 1 if packet is handled by UPF, 0 if find any matched rule or -1 if do NOT match any rule
 */
int PacketInWithL3(uint8_t *pkt, uint16_t pktlen, void *matchedPDR);

/**
 * PacketInWithGTPU - Find the matched rule, handle the GTP-U packet and return the rule
 * 
 * @pkt: GTP-U packet pointer 
 * @pktlen: Total length of @pkt
 * @remoteIP: Sender IPv4 address with network type
 * @_remotePort: Sender port with network type 
 * @matchedPDR: A allocated space used to store matched rule
 * @return: 1 if packet is handled by UPF, 0 if find any matched rule or -1 if do NOT match any rule
 */
int PacketInWithGTPU(uint8_t *pkt, uint16_t pktlen, uint32_t remoteIP, uint16_t _remotePort, void *matchedPDR);

#endif /* __UP_MATCH_H__ */