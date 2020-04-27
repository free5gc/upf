#include "utlt_event.h"

#include <stdlib.h>

#include "utlt_buff.h"
#include "utlt_timer.h"

typedef struct {
    MQId mqId;
    long msgsize;
} EvtQInfo;

EvtQId EventQueueCreate(int option) {
    EvtQInfo *evtq = UTLT_Malloc(sizeof(EvtQInfo));
    UTLT_Assert(evtq, return (EvtQId) NULL, "");

    evtq->mqId = MQCreate(O_RDWR | option);

    if (evtq->mqId != (MQId) NULL) {
        evtq->msgsize = MQGetMsgSize(evtq->mqId);
        return (EvtQId) evtq;
    } else {
        return (EvtQId) NULL;
    }
}

Status EventQueueDelete(EvtQId eqId) {
    EvtQInfo *evtq = (EvtQInfo*) eqId;
    Status status;

    UTLT_Assert(evtq, return STATUS_ERROR, "");
    status = MQDelete(evtq->mqId);

    UTLT_Free(evtq);
    return status;
}

Status EventSend(EvtQId eqId, uintptr_t eventType, int argc, ...) {
    EvtQInfo *evtq = (EvtQInfo*) eqId;
    va_list ap;
    Event event;
    uintptr_t *ptr;
    int i;

    UTLT_Assert(evtq, return STATUS_ERROR, "");
    UTLT_Assert(argc <= 8, return STATUS_ERROR, "Number of event parameters should not more than 8");
    event.type = eventType;
    event.argc = argc;

    va_start(ap, argc);
    ptr = &event.arg0;
    for (i = 0; i < argc; i++) {
        *ptr = va_arg(ap, uintptr_t);
        ptr++;
    }
    va_end(ap);

    return MQSend(evtq->mqId, (const char*) &event, sizeof(Event));
}

Status EventRecv(EvtQId eqId, Event *event) {
    EvtQInfo *evtq = (EvtQInfo*) eqId;
    UTLT_Assert(evtq, return STATUS_ERROR, "");

    return MQRecv(evtq->mqId, (char*) event, evtq->msgsize);
}

void EventTimerExpire(uintptr_t data, uintptr_t param[]) {
    EvtQId queue = data;
    Event event;
    Status status;

    UTLT_Assert(queue, return, "queue error");

    event.type = param[0];
    event.arg0 = param[1];
    event.arg1 = param[2];
    event.arg2 = param[3];
    event.arg3 = param[4];
    event.arg4 = param[5];

    status = EventSend(queue, event.type, 5, event.arg0, event.arg1, event.arg2, event.arg3, event.arg4);

    if (status != STATUS_OK) {
        UTLT_Error("event send error: %d", status);
    }
}

TimerBlkID EventTimerCreate(TimerList *timerList, int type, uint32_t duration, uintptr_t event) {
    TimerBlkID id;

    id = TimerCreate(timerList, type, duration, EventTimerExpire);
    TimerSet(PARAM1, id, event);

    return id;
}
