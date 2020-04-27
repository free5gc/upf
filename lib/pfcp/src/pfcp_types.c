#define TRACE_MODULE _pfcptypes

#include "utlt_3gppTypes.h"
#include "utlt_debug.h"

#include "pfcp_types.h"

char* PfcpCauseGetName(uint8_t cause)
{
    switch(cause)
    {
        case PFCP_CAUSE_REQUEST_ACCEPTED:
            return "PFCP_CAUSE_REQUEST_ACCEPTED";
            break;
        case PFCP_CAUSE_REQUEST_REJECTED:
            return "PFCP_CAUSE_REQUEST_REJECTED";
            break;
        case PFCP_CAUSE_SESSION_CONTEXT_NOT_FOUND:
            return "PFCP_CAUSE_SESSION_CONTEXT_NOT_FOUND";
            break;
        case PFCP_CAUSE_MANDATORY_IE_MISSING:
            return "PFCP_CAUSE_MANDATORY_IE_MISSING";
            break;
        case PFCP_CAUSE_CONDITIONAL_IE_MISSING:
            return "PFCP_CAUSE_CONDITIONAL_IE_MISSING";
            break;
        case PFCP_CAUSE_INVALID_LENGTH:
            return "PFCP_CAUSE_INVALID_LENGTH";
            break;
        case PFCP_CAUSE_MANDATORY_IE_INCORRECT:
            return "PFCP_CAUSE_MANDATORY_IE_INCORRECT";
            break;
        case PFCP_CAUSE_INVALID_FORWARDING_POLICY:
            return "PFCP_CAUSE_INVALID_FORWARDING_POLICY";
            break;
        case PFCP_CAUSE_INVALID_F_TEID_ALLOCATION_OPTION:
            return "PFCP_CAUSE_INVALID_F_TEID_ALLOCATION_OPTION";
            break;
        case PFCP_CAUSE_NO_ESTABLISHED_PFCP_ASSOCIATION:
            return "PFCP_CAUSE_NO_ESTABLISHED_PFCP_ASSOCIATION";
            break;
        case PFCP_CAUSE_RULE_CREATION_MODIFICATION_FAILURE:
            return "PFCP_CAUSE_RULE_CREATION_MODIFICATION_FAILURE";
            break;
        case PFCP_CAUSE_PFCP_ENTITY_IN_CONGESTION:
            return "PFCP_CAUSE_PFCP_ENTITY_IN_CONGESTION";
            break;
        case PFCP_CAUSE_NO_RESOURCES_AVAILABLE:
            return "PFCP_CAUSE_NO_RESOURCES_AVAILABLE";
            break;
        case PFCP_CAUSE_SERVICE_NOT_SUPPORTED:
            return "PFCP_CAUSE_SERVICE_NOT_SUPPORTED";
            break;
        case PFCP_CAUSE_SYSTEM_FAILURE:
            return "PFCP_CAUSE_SYSTEM_FAILURE";
            break;
    }
    return "PFCP_CAUSE_UNKNOWN";
}
