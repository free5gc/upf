/*
  The file is for handling UP issue.
  Including Echo Request, Echo response, Error Indication, End marker, buffer
  This will write a little route rule for UE route
  This will create udp/ipv4 socket to handle first four.
  This will create name pipe for kernel sending unmatch packet up to user space.
 */

#define TRACE_MODULE _up_path

#include "up_path.h"

#include "utlt_debug.h"
#include "utlt_network.h"
#include "utlt_buff.h"
#include "pfcp_types.h"
#include "upf_context.h"
#include "utlt_netheader.h"

/* TODO: It is not use gtpv1DevList in upf_context, need to change to VirtualDevice
Status GTPv1ServerInit() {
    Status status;

    // TODO : One IP mapping to one dev, need to discuss
    status = GtpDevListCreate(Self()->epfd, AF_INET, &Self()->gtpv1DevList,
                              GtpHandler, NULL);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                "GtpLinkListCreate Fail");

    return STATUS_OK;
}

Status GTPv1ServerTerminate() {
    Status status = STATUS_OK;

    status = GtpDevListFree(Self()->epfd, &Self()->gtpv1DevList);
    UTLT_Assert(status == STATUS_OK, status |= STATUS_ERROR,
                "GTPv1 tunnel list free fail");

    return status;
}

Status GtpHandler(Sock *sock, void *data) {
    UTLT_Assert(sock, return STATUS_ERROR, "GTP socket not found");
    Status status = STATUS_ERROR;

    Bufblk *pktbuf = BufblkAlloc(1, MAX_OF_GTPV1_PACKET_SIZE);
    int readNum = GtpRecv(sock, pktbuf);
    UTLT_Assert(readNum >= 0, goto FREEBUFBLK, "GTP receive fail");

    // TODO : Need to handle buffering and reject, including GTP and general packet
    // Not only GTP packet
    Gtpv1Header *gtpHdr = pktbuf->buf;
    UTLT_Assert((gtpHdr->flags >> 5) == 1, goto FREEBUFBLK,
                "Only handle the GTP version 1 in user plane");

    switch (gtpHdr->type) {
        case GTPV1_ECHO_REQUEST:
            status = GtpHandleEchoRequest(sock, gtpHdr);
            break;
        case GTPV1_ECHO_RESPONSE:
            status = GtpHandleEchoResponse(gtpHdr);
            break;
        case GTPV1_ERROR_INDICATION:
            UTLT_Warning("GTPv1 header type ERROR_INDICATION not implemented");
            break;
        case GTPV1_END_MARK:
            // TODO : Need to deal with the UE packet that does not have tunnel yet
            status = GtpHandleEndMark(sock, gtpHdr);
            break;
        case GTPV1_T_PDU:
            UTLT_Debug("Got GTPv1 T_PDU packet");
            status = STATUS_OK;
            break;
        default :
            UTLT_Warning("Unknown GTPv1 header type[%d]", gtpHdr->type);
    }

FREEBUFBLK:
    UTLT_Assert(BufblkFree(pktbuf) == STATUS_OK, , "Bufblk free fail");

    return status;
}
*/

Status GtpHandleEchoRequest(Sock *sock, void *data) {
    UTLT_Assert(data, return STATUS_ERROR, "GTP data is NULL");

    Gtpv1Header *gtpHdr = data;
    UTLT_Assert(gtpHdr->type == GTPV1_ECHO_REQUEST, return STATUS_ERROR,
                "The type of GTP data is not 'Echo Request'");

    Status status = STATUS_OK;

    // Build the Echo Response packet
    Gtpv1Header gtpRespHrd = {
        .flags = 0x30 + (gtpHdr->flags & 0x03),
        .type = GTPV1_ECHO_RESPONSE,
    };

    Bufblk *optPkt = BufblkAlloc(1, 0x40);
    if (gtpRespHrd.flags & 0x03) {
        Gtpv1OptHeader *opthrd = (void *)((uint8_t *) data + GTPV1_HEADER_LEN);
        Gtpv1OptHeader gtpOptHrd = {
            ._seqNum = (gtpRespHrd.flags & 0x02) ? htons(ntohs(opthrd->_seqNum) + 1) : 0,
            .nPdnNum = (gtpRespHrd.flags & 0x01) ? opthrd->nPdnNum : 0,
        };
        BufblkBytes(optPkt, (void *) &gtpOptHrd, sizeof(gtpOptHrd));
    }

    /* Recover IE */
    uint8_t recoverType = 14, recoverCnt = 0;
    BufblkBytes(optPkt, (void *) &recoverType, 1);
    BufblkBytes(optPkt, (void *) &recoverCnt, 1);

    gtpRespHrd._length = htons(optPkt->len);

    Bufblk *pkt = BufblkAlloc(1, 0x40);
    BufblkBytes(pkt, (void *) &gtpRespHrd, GTPV1_HEADER_LEN);
    BufblkBuf(pkt, optPkt);

    BufblkFree(optPkt);

    UTLT_Assert(UdpSendTo(sock, pkt->buf, pkt->len) == STATUS_OK, status = STATUS_ERROR,
                "GTP Send fail");

    BufblkFree(pkt);

    return status;
}

Status GtpHandleEchoResponse(void *data) {
    UTLT_Assert(data, return STATUS_ERROR, "GTP data is NULL");

    Gtpv1Header *gtpHdr = data;
    UTLT_Assert(gtpHdr->type == GTPV1_ECHO_RESPONSE, return STATUS_ERROR,
                "The type of GTP data is not 'Echo Response'");

    // TODO : Check the peer device exists, and ....
    // 29.281 says the restart conter shall be ignore by the receiver

    return STATUS_OK;
}

// TODO : Need to check fepc code
Status GtpHandleEndMark(Sock *sock, void *data) {
    UTLT_Assert(sock && data, return STATUS_ERROR, "GTP data is NULL");
    Status status = STATUS_ERROR;
    /*
    Gtpv1Header *gtpHdr = data;
    int teid = ntohl(gtpHdr->_teid);
    int gtpPayloadLen = GTPV1_HEADER_LEN + ((gtpHdr->flags & 0x02) ? GTPV1_OPT_HEADER_LEN : 0);

    */

    // TODO : Check PDR, FAR to forward packet, or maybe do paging and buffer UE packet
    /*
    UTLT_Assert(UdpSendTo(sock, pktbuf->buf, pktbuf->len) == STATUS_OK, goto FREEBUFBLK, "GTP Send fail");

    status = STATUS_OK;

FREEBUFBLK:
    UTLT_Assert(BufblkFree(pktbuf) == STATUS_OK, status = STATUS_ERROR, "Bufblk free fail");
*/
    status = STATUS_OK;
    return status;
}

Status UpSendPacketByPdrFar(UpfPDR *pdr, UpfFAR *far, Sock *sock) {
    UTLT_Assert(pdr, return STATUS_ERROR, "PDR error");
    UTLT_Assert(far, return STATUS_ERROR, "FAR error");
    UTLT_Assert(sock, return STATUS_ERROR, "Send packet sock error");
    Status status = STATUS_OK;

    UTLT_Assert(far->flags.forwardingParameters && far->forwardingParameters.flags.outerHeaderCreation,
        return STATUS_ERROR, "Need OuterHeaderCreation to send packet");

    UPDK_OuterHeaderCreation *outerHeaderCreation = &far->forwardingParameters.outerHeaderCreation;

    if (outerHeaderCreation->description & UPDK_OUTER_HEADER_CREATION_DESCRIPTION_GTPU_UDP_IPV4) {
        Gtpv1Header gtpHdr = {
            .flags = 0x30,
            .type = GTPV1_T_PDU,
            ._teid = ntohl(outerHeaderCreation->teid),
        };

        uint16_t pdrId = pdr->pdrId;
        UpfBufPacket *bufStorage = UpfBufPacketFindByPdrId(pdrId);
        if (bufStorage->packetBuffer) {
            UTLT_Assert(!pthread_spin_lock(&Self()->buffLock),
                        return STATUS_ERROR, "spin lock buffLock error");

            Bufblk *sendBuf = BufblkAlloc(1, 0x40);
            gtpHdr._length = htons(bufStorage->packetBuffer->len);
            BufblkBytes(sendBuf, (void*)&gtpHdr, GTPV1_HEADER_LEN);
            BufblkBuf(sendBuf, bufStorage->packetBuffer);

            status = UdpSendTo(sock, sendBuf->buf, sendBuf->len);
            UTLT_Assert(status == STATUS_OK, return status, "UdpSendTo failed");
            BufblkClear(sendBuf);

            while (pthread_spin_unlock(&Self()->buffLock)) {
                // if unlock failed, keep trying
                UTLT_Error("spin unlock error");
            }
            status = BufblkFree(bufStorage->packetBuffer);
        } else {
            UTLT_Debug("bufStorage is NULL");
        }
        UTLT_Assert(status == STATUS_OK, return status,
                    "Free packet buffer failed");
        bufStorage->packetBuffer = NULL;
    } else {
        UTLT_Warning("outer header creatation not implement: "
                     "GTP-IPV6, IPV4, IPV6");
    }

    return status;
}
