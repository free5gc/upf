#ifndef __PFCP_CONVERT_H__
#define __PFCP_CONVERT_H__

#include <stdint.h>

#include "utlt_debug.h"
#include "pfcp_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Status PfcpFSeidToSockaddr(
    PfcpFSeid *fSeid, uint16_t port, SockAddr **list);
Status PfcpSockaddrToFSeid(
    SockAddr *addr, SockAddr *addr6, PfcpFSeid *fSeid, int *len);
Status PfcpFSeidToIp(PfcpFSeid *fSeid, Ip *ip);
Status PfcpIpToFSeid(Ip *ip, PfcpFSeid *fSeid, int *len);
Status PfcpIpToFTeid(Ip *ip, PfcpFTeid *fTeid, int *len);
Status PfcpIpToUeIpAddr(Ip *ip, PfcpUeIpAddr *ueIp, int *len);
Status PfcpOuterHdrToIp(PfcpOuterHdr *outerHdr, Ip *ip);
Status PfcpSockaddrToFTeid(
    SockAddr *addr, SockAddr *addr6, PfcpFTeid *fTeid, int *len);
Status Pfcp5ByteBitRateToHost(uint8_t *bitRate, uint64_t *hostType);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
