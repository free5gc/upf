#ifndef __UPDK_RULE_FAR_H__
#define __UPDK_RULE_FAR_H__

#include <stdint.h>
#include <arpa/inet.h>

enum {
    UPDK_REDIRECT_ADDRESS_TYPE_IPV4 = 0,
    UPDK_REDIRECT_ADDRESS_TYPE_IPV6,
    UPDK_REDIRECT_ADDRESS_TYPE_URL,
    UPDK_REDIRECT_ADDRESS_TYPE_SIP_URI,
    
    UPDK_REDIRECT_ADDRESS_TYPE_MAX,
};

typedef struct {
    uint8_t redirectAddressType;
    uint16_t redirectServerAddressLength;

    // TODO: dynamic alloc?
    char redirectServerAddress[0x40];
} UPDK_RedirectInformation;

enum {
    UPDK_OUTER_HEADER_CREATION_DESCRIPTION_UPSPEC = 0,

    UPDK_OUTER_HEADER_CREATION_DESCRIPTION_GTPU_UDP_IPV4,
    UPDK_OUTER_HEADER_CREATION_DESCRIPTION_GTPU_UDP_IPV6,
    UPDK_OUTER_HEADER_CREATION_DESCRIPTION_UDP_IPV4,
    UPDK_OUTER_HEADER_CREATION_DESCRIPTION_UDP_IPV6,

    UPDK_OUTER_HEADER_CREATION_DESCRIPTION_MAX,
};

typedef struct {
    uint16_t description;
    uint32_t teid;
    struct in_addr ipv4;
    struct in6_addr ipv6;
    uint16_t port;
} UPDK_OuterHeaderCreation;

typedef struct {
    uint8_t forwardingPolicyIdentifierLength;

    // TODO: dynamic alloc?
    char forwardingPolicyIdentifier[0x40];
} UPDK_ForwardingPolicy;

// TODO:
typedef struct {

} UPDK_HeaderEnrichment;

enum {
    UPDK_INTERFACE_VALUE_ACCESS = 0,
    UPDK_INTERFACE_VALUE_CORE,
    UPDK_INTERFACE_VALUE_N6_LAN,
    UPDK_INTERFACE_VALUE_CP,
    UPDK_INTERFACE_VALUE_LI,
    
    UPDK_INTERFACE_VALUE_MAX,
};

enum {
    UPDK_PROXYING_UNSPEC = 0,

    UPDK_PROXYING_APP = 1,
    UPDK_PROXYING_INS = 2,

    UPDK_PROXYING_MAX,
};

typedef struct {
    struct {
        uint16_t destinationInterface:1;
        uint16_t networkInstance:1;
        uint16_t redirectInformation:1;
        uint16_t outerHeaderCreation:1;
        uint16_t transportLevelMarking:1;
        uint16_t forwardingPolicy:1;
        uint16_t headerEnrichment:1;
        uint16_t trafficEndpointId:1;
        uint16_t proxying:1;
        uint16_t spare:7;
    } flags;

    uint8_t destinationInterface;
    
    // TODO: dynamic alloc?
    char networkInstance[0x40];

    UPDK_RedirectInformation redirectInformation;
    UPDK_OuterHeaderCreation outerHeaderCreation;
    uint16_t transportLevelMarking;
    UPDK_ForwardingPolicy forwardingPolicy;
    UPDK_HeaderEnrichment headerEnrichment;
    uint8_t trafficEndpointId;
    uint8_t proxying;
} UPDK_ForwardingParameters;

typedef struct {

} UPDK_DuplicatingParameters;

enum {
    UPDK_FAR_APPLY_ACTION_UNSPEC = 0,

    UPDK_FAR_APPLY_ACTION_DROP = 1,
    UPDK_FAR_APPLY_ACTION_FORW = 2,
    UPDK_FAR_APPLY_ACTION_BUFF = 4,
    UPDK_FAR_APPLY_ACTION_NOCP = 8,
    UPDK_FAR_APPLY_ACTION_DUPL = 16,

    UPDK_FAR_APPLY_ACTION_MAX,
};

typedef struct {
    struct {
        uint8_t seid:1;
        uint8_t farId:1;
        uint8_t applyAction:1;
        uint8_t forwardingParameters:1;
        uint8_t barId:1;
        uint8_t spare:3;
    } flags;

    uint64_t seid;

    uint32_t farId;
    uint8_t applyAction;

    UPDK_ForwardingParameters forwardingParameters;
    UPDK_DuplicatingParameters duplicatingParameters;
    uint8_t barId;
} UPDK_FAR;

#endif /* __UPDK_RULE_FAR_H__ */