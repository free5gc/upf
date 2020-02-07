#ifndef __UTLT_NETHEADER_H__
#define __UTLT_NETHEADER_H__

#include <stdint.h>

#include "utlt_lib.h"

typedef struct {
ENDIAN2(
    uint8_t     ihl:4;,
    uint8_t     version:4;
)
    uint8_t     tos;
    uint16_t    totalLen;
    uint16_t    id;
    uint16_t    fragOff;
    uint8_t     ttl;
    uint8_t     proto;
    uint16_t    check;
    uint32_t    saddr;
    uint32_t    daddr;
} __attribute__ ((packed)) IPv4Header;

typedef struct {
	uint16_t	source;
	uint16_t	dest;
	uint16_t    len;
	uint16_t    check;
} __attribute__ ((packed)) UDPHeader;

typedef struct _Gtpv1Header {
    uint8_t  flags;
    uint8_t  type;
    uint16_t _length;
    uint32_t _teid;
} __attribute__ ((packed)) Gtpv1Header;

typedef struct _Gtpv1OptHeader {
    uint16_t _seqNum;
    uint8_t  nPdnNum;
    uint8_t  nextExtHrdType;

} __attribute__ ((packed)) Gtpv1OptHeader;

#define GTPV1_HEADER_LEN       8
#define GTPV1_OPT_HEADER_LEN   4

#define GTPV1_ECHO_REQUEST     1
#define GTPV1_ECHO_RESPONSE    2
#define GTPV1_ERROR_INDICATION 26
#define GTPV1_END_MARK         254
#define GTPV1_T_PDU            255

#endif /* __UTLT_NETHEADER_H__ */