#ifndef __UPDK_RULE_PDR_H__
#define __UPDK_RULE_PDR_H__

#include <stdint.h>
#include <arpa/inet.h>

/**
 * These IEs is save as TV type (Tag and Value). Tag is to defined if
 * this IE is existed and Value is to store this IE.
 * 
 * Not all IEs are mandatory, so please check the tag if this IE is existed
 * before access value.
 */

/**
 * UPDK_UEIPAddress - TS 29.244 and Chapter 8.2.62
 * 
 * @flags.v6: If this bit is set to "1", then the IPv6 address field shall
 *     be present in the UE IP Address, otherwise the IPv6 address field
 *     shall not be present.
 * @flags.v4: If this bit is set to "1", then the IPv4 address field shall
 *     be present in the UE IP Address, otherwise the IPv4 address field
 *     shall not be present.
 * @flags.sd: This bit is only applicable to the UE IP Address IE in the
 *     PDI IE. It shall be set to "0" and ignored by the receiver in IEs
 *     other than PDI IE. In the PDI IE, if this bit is set to "0", this
 *     indicates a Source IP address; if this bit is set to "1", this 
 *     indicates a Destination IP address.
 * @flags.ipv6d: This bit is only applicable to the UE IP address IE in
 *     the PDI IE and whhen V6 bit is set to "1". If this bit is set to "1",
 *     then the IPv6 Prefix Delegation Bits field shall be present,
 *     otherwise the UP function shall consider IPv6 prefix is default /64.
 * @ipv4: IPv4 address of UE
 * @ipv6: IPv6 address of UE
 * @ipv6PrefixDelegationBit: Prefix delegation bit for IPv6
 */
typedef struct {
    struct {
        uint8_t v6:1;
        uint8_t v4:1;
        uint8_t sd:1;
        uint8_t ipv6d:1;
        uint8_t spare:3;
    } flags;

    struct in_addr ipv4;
    struct in6_addr ipv6;

    uint8_t ipv6PrefixDelegationBit;

} UPDK_UEIPAddress;

/**
 * UPDK_FTEID - TS 29.244 and Chapter 8.2.3
 * 
 * @flags.v4: If this bit is set to "1" and the CH bit is not set, then the IPv4 address
 *     field shall be present, otherwise the IPv4 address field shall not be present.
 * @flags.v6: If this bit is set to "1" and the CH bit is not set, then the IPv6 address
 *     field shall be present, otherwise the IPv6 address field shall not be present.
 * @flags.ch: If this bit is set to "1", then the TEID, IPv4 address and IPv6 address fields
 *     shall not be present and the UP function shall assign an F-TEID with an IP4 or an IPv6
 *     address if the V4 or V6 bit is set respectively. This bit shall only be set by the CP function.
 * @flags.chid:  If this bit is set to "1", then the UP function shall assign the same F-TEID
 *     to the PDRs requested to be created in a PFCP Session Establishment Request or PFCP Session
 *     Modification Request with the same CHOOSE ID value. This bit may only be set to "1" if the
 *     CH bit it set to "1". This bit shall only be set by the CP function.
 * @teid: do the matching in GTPv1 header for incoming packet
 * @ipv4: IPv4 address of this NF if v4 bit is present
 * @ipv6: IPv6 address of this NF if v6 bit is present
 * @chooseId: a binary integer value
 */
typedef struct {
    struct {
        uint8_t v4:1;
        uint8_t v6:1;
        uint8_t ch:1;
        uint8_t chid:1;
        uint8_t spare:4;
    } flags;

    uint32_t teid;
    struct in_addr ipv4;
    struct in6_addr ipv6;
    uint8_t chooseId;
} UPDK_FTEID;

/**
 * UPDK_SDFFilter - TS 29.244 and Chapter 8.2.5
 * 
 * @flags.fd: If this bit is set to "1", then the Length of Flow Description
 *     and the Flow Description fields shall be present, otherwise they shall
 *     not be present.
 * @flags.ttc: If this bit is set to "1", then the ToS Traffic Class field
 *     shall be present, otherwise the ToS Traffic Class field shall not be present.
 * @flags.spi: If this bit is set to "1", then the Security Parameter Index
 *     field shall be present, otherwise the Security Parameter Index field shall
 *     not be present.
 * @flags.fl: If this bit is set to "1", then the Flow Label field shall be present,
 *     otherwise the Flow Label field shall not be present.
 * @flags.bid: If this bit is set to "1", then the SDF Filter ID shall be present,
 *     otherwise the SDF Filter ID shall not be present.
 * @lenOfFlowDescription: Length of flow description
 * @flowDescription: Octect string for a pattern for matching the IP 5 tuple
 * @tosTrafficClass: ToS Traffic Class
 * @securityParameterIndex: Security Parameter Index
 * @flowLabel: Flow Label
 * @sdfFilterId: SDF Filter ID
 */
typedef struct {
    struct {
        uint8_t fd:1;
        uint8_t ttc:1;
        uint8_t spi:1;
        uint8_t fl:1;
        uint8_t bid:1;
        uint8_t spare:3;
    } flags;

    uint16_t lenOfFlowDescription;
    
    // TODO: dynamic alloc?
    char flowDescription[0x40];
    
    uint16_t tosTrafficClass;
    uint32_t securityParameterIndex;
    char flowLabel[3];
    uint32_t sdfFilterId;

} UPDK_SDFFilter;

// TODO: 
typedef struct {

} UPDK_EthernetPDUSessionInformation;

// TODO: 
typedef struct {

} UPDK_EthernetPacketFilter;

/**
 * UPDK_PDI - An IE from TS 29.244 and Table 7.5.2.2-2
 * 
 * @sourceInterface: Identify the source interface of the incoming packet.
 * @fTeid: Identify the local F-TEID to match for an incoming packet.
 * @networkInstance: Identify the Network instance to match for the incoming packet.
 * @ueIpAddress: Identify the source or destination IP address to match for the incoming packet.
 * @trafficEndpointId: The UP function has indicated the support of PDI optimization.
 * @sdfFilter: Identify the SDF filter to match for the incoming packet.  Several IEs
 *     with the same IE type may be present to provision a list of SDF Filters.
 * @applicationId: Identify the Application ID to match for the incoming packet.
 * @ethernetPduSessionInformation: Identify all the (DL) Ethernet packets matching
 *     an Ethernet PDU session
 * @ethernetPacketFilter: Identify the Ethernet PDU to match for the incoming packet. Several IEs
 *     with the same IE type may be present to represent a list of Ethernet Packet Filters.
 * @qfi: identify the QoS Flow Identifier to match for the incoming packet. Several IEs with
 *     the same IE type may be present to provision a list of QFIs.
 * @framedRoute: For a DL PDR if the UPF indicated support of Framed Routing. Several IEs with
 *     the same IE type may be present to provision a list of framed routes.
 * @framedRouting: For a DL PDR if the UPF indicated support of Framed Routing.
 * @framedIpv6Route: For a DL PDR if the UPF indicated support of Framed Routing
 */
typedef struct {
    struct {
        uint16_t spare:3;
        uint16_t sourceInterface:1;
        uint16_t fTeid:1;
        uint16_t networkInstance:1;
        uint16_t ueIpAddress:1;
        uint16_t trafficEndpointId:1;
        uint16_t sdfFilter:1;
        uint16_t applicationId:1;
        uint16_t ethernetPduSessionInformation:1;
        uint16_t ethernetPacketFilter:1;
        uint16_t qfi:1;
        uint16_t framedRoute:1;
        uint16_t framedRouting:1;
        uint16_t framedIpv6Route:1;
    } flags;

    uint8_t sourceInterface;
    UPDK_FTEID fTeid;

    // TODO: dynamic alloc?
    char networkInstance[0x40];
    UPDK_UEIPAddress ueIpAddress;
    uint8_t trafficEndpointId;
    UPDK_SDFFilter sdfFilter;

    // TODO: dynamic alloc?
    char applicationId[0x40];

    UPDK_EthernetPDUSessionInformation ethernetPduSessionInformation;
    UPDK_EthernetPacketFilter ethernetPacketFilter;

    uint8_t qfi;

    // TODO: dynamic alloc?
    char framedRoute[0x40];

    uint32_t framedRouting;

    // TODO: dynamic alloc?
    char framedIpv6Route[0x40];
} UPDK_PDI;

/**
 * UPDK_PDR - An IE from TS 29.244 and Table 7.5.2.2-1
 * 
 * @flags.*: 1 or 0 if the IE under PDR is not existed
 * @pdrId: Uniquely identify the PDR among all the PDRs configured for that PFCP session.
 * @precedence: The UP function among all PDRs of the PFCP session when looking for
 *     a PDR matching an incoming packet.
 * @pdi: Incoming packets will be matched.
 * @outerHeaderRemoval: UP function is required to remove one or more outer header(s) 
 *     from the packets matching this PDR.
 * @farId: The FAR ID to be associated to the PDR.
 * @urrId: The QER IDs to be associated to the PDR. Several IEs within the same IE type
 *     may be present to represent a list of QERs to be associated to the PDR.
 * @qerId: The QER IDs to be associated to the PDR. Several IEs within the same IE type
 *     may be present to represent a list of QERs to be associated to the PDR.
 * @activatePredefinedRules: Octect string contains one Predefined Rules name. Several IEs
 *     with the same IE type may be present to represent multiple "Activate Predefined Rules" names.
 */
typedef struct {
    struct {
        uint16_t seid:1;
        uint16_t pdrId:1;
        uint16_t precedence:1;
        uint16_t pdi:1;
        uint16_t outerHeaderRemoval:1;
        uint16_t farId:1;
        uint16_t urrId:1;
        uint16_t qerId:1;
        uint16_t activatePredefinedRules:1;
        uint16_t deactivatePredefinedRules:1;
        uint16_t spare:7;
    } flags;

    uint64_t seid;

    uint16_t pdrId;
    uint32_t precedence;
    UPDK_PDI      pdi;
    uint8_t  outerHeaderRemoval;
    uint32_t farId;

    // TODO: Need to handle multiple URR
    uint32_t urrId;

    // TODO: Need to handle multiple QER
    uint32_t qerId;

    // TODO: Need to handle multiple ActivatePredefinedRules, dynamic alloc?
    char     activatePredefinedRules[0x40];

    // TODO: Need to handle multiple DeactivatePredefinedRules, dynamic alloc?
    char     deactivatePredefinedRules[0x40];
} UPDK_PDR;

#endif /* __UPDK_RULE_PDR_H__ */