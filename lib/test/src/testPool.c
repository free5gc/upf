#include <stdio.h>
#include <stdint.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_pool.h"

#define MAX_NUM_OF_SIZE 0xff

#define MAX_NUM_OF_CAP  0x7f
#define SMALL_NUM_OF_CAP  0x03

typedef uint8_t poolCluster[MAX_NUM_OF_SIZE];

PoolDeclare(testPool, poolCluster, MAX_NUM_OF_CAP);
PoolDeclare(smallPool, poolCluster, SMALL_NUM_OF_CAP);

Status TestPool_1() {
    PoolInit(&testPool, MAX_NUM_OF_CAP);
//    printf("[Testing] Record the first available address = %p\n", testPool.queueAvail[0]);
//    printf("[Testing] After init, get pool Size = %d, Cap = %d\n", PoolSize(&testPool), PoolCap(&testPool));
    UTLT_Assert(PoolSize(&testPool) == MAX_NUM_OF_CAP, return STATUS_ERROR, "The size of pool is %d, not %d", MAX_NUM_OF_CAP, PoolSize(&testPool));
    UTLT_Assert(PoolCap(&testPool) == MAX_NUM_OF_CAP, return STATUS_ERROR, "The capacity of pool is %d, not %d", MAX_NUM_OF_CAP, PoolSize(&testPool));

    poolCluster *poolBuf;
    PoolAlloc(&testPool, poolBuf);
//    printf("[Testing] After alloc, get pool Size = %d, Cap = %d\n", PoolSize(&testPool), PoolCap(&testPool));
    UTLT_Assert(PoolSize(&testPool) == MAX_NUM_OF_CAP - 1, return STATUS_ERROR, "The size of pool is %d, not %d", MAX_NUM_OF_CAP - 1, PoolSize(&testPool));

    (*poolBuf)[0] = 0;
    for(int i = 1; i < MAX_NUM_OF_SIZE; i++) {
        (*poolBuf)[i] = 1 + (*poolBuf)[i - 1];
    }
//    printf("[Testing] Record alloc address = %p and the last one value is %d\n", *poolBuf, (*poolBuf)[MAX_NUM_OF_SIZE - 1]);

    PoolFree(&testPool, poolBuf);
//    printf("[Testing] After alloc, get pool Size = %d, Cap = %d\n", PoolSize(&testPool), PoolCap(&testPool));
    UTLT_Assert(PoolSize(&testPool) == MAX_NUM_OF_CAP, return STATUS_ERROR, "The size of pool is %d, not %d", MAX_NUM_OF_CAP, PoolSize(&testPool));

//    printf("[Testing] Free address is %p\n", testPool.queueAvail[MAX_NUM_OF_CAP]);
    UTLT_Assert(testPool.queueAvail[MAX_NUM_OF_CAP] == testPool.queueAvail[0], return STATUS_ERROR, 
        "Free error memory address, the address should be %p, not %p", testPool.queueAvail[0], testPool.queueAvail[MAX_NUM_OF_CAP]);

    PoolTerminate(&testPool);

    return STATUS_OK;
}

Status TestPool_2() {
    PoolInit(&smallPool, SMALL_NUM_OF_CAP);
    poolCluster *poolBuf[3], *nullBuf;
    PoolAlloc(&smallPool, poolBuf[0]);
    PoolAlloc(&smallPool, poolBuf[1]);
//    printf("[Testing] After alloc 2 buffer, get pool Size = %d, Cap = %d\n", PoolSize(&smallPool), PoolCap(&smallPool));
    UTLT_Assert(PoolSize(&smallPool) == SMALL_NUM_OF_CAP - 2, return STATUS_ERROR, "The size of pool is %d, not %d", SMALL_NUM_OF_CAP - 2, PoolSize(&smallPool));

    PoolAlloc(&smallPool, poolBuf[2]);
//    printf("[Testing] After alloc 3 buffer, get pool Size = %d, Cap = %d\n", PoolSize(&smallPool), PoolCap(&smallPool));
    UTLT_Assert(PoolSize(&smallPool) == SMALL_NUM_OF_CAP - 3, return STATUS_ERROR, "The size of pool is %d, not %d", SMALL_NUM_OF_CAP - 3, PoolSize(&smallPool));

    PoolAlloc(&smallPool, nullBuf);
//    printf("[Testing] Alloc but not source, get pool Size = %d, Cap = %d\n", PoolSize(&smallPool), PoolCap(&smallPool));
    UTLT_Assert(PoolSize(&smallPool) == SMALL_NUM_OF_CAP - 3, return STATUS_ERROR, "The size of pool is %d, not %d", SMALL_NUM_OF_CAP - 3, PoolSize(&smallPool));
    UTLT_Assert(nullBuf == NULL, return STATUS_ERROR, "The pool is empty, but it still can be taken");

    PoolFree(&smallPool, poolBuf[1]);
//    printf("[Testing] After free 1 buffer, get pool Size = %d, Cap = %d\n", PoolSize(&smallPool), PoolCap(&smallPool));
    UTLT_Assert(PoolSize(&smallPool) == SMALL_NUM_OF_CAP - 2, return STATUS_ERROR, "The size of pool is %d, not %d", SMALL_NUM_OF_CAP - 2, PoolSize(&smallPool));

    PoolFree(&smallPool, poolBuf[0]);
//    printf("[Testing] After free 2 buffer, get pool Size = %d, Cap = %d\n", PoolSize(&smallPool), PoolCap(&smallPool));
    UTLT_Assert(PoolSize(&smallPool) == SMALL_NUM_OF_CAP - 1, return STATUS_ERROR, "The size of pool is %d, not %d", SMALL_NUM_OF_CAP - 1, PoolSize(&smallPool));

    PoolAlloc(&smallPool, poolBuf[0]);
//    printf("[Testing] After alloc 1 buffer, get pool Size = %d, Cap = %d\n", PoolSize(&smallPool), PoolCap(&smallPool));
    UTLT_Assert(PoolSize(&smallPool) == SMALL_NUM_OF_CAP - 2, return STATUS_ERROR, "The size of pool is %d, not %d", SMALL_NUM_OF_CAP - 2, PoolSize(&smallPool));

    PoolFree(&smallPool, poolBuf[0]);
    PoolFree(&smallPool, poolBuf[2]);
//    printf("[Testing] After free 2 buffer, get pool Size = %d, Cap = %d\n", PoolSize(&smallPool), PoolCap(&smallPool));
    UTLT_Assert(PoolSize(&smallPool) == SMALL_NUM_OF_CAP, return STATUS_ERROR, "The size of pool is %d, not %d", SMALL_NUM_OF_CAP, PoolSize(&smallPool));

    PoolFree(&smallPool, poolBuf[0]);
//    printf("[Testing] Free but the pool is full, get pool Size = %d, Cap = %d\n", PoolSize(&smallPool), PoolCap(&smallPool));
    UTLT_Assert(PoolSize(&smallPool) == SMALL_NUM_OF_CAP, return STATUS_ERROR, "The size of pool is %d, not %d", SMALL_NUM_OF_CAP, PoolSize(&smallPool));

    PoolTerminate(&smallPool);

    return STATUS_OK;
}

Status PoolTest(void *data) {
    Status status;

    status = TestPool_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestPool_1 fail");

    status = TestPool_2();
    UTLT_Assert(status == STATUS_OK, return status, "TestPool_2 fail");

    return STATUS_OK;
}
