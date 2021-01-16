#ifndef __UPDK_RULE_QER_H__
#define __UPDK_RULE_QER_H__

#include <stdint.h>
#include <arpa/inet.h>

typedef struct {
    uint64_t ul;    // Only use last 5 byte
    uint64_t dl;    // Only use last 5 byte
} UPDK_Bitrate;

typedef struct {
    struct {
        uint8_t ulpr:1;
        uint8_t dlpr:1;
        uint8_t spare0:6;
    } flags;

    // Presented if ulpr = 1
    uint8_t uplinkTimeUnit:4;
    uint8_t spare1:4;
    uint16_t maximumUplinkPacketRate;

    // Presented if dlpr = 1
    uint8_t downlinkTimeUnit:4;
    uint8_t spare2:4;
    uint16_t maximumDownlinkPacketRate;
} UPDK_PacketRate;

typedef struct {
    struct {
        uint8_t ttc:1;
        uint8_t sci:1;
        uint8_t spare0:6;
    } flags;

    // Presented if ttc = 1
    uint16_t tosTrafficClass;

    // Presented if sci = 1
    uint16_t serviceClassIndicator;
} UPDK_DLFlowLevelMarking;

typedef struct {
    struct {
        uint16_t spare:6;
        uint16_t seid:1;
        uint16_t qerId:1;
        uint16_t qerCorrelationId:1;
        uint16_t gateStatus:1;
        uint16_t maximumBitrate:1;
        uint16_t guaranteedBitrate:1;
        uint16_t packetRate:1;
        uint16_t dlFlowLevelMarking:1;
        uint16_t qosFlowIdentifier:1;
        uint16_t reflectiveQos:1;
    } flags;

    uint64_t seid;

    uint32_t qerId;
    uint32_t qerCorrelationId;


    uint8_t gateStatus;
    // ULGate ((gateStatus & 0x0F) >> 2)
    // DLGate (gateStatus & 0x03)

    UPDK_Bitrate maximumBitrate;

    UPDK_Bitrate guaranteedBitrate;

    UPDK_PacketRate packetRate;

    UPDK_DLFlowLevelMarking dlFlowLevelMarking;

    uint8_t qosFlowIdentifier;
    // QFIValue (qosFlowIdentifier & 0x3F)

    uint8_t reflectiveQos;
    // RQI (reflectiveQos & 0x01)

} UPDK_QER;

/**
 * CheckQERIeIsPresent - Check IE under UPDK_QER is present
 * 
 * @qer: UPDK_QER pointer
 * @ieName: variable name under UPDK_QER
 * @return: 1 or 0 if it is not present
 */
#define CheckQERIeIsPresent(qer, ieName) ((qer)->flags.ieName)

/**
 * QERGet* - Short cut for get QER value
 * 
 * @qer: UPDK_QER pointer
 * @return: value of this IE
 */
#define QERGetID(qer) ((qer)->qerId)

#define QERGetCorrelationID(qer) ((qer)->qerCorrelationId)

#define QERGetGateStatus(qer) ((qer)->gateStatus)
#define QERGetULGate(qer) ((QERGetGateStatus(qer) & 0x0F) >> 2)
#define QERGetDLGate(qer) (QERGetGateStatus(qer) & 0x03)

#define QERGetMBR(qer) ((qer)->maximumBitrate)
#define QERGetMBRUL(qer) (QERGetMBR(qer).ul)
#define QERGetMBRDL(qer) (QERGetMBR(qer).dl)

#define QERGetGBR(qer) ((qer)->guaranteedBitrate)
#define QERGetGBRUL(qer) (QERGetGBR(qer).ul)
#define QERGetGBRDL(qer) (QERGetGBR(qer).dl)

#define QERGetPacketRate(qer) ((qer)->packetRate)
#define QERGetPacketRateULTimeUnit(qer) (QERGetPacketRate(qer).uplinkTimeUnit)
#define QERGetPacketRateMaxUL(qer) (QERGetPacketRate(qer).maximumUplinkPacketRate)
#define QERGetPacketRateDLTimeUnit(qer) (QERGetPacketRate(qer).downlinkTimeUnit)
#define QERGetPacketRateMaxDL(qer) (QERGetPacketRate(qer).maximumDownlinkPacketRate)

#define QERGetDLFlowLevelMark(qer) ((qer)->dlFlowLevelMarking)
#define QERGetDLFlowLevelMarkTTC(qer) (QERGetDLFlowLevelMark(qer).tosTrafficClass)
#define QERGetDLFlowLevelMarkSCI(qer) (QERGetDLFlowLevelMark(qer).serviceClassIndicator)

#define QERGetQFI(qer) ((qer)->qosFlowIdentifier & 0x3F)

#define QERGetRQI(qer) ((qer)->reflectiveQos & 0x01)

#endif /* __UPDK_RULE_QER_H__ */