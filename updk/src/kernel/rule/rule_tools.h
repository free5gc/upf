#ifndef __KERNEL_RULE_TOOLS_H__
#define __KERNEL_RULE_TOOLS_H__

/*
 * This file is not necessarily existed.
 * Due to dependency of header files under updk/include/updk,
 * Now the file is used to customize and demo for customized function.
 */

#include "utlt_debug.h"

#include "updk/rule_qer.h"

/**
 * ShowQERIEsInInfoLog - Print information in QER in info level log
 * 
 * @qer: UPDK_QER pointer
 */
#define ShowQERIEsInInfoLog(qer) \
    { \
        if (CheckQERIeIsPresent(qer, qerId)) \
            UTLT_Debug("QER ID: %u", QERGetID(qer)); \
        if (CheckQERIeIsPresent(qer, qerCorrelationId)) \
            UTLT_Debug("QER Correlation ID value: %u", QERGetCorrelationID(qer)); \
        if (CheckQERIeIsPresent(qer, gateStatus)) \
            UTLT_Debug("QER Gate Status UL: %u, DL: %u", QERGetULGate(qer), QERGetDLGate(qer)); \
        if (CheckQERIeIsPresent(qer, maximumBitrate)) \
            UTLT_Debug("QER MBR UL: %u, DL: %u", QERGetMBRUL(qer), QERGetMBRDL(qer)); \
        if (CheckQERIeIsPresent(qer, guaranteedBitrate)) \
            UTLT_Debug("QER GBR UL: %u, DL: %u", QERGetGBRUL(qer), QERGetGBRDL(qer)); \
        if (CheckQERIeIsPresent(qer, packetRate)) { \
            if (QERGetPacketRate(qer).flags.ulpr) \
                UTLT_Debug("QER Packet Rate Uplink Time Unit: %u, Maximum Uplink Packet Rate: %u", \
                        QERGetPacketRateULTimeUnit(qer), QERGetPacketRateMaxUL(qer)); \
            if (QERGetPacketRate(qer).flags.dlpr) \
                UTLT_Debug("QER Packet Rate Downlink Time Unit: %u, Maximum Downlink Packet Rate: %u", \
                        QERGetPacketRateDLTimeUnit(qer), QERGetPacketRateMaxDL(qer)); \
        } \
        if (CheckQERIeIsPresent(qer, dlFlowLevelMarking)) { \
            if (QERGetDLFlowLevelMark(qer).flags.ttc) \
                UTLT_Debug("QER DL Flow Level Marking ToS/Traffic Class: %u", \
                        QERGetDLFlowLevelMarkTTC(qer)); \
            if (QERGetDLFlowLevelMark(qer).flags.sci) \
                UTLT_Debug("QER DL Flow Level Marking Service Class Indicator: %u", \
                        QERGetDLFlowLevelMarkSCI(qer)); \
        } \
        if (CheckQERIeIsPresent(qer, qosFlowIdentifier)) \
            UTLT_Debug("QER QoS Flow Identifier: %u", QERGetQFI(qer)); \
        if (CheckQERIeIsPresent(qer, reflectiveQos)) \
            UTLT_Debug("QER Reflective QoS: %u", QERGetRQI(qer)); \
    }

#endif /* __KERNEL_RULE_TOOLS_H__ */