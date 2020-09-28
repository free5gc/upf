#ifndef __N4_PFCP_HANDLER_H__
#define __N4_PFCP_HANDLER_H__

#include "upf_context.h"
#include "pfcp_message.h"
#include "pfcp_xact.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void UpfN4HandleCreatePdr(UpfSession *session, CreatePDR *createPdr);
void UpfN4HandleCreateFar(UpfSession *session, CreateFAR *createFar);
void UpfN4HandleUpdatePdr(UpfSession *session, UpdatePDR *updatePdr);
void UpfN4HandleUpdateFar(UpfSession *session, UpdateFAR *updateFar);
Status UpfN4HandleRemovePdr(UpfSession *session, uint16_t nPDRID);
Status UpfN4HandleRemoveFar(UpfSession *session, uint32_t nFARID);
void UpfN4HandleSessionEstablishmentRequest(
        UpfSession *session, PfcpXact *pfcpXact, PFCPSessionEstablishmentRequest *request);
void UpfN4HandleSessionModificationRequest(
        UpfSession *session, PfcpXact *xact, PFCPSessionModificationRequest *request);
void UpfN4HandleSessionDeletionRequest(UpfSession *session, PfcpXact *xact, PFCPSessionDeletionRequest *request);
void UpfN4HandleSessionReportResponse(
        UpfSession *session, PfcpXact *xact, PFCPSessionReportResponse *response);
void UpfN4HandleAssociationSetupRequest(PfcpXact *xact, PFCPAssociationSetupRequest *request);
void UpfN4HandleAssociationUpdateRequest(PfcpXact *xact, PFCPAssociationUpdateRequest *request);
void UpfN4HandleAssociationReleaseRequest(PfcpXact *xact, PFCPAssociationReleaseRequest *request);
void UpfN4HandleHeartbeatRequest(PfcpXact *xact, HeartbeatRequest *request);
void UpfN4HandleHeartbeatResponse(PfcpXact *xact, HeartbeatResponse *response);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __N4_PFCP_HANDLER_H__ */
