#include <stdio.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_buff.h"
#include "utlt_event.h"

#define TEST_EVENT_TYPE_A 0x0a
#define TEST_EVENT_TYPE_B 0x0b

#define TEST_EVENT_ARG_0 0x00
#define TEST_EVENT_ARG_1 0x01
#define TEST_EVENT_ARG_2 0x02
#define TEST_EVENT_ARG_3 0x03
#define TEST_EVENT_ARG_4 0x04
#define TEST_EVENT_ARG_5 0x05
#define TEST_EVENT_ARG_6 0x06
#define TEST_EVENT_ARG_7 0x07

// Single event queue open close
Status TestEvent_1() {
    Status status;

    EvtQId eqId = EventQueueCreate(EVTQ_O_BLOCK);
    UTLT_Assert(eqId > 0, return STATUS_ERROR, "");

    status = EventQueueDelete(eqId);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    return STATUS_OK;
}

// Multiple event queue open close
Status TestEvent_2() {
    Status status;
    int i;
    EvtQId eqId[8];
    
    for (i = 0; i < 8; i++) {
        eqId[i] = EventQueueCreate(EVTQ_O_BLOCK);
        UTLT_Assert(eqId[i] > 0, return STATUS_ERROR, "");
    }

    for (i = 0; i < 8; i++) {
        status = EventQueueDelete(eqId[i]);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");
    }

    return STATUS_OK;
}

// Single event queue send recv events - 2 params
Status TestEvent_3() {
    Status status;
    Event event;

    EvtQId eqId = EventQueueCreate(O_RDWR);
    UTLT_Assert(eqId > 0, return STATUS_ERROR, "");

    status = EventSend(eqId, TEST_EVENT_TYPE_A, 2, TEST_EVENT_ARG_0, TEST_EVENT_ARG_1);
    UTLT_Assert(status == STATUS_OK,, "");
    
    status = EventRecv(eqId, &event);
    UTLT_Assert(status == STATUS_OK,, "");
    UTLT_Assert(event.argc == 2,, "Received event.argc is not correct");
    UTLT_Assert(event.arg0 == TEST_EVENT_ARG_0,, "Received event.arg0 is not correct");
    UTLT_Assert(event.arg1 == TEST_EVENT_ARG_1,, "Received event.arg1 is not correct");

    status = EventQueueDelete(eqId);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    return STATUS_OK;
}

// Single event queue send recv events - 8 params
Status TestEvent_4() {
    Status status;
    Event event;

    EvtQId eqId = EventQueueCreate(O_RDWR);
    UTLT_Assert(eqId > 0, return STATUS_ERROR, "");

    status = EventSend(eqId, TEST_EVENT_TYPE_B, 8, 
        TEST_EVENT_ARG_0, TEST_EVENT_ARG_1, TEST_EVENT_ARG_2, TEST_EVENT_ARG_3, TEST_EVENT_ARG_4, TEST_EVENT_ARG_5,
        TEST_EVENT_ARG_6, TEST_EVENT_ARG_7);
    UTLT_Assert(status == STATUS_OK,, "");
    
    status = EventRecv(eqId, &event);
    UTLT_Assert(status == STATUS_OK,, "");
    UTLT_Assert(event.argc == 8,, "Received event.argc is not correct");
    UTLT_Assert(event.arg0 == TEST_EVENT_ARG_0,, "Received event.arg0 is not correct");
    UTLT_Assert(event.arg1 == TEST_EVENT_ARG_1,, "Received event.arg1 is not correct");
    UTLT_Assert(event.arg2 == TEST_EVENT_ARG_2,, "Received event.arg2 is not correct");
    UTLT_Assert(event.arg3 == TEST_EVENT_ARG_3,, "Received event.arg3 is not correct");
    UTLT_Assert(event.arg4 == TEST_EVENT_ARG_4,, "Received event.arg4 is not correct");
    UTLT_Assert(event.arg5 == TEST_EVENT_ARG_5,, "Received event.arg5 is not correct");
    UTLT_Assert(event.arg6 == TEST_EVENT_ARG_6,, "Received event.arg6 is not correct");
    UTLT_Assert(event.arg7 == TEST_EVENT_ARG_7,, "Received event.arg7 is not correct");

    status = EventQueueDelete(eqId);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    return STATUS_OK;
}

Status EventTest(void *data) {
    Status status;
    
    status = BufblkPoolInit();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolInit fail");
    
    status = TestEvent_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestEvent_1 fail");

    status = TestEvent_2();
    UTLT_Assert(status == STATUS_OK, return status, "TestEvent_2 fail");

    status = TestEvent_3();
    UTLT_Assert(status == STATUS_OK, return status, "TestEvent_3 fail");

    status = TestEvent_4();
    UTLT_Assert(status == STATUS_OK, return status, "TestEvent_4 fail");

    status = BufblkPoolFinal();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolFinal fail");
 
    return STATUS_OK;
}
