#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_buff.h"
#include "utlt_timer.h"

uint8_t expireCheck[5];

typedef struct _TestTimerEliment {
    int type;
    uint32_t duration;
} TestTimerEliment;

// test both type of timer
TestTimerEliment timerEliment[] ={
    {TIMER_TYPE_ONCE, 30},
    {TIMER_TYPE_PERIOD, 50},
    {TIMER_TYPE_PERIOD, 70},
    {TIMER_TYPE_ONCE, 130}
};

void TestExpireFunc(uintptr_t data, uintptr_t param[]) {
    uint32_t index = param[1];

    expireCheck[index]++;
}

Status TestTimer_1() {
    int n = 0;
    TimerBlkID idArray[5];
    TimerBlkID id;
    TimerList timerList;

    memset((char*)idArray, 0x00, sizeof(timerList));
    memset(expireCheck, 0x00, sizeof(expireCheck));

    TimerListInit(&timerList);

    for(n = 0; n < sizeof(timerEliment) / sizeof(TestTimerEliment); n++) {
        idArray[n] = TimerCreate(&timerList, timerEliment[n].type,
                timerEliment[n].duration, TestExpireFunc);
        
        TimerSet(PARAM1, idArray[n], (uintptr_t)idArray[n]);
        TimerSet(PARAM2, idArray[n], n);
    }
    
    for(n = 0; n < sizeof(timerEliment) / sizeof(TestTimerEliment); n++)
        TimerStart(idArray[n]);

    id = (TimerBlkID)ListFirst(&timerList.idle);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer idle list error : need %d, not %d", 0, id);

    id = (TimerBlkID)ListFirst(&timerList.active);
    UTLT_Assert(id == idArray[0], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[0], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[1], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[1], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[2], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[2], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[3], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[3], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer active list error : need %d, not %d", 0, id);
    
    usleep(40 * 1000);  // Sleep for the specified number of micro-seconds.
    TimerExpireCheck(&timerList, 0);

    id = (TimerBlkID)ListFirst(&timerList.idle);
    UTLT_Assert(id == idArray[0], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[0], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer idle list error : need %d, not %d", 0, id);

    id = (TimerBlkID)ListFirst(&timerList.active);
    UTLT_Assert(id == idArray[1], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[1], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[2], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[2], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[3], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[3], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer active list error : need %d, not %d", 0, id);
    
    UTLT_Assert(expireCheck[0] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 0, 1, expireCheck[0]);
    UTLT_Assert(expireCheck[1] == 0, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 1, 0, expireCheck[1]);
    UTLT_Assert(expireCheck[2] == 0, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 2, 0, expireCheck[2]);
    UTLT_Assert(expireCheck[3] == 0, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 3, 0, expireCheck[3]);
    
    usleep(20 * 1000);  // 40 + 20
    TimerExpireCheck(&timerList, 0);

    id = (TimerBlkID)ListFirst(&timerList.idle);
    UTLT_Assert(id == idArray[0], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[0], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer idle list error : need %d, not %d", 0, id);
    
    id = (TimerBlkID)ListFirst(&timerList.active);
    UTLT_Assert(id == idArray[2], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[2], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[1], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[1], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[3], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[3], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer active list error : need %d, not %d", 0, id);
    
    UTLT_Assert(expireCheck[0] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 0, 1, expireCheck[0]);
    UTLT_Assert(expireCheck[1] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 1, 1, expireCheck[1]);
    UTLT_Assert(expireCheck[2] == 0, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 2, 0, expireCheck[2]);
    UTLT_Assert(expireCheck[3] == 0, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 3, 0, expireCheck[3]);
    
    usleep(20 * 1000);  // 40 + 20 + 20
    TimerExpireCheck(&timerList, 0);

    id = (TimerBlkID)ListFirst(&timerList.idle);
    UTLT_Assert(id == idArray[0], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[0], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer idle list error : need %d, not %d", 0, id);

    id = (TimerBlkID)ListFirst(&timerList.active);
    UTLT_Assert(id == idArray[1], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[1], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[3], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[3], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[2], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[2], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer active list error : need %d, not %d", 0, id);
    
    UTLT_Assert(expireCheck[0] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 0, 1, expireCheck[0]);
    UTLT_Assert(expireCheck[1] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 1, 1, expireCheck[1]);
    UTLT_Assert(expireCheck[2] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 2, 1, expireCheck[2]);
    UTLT_Assert(expireCheck[3] == 0, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 3, 0, expireCheck[3]);
    
    TimerStop(idArray[1]);  // stop period timer idArray[1]

    usleep(40 * 1000); // 40 + 20 + 20 + 40
    TimerExpireCheck(&timerList, 0);

    id = (TimerBlkID)ListFirst(&timerList.idle);
    UTLT_Assert(id == idArray[0], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[0], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[1], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[1], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer idle list error : need %d, not %d", 0, id);

    id = (TimerBlkID)ListFirst(&timerList.active);
    UTLT_Assert(id == idArray[3], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[3], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[2], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[2], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer active list error : need %d, not %d", 0, id);
    
    UTLT_Assert(expireCheck[0] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 0, 1, expireCheck[0]);
    UTLT_Assert(expireCheck[1] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 1, 1, expireCheck[1]);
    UTLT_Assert(expireCheck[2] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 2, 1, expireCheck[2]);
    UTLT_Assert(expireCheck[3] == 0, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 3, 0, expireCheck[3]);
    
    usleep(15 * 1000); // 40 + 20 + 20 + 40 + 15
    TimerExpireCheck(&timerList, 0);

    id = (TimerBlkID)ListFirst(&timerList.idle);
    UTLT_Assert(id == idArray[0], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[0], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[1], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[1], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[3], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[3], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer idle list error : need %d, not %d", 0, id);

    id = (TimerBlkID)ListFirst(&timerList.active);
    UTLT_Assert(id == idArray[2], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[2], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer active list error : need %d, not %d", 0, id);
    
    UTLT_Assert(expireCheck[0] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 0, 1, expireCheck[0]);
    UTLT_Assert(expireCheck[1] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 1, 1, expireCheck[1]);
    UTLT_Assert(expireCheck[2] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 2, 1, expireCheck[2]);
    UTLT_Assert(expireCheck[3] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 3, 1, expireCheck[3]);
    
    usleep(20 * 1000); // 40 + 20 + 20 + 40 + 15 + 20
    TimerExpireCheck(&timerList, 0);

    id = (TimerBlkID)ListFirst(&timerList.idle);
    UTLT_Assert(id == idArray[0], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[0], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[1], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[1], id);
    id = (TimerBlkID)ListNext(id);
    UTLT_Assert(id == idArray[3], return STATUS_ERROR, "Timer idle list error : need %d, not %d", idArray[3], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer idle list error : need %d, not %d", 0, id);

    id = (TimerBlkID)ListFirst(&timerList.active);
    UTLT_Assert(id == idArray[2], return STATUS_ERROR, "Timer active list error : need %d, not %d", idArray[2], id);
    id = (TimerBlkID)ListNext(id);
    // UTLT_Assert(id == 0, return STATUS_ERROR, "Timer active list error : need %d, not %d", 0, id);
    
    UTLT_Assert(expireCheck[0] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 0, 1, expireCheck[0]);
    UTLT_Assert(expireCheck[1] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 1, 1, expireCheck[1]);
    UTLT_Assert(expireCheck[2] == 2, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 2, 2, expireCheck[2]);
    UTLT_Assert(expireCheck[3] == 1, return STATUS_ERROR, "Expire check [%d] error : need %d, not %d", 3, 1, expireCheck[3]);
    
    for( n = 0; n < sizeof(timerEliment) / sizeof(TestTimerEliment); n++)
        TimerDelete(idArray[n]);

    UTLT_Assert(TimerGetPoolSize() == MAX_NUM_OF_TIMER, return STATUS_ERROR, "Pool size error : need %d, not %d", MAX_NUM_OF_TIMER, TimerGetPoolSize());

    return STATUS_OK;
}

Status TimerTest(void *data) {
    Status status;

    status = BufblkPoolInit();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolInit fail");

    status = TimerPoolInit();
    UTLT_Assert(status == STATUS_OK, return status, "TimerPoolInit fail");

    status = TestTimer_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestTimer_1 fail");

    status = TimerFinal();
    UTLT_Assert(status == STATUS_OK, return status, "TimerFinal fail");

    status = BufblkPoolFinal();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolFinal fail");

    return STATUS_OK;
}
