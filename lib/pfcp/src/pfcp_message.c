#define TRACE_MODULE _pfcp_message

#include <endian.h>
#include <string.h>
#include <netinet/in.h>

#include "utlt_debug.h"
#include "utlt_buff.h"

#include "pfcp_message.h"

static IeDescription ieDescriptionTable[] = {\
{0, sizeof(Reserved), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{1, sizeof(CreatePDR), 0, 8, {56, 29, 2, 95, 108, 81, 109, 106, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{2, sizeof(PDI), 0, 13, {20, 21, 22, 93, 131, 23, 24, 142, 132, 124, 153, 154, 155, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{3, sizeof(CreateFAR), 0, 5, {108, 44, 4, 5, 88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{4, sizeof(ForwardingParameters), 0, 9, {42, 22, 38, 84, 30, 41, 98, 131, 137, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{5, sizeof(DuplicatingParameters), 0, 4, {42, 84, 30, 41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{6, sizeof(CreateURR), 0, 24, {81, 62, 37, 64, 31, 73, 32, 74, 71, 72, 33, 148, 34, 35, 121, 122, 36, 82, 100, 115, 118, 108, 146, 147, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{7, sizeof(CreateQER), 0, 9, {109, 28, 25, 26, 27, 94, 97, 124, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{8, sizeof(CreatedPDR), 0, 2, {56, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{9, sizeof(UpdatePDR), 0, 9, {56, 95, 29, 2, 108, 81, 109, 106, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{10, sizeof(UpdateFAR), 0, 5, {108, 44, 11, 105, 88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{11, sizeof(UpdateForwardingParameters), 0, 9, {42, 22, 38, 84, 30, 41, 98, 49, 131, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{12, sizeof(UpdateBARPFCPSessionReportResponse), 0, 5, {88, 46, 47, 48, 140, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{13, sizeof(UpdateURR), 0, 24, {81, 62, 37, 64, 31, 73, 32, 74, 71, 72, 33, 148, 34, 35, 121, 122, 36, 82, 100, 115, 118, 108, 146, 147, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{14, sizeof(UpdateQER), 0, 9, {109, 28, 25, 26, 27, 94, 97, 124, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{15, sizeof(RemovePDR), 0, 1, {56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{16, sizeof(RemoveFAR), 0, 1, {108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{17, sizeof(RemoveURR), 0, 1, {81, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{18, sizeof(RemoveQER), 0, 1, {109, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{19, sizeof(Cause), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{20, sizeof(SourceInterface), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{21, sizeof(FTEID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{22, sizeof(NetworkInstance), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{23, sizeof(SDFFilter), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{24, sizeof(ApplicationID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{25, sizeof(GateStatus), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{26, sizeof(MBR), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{27, sizeof(GBR), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{28, sizeof(QERCorrelationID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{29, sizeof(Precedence), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{30, sizeof(TransportLevelMarking), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{31, sizeof(VolumeThreshold), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{32, sizeof(TimeThreshold), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{33, sizeof(MonitoringTime), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{34, sizeof(SubsequentVolumeThreshold), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{35, sizeof(SubsequentTimeThreshold), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{36, sizeof(InactivityDetectionTime), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{37, sizeof(ReportingTriggers), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{38, sizeof(RedirectInformation), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{39, sizeof(ReportType), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{40, sizeof(OffendingIE), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{41, sizeof(ForwardingPolicy), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{42, sizeof(DestinationInterface), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{43, sizeof(UPFunctionFeatures), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{44, sizeof(ApplyAction), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{45, sizeof(DownlinkDataServiceInformation), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{46, sizeof(DownlinkDataNotificationDelay), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{47, sizeof(DLBufferingDuration), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{48, sizeof(DLBufferingSuggestedPacketCount), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{49, sizeof(PFCPSMReqFlags), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{50, sizeof(PFCPSRRspFlags), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{51, sizeof(LoadControlInformation), 0, 2, {52, 53, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{52, sizeof(SequenceNumber), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{53, sizeof(Metric), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{54, sizeof(OverloadControlInformation), 0, 4, {52, 53, 55, 110, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{55, sizeof(Timer), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{56, sizeof(PacketDetectionRuleID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{57, sizeof(FSEID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{58, sizeof(ApplicationIDsPFDs), 0, 2, {24, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{59, sizeof(PFDContext), 0, 1, {61, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{60, sizeof(NodeID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{61, sizeof(PFDContents), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{62, sizeof(MeasurementMethod), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{63, sizeof(UsageReportTrigger), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{64, sizeof(MeasurementPeriod), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{65, sizeof(FQCSID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{66, sizeof(VolumeMeasurement), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{67, sizeof(DurationMeasurement), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{68, sizeof(ApplicationDetectionInformation), 0, 3, {24, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{69, sizeof(TimeOfFirstPacket), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{70, sizeof(TimeOfLastPacket), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{71, sizeof(QuotaHoldingTime), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{72, sizeof(DroppedDLTrafficThreshold), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{73, sizeof(VolumeQuota), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{74, sizeof(TimeQuota), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{75, sizeof(StartTime), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{76, sizeof(EndTime), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{77, sizeof(QueryURR), 0, 1, {81, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{78, sizeof(UsageReportPFCPSessionModificationResponse), 0, 12, {81, 104, 63, 75, 76, 66, 67, 69, 70, 90, 125, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{79, sizeof(UsageReportPFCPSessionDeletionResponse), 0, 11, {81, 104, 63, 75, 76, 66, 67, 69, 70, 90, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{80, sizeof(UsageReportPFCPSessionReportRequest), 0, 16, {81, 104, 63, 75, 76, 66, 67, 68, 93, 22, 69, 70, 90, 125, 149, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{81, sizeof(URRID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{82, sizeof(LinkedURRID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{83, sizeof(DownlinkDataReport), 0, 2, {56, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{84, sizeof(OuterHeaderCreation), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{85, sizeof(CreateBAR), 0, 3, {88, 46, 140, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{86, sizeof(UpdateBARPFCPSessionModificationRequest), 0, 3, {88, 46, 140, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{87, sizeof(RemoveBAR), 0, 1, {88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{88, sizeof(BARID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{89, sizeof(CPFunctionFeatures), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{90, sizeof(UsageInformation), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{91, sizeof(ApplicationInstanceID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{92, sizeof(FlowInformation), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{93, sizeof(UEIPAddress), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{94, sizeof(PacketRate), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{95, sizeof(OuterHeaderRemoval), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{96, sizeof(RecoveryTimeStamp), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{97, sizeof(DLFlowLevelMarking), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{98, sizeof(HeaderEnrichment), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{99, sizeof(ErrorIndicationReport), 0, 1, {21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{100, sizeof(MeasurementInformation), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{101, sizeof(NodeReportType), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{102, sizeof(UserPlanePathFailureReport), 0, 1, {103, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{103, sizeof(RemoteGTPUPeer), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{104, sizeof(URSEQN), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{105, sizeof(UpdateDuplicatingParameters), 0, 4, {42, 84, 30, 41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{106, sizeof(ActivatePredefinedRules), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{107, sizeof(DeactivatePredefinedRules), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{108, sizeof(FARID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{109, sizeof(QERID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{110, sizeof(OCIFlags), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{111, sizeof(PFCPAssociationReleaseRequest), 0, 1, {60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{112, sizeof(GracefulReleasePeriod), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{113, sizeof(PDNType), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{114, sizeof(FailedRuleID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{115, sizeof(TimeQuotaMechanism), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{116, sizeof(UserPlaneIPResourceInformation), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{117, sizeof(UserPlaneInactivityTimer), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{118, sizeof(AggregatedURRs), 0, 2, {120, 119, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{119, sizeof(Multiplier), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{120, sizeof(AggregatedURRID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{121, sizeof(SubsequentVolumeQuota), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{122, sizeof(SubsequentTimeQuota), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{123, sizeof(RQI), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{124, sizeof(QFI), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{125, sizeof(QueryURRReference), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{126, sizeof(AdditionalUsageReportsInformation), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{127, sizeof(CreateTrafficEndpoint), 0, 8, {131, 21, 22, 93, 142, 153, 154, 155, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{128, sizeof(CreatedTrafficEndpoint), 0, 2, {131, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{129, sizeof(UpdateTrafficEndpoint), 0, 7, {131, 21, 22, 93, 153, 154, 155, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{130, sizeof(RemoveTrafficEndpoint), 0, 1, {131, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{131, sizeof(TrafficEndpointID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{132, sizeof(EthernetPacketFilter), 0, 7, {138, 139, 133, 136, 134, 135, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{133, sizeof(MACAddress), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{134, sizeof(CTAG), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{135, sizeof(STAG), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{136, sizeof(Ethertype), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{137, sizeof(Proxying), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{138, sizeof(EthernetFilterID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{139, sizeof(EthernetFilterProperties), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{140, sizeof(SuggestedBufferingPacketsCount), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{141, sizeof(UserID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{142, sizeof(EthernetPDUSessionInformation), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{143, sizeof(EthernetTrafficInformation), 0, 2, {144, 145, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{144, sizeof(MACAddressesDetected), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{145, sizeof(MACAddressesRemoved), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{146, sizeof(EthernetInactivityTimer), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{147, sizeof(AdditionalMonitoringTime), 0, 5, {33, 34, 35, 121, 122, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{148, sizeof(EventInformation), 0, 2, {150, 151, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{149, sizeof(EventReporting), 0, 1, {150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{150, sizeof(EventID), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{151, sizeof(EventThreshold), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{152, sizeof(TraceInformation), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{153, sizeof(FramedRoute), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{154, sizeof(FramedRouting), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{155, sizeof(FramedIPv6Route), 1, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(HeartbeatRequest), 0, 1, {96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(HeartbeatResponse), 0, 1, {96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPPFDManagementRequest), 0, 1, {58, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPPFDManagementResponse), 0, 2, {19, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPAssociationSetupRequest), 0, 5, {60, 96, 43, 89, 116, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPAssociationSetupResponse), 0, 6, {60, 19, 96, 43, 89, 116, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPAssociationUpdateRequest), 0, 6, {60, 43, 89, 111, 112, 116, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPAssociationUpdateResponse), 0, 4, {60, 19, 43, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPAssociationReleaseResponse), 0, 2, {60, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPNodeReportRequest), 0, 3, {60, 101, 102, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPNodeReportResponse), 0, 3, {60, 19, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPSessionSetDeletionRequest), 0, 8, {60, 65, 65, 65, 65, 65, 65, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPSessionSetDeletionResponse), 0, 3, {60, 19, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPSessionEstablishmentRequest), 0, 19, {60, 57, 1, 1, 3, 3, 6, 7, 85, 127, 113, 65, 65, 65, 65, 65, 117, 141, 152, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPSessionEstablishmentResponse), 0, 11, {60, 19, 40, 57, 8, 51, 54, 65, 65, 114, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPSessionModificationRequest), 0, 31, {57, 15, 16, 17, 18, 87, 130, 1, 1, 3, 3, 6, 7, 85, 127, 9, 10, 13, 14, 86, 129, 49, 77, 65, 65, 65, 65, 65, 117, 125, 152, 0, 0, 0, 0}}, \
{0, sizeof(PFCPSessionModificationResponse), 0, 9, {19, 40, 8, 51, 54, 78, 114, 126, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPSessionDeletionRequest), 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPSessionDeletionResponse), 0, 5, {19, 40, 51, 54, 79, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPSessionReportRequest), 0, 7, {39, 83, 80, 99, 51, 54, 126, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
{0, sizeof(PFCPSessionReportResponse), 0, 4, {19, 40, 12, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, \
};

_Bool dbf = 0;

int _TlvParseMessage(void * msg, IeDescription * msgDes, void * buff, int buffLen) {
    int msgPivot = 0; // msg (struct) offset
    //void *root = buff;
    int buffOffset = 0; // buff offset
    int idx;
    for (idx = 0; idx < msgDes->numToParse; ++idx) {
        if (dbf) { if (ieDescriptionTable[msgDes->next[idx]].msgType == 57) {
                UTLT_Warning("Get F-SEID");
            } }
        IeDescription *ieDes = &ieDescriptionTable[msgDes->next[idx]];
        uint16_t type;
        uint16_t length;
        memcpy(&type, buff + buffOffset, sizeof(uint16_t));
        memcpy(&length, buff + buffOffset + sizeof(uint16_t), sizeof(uint16_t));
        type = ntohs(type);
        length = ntohs(length);
        if (dbf) { UTLT_Info("type: %d, len: %d", type, length); }
        if (type != ieDes->msgType) {
            if (dbf) { UTLT_Warning("%d not present, type: %d", ieDes->msgType, type); }
            // not present
            (*(unsigned long*)(msg + msgPivot)) = 0; // presence
            msgPivot += ieDes->msgLen;
            continue;
        }

        if (ieDes->isTlvObj) {
            if (dbf) { UTLT_Info("is TLV: %p", msg+msgPivot); }
            ((TlvOctet*)(msg+msgPivot))->presence = 1;
            ((TlvOctet*)(msg+msgPivot))->type = type;
            void *newBuf = UTLT_Malloc(length);
            memcpy(newBuf, buff + buffOffset + 2*sizeof(uint16_t), length);
            ((TlvOctet*)(msg+msgPivot))->len = length;
            ((TlvOctet*)(msg+msgPivot))->value = newBuf;
            buffOffset += sizeof(uint16_t)*2 + length;
            msgPivot += sizeof(TlvOctet);
            continue;
        } else {
            if (dbf) { UTLT_Info("not TLV, desTB mstype: %d", ieDes->msgType); }
            // recursive
            *((unsigned long*)(msg+msgPivot)) = 1; // presence
            _TlvParseMessage(msg+msgPivot+sizeof(unsigned long), ieDes, buff + buffOffset + sizeof(uint16_t)*2, buffLen - buffOffset);
            //int size = _TlvParseMessage(msg+msgPivot, ieDes, buff + buffOffset, buffLen - buffOffset);
            buffOffset += length + sizeof(uint16_t)*2;
            msgPivot += ieDes->msgLen;
        }
    }
    return buffOffset;
}

Status PfcpParseMessage(PfcpMessage *pfcpMessage, Bufblk *bufBlk) {
    Status status = STATUS_OK;
    PfcpHeader *header = NULL;
    uint16_t size = 0;
    void *body = NULL;
    uint16_t bodyLen = 0;

    UTLT_Assert(pfcpMessage, return STATUS_ERROR, "Message error");
    UTLT_Assert(bufBlk, return STATUS_ERROR, "buffer error");
    UTLT_Assert(bufBlk->buf, return STATUS_ERROR, "buffer payload error");

    header = bufBlk->buf;
    UTLT_Assert(header, return STATUS_ERROR, "header hasn't get pointer");

    memset(pfcpMessage, 0, sizeof(PfcpMessage)); // clear pfcpMessage

    if (header->seidP) {
        size = PFCP_HEADER_LEN;
    } else {
        size = PFCP_HEADER_LEN - PFCP_SEID_LEN;
    }

    memcpy(&pfcpMessage->header, bufBlk->buf, size);
    body = bufBlk->buf + size;
    bodyLen = bufBlk->len - size;

    if (header->seidP) {
        pfcpMessage->header.seid = be64toh(pfcpMessage->header.seid);
    } else { // not sure what is this for
        pfcpMessage->header.sqn = pfcpMessage->header.sqn_only;
        pfcpMessage->header.sqn_only = pfcpMessage->header.sqn_only;
    }

    if (bodyLen == 0) {
        return STATUS_OK;
    }

    switch(pfcpMessage->header.type) {
        case PFCP_HEARTBEAT_REQUEST:
            pfcpMessage->heartbeatRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->heartbeatRequest + 1, &ieDescriptionTable[PFCP_HEARTBEAT_REQUEST + 155], body, bodyLen);
            break;
        case PFCP_HEARTBEAT_RESPONSE:
            pfcpMessage->heartbeatResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->heartbeatResponse + 1, &ieDescriptionTable[PFCP_HEARTBEAT_RESPONSE + 155], body, bodyLen);
            break;
        case PFCPPFD_MANAGEMENT_REQUEST:
            pfcpMessage->pFCPPFDManagementRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPPFDManagementRequest + 1, &ieDescriptionTable[PFCPPFD_MANAGEMENT_REQUEST + 155], body, bodyLen);
            break;
        case PFCPPFD_MANAGEMENT_RESPONSE:
            pfcpMessage->pFCPPFDManagementResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPPFDManagementResponse + 1, &ieDescriptionTable[PFCPPFD_MANAGEMENT_RESPONSE + 155], body, bodyLen);
            break;
        case PFCP_ASSOCIATION_SETUP_REQUEST:
            pfcpMessage->pFCPAssociationSetupRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPAssociationSetupRequest + 1, &ieDescriptionTable[PFCP_ASSOCIATION_SETUP_REQUEST+155], body, bodyLen);
            break;
        case PFCP_ASSOCIATION_SETUP_RESPONSE:
            pfcpMessage->pFCPAssociationSetupResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPAssociationSetupResponse + 1, &ieDescriptionTable[PFCP_ASSOCIATION_SETUP_RESPONSE+155], body, bodyLen);
            break;
        case PFCP_ASSOCIATION_UPDATE_REQUEST:
            pfcpMessage->pFCPAssociationUpdateRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPAssociationUpdateRequest + 1, &ieDescriptionTable[PFCP_ASSOCIATION_UPDATE_REQUEST+155], body, bodyLen);
            break;
        case PFCP_ASSOCIATION_UPDATE_RESPONSE:
            pfcpMessage->pFCPAssociationUpdateResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPAssociationUpdateResponse + 1, &ieDescriptionTable[PFCP_ASSOCIATION_UPDATE_RESPONSE+155], body, bodyLen);
            break;
        case PFCP_ASSOCIATION_RELEASE_REQUEST:
            pfcpMessage->pFCPAssociationReleaseRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPAssociationReleaseRequest + 1, &ieDescriptionTable[PFCP_ASSOCIATION_RELEASE_REQUEST+155], body, bodyLen);
            break;
        case PFCP_ASSOCIATION_RELEASE_RESPONSE:
            pfcpMessage->pFCPAssociationReleaseResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPAssociationReleaseResponse + 1, &ieDescriptionTable[PFCP_ASSOCIATION_RELEASE_RESPONSE+155], body, bodyLen);
            break;
        case PFCP_VERSION_NOT_SUPPORTED_RESPONSE:
            break;
        case PFCP_NODE_REPORT_REQUEST:
            pfcpMessage->pFCPNodeReportRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPNodeReportRequest + 1, &ieDescriptionTable[PFCP_NODE_REPORT_REQUEST + 155 - 1], body, bodyLen);
            break;
        case PFCP_NODE_REPORT_RESPONSE:
            pfcpMessage->pFCPNodeReportResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPNodeReportResponse + 1, &ieDescriptionTable[PFCP_NODE_REPORT_RESPONSE + 155 - 1], body, bodyLen);
            break;
        case PFCP_SESSION_SET_DELETION_REQUEST:
            pfcpMessage->pFCPSessionSetDeletionRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPSessionSetDeletionRequest + 1, &ieDescriptionTable[PFCP_SESSION_SET_DELETION_REQUEST + 155 - 1], body, bodyLen);
            break;
        case PFCP_SESSION_SET_DELETION_RESPONSE:
            pfcpMessage->pFCPSessionSetDeletionResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPSessionSetDeletionResponse + 1, &ieDescriptionTable[PFCP_SESSION_SET_DELETION_RESPONSE + 155 - 1], body, bodyLen);
            break;
        case PFCP_SESSION_ESTABLISHMENT_REQUEST:
            pfcpMessage->pFCPSessionEstablishmentRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPSessionEstablishmentRequest + 1, &ieDescriptionTable[PFCP_SESSION_ESTABLISHMENT_REQUEST + 155 - (50-15) - 1], body, bodyLen);
            break;
        case PFCP_SESSION_ESTABLISHMENT_RESPONSE:
            pfcpMessage->pFCPSessionEstablishmentResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPSessionEstablishmentResponse + 1, &ieDescriptionTable[PFCP_SESSION_ESTABLISHMENT_RESPONSE + 155 - (50-15) - 1], body, bodyLen);
            break;
        case PFCP_SESSION_MODIFICATION_REQUEST:
            pfcpMessage->pFCPSessionModificationRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPSessionModificationRequest + 1, &ieDescriptionTable[PFCP_SESSION_MODIFICATION_REQUEST + 155 - (50-15) - 1], body, bodyLen);
            break;
        case PFCP_SESSION_MODIFICATION_RESPONSE:
            pfcpMessage->pFCPSessionModificationResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPSessionModificationResponse + 1, &ieDescriptionTable[PFCP_SESSION_MODIFICATION_RESPONSE + 155 - (50-15) - 1], body, bodyLen);
            break;
        case PFCP_SESSION_DELETION_REQUEST:
            pfcpMessage->pFCPSessionDeletionRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPSessionDeletionRequest + 1, &ieDescriptionTable[PFCP_SESSION_DELETION_REQUEST + 155 - (50-15) - 1], body, bodyLen);
            break;
        case PFCP_SESSION_DELETION_RESPONSE:
            pfcpMessage->pFCPSessionDeletionResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPSessionDeletionResponse + 1, &ieDescriptionTable[PFCP_SESSION_DELETION_RESPONSE + 155 - (50-15) - 1], body, bodyLen);
            break;
        case PFCP_SESSION_REPORT_REQUEST:
            pfcpMessage->pFCPSessionReportRequest.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPSessionReportRequest + 1, &ieDescriptionTable[PFCP_SESSION_REPORT_REQUEST + 155 - (50-15) - 1], body, bodyLen);
            break;
        case PFCP_SESSION_REPORT_RESPONSE:
            pfcpMessage->pFCPSessionReportResponse.presence = 1;
            _TlvParseMessage((unsigned long *)&pfcpMessage->pFCPSessionReportResponse + 1, &ieDescriptionTable[PFCP_SESSION_REPORT_RESPONSE + 155 - (50-15) - 1], body, bodyLen);
            break;
        default:
            UTLT_Warning("Not implmented(type:%d)", &pfcpMessage->header.type);
    }

    return status;
}

int _TlvBuildMessage(Bufblk **bufBlkPtr, void *msg, IeDescription *ieDescription) {
    //UTLT_Warning("Addr : %p", msg);
    UTLT_Assert(bufBlkPtr, return 0, "buffer error");
    UTLT_Assert(msg, return 0, "message error");
    if (*(unsigned long *)msg == 0) {
        // present bit
        //UTLT_Warning("no ie");
        return 0;
    }

    if (ieDescription->isTlvObj) {
      if (dbf) { UTLT_Info("TLV: type: %d, %d, len: %d, presence: %d",
                           ieDescription->msgType, ((TlvOctet*)msg)->type, ((TlvOctet *)msg)->len, ((unsigned long*)msg)[0]); }
        //UTLT_Info("msgType: %d, msgLen: %d", ieDescription->msgType, ((TlvOctet *)msg)->len);
        int buffLen = sizeof(uint16_t) * 2 + ((TlvOctet *)msg)->len;
        *bufBlkPtr = BufblkAlloc(1, buffLen);
        uint16_t *tagPtr = (uint16_t *) ((*bufBlkPtr)->buf);
        uint16_t *lenPtr = &tagPtr[1];

        (*bufBlkPtr)->len = buffLen;
        *tagPtr = htons(ieDescription->msgType);
        *lenPtr = htons(buffLen - sizeof(uint16_t) * 2);
        memcpy((void *) &tagPtr[2], ((TlvOctet *)msg)->value, ((TlvOctet *)msg)->len);
    } else {
      if (dbf) { UTLT_Info("not TLV"); }
        size_t idx;
        int msgPivot = 0;
        *bufBlkPtr = BufblkAlloc(1, sizeof(uint16_t) * 2);
        uint16_t *tagPtr = (*bufBlkPtr)->buf;
        uint16_t *lenPtr = &tagPtr[1];
        (*bufBlkPtr)->len = sizeof(uint16_t) * 2;

        *tagPtr = htons(ieDescription->msgType);
        if (dbf) { UTLT_Warning("Check addr: tag: %p, buf: %p", tagPtr, (*bufBlkPtr)->buf); }
        if (dbf) { UTLT_Info("msgType: %u, tagPtr value: %u, first type: %u", ieDescription->msgType, ((uint16_t*)tagPtr)[0],ntohs(((uint16_t*)(*bufBlkPtr)->buf)[0])); }
        *lenPtr = htons(0);

        int bufOffset = 0;
        void *msgNoPresentPtr = &((unsigned long*)msg)[1];
        for (idx = 0; idx < ieDescription->numToParse; ++idx) {
            Bufblk *tmpBufBlkPtr = NULL;
            bufOffset += _TlvBuildMessage(&tmpBufBlkPtr, &((uint8_t *)msgNoPresentPtr)[msgPivot], &ieDescriptionTable[ieDescription->next[idx]]);
            if (tmpBufBlkPtr == NULL) {
                msgPivot += ieDescriptionTable[ieDescription->next[idx]].msgLen;
                //UTLT_Info("TL type[%d], pivot %d", ieDescriptionTable[ieDescription->next[idx]].msgType, msgPivot);
                continue;
            }
            if (dbf) {
                    UTLT_Warning("tmpBuf T: %u, L: %d", ntohs(((uint16_t *)tmpBufBlkPtr->buf)[0]), ntohs(((uint16_t *)tmpBufBlkPtr->buf)[1]));
            }
            BufblkBuf(*bufBlkPtr, tmpBufBlkPtr);
            //UTLT_Warning("bufBlk len %d", (*bufBlkPtr)->buf);
            BufblkFree(tmpBufBlkPtr);
            msgPivot += ieDescriptionTable[ieDescription->next[idx]].msgLen;
            if (dbf) { UTLT_Info("buff offset: %d, buff Len: %d", bufOffset, (*bufBlkPtr)->len); }
        }

        *lenPtr = htons(bufOffset);
    }

    //UTLT_Warning("buf len: %d, first type: %d", (*bufBlkPtr)->len, ((uint16_t*)(*bufBlkPtr)->buf)[0]);
    return (*bufBlkPtr)->len;
}

void _PfcpBuildBody(Bufblk **bufBlkPtr, void *msg, IeDescription *ieDescription) {
    UTLT_Assert(bufBlkPtr, return, "buffer error");
    UTLT_Assert(msg, return, "message error");

    int idx;
    void *root = msg + sizeof(unsigned long);
    (*bufBlkPtr) = BufblkAlloc(1, 0);
    for (idx = 0; idx < ieDescription->numToParse; ++idx) {
        Bufblk *tmpBufBlkPtr;
        int rt = _TlvBuildMessage(&tmpBufBlkPtr, root, &ieDescriptionTable[ieDescription->next[idx]]);
        if (rt == 0) {
            root += ieDescriptionTable[ieDescription->next[idx]].msgLen;
            continue;
        }
        BufblkBuf(*bufBlkPtr, tmpBufBlkPtr);
        BufblkFree(tmpBufBlkPtr);
        root += ieDescriptionTable[ieDescription->next[idx]].msgLen;
    }
}

Status PfcpBuildMessage(Bufblk **bufBlkPtr, PfcpMessage *pfcpMessage) {
    Status status = STATUS_OK;
    UTLT_Assert(pfcpMessage, return STATUS_ERROR, "pfcpMessage error");

    switch(pfcpMessage->header.type) {
        case PFCP_HEARTBEAT_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->heartbeatRequest, &ieDescriptionTable[PFCP_HEARTBEAT_REQUEST + 155]);
            break;
        case PFCP_HEARTBEAT_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->heartbeatResponse, &ieDescriptionTable[PFCP_HEARTBEAT_RESPONSE + 155]);
            break;
        case PFCPPFD_MANAGEMENT_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPPFDManagementRequest, &ieDescriptionTable[PFCPPFD_MANAGEMENT_REQUEST + 155]);
            break;
        case PFCPPFD_MANAGEMENT_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPPFDManagementResponse, &ieDescriptionTable[PFCPPFD_MANAGEMENT_RESPONSE + 155]);
            break;
        case PFCP_ASSOCIATION_SETUP_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPAssociationSetupRequest, &ieDescriptionTable[PFCP_ASSOCIATION_SETUP_REQUEST + 155]);
            break;
        case PFCP_ASSOCIATION_SETUP_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPAssociationSetupResponse, &ieDescriptionTable[PFCP_ASSOCIATION_SETUP_RESPONSE + 155]);
            break;
        case PFCP_ASSOCIATION_UPDATE_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPAssociationUpdateRequest, &ieDescriptionTable[PFCP_ASSOCIATION_UPDATE_REQUEST + 155]);
            break;
        case PFCP_ASSOCIATION_UPDATE_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPAssociationUpdateResponse, &ieDescriptionTable[PFCP_ASSOCIATION_UPDATE_RESPONSE + 155]);
            break;
        case PFCP_ASSOCIATION_RELEASE_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPAssociationReleaseRequest, &ieDescriptionTable[PFCP_ASSOCIATION_RELEASE_REQUEST + 155]);
            break;
        case PFCP_ASSOCIATION_RELEASE_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPAssociationReleaseResponse, &ieDescriptionTable[PFCP_ASSOCIATION_RELEASE_RESPONSE + 155]);
            break;
        case PFCP_VERSION_NOT_SUPPORTED_RESPONSE:
            break;
        case PFCP_NODE_REPORT_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPNodeReportRequest, &ieDescriptionTable[PFCP_NODE_REPORT_REQUEST + 155 - 1]);
            break;
        case PFCP_NODE_REPORT_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPNodeReportResponse, &ieDescriptionTable[PFCP_NODE_REPORT_RESPONSE + 155 - 1]);
            break;
        case PFCP_SESSION_SET_DELETION_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPSessionSetDeletionRequest, &ieDescriptionTable[PFCP_SESSION_SET_DELETION_REQUEST + 155 - 1]);
            break;
        case PFCP_SESSION_SET_DELETION_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPSessionSetDeletionResponse, &ieDescriptionTable[PFCP_SESSION_SET_DELETION_RESPONSE + 155 - 1]);
            break;
        case PFCP_SESSION_ESTABLISHMENT_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPSessionEstablishmentRequest, &ieDescriptionTable[PFCP_SESSION_ESTABLISHMENT_REQUEST + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_ESTABLISHMENT_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPSessionEstablishmentResponse, &ieDescriptionTable[PFCP_SESSION_ESTABLISHMENT_RESPONSE + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_MODIFICATION_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPSessionModificationRequest, &ieDescriptionTable[PFCP_SESSION_MODIFICATION_REQUEST + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_MODIFICATION_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPSessionModificationResponse, &ieDescriptionTable[PFCP_SESSION_MODIFICATION_RESPONSE + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_DELETION_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPSessionDeletionRequest, &ieDescriptionTable[PFCP_SESSION_DELETION_REQUEST + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_DELETION_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPSessionDeletionResponse, &ieDescriptionTable[PFCP_SESSION_DELETION_RESPONSE + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_REPORT_REQUEST:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPSessionReportRequest, &ieDescriptionTable[PFCP_SESSION_REPORT_REQUEST + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_REPORT_RESPONSE:
            _PfcpBuildBody(bufBlkPtr, &pfcpMessage->pFCPSessionReportResponse, &ieDescriptionTable[PFCP_SESSION_REPORT_RESPONSE + 155 - (50-15) - 1]);
            break;
        default:
            UTLT_Warning("Not implmented(type:%d)", &pfcpMessage->header.type);
    }
    return status;
}

Status _PfcpFreeIe(void *msg, IeDescription *ieDescription) {
    UTLT_Assert(msg, return STATUS_ERROR, "message error");
    if (((unsigned long*)msg)[0] == 0) {
        // check present
        return STATUS_OK;
    }
    Status status = STATUS_OK;

    if (ieDescription->isTlvObj) {
        status = UTLT_Free(((TlvOctet*)msg)->value);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Free pfcp ie error");
        return STATUS_OK;
    }

    int idx = 0;
    void *root = msg + sizeof(unsigned long);
    for (idx = 0; idx < ieDescription->numToParse; ++idx) {
        status = _PfcpFreeIe(root, &ieDescriptionTable[ieDescription->next[idx]]);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "PFCP free error");

        root += ieDescriptionTable[ieDescription->next[idx]].msgLen;
    }

    return status;
}

Status PfcpStructFree(PfcpMessage *pfcpMessage) {
    Status status = STATUS_OK;
    UTLT_Assert(pfcpMessage, return STATUS_ERROR, "pfcpMessage error");

    switch(pfcpMessage->header.type) {
        case PFCP_HEARTBEAT_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->heartbeatRequest, &ieDescriptionTable[PFCP_HEARTBEAT_REQUEST + 155]);
            break;
        case PFCP_HEARTBEAT_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->heartbeatResponse, &ieDescriptionTable[PFCP_HEARTBEAT_RESPONSE + 155]);
            break;
        case PFCPPFD_MANAGEMENT_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->pFCPPFDManagementRequest, &ieDescriptionTable[PFCPPFD_MANAGEMENT_REQUEST + 155]);
            break;
        case PFCPPFD_MANAGEMENT_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->pFCPPFDManagementResponse, &ieDescriptionTable[PFCPPFD_MANAGEMENT_RESPONSE + 155]);
            break;
        case PFCP_ASSOCIATION_SETUP_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->pFCPAssociationSetupRequest, &ieDescriptionTable[PFCP_ASSOCIATION_SETUP_REQUEST + 155]);
            break;
        case PFCP_ASSOCIATION_SETUP_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->pFCPAssociationSetupResponse, &ieDescriptionTable[PFCP_ASSOCIATION_SETUP_RESPONSE + 155]);
            break;
        case PFCP_ASSOCIATION_UPDATE_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->pFCPAssociationUpdateRequest, &ieDescriptionTable[PFCP_ASSOCIATION_UPDATE_REQUEST + 155]);
            break;
        case PFCP_ASSOCIATION_UPDATE_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->pFCPAssociationUpdateResponse, &ieDescriptionTable[PFCP_ASSOCIATION_UPDATE_RESPONSE + 155]);
            break;
        case PFCP_ASSOCIATION_RELEASE_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->pFCPAssociationReleaseRequest, &ieDescriptionTable[PFCP_ASSOCIATION_RELEASE_REQUEST + 155]);
            break;
        case PFCP_ASSOCIATION_RELEASE_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->pFCPAssociationReleaseResponse, &ieDescriptionTable[PFCP_ASSOCIATION_RELEASE_RESPONSE + 155]);
            break;
        case PFCP_VERSION_NOT_SUPPORTED_RESPONSE:
            break;
        case PFCP_NODE_REPORT_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->pFCPNodeReportRequest, &ieDescriptionTable[PFCP_NODE_REPORT_REQUEST + 155 - 1]);
            break;
        case PFCP_NODE_REPORT_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->pFCPNodeReportResponse, &ieDescriptionTable[PFCP_NODE_REPORT_RESPONSE + 155 - 1]);
            break;
        case PFCP_SESSION_SET_DELETION_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->pFCPSessionSetDeletionRequest, &ieDescriptionTable[PFCP_SESSION_SET_DELETION_REQUEST + 155 - 1]);
            break;
        case PFCP_SESSION_SET_DELETION_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->pFCPSessionSetDeletionResponse, &ieDescriptionTable[PFCP_SESSION_SET_DELETION_RESPONSE + 155 - 1]);
            break;
        case PFCP_SESSION_ESTABLISHMENT_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->pFCPSessionEstablishmentRequest, &ieDescriptionTable[PFCP_SESSION_ESTABLISHMENT_REQUEST + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_ESTABLISHMENT_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->pFCPSessionEstablishmentResponse, &ieDescriptionTable[PFCP_SESSION_ESTABLISHMENT_RESPONSE + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_MODIFICATION_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->pFCPSessionModificationRequest, &ieDescriptionTable[PFCP_SESSION_MODIFICATION_REQUEST + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_MODIFICATION_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->pFCPSessionModificationResponse, &ieDescriptionTable[PFCP_SESSION_MODIFICATION_RESPONSE + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_DELETION_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->pFCPSessionDeletionRequest, &ieDescriptionTable[PFCP_SESSION_DELETION_REQUEST + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_DELETION_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->pFCPSessionDeletionResponse, &ieDescriptionTable[PFCP_SESSION_DELETION_RESPONSE + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_REPORT_REQUEST:
            status = _PfcpFreeIe(&pfcpMessage->pFCPSessionReportRequest, &ieDescriptionTable[PFCP_SESSION_REPORT_REQUEST + 155 - (50-15) - 1]);
            break;
        case PFCP_SESSION_REPORT_RESPONSE:
            status = _PfcpFreeIe(&pfcpMessage->pFCPSessionReportResponse, &ieDescriptionTable[PFCP_SESSION_REPORT_RESPONSE + 155 - (50-15) - 1]);
            break;
        default:
            UTLT_Warning("Not implmented(type:%d)", &pfcpMessage->header.type);
  }

  return status;
}
