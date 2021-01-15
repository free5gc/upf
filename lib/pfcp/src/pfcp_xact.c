#define TRACE_MODULE _pfcp_xact

#include <endian.h>

#include "utlt_debug.h"
#include "utlt_pool.h"
#include "utlt_3gppTypes.h"
#include "utlt_index.h"
#include "utlt_timer.h"
#include "utlt_event.h"

#include "pfcp_types.h"
#include "pfcp_message.h"
#include "pfcp_path.h"

#include "pfcp_xact.h"

#define SIZE_OF_PFCP_XACT_POOL          64
#define PFCP_MIN_XACT_ID                1
#define PFCP_MAX_XACT_ID                0x800000

#define PFCP_T3_RESPONSE_DURATION       3000 /* 3 seconds */
#define PFCP_T3_RESPONSE_RETRY_COUNT    3
#define PFCP_T3_DUPLICATED_DURATION \
    (PFCP_T3_RESPONSE_DURATION * PFCP_T3_RESPONSE_RETRY_COUNT)  /* 9 seconds */
#define PFCP_T3_DUPLICATED_RETRY_COUNT  1


static int pfcpXactInitialized = 0; // if xact exist
static TimerList *globalTimerList = NULL;
static uintptr_t globalResponseEvent = 0;
static uintptr_t globalHoldingEvent = 0;
static uint32_t globalXactId = 0;

IndexDeclare(pfcpXactPool, PfcpXact, SIZE_OF_PFCP_XACT_POOL);

Status PfcpXactInit(TimerList *timerList, uintptr_t responseEvent, uintptr_t holdingEvent) {
    UTLT_Assert(pfcpXactInitialized == 0, return STATUS_ERROR, "PFCP Xact have alread initialized");
    IndexInit(&pfcpXactPool, SIZE_OF_PFCP_XACT_POOL);

    globalXactId = 0;
    globalTimerList = timerList;
    globalResponseEvent = responseEvent;
    globalHoldingEvent = holdingEvent;

    pfcpXactInitialized = 1;

    return STATUS_OK;
}

Status PfcpXactTerminate() {
    UTLT_Assert(pfcpXactInitialized == 1, return STATUS_ERROR,
                "PFCP Xact already finalized");

    if (PoolUsedCheck(&pfcpXactPool)) {
        UTLT_Warning("%d not freed in pfcpXactPool[%d] of PFCP Transaction",
                   PoolUsedCheck(&pfcpXactPool), PoolSize(&pfcpXactPool));
    }
    UTLT_Trace("%d freed in pfcpXactPool[%d] of PFCP Transaction",
               PoolUsedCheck(&pfcpXactPool), PoolSize(&pfcpXactPool));

    IndexTerminate(&pfcpXactPool);
    pfcpXactInitialized = 0;

    return STATUS_OK;
}

PfcpXact *PfcpXactLocalCreate(PfcpNode *gnode, PfcpHeader *header, Bufblk *bufBlk) {
    Status status;
    PfcpXact *xact = NULL;

    UTLT_Assert(gnode, return NULL, "node error");

    IndexAlloc(&pfcpXactPool, xact);
    // TODO: drop transaction allocation failed
    UTLT_Assert(xact, return NULL, "Transaction allocation failed");

    UTLT_Trace("Alloc PFCP pool");
    UTLT_Assert(xact, return NULL, "Transaction allocation failed");

    xact->origin = PFCP_LOCAL_ORIGINATOR;
    xact->transactionId = (globalXactId == PFCP_MAX_XACT_ID ? PFCP_MIN_XACT_ID : globalXactId + 1);
    xact->gnode = gnode;
    /*TODO: fix this
    if (globalResponseEvent) {
        xact->timerResponse = EventTimerCreate(globalTimerList, TIMER_TYPE_ONCE, PFCP_T3_RESPONSE_DURATION);
        UTLT_Assert(xact->timerResponse, return NULL, "Timer allocation failed");
        TimerSet(PARAM1, xact->timerHolding, xact->index);
        xact->responseReCount = PFCP_T3_DUPLICATED_RETRY_COUNT;
    }
    if (globalHoldingEvent) {
        xact->timerHolding = EventTimerCreate(globalTimerList, TIMER_TYPE_ONCE, PFCP_T3_DUPLICATED_DURATION);
        UTLT_Assert(xact->timerHolding, return NULL, "Timer allocation failed");
        TimerSet(PARAM1, xact->timerHolding, xact->index);
        xact->holdingReCount = PFCP_T3_DUPLICATED_RETRY_COUNT;
    }
    */

    if (xact->origin == PFCP_LOCAL_ORIGINATOR) {
        ListHeadInit(&xact->gnode->localList);
        ListInsert(xact, &xact->gnode->localList);
    } else {
        ListHeadInit(&xact->gnode->remoteList);
        ListInsert(xact, &xact->gnode->remoteList);
    }

    status = PfcpXactUpdateTx(xact, header, bufBlk);
    UTLT_Assert(status == STATUS_OK, goto err, "Update Tx failed");

    UTLT_Trace("[%d] %s Create peer [%s]:%d\n",
               xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
               "local" : "remote",
            GetIP(&gnode->sock->remoteAddr), GetPort(&gnode->sock->remoteAddr));
    return xact;

err:
    IndexFree(&pfcpXactPool, xact);
    return NULL;
}

PfcpXact *PfcpXactRemoteCreate(PfcpNode *gnode, uint32_t sqn) {
    PfcpXact *xact = NULL;

    UTLT_Assert(gnode, return NULL, "node error");

    IndexAlloc(&pfcpXactPool, xact);
    // TODO: drop transaction allocation failed
    UTLT_Assert(xact, return NULL, "Transaction allocation failed");

    xact->origin = PFCP_REMOTE_ORIGINATOR;
    xact->transactionId = PfcpSqn2TransactionId(sqn);
    xact->gnode = gnode;

    if (globalResponseEvent) {
        xact->timerResponse = EventTimerCreate(
                                globalTimerList, TIMER_TYPE_ONCE,
                                PFCP_T3_RESPONSE_DURATION,
                                globalResponseEvent);
        UTLT_Assert(xact->timerResponse, goto err, "Timer allocation failed");
        TimerSet(PARAM2, xact->timerResponse, xact->index);
        xact->responseReCount = PFCP_T3_RESPONSE_RETRY_COUNT;
    }

    if (globalHoldingEvent) {
        xact->timerHolding = EventTimerCreate(globalTimerList,
                               TIMER_TYPE_ONCE, PFCP_T3_RESPONSE_DURATION,
                               globalHoldingEvent);
        UTLT_Assert(xact->timerHolding, goto err, "Timer allocation failed");
        TimerSet(PARAM2, xact->timerHolding, xact->index);
        xact->holdingReCount = PFCP_T3_DUPLICATED_RETRY_COUNT;
    }

    if (xact->origin == PFCP_LOCAL_ORIGINATOR) {
        ListHeadInit(&xact->gnode->localList);
        ListInsert(xact, &xact->gnode->localList);
    } else {
        ListHeadInit(&xact->gnode->remoteList);
        ListInsert(xact, &xact->gnode->remoteList);
    }

    UTLT_Trace("[%d] %s Create  peer [%s]:%d\n", xact->transactionId,
               xact->origin == PFCP_LOCAL_ORIGINATOR ? "local " : "remote",
            GetIP(&gnode->sock->remoteAddr), GetPort(&gnode->sock->remoteAddr));

    return xact;

err:
    IndexFree(&pfcpXactPool, xact);
    return NULL;
}

void PfcpXactDeassociate(PfcpXact *xact1, PfcpXact *xact2) {
    UTLT_Assert(xact1, return, "xact1 error");
    UTLT_Assert(xact2, return, "xact2 error");

    UTLT_Assert(xact1->associatedXact != NULL, return, "1 Already deassocaited");
    UTLT_Assert(xact2->associatedXact != NULL, return, "2 Already deassocaited");

    xact1->associatedXact = NULL;
    xact2->associatedXact = NULL;

    return;
}

Status PfcpXactDelete(PfcpXact *xact) {

    UTLT_Assert(xact, , "xact error");
    UTLT_Assert(xact->gnode, , "node of xact error");

    UTLT_Trace("[%d] %s Delete  peer [%s]:%d\n", xact->transactionId,
            xact->origin == PFCP_LOCAL_ORIGINATOR ? "local" : "remote",
            GetIP(&xact->gnode->sock->remoteAddr), GetPort(&xact->gnode->sock->remoteAddr));
    if (xact->origin == PFCP_LOCAL_ORIGINATOR) {
        ListRemove(xact);
    } else if (xact->origin == PFCP_REMOTE_ORIGINATOR) {
        ListRemove(xact);
    }

    xact->origin = 0;
    xact->transactionId = 0;
    xact->step = 0;
    xact->seq[0].type = 0;
    xact->seq[1].type = 0;
    xact->seq[2].type = 0;

    if (xact->seq[0].bufBlk) {
        BufblkFree(xact->seq[0].bufBlk);
    }
    if (xact->seq[1].bufBlk) {
        BufblkFree(xact->seq[1].bufBlk);
    }
    if (xact->seq[2].bufBlk) {
        BufblkFree(xact->seq[2].bufBlk);
    }

    if (xact->timerResponse) {
        TimerDelete(xact->timerResponse);
    }
    if (xact->timerHolding) {
        TimerDelete(xact->timerHolding);
    }

    if (xact->associatedXact) {
        PfcpXactDeassociate(xact, xact->associatedXact);
    }

    IndexFree(&pfcpXactPool, xact);
    return STATUS_OK;
}

void PfcpXactDeleteAll(PfcpNode *gnode) {
    PfcpXact *xact, *nextNode = NULL;
    
    ListForEachSafe(xact, nextNode, &gnode->localList) {
        PfcpXactDelete(xact);
    }

    ListForEachSafe(xact, nextNode, &gnode->remoteList) {
        PfcpXactDelete(xact);
    }

    return;
}

static PfcpXactStage PfcpXactGetStage(uint8_t type, uint32_t transactionId) {
    PfcpXactStage stage = PFCP_XACT_UNKNOWN_STAGE;

    switch(type) {
        case PFCP_HEARTBEAT_REQUEST:
        case PFCP_ASSOCIATION_SETUP_REQUEST:
        case PFCP_ASSOCIATION_UPDATE_REQUEST:
        case PFCP_ASSOCIATION_RELEASE_REQUEST:
        case PFCP_SESSION_ESTABLISHMENT_REQUEST:
        case PFCP_SESSION_MODIFICATION_REQUEST:
        case PFCP_SESSION_DELETION_REQUEST:
        case PFCP_SESSION_REPORT_REQUEST:
            stage = PFCP_XACT_INITIAL_STAGE;
            break;
        case PFCP_HEARTBEAT_RESPONSE:
        case PFCP_ASSOCIATION_SETUP_RESPONSE:
        case PFCP_ASSOCIATION_UPDATE_RESPONSE:
        case PFCP_ASSOCIATION_RELEASE_RESPONSE:
        case PFCP_VERSION_NOT_SUPPORTED_RESPONSE:
        case PFCP_SESSION_ESTABLISHMENT_RESPONSE:
        case PFCP_SESSION_MODIFICATION_RESPONSE:
        case PFCP_SESSION_DELETION_RESPONSE:
        case PFCP_SESSION_REPORT_RESPONSE:
            stage = PFCP_XACT_FINAL_STAGE;
            break;

        default:
            UTLT_Error("Not implemented PFCP Message Type(%d)", type);
            break;
    }

    return stage;
}

Status PfcpXactUpdateTx(PfcpXact *xact, PfcpHeader *header, Bufblk *bufBlk) {
    PfcpXactStage stage;
    PfcpHeader *localHeader = NULL;
    uint8_t headerLen = 0;
    Bufblk *fullPacket;

    UTLT_Assert(xact, return STATUS_ERROR, "xact error");
    UTLT_Assert(xact->gnode, return STATUS_ERROR, "node of xact error");
    UTLT_Assert(header, return STATUS_ERROR, "header error");
    UTLT_Assert(bufBlk, return STATUS_ERROR, "buffer error");

    UTLT_Trace("[%d] %s UDP TX-%d peer [%s]:%d\n",
               xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
               "local " : "remote",
               header->type, GetIP(&xact->gnode->sock->remoteAddr),
               GetPort(&xact->gnode->sock->remoteAddr));

    stage = PfcpXactGetStage(header->type, xact->transactionId);
    if (xact->origin == PFCP_LOCAL_ORIGINATOR) {
        switch(stage) {
            case PFCP_XACT_INITIAL_STAGE:
                UTLT_Assert(xact->step == 0, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d\n",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local " : "remote", xact->step, header->type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));
                break;
            case PFCP_XACT_INTERMEDIATE_STAGE:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);
                break;
            case PFCP_XACT_FINAL_STAGE:
                UTLT_Assert(xact->step == 2, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d\n",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local " : "remote", xact->step, header->type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));
                break;
            default:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);
        }
    } else if (xact->origin == PFCP_REMOTE_ORIGINATOR) {
        switch(stage) {
            case PFCP_XACT_INITIAL_STAGE:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);
                break;
            case PFCP_XACT_INTERMEDIATE_STAGE:
            case PFCP_XACT_FINAL_STAGE:
                UTLT_Assert(xact->step == 1, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d\n",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local " : "remote", xact->step, header->type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));
                break;
            default:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);
        }
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "invalid origination(%d)", xact->origin);
    }

    if(header->type >= PFCP_SESSION_ESTABLISHMENT_REQUEST) { // with SEID
        headerLen = PFCP_HEADER_LEN;
    } else { // no SEID
        headerLen = PFCP_HEADER_LEN - PFCP_SEID_LEN;
    }

    fullPacket = BufblkAlloc(1, headerLen);
    localHeader = fullPacket->buf;
    fullPacket->len = headerLen;

    memset(localHeader, 0, headerLen);
    localHeader->version = PFCP_VERSION;
    localHeader->type = header->type;
    if(header->type >= PFCP_SESSION_ESTABLISHMENT_REQUEST) { // with SEID
        localHeader->seidP = 1;
        localHeader->seid = htobe64(header->seid);
        localHeader->sqn = PfcpTransactionId2Sqn(xact->transactionId);
    } else { // no SEID
        localHeader->seidP = 0;
        localHeader->sqn_only = PfcpTransactionId2Sqn(xact->transactionId);
    }

    localHeader->length = htons(bufBlk->len + headerLen - 4);

    BufblkBuf(fullPacket, bufBlk);
    BufblkFree(bufBlk);

    xact->seq[xact->step].type = localHeader->type;
    xact->seq[xact->step].bufBlk = fullPacket;

    xact->step++;

    return STATUS_OK;
}

Status PfcpXactUpdateRx(PfcpXact *xact, uint8_t type) {
    Status status = STATUS_OK;
    PfcpXactStage stage;

    UTLT_Trace("[%d] %s UDP RX-%d peer [%s]:%d\n", xact->transactionId,
               xact->origin == PFCP_LOCAL_ORIGINATOR ? "local " : "remote",
               type, GetIP(&xact->gnode->sock->remoteAddr),
               GetPort(&xact->gnode->sock->remoteAddr) );

    stage = PfcpXactGetStage(type, xact->transactionId);
    if (xact->origin == PFCP_LOCAL_ORIGINATOR) {
        switch(stage) {
            case PFCP_XACT_INITIAL_STAGE:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);
                break;
            case PFCP_XACT_INTERMEDIATE_STAGE:
                if (xact->seq[1].type == type) {
                    Bufblk *bufBlk = NULL;

                    UTLT_Assert(xact->step == 2 || xact->step == 3,
                                return STATUS_ERROR,
                                "[%d] %s invalid step %d for type %d peer [%s]:%d",
                                xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                                "local " : "remote", xact->step, type,
                                GetIP(&xact->gnode->sock->remoteAddr),
                                GetPort(&xact->gnode->sock->remoteAddr));

                    bufBlk = xact->seq[2].bufBlk;
                    if (bufBlk) {
                        if (xact->timerHolding) {
                            TimerStart(xact->timerHolding);
                        }
                        UTLT_Warning("[%d] %s Request Duplicated. Retransmit! for type %d peer [%s]:%d",
                                     xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                                     "local" : "remote", xact->step, type,
                                     GetIP(&xact->gnode->sock->remoteAddr),
                                     GetPort(&xact->gnode->sock->remoteAddr));
                        status = PfcpSend(xact->gnode, bufBlk);
                        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "PfcpSend error");
                    } else {
                        UTLT_Warning("[%d] %s Request Duplicated. Discard! for type %d peer [%s]:%d",
                                     xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                                     "local" : "remote", xact->step, type,
                                     GetIP(&xact->gnode->sock->remoteAddr),
                                     GetPort(&xact->gnode->sock->remoteAddr));
                    }

                    return STATUS_EAGAIN;
                }

                UTLT_Assert(xact->step == 1, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local" : "remote", xact->step, type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));

                if (xact->timerHolding) {
                    TimerStart(xact->timerHolding);
                }
                break;
            case PFCP_XACT_FINAL_STAGE:
                UTLT_Assert(xact->step == 1, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local " : "remote", xact->step, type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));
                break;
            default:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);
        }
    } else if (xact->origin == PFCP_REMOTE_ORIGINATOR) {
        switch(stage) {
            case PFCP_XACT_INITIAL_STAGE:
                if (xact->seq[0].type == type) {
                    Bufblk *bufBlk = NULL;

                    UTLT_Assert(xact->step == 1 || xact->step == 2, return STATUS_ERROR,
                                "[%d] %s invalid step %d for type %d peer [%s]:%d",
                                xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                                "local" : "remote", xact->step, type,
                                GetIP(&xact->gnode->sock->remoteAddr),
                                GetPort(&xact->gnode->sock->remoteAddr));

                    bufBlk = xact->seq[1].bufBlk;
                    if (bufBlk) {
                        if (xact->timerHolding) {
                            TimerStart(xact->timerHolding);
                        }

                        UTLT_Warning("[%d] %s Request Duplicated. Retransmit! for step %d type %d peer [%s]:%d",
                                     xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                                     "local" : "remote", xact->step, type,
                                     GetIP(&xact->gnode->sock->remoteAddr),
                                     GetPort(&xact->gnode->sock->remoteAddr));
                        status = PfcpSend(xact->gnode, bufBlk);
                        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "PfcpSend error");
                    } else {
                        UTLT_Warning("[%d] %s Request Duplicated. Discard!  for step %d type %d peer [%s]:%d",
                                     xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                                     "local" : "remote", xact->step, type,
                                     GetIP(&xact->gnode->sock->remoteAddr),
                                     GetPort(&xact->gnode->sock->remoteAddr));
                    }

                    return STATUS_EAGAIN;
                }

                UTLT_Assert(xact->step == 0, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local" : "remote", xact->step, type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));

                if (xact->timerHolding) {
                    TimerStart(xact->timerHolding);
                }
                break;

            case PFCP_XACT_INTERMEDIATE_STAGE:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);
                break;

            case PFCP_XACT_FINAL_STAGE:
                UTLT_Assert(xact->step == 2, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local" : "remote", xact->step, type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));
                break;

            default:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);
        }
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "invalid orginator(%d)", xact->origin);
    }

    if (xact->timerResponse) {
        TimerStop(xact->timerResponse);
    }

    xact->seq[xact->step].type = type;
    xact->step++;

    return STATUS_OK;
}


Status PfcpXactCommit(PfcpXact *xact) {
    Status status;

    uint8_t type;
    Bufblk *bufBlk = NULL;
    PfcpXactStage stage;

    UTLT_Assert(xact, return STATUS_ERROR, "xact error");
    UTLT_Assert(xact->gnode, return STATUS_ERROR, "node of xact error");

    UTLT_Trace("[%d] %s Commit  peer [%s]:%d\n", xact->transactionId,
            xact->origin == PFCP_LOCAL_ORIGINATOR ? "local" : "remote",
            GetIP(&xact->gnode->sock->remoteAddr), GetPort(&xact->gnode->sock->remoteAddr));

    type = xact->seq[xact->step-1].type;
    stage = PfcpXactGetStage(type, xact->transactionId);

    if (xact->origin == PFCP_LOCAL_ORIGINATOR) {
        switch(stage) {
            case PFCP_XACT_INITIAL_STAGE:
                UTLT_Assert(xact->step == 1, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local" : "remote", xact->step, type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));

                if (xact->timerResponse) {
                    TimerStart(xact->timerResponse);
                }
                break;

            case PFCP_XACT_INTERMEDIATE_STAGE:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);

            case PFCP_XACT_FINAL_STAGE:
                UTLT_Assert(xact->step == 2 || xact->step == 3, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local" : "remote", xact->step, type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));

                if (xact->step == 2) {
                    PfcpXactDelete(xact);
                    return STATUS_OK;
                }
                break;

            default:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);
        }
    } else if (xact->origin == PFCP_REMOTE_ORIGINATOR) {
        switch(stage)
        {
            case PFCP_XACT_INITIAL_STAGE:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);

            case PFCP_XACT_INTERMEDIATE_STAGE:
                UTLT_Assert(xact->step == 2, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local" : "remote", xact->step, type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));

                if (xact->timerResponse) {
                    TimerStart(xact->timerResponse);
                }
                break;

            case PFCP_XACT_FINAL_STAGE:
                UTLT_Assert(xact->step == 2 || xact->step == 3, return STATUS_ERROR,
                            "[%d] %s invalid step %d for type %d peer [%s]:%d",
                            xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                            "local" : "remote", xact->step, type,
                            GetIP(&xact->gnode->sock->remoteAddr),
                            GetPort(&xact->gnode->sock->remoteAddr));

                if (xact->step == 3) {
                    PfcpXactDelete(xact);
                    return STATUS_OK;
                }
                break;

            default:
                UTLT_Assert(0, return STATUS_ERROR, "invalid step(%d)", xact->step);
        }
    } else {
        UTLT_Assert(0, return STATUS_ERROR, "invalid orginator(%d)", xact->origin);
    }

    bufBlk = xact->seq[xact->step-1].bufBlk;
    UTLT_Assert(bufBlk, return STATUS_ERROR, "buffer error");

    status = PfcpSend(xact->gnode, bufBlk);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "PfcpSend error");

    return STATUS_OK;
}

Status PfcpXactTimeout(uint32_t index, uint32_t event, uint8_t *type) {
    UTLT_Trace("[Timer Debug] PfcpXactTimeout index %d and event %d", index, event);
    PfcpXact *xact = NULL;
    
    xact = IndexFind(&pfcpXactPool, index);
    UTLT_Assert(xact, goto out, "index find from pool error");
    UTLT_Assert(xact->gnode, goto out, "node of xact error");

    UTLT_Assert(type, goto out, "type error");
    *type = xact->seq[xact->step-1].type;

    if (event == globalResponseEvent) {
        UTLT_Trace("[%d] %s Response Timeout for step %d type %d peer [%s]:%d\n",
                   xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ?
                   "local" : "remote", xact->step, xact->seq[xact->step-1].type,
                   GetIP(&xact->gnode->sock->remoteAddr),
                   GetPort(&xact->gnode->sock->remoteAddr));

        if (--xact->responseReCount > 0) {
            Bufblk *bufBlk = NULL;

            if (xact->timerResponse) {
                TimerStart(xact->timerResponse);
            }

            bufBlk = xact->seq[xact->step-1].bufBlk;
            UTLT_Assert(bufBlk, return STATUS_ERROR, "buff error");

            UTLT_Assert(PfcpSend(xact->gnode, bufBlk) == STATUS_OK, goto out, "PfcpSend error");
        } else {
            UTLT_Warning("[%d] %s No Reponse. Give up! for step %d type %d peer [%s]:%d",
                    xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ? "local" : "remote",
                    xact->step, xact->seq[xact->step-1].type,
                    GetIP(&xact->gnode->sock->remoteAddr), GetPort(&xact->gnode->sock->remoteAddr));
            PfcpXactDelete(xact);
            return STATUS_ERROR;            
        }
    } else if (event == globalHoldingEvent) {
        UTLT_Trace("[%d] %s Holding Timeout for step %d type %d peer [%s]:%d\n",
                xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ? "local" : "remote",
                xact->step, xact->seq[xact->step-1].type,
                GetIP(&xact->gnode->sock->remoteAddr), GetPort(&xact->gnode->sock->remoteAddr));

        if (--xact->holdingReCount > 0) {
            if (xact->timerHolding) {
                TimerStart(xact->timerHolding);
            }
        } else {
            UTLT_Trace("[%d] %s Delete Transaction for step %d type %d peer [%s]:%d\n",
                    xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ? "local" : "remote",
                    xact->step, xact->seq[xact->step-1].type,
                    GetIP(&xact->gnode->sock->remoteAddr), GetPort(&xact->gnode->sock->remoteAddr));
            PfcpXactDelete(xact);
        }
    }

    return STATUS_OK;

out:
    PfcpXactDelete(xact);
    return STATUS_ERROR;
}

Status PfcpXactReceive(PfcpNode *gnode, PfcpHeader *header, PfcpXact **xact) {
    Status status;
    PfcpXact *newXact = NULL;

    UTLT_Assert(gnode, return STATUS_ERROR, "node error");
    UTLT_Assert(header, return STATUS_ERROR, "header error");

    newXact = PfcpXactFindByTransactionId(gnode, header->type, PfcpSqn2TransactionId(header->sqn));
    if (!newXact) {
        newXact = PfcpXactRemoteCreate(gnode, header->sqn);
    }
    UTLT_Assert(newXact, return STATUS_ERROR, "new Xact error");

    UTLT_Trace("[%d] %s Receive peer [%s]:%d\n",
            newXact->transactionId, newXact->origin == PFCP_LOCAL_ORIGINATOR ? "local" : "remote",
            GetIP(&gnode->sock->remoteAddr), GetPort(&gnode->sock->remoteAddr));

    status = PfcpXactUpdateRx(newXact, header->type);
    if (status != STATUS_OK) {
        IndexFree(&pfcpXactPool, newXact);
        return status;
    }

    *xact = newXact;
    return STATUS_OK;
}

PfcpXact *PfcpXactFind(uint32_t index) {
    UTLT_Assert(index, return NULL, "Invalid Index");
    return IndexFind(&pfcpXactPool, index);
}

PfcpXact *PfcpXactFindByTransactionId(PfcpNode *gnode, uint8_t type, uint32_t transactionId) {
    PfcpXact *xact = NULL;
    PfcpXact *nextNode = NULL;
    ListHead *list = NULL; 
    UTLT_Assert(gnode, return NULL, "node error");

    switch (PfcpXactGetStage(type, transactionId)) {
        case PFCP_XACT_INITIAL_STAGE:
            list = &gnode->remoteList;
            xact = ListFirst(&gnode->remoteList);
            break;
        case PFCP_XACT_INTERMEDIATE_STAGE:
            list = &gnode->localList;
            xact = ListFirst(&gnode->localList);
            break;
        case PFCP_XACT_FINAL_STAGE:
            if (transactionId & PFCP_MAX_XACT_ID) {
                list = &gnode->remoteList;
                xact = ListFirst(&gnode->remoteList);
            }  else {
                list = &gnode->localList;
                xact = ListFirst(&gnode->localList);
            }

                
            break;

        default:
            UTLT_Assert(0, return NULL, "Unknown stage");
    }
    
    ListForEachSafe(xact, nextNode, list) {
        if (xact->transactionId == transactionId)
            break;
    }

    if (xact == (PfcpXact *) list) {
        return NULL;
    }

    if (xact) {
        UTLT_Trace("[%d] %s Find peer [%s]:%d\n",
                xact->transactionId, xact->origin == PFCP_LOCAL_ORIGINATOR ? "local" : "remote",
                GetIP(&gnode->sock->remoteAddr), GetPort(&gnode->sock->remoteAddr));
    }

    return xact;
}

void PfcpXactAssociate(PfcpXact *xact1, PfcpXact *xact2) {
    UTLT_Assert(xact1, return, "xact1 error");
    UTLT_Assert(xact2, return, "xact2 error");

    UTLT_Assert(xact1->associatedXact == NULL, return, "1 Already assocaited");
    UTLT_Assert(xact2->associatedXact == NULL, return, "2 Already assocaited");

    xact1->associatedXact = xact2;
    xact2->associatedXact = xact1;

    return;
}
