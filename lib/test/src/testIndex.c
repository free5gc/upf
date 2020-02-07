#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_index.h"

/*****************************************************************************
 * Index is similar to Pool, only add some code from "utlt_pool.h".
 * This file only test the unique function in "utlt_index.h".
 *****************************************************************************/

#define MAX_NUM_OF_CAP  0x10

int globalCnt = 0;

typedef struct {
    int index;
    int x, y;
} Pos;

IndexDeclare(testPos, Pos, MAX_NUM_OF_CAP);

Status TestIndex_1() {
    UTLT_Assert(IndexSize(&testPos) == MAX_NUM_OF_CAP, return STATUS_ERROR, 
                "The size of index is %d, not %d", MAX_NUM_OF_CAP, IndexSize(&testPos));
    UTLT_Assert(IndexCap(&testPos) == MAX_NUM_OF_CAP, return STATUS_ERROR, 
                "The capacity of index is %d, not %d", MAX_NUM_OF_CAP, IndexSize(&testPos));

    Pos *p1 = NULL;
    IndexAlloc(&testPos, p1);
    UTLT_Assert(p1, return STATUS_ERROR, "IndexAlloc fail");
    UTLT_Assert(IndexSize(&testPos) == MAX_NUM_OF_CAP - 1, return STATUS_ERROR, 
                "The size of index is %d, not %d", MAX_NUM_OF_CAP - 1, IndexSize(&testPos));
    UTLT_Assert(p1->index == 0, return STATUS_ERROR, 
                "This index is wrong, need index[%d] not index[%d]", globalCnt, p1->index);
    
    Pos *ptr = IndexFind(&testPos, p1->index);
    UTLT_Assert(p1 == ptr, return STATUS_ERROR, 
                "IndexFind fail, need address[%p] not address[%p]", p1, ptr);

    IndexFree(&testPos, p1);
    int lastQEnd = (MAX_NUM_OF_CAP + globalCnt) % (MAX_NUM_OF_CAP + 1);
    int lastQFirst = globalCnt % (MAX_NUM_OF_CAP + 1); 
    UTLT_Assert(testPos.queueAvail[lastQEnd] == testPos.queueAvail[lastQFirst],
                return STATUS_ERROR, "IndexFree fail, need address[%p] not address[%p]", 
                testPos.queueAvail[lastQFirst], testPos.queueAvail[lastQEnd]);

    globalCnt++;

    return STATUS_OK;
}

Status IndexTest(void *data) {
    Status status;

    IndexInit(&testPos, MAX_NUM_OF_CAP);

    status = TestIndex_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestIndex_1 fail");

    status = IndexTerminate(&testPos);
    UTLT_Assert(status == STATUS_OK, return status, "IndexTerminate fail");

    return STATUS_OK;
}
