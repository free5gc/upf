#define TRACE_MODULE _n4_pfcp_handler

#include <endian.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "utlt_list.h"
#include "utlt_network.h"

#include "upf_context.h"
#include "pfcp_message.h"
#include "pfcp_xact.h"
#include "pfcp_convert.h"
#include "gtp_path.h"
#include "n4_pfcp_build.h"
#include "up/up_gtp_path.h"
#include "gtp5g.h"
#include "gtp5gnl.h"


#define _PDR_ADD 0
#define _PDR_MOD 1
#define _PDR_DEL 2
#define _FAR_ADD 0
#define _FAR_MOD 1
#define _FAR_DEL 2


Status _pushPdrToKernel(struct gtp5g_pdr *pdr, int action) {
    // Create netlink socket
    struct mnl_socket *nl = genl_socket_open();
    UTLT_Assert(nl != NULL, return STATUS_ERROR, "mnl socket open");

    // Find the specific devices by its type
    int genl_id = genl_lookup_family(nl, "gtp5g");
    UTLT_Assert(genl_id >= 0, return STATUS_ERROR,
                "GTP gen family not found");

    // Find target devices
    // TODO: get target device by config file
    char gtp5g_int[] = "gtp5gtest";
    uint32_t ifidx = if_nametoindex(gtp5g_int);
    UTLT_Assert(ifidx != 0, return STATUS_ERROR, "wrong GTP interface");

    // Set device information
    // TODO: if dev can get from UPF context
    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, ifidx);

    switch (action) {
    case _PDR_ADD:
        gtp5g_add_pdr(genl_id, nl, dev, pdr);
    case _PDR_MOD:
        gtp5g_mod_pdr(genl_id, nl, dev, pdr);
    case _PDR_DEL:
        gtp5g_del_pdr(genl_id, nl, dev, pdr);
    }

    // Free device
    gtp5g_dev_free(dev);
    // Close socket
    genl_socket_close(nl);

    return STATUS_OK;
}

Status _pushFarToKernel(struct gtp5g_far *far, int action) {
    // Create netlink socket
    struct mnl_socket *nl = genl_socket_open();
    UTLT_Assert(nl != NULL, return STATUS_ERROR, "mnl socket open");

    // Find the specific devices by its type
    int genl_id = genl_lookup_family(nl, "gtp5g");
    UTLT_Assert(genl_id >= 0, return STATUS_ERROR,
                "GTP gen family not found");

    // Find target devices
    // TODO: get target device by config file
    char gtp5g_int[] = "gtp5gtest";
    uint32_t ifidx = if_nametoindex(gtp5g_int);
    UTLT_Assert(ifidx != 0, return STATUS_ERROR, "wrong GTP interface");

    // Set device information
    // TODO: if dev can get from UPF context
    struct gtp5g_dev *dev = gtp5g_dev_alloc();
    gtp5g_dev_set_ifidx(dev, ifidx);

    switch (action) {
    case _FAR_ADD:
        gtp5g_add_far(genl_id, nl, dev, far);
    case _FAR_MOD:
        gtp5g_mod_far(genl_id, nl, dev, far);
    case _FAR_DEL:
        gtp5g_del_far(genl_id, nl, dev, far);
    }

    // Free device
    gtp5g_dev_free(dev);
    // Close socket
    genl_socket_close(nl);

    return STATUS_OK;
}

Status UpfN4HandleCreatePdr(UpfSession *session, CreatePDR *createPdr) {
    UpfPdr *tmpPdr = NULL;

    UTLT_Assert(createPdr->pDRID.presence, return STATUS_ERROR,
                "pdr id not presence");
    UTLT_Assert(createPdr->precedence.presence, return STATUS_ERROR,
                "precedence not presence");
    UTLT_Assert(createPdr->pDI.presence, return STATUS_ERROR,
                "Pdi not exist");
    UTLT_Assert(createPdr->pDI.sourceInterface.presence,
                return STATUS_ERROR, "PDI SourceInterface not presence");

    tmpPdr = gtp5g_pdr_alloc();
    tmpPdr->precedence = ntohl(*((uint32_t *)createPdr->precedence.value));
    //tmpPdr->sourceInterface = *((uint8_t *)(createPdr->pDI.sourceInterface.value));
    tmpPdr->pdrId = ntohs(*((uint16_t *)createPdr->pDRID.value));

    // F-TEID
    if (createPdr->pDI.localFTEID.presence) {
        PfcpFTeid *fTeid = (PfcpFTeid*)createPdr->pDI.localFTEID.value;
        uint32_t teid = ntohl(fTeid->teid);

        if (fTeid->v4 && fTeid->v6) {
            // TODO: Dual Stack
        } else if (fTeid->v4) {
            gtp5g_pdr_set_local_f_teid(tmpPdr, teid, &(fTeid->addr4));
        } else if (fTeid->v6) {
            // TODO: ipv6
            //gtp5g_pdr_set_local_f_teid(tmpPdr, teid, &(fTeid->addr6));
        }
    }

    // UE IP
    if (createPdr->pDI.uEIPAddress.presence) {
        PfcpUeIpAddr *ueIp =
          (PfcpUeIpAddr*)createPdr->pDI.uEIPAddress.value;
        if (ueIp->v4 && ueIp->v6) {
            // TODO: Dual Stack
        } else if (ueIp->v4) {
            gtp5g_pdr_set_ue_addr_ipv4(tmpPdr, &(ueIp->addr4));
        } else if (ueIp->v6) {
            // TODO: IPv6
        }
    }

    // Outer Header Removal
    if (createPdr->outerHeaderRemoval.presence) {
        uint8_t outerHeader =
          *(uint8_t*)createPdr->outerHeaderRemoval.value;
        gtp5g_pdr_set_outer_header_removal(tmpPdr, outerHeader);
    }

    // FAR ID
    if (createPdr->fARID.presence) {
        uint32_t farId = ntohl(*((uint8_t *)createPdr->fARID.value));
        gtp5g_pdr_set_far_id(tmpPdr, farId);
    }

    // Send PDR to kernel
    _pushPdrToKernel(tmpPdr, _PDR_ADD);
    gtp5g_pdr_free(tmpPdr);
    UTLT_Assert(tmpPdr != NULL, return, "Free PDR struct error");

    ListAppend(&session->pdrIdList, &pdrId);

    return STATUS_OK;
}

Status UpfN4HandleCreateFar(CreateFAR *createFar) {
    UpfFar *tmpFar = NULL;
    UTLT_Assert(createFar->fARID.presence, return STATUS_ERROR,
                "Far ID not presence");
    UTLT_Assert(createFar->applyAction.presence,
                return STATUS_ERROR, "Apply Action not presence");

    // Create FAR
    tmpFar = gtp5g_far_alloc();
    uint32_t farId = ntohl(*((uint32_t *)createFar->fARID.value));
    uint8_t applyAction = *((uint8_t *)(createFar->applyAction.value));
    gtp5g_far_set_id(tmpFar, farId);
    gtp5g_far_set_apply_action(tmpFar, applyAction);

    // Forwarding Parameters
    if (createFar->forwardingParameters.presence) {
        // Destination Interface
        /*
        if (createFar->forwardingParameters.destinationInterface.presence) {
            uint8_t destinationInterface =
              *((uint8_t *)(createFar->forwardingParameters.destinationInterface.value));
        }
        // Network Instance
        if (createFar->forwardingParameters.networkInstance.presence) {
        }
        */
        // Outer Header Creation
        if (createFar->forwardingParameters.outerHeaderCreation.presence) {
            PfcpOuterHdr *outerHdr =
              (PfcpOuterHdr *)(createFar->forwardingParameters.outerHeaderCreation.value);
            uint16_t description = *((uint16_t *)outerHdr);

            if (outerHdr->gtpuIpv4 || outerHdr->udpIpv4) {
                gtp5g_far_set_outer_header_creation(tmpFar, description,
                  ntohl(outerHdr->teid), &(outerHdr->addr4), 2152);
            } else if (outerHdr->udpIpv4) {
                // only with UDP enabled has port number
                gtp5g_far_set_outer_header_creation(tmpFar, description,
                  0, &(outerHdr->addr4), ntohl(outerHdr->port));
            }
        }
    }

    // Send FAR to kernel
    _pushFarToKernel(tmpFar, _FAR_ADD);
    gtp5g_far_free(tmpFar);
    UTLT_Assert(tmpFar != NULL, return STATUS_ERROR, "Free FAR struct error");

    return STATUS_OK;
}

Status UpfN4HandleUpdatePdr(UpdatePDR *updatePdr) {
    UpfPdr *tmpPdr = NULL;
    UTLT_Assert(updatePdr->pDRID.presence == 1,
                return STATUS_ERROR, "updatePDR no pdrId");

    // Find PDR
    uint16_t pdrId = ntohs(*((uint16_t*)updatePdr->pDRID.value));
    tmpPdr = UpfPdrFindByPdrId(pdrId);

    if (tmpPdr) {
        // TODO: other IE of update PDR
        if (updatePdr->precedence.presence) {
            gtp5g_pdr_set_id(tmpPdr, *(uint32_t)updatePdr->pDRID.value);
        }
        if (updatePdr->outerHeaderRemoval.presence) {
            gtp5g_pdr_set_outer_header_removal(tmpPdr,
              *((uint8_t*)(updatePdr->outerHeaderRemoval.value)));
        }
        if (updatePdr->precedence.presence) {
            gtp5g_pdr_set_procedence(tmpPdr,
              *((uint32_t *)(updatePdr->precedence.value)));
        }
        if (updatePdr->pDI.presence) {
            if (updatePdr->pDI.localFTEID.presence) {
                PfcpFTeid *fTeid = (PfcpFTeid*)updatePdr->pDI.localFTEID.value;
                uint32_t teid = ntohl(fTeid->teid);

                if (fTeid->v4 && fTeid->v6) {
                    // TODO: Dual Stack
                } else if (fTeid->v4) {
                    gtp5g_pdr_set_local_f_teid(tmpPdr, teid, &(fTeid->addr4));
                } else if (fTeid->v6) {
                    // TODO: ipv6
                    //gtp5g_pdr_set_local_f_teid(tmpPdr, teid, &(fTeid->addr6));
                }
            }
            if (updatePdr->pDI.uEIPAddress.presence) {
                PfcpUeIpAddr *ueIp = (PfcpUeIpAddr*)updatePdr->pDI.uEIPAddress.value;
                if (ueIp->v4 && ueIp->v6) {
                    // TODO: Dual Stack
                } else if (ueIp->v4) {
                    gtp5g_pdr_set_ue_addr_ipv4(tmpPdr, &(ueIp->addr4));
                } else if (ueIp->v6) {
                    // TODO: IPv6
                }
            }
        }
        if (updatePdr->fARID.presence) {
            gtp5g_pdr_set_far_id(tmpPdr, *(uint32_t *)updatePdr->fARID.value);
        }
    } else {
        /* PDR missing in updatePDR */
        UTLT_Assert(0, , "[PFCP] Session Modification Request miss of PDR context");
    }

    // update PDR to kernel
    _pushPdrToKernel(tmpPdr, _PDR_MOD);
    gtp5g_pdr_free(tmpPdr);
    UTLT_Assert(tmpPdr != NULL, return STATUS_ERROR, "Free PDR struct error");

    return STATUS_OK;
}

Status UpfN4HandleUpdateFar(UpdateFAR *updateFar) {
    UpfFar *tmpFar = NULL;
    UTLT_Assert(updateFar->fARID.presence,
                return STATUS_ERROR, "Far ID not presence");

    // Find FAR
    uint32_t farId = ntohl(*((uint32_t *)updateFar->fARID.value));
    tmpFar = UpfFarFindByFarId(farId);

    if (tmpFar) {
        // update ApplyAction
        if (updateFar->applyAction.presence) {
            gtp5g_far_set_apply_action(tmpFar,
              *(uint8_t *)updateFar->applyAction.value);
        }
        // update Forwarding parameters
        if (updateFar->updateForwardingParameters.outerHeaderCreation.value) {
            PfcpOuterHdr *outerHdr = (PfcpOuterHdr *)
              (updateFar->updateForwardingParameters.outerHeaderCreation.value);
            uint16_t description = *((uint16_t *)outerHdr);

            if (outerHdr->gtpuIpv4) {
                gtp5g_far_set_outer_header_creation(tmpFar, description,
                  ntohl(outerHdr->teid), &(outerHdr->addr4), 2152);
            } else if (outerHdr->udpIpv4) {
                gtp5g_far_set_outer_header_creation(tmpFar, description,
                  0, &(outerHdr->addr4), ntohl(outerHdr->port));
            }
        }
        // TODO: update Duplicating parameters
        // TODO: update BAR
    } else {
        /* FAR not found */
        UTLT_Assert(0, return STATUS_ERROR,
                    "[PFCP] updateFar FAR not found");
    }

    // TODO: update FAR to kernel
    _pushFarToKernel(tmpFar, _FAR_MOD);
    gtp5g_far_free(tmpFar);
    UTLT_Assert(tmpFar != NULL, return STATUS_ERROR,
                "Free FAR struct error");

    return STATUS_OK;
}

Status UpfN4HandleRemovePdr(uint16_t pdrId) {
    UpfPdr *pdr = UpfPdrFindByPdrId(pdrId);
    UTLT_Assert(pdr != NULL, return STATUS_ERROR,
                "Cannot find PDR by PdrId");

    uint16_t *id = ListFirst(&session->pdrIdList);
    while(id) {
        if (*id == pdrId) {
            // remove PDR from kernel
            _pushPdrToKernel(pdr, _PDR_DEL);
            gtp5g_pdr_free(pdr);
            UTLT_Assert(pdr != NULL, return STATUS_ERROR,
                        "Free PDR struct error");
            ListRemove(&session->pdrIdList, id);
            return STATUS_OK;
        }

        id = ListNext(id);
    }

    UTLT_Warning("The PDR ID not in this session!");
    return STATUS_OK;
}

Status UpfN4HandleRemoveFar(uint32_t farId) {
    // TODO: This function is Marked
    UpfFar *far = UpfFarFindByFarId(farId);
    UTLT_Assert(far != NULL, return STATUS_ERROR,
                "Cannot find FAR by FarId");

    UpfPdr *pdr = UpfPdrFindByFarId(farId);
    if (pdr) {
        gtp5g_pdr4_set_far_id(pdr, 0);
    }

    _pushFarToKernel(far, _FAR_DEL);
    gtp5g_far_free(far);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Remove FAR error");

    return STATUS_OK;
}

void UpfN4HandleSessionEstablishmentRequest(
        UpfSession *session, PfcpXact *pfcpXact, PFCPSessionEstablishmentRequest *request) {
    UpfPdr *pdr0;
    UpfPdr *pdr1;
    UpfFar *far0;
    UpfFar *far1;

    UTLT_Assert(session, return, "Upf Session error");
    UTLT_Assert(pfcpXact, return, "pfcpXact error");
    //UTLT_Assert(pfcpXact->gtpBuf, return, "GTP buffer of pfcpXact error");
    //UTLT_Assert(pfcpXact->gtpXact, return, "GTP Xact of pfcpXact error");

    if (request->createFAR[0].presence) {
        UpfN4HandleCreateFar(&request->createFAR[0], &far0);
    }
    if (request->createFAR[1].presence) {
        UpfN4HandleCreateFar(&request->createFAR[1], &far1);
    }
    if (request->createURR.presence) {
        // TODO
    }
    if (request->createBAR.presence) {
        // TODO
    }
    if (request->createQER.presence) {
        // TODO
    }
    if (request->createPDR[0].presence) {
        UpfN4HandleCreatePdr(session, &request->createPDR[0], &pdr0);
    }
    if (request->createPDR[1].presence) {
        UpfN4HandleCreatePdr(session, &request->createPDR[1], &pdr1);
    }

    Status status;
    PfcpHeader header;
    Bufblk *bufBlk = NULL;
    PfcpFSeid *smfFSeid = NULL;

    if (!request->cPFSEID.presence) {
        UTLT_Error("Session Establishment Response: No CP F-SEID");
        return;
    }

    smfFSeid = request->cPFSEID.value;
    session->smfSeid = be64toh(smfFSeid->seid);

    /* Send Response */
    memset(&header, 0, sizeof(PfcpHeader));
    header.type = PFCP_SESSION_ESTABLISHMENT_RESPONSE;
    header.seid = session->smfSeid;

    status = UpfN4BuildSessionEstablishmentResponse(
        &bufBlk, header.type, session, request);
    UTLT_Assert(status == STATUS_OK, return, "N4 build error");

    status = PfcpXactUpdateTx(pfcpXact, &header, bufBlk);
    UTLT_Assert(status == STATUS_OK, return, "pfcpXact update TX error");

    status = PfcpXactCommit(pfcpXact);
    UTLT_Assert(status == STATUS_OK, return, "xact commit error");

    UTLT_Info("[PFCP] Session Establishment Response");
    return;
}

void UpfN4HandleSessionModificationRequest(
        UpfSession *session, PfcpXact *xact, PFCPSessionModificationRequest *request) {
    UTLT_Assert(session, return, "Session error");
    UTLT_Assert(xact, return, "xact error");

    Status status;
    PfcpHeader header;
    Bufblk *bufBlk;

    /* Create FAR */
    if (request->createFAR[0].presence) {
        UpfFar *far0;
        UpfN4HandleCreateFar(&request->createFAR[0], &far0);
    }
    if (request->createFAR[1].presence) {
        UpfFar *far1;
        UpfN4HandleCreateFar(&request->createFAR[1], &far1);
    }
    if (request->createPDR[0].presence) {
        UpfPdr *pdr0;
        UpfN4HandleCreatePdr(session, &request->createPDR[0], &pdr0);
    }
    if (request->createPDR[1].presence) {
        UpfPdr *pdr1;
        UpfN4HandleCreatePdr(session, &request->createPDR[1], &pdr1);
    }

    /* Update FAR */
    if (request->updateFAR.presence) {
        UpfFar *far;
        UpfN4HandleUpdateFar(&request->updateFAR, &far);
    }
    /* Update PDR */
    if (request->updatePDR.presence) {
        UpfPdr *pdr;
        UpfN4HandleUpdatePdr(&request->updatePDR, &pdr);
    }
    /* Remove PDR */
    if (request->removePDR.presence) {
        UTLT_Assert(request->removePDR.pDRID.presence == 1,
		    , "[PFCP] PdrId in removePDR not presence!");
        UpfN4HandleRemovePdr(*(uint16_t*)request->removePDR.pDRID.value);
    }
    /* Remove FAR */
    if (request->removeFAR.presence) {
      UTLT_Assert(request->removeFAR.fARID.presence == 1,
		  ,"[PFCP] FarId in removeFAR not presence");
        UpfN4HandleRemoveFar(*(uint32_t*)request->removeFAR.fARID.value);
    }
    //TODO

    /* Send Session Modification Response */
    memset(&header, 0, sizeof(PfcpHeader));
    header.type = PFCP_SESSION_MODIFICATION_RESPONSE;
    header.seid = session->smfSeid;

    status = UpfN4BuildSessionModificationResponse(
        &bufBlk, header.type, session, request);
    UTLT_Assert(status == STATUS_OK, return, "N4 build error");

    status = PfcpXactUpdateTx(xact, &header, bufBlk);
    UTLT_Assert(status == STATUS_OK, return, "PfcpXactUpdateTx error");

    status = PfcpXactCommit(xact);
    UTLT_Assert(status == STATUS_OK, return, "PFCP Commit error");

    UTLT_Info("[PFCP] Session Modification Response");
    return;
}

void UpfN4HandleSessionDeletionRequest(UpfSession *session, PfcpXact *xact, PFCPSessionDeletionRequest *request) {
    UTLT_Assert(session, return, "session error");
    UTLT_Assert(xact, return, "xact error");

    Status status;
    PfcpHeader header;
    Bufblk *bufBlk = NULL;

    // Remove all Gtp Tunnel of this session
    Gtpv1TunDevNode *gtpList = (Gtpv1TunDevNode*)ListFirst(&Self()->gtpv1DevList);
    char *ifname = gtpList->ifname;
    UpfPdr *tmpPdr;
    for (tmpPdr = (UpfPdr*)ListFirst(&session->dlPdrList); tmpPdr; tmpPdr = (UpfPdr*)ListNext(tmpPdr)) {
        status = GtpTunnelDel(ifname, tmpPdr->upfGtpUTeid);
        UTLT_Assert(status == STATUS_OK, , "Gtp Tunnel delete failed");
    }
    // TODO: Remove all PDR
    // TODO: Remove all FAR

    /* delete session */
    UpfSessionRemove(session);

    /* Send Session Deletion Response */
    memset(&header, 0, sizeof(PfcpHeader));

    header.type = PFCP_SESSION_DELETION_RESPONSE;
    header.seid = session->smfSeid;

    status = UpfN4BuildSessionDeletionResponse(&bufBlk, header.type, session, request);
    UTLT_Assert(status == STATUS_OK, return, "N4 build error");

    status = PfcpXactUpdateTx(xact, &header, bufBlk);
    UTLT_Assert(status == STATUS_OK, return, "PfcpXactUpdateTx error");

    status = PfcpXactCommit(xact);
    UTLT_Assert(status == STATUS_OK, return, "xact commit error");

    UTLT_Info("[PFCP] Session Deletion Response");
    return;
}

void UpfN4HandleSessionReportResponse(
        UpfSession *session, PfcpXact *xact, PFCPSessionReportResponse *response) {
    Status status;

    UTLT_Assert(session, return, "Session error");
    UTLT_Assert(xact, return, "xact error");
    UTLT_Assert(response->cause.presence, return, "SessionReportResponse error: no Cause");

    status = PfcpXactCommit(xact);
    UTLT_Assert(status == STATUS_OK, return, "xact commit error");

    UTLT_Info("[PFCP] Session Report Response");
    return;
}

void UpfN4HandleAssociationSetupRequest(PfcpXact *xact, PFCPAssociationSetupRequest *request) {
    PfcpNodeId *nodeId;

    UTLT_Assert(xact, return, "xact error");
    UTLT_Assert(xact->gnode, return, "gNode of xact error");
    UTLT_Assert(request->nodeID.presence, return, "Request missing nodeId");

    nodeId = (PfcpNodeId *)request->nodeID.value;

    xact->gnode->nodeId.type = nodeId->type;
    switch (nodeId->type) {
        case PFCP_NODE_ID_IPV4:
            xact->gnode->nodeId.addr4 = nodeId->addr4;
            break;
        case PFCP_NODE_ID_IPV6:
            xact->gnode->nodeId.addr6 = nodeId->addr6;
            break;
        default:
            UTLT_Assert(0, return, "Request no node id type");
            break;
    }

    /* Accept */
    xact->gnode->state = PFCP_NODE_ST_ASSOCIATED;

    Status status;
    PfcpHeader header;
    Bufblk *bufBlk = NULL;

    /* Send */
    memset(&header, 0, sizeof(PfcpHeader));
    header.type = PFCP_ASSOCIATION_SETUP_RESPONSE;
    header.seid = 0;

    status = UpfN4BuildAssociationSetupResponse(&bufBlk, header.type);
    UTLT_Assert(status == STATUS_OK, return, "N4 build error");

    status = PfcpXactUpdateTx(xact, &header, bufBlk);
    UTLT_Assert(status == STATUS_OK, return, "PfcpXactUpdateTx error");

    status = PfcpXactCommit(xact);
    UTLT_Assert(status == STATUS_OK, return, "xact commit error");

    UTLT_Info("[PFCP] Association Session Setup Response");
    return;
}

void UpfN4HandleAssociationUpdateRequest(PfcpXact *xact, PFCPAssociationUpdateRequest *request) {
    // TODO
    UTLT_Info("[PFCP] TODO Association Update Request");
}

void UpfN4HandleAssociationReleaseRequest(PfcpXact *xact, PFCPAssociationReleaseRequest *request) {
    // TODO
    UTLT_Info("[PFCP] TODO Association Release Request");
}

void UpfN4HandleHeartbeatRequest(PfcpXact *xact, HeartbeatRequest *request) {
    Status status;
    PfcpHeader header;
    Bufblk *bufBlk = NULL;

    UTLT_Info("[PFCP] Heartbeat Request");

    /* Send */
    memset(&header, 0, sizeof(PfcpHeader));
    header.type = PFCP_HEARTBEAT_RESPONSE;
    header.seid = 0;

    status = UpfN4BuildHeartbeatResponse(&bufBlk, header.type);
    UTLT_Assert(status == STATUS_OK, return, "N4 build error");

    status = PfcpXactUpdateTx(xact, &header, bufBlk);
    UTLT_Assert(status == STATUS_OK, return, "PfcpXactUpdateTx error");

    status = PfcpXactCommit(xact);
    UTLT_Assert(status == STATUS_OK, return, "xact commit error");

    UTLT_Info("[PFCP] Heartbeat Response");
    return;
}

void UpfN4HandleHeartbeatResponse(PfcpXact *xact, HeartbeatResponse *response) {
    // if rsv response, nothing to do, else peer may be not alive
    UTLT_Info("[PFCP] Heartbeat Response");
    return;
}
