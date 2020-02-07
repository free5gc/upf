#include "utlt_mq.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "utlt_buff.h"
#include "utlt_time.h"

typedef struct {
    mqd_t mqd;
    char name[40];
    int oflag;
    long msgsize;
} MQInfo;

MQId MQCreate(int oflag) {
    MQInfo *mq = UTLT_Malloc(sizeof(MQInfo));
    UTLT_Assert(mq, return (MQId) NULL, "");

    sprintf(mq->name, "/free5gc_upfmq_%ld", TimeNow());

    // Create msg queue
    mq_unlink(mq->name);
    mq->mqd = mq_open(mq->name, oflag | O_CREAT, 0666, NULL);
    mq->oflag = oflag;
    UTLT_Assert(mq->mqd >= 0, return (MQId) NULL, 
                "Error creating message queue: %s", strerror(errno));

    // Get msg queue attributes
    struct mq_attr mqAttr;
    UTLT_Assert(mq_getattr(mq->mqd, &mqAttr) >= 0,
                mq_close(mq->mqd); mq_unlink(mq->name); return (MQId) NULL,
                "Error getting message queue attributes: %s", strerror(errno));
    mq->msgsize = mqAttr.mq_msgsize;

    return (MQId) mq;
}

Status MQDelete(MQId mqId) {
    MQInfo *mq = (MQInfo*) mqId;

    mq_close(mq->mqd);
    UTLT_Assert(mq_unlink(mq->name) >= 0, return STATUS_ERROR, 
                "Error deleting message queue: %s", strerror(errno));

    UTLT_Free(mq);
    return STATUS_OK;
}

long MQGetMsgSize(MQId mqId) {
    return ((MQInfo*) mqId)->msgsize;
}

Status MQSend(MQId mqId, const char *msg, int msgLen) {
    MQInfo *mq = (MQInfo*) mqId;

    if (mq_send(mq->mqd, msg, msgLen, 0) < 0) {
        if (errno == EAGAIN) {
            return STATUS_EAGAIN;
        } else {
            UTLT_Assert(0, return STATUS_ERROR, 
                        "Error sending message: %s", strerror(errno));
        }
    }

    return STATUS_OK;
}

Status MQRecv(MQId mqId, char *msg, int msgLen) {
    MQInfo *mq = (MQInfo*) mqId;

    UTLT_Assert(msgLen >= mq->msgsize, return STATUS_ERROR,
                "Receive buffer size must larger or equal to the msg size of the queue");

    if (mq_receive(mq->mqd, msg, msgLen, NULL) < 0) {
        if (errno == EAGAIN) {
            return STATUS_EAGAIN;
        } else {
            UTLT_Assert(0, return STATUS_ERROR, 
                    "Error receiving message: %s", strerror(errno));
        }
    }

    return STATUS_OK;
}
