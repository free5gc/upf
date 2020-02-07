#include <stdio.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_buff.h"
#include "utlt_thread.h"

#define THREAD_NUM  5
#define FUNC_ADD_MAX    1111

static pthread_mutex_t mutex;
static int count = 0;

static ThreadID tid[THREAD_NUM];

static void ThreadFunc(ThreadID id, void *data) {
    int i;

    for (i = 0; i < FUNC_ADD_MAX; i++) {
        pthread_mutex_lock(&mutex);
        count++;
        pthread_mutex_unlock(&mutex);
    }

    return;
}

Status TestThread_1() {
    Status status;
    int i;
    ThreadFuncType func = ThreadFunc;

    status = pthread_mutex_init(&mutex, NULL);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "pthread_mutex_init error");
    
    for (i = 0; i < THREAD_NUM; i++) {
        status = ThreadCreate(&tid[i], func, NULL);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Thread tid[%d] create error", i);
    }

    for (i = 0; i < THREAD_NUM; i++) {
        status = ThreadJoin(tid[i]);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Thread tid[%d] join error", i);
    }
    
    UTLT_Assert(count == THREAD_NUM * FUNC_ADD_MAX, return STATUS_ERROR, "Count of ThreadFunc error : need %d, not %d", THREAD_NUM * FUNC_ADD_MAX, count);
    
    status = pthread_mutex_destroy(&mutex);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Thread create error");
    
    return STATUS_OK;
}

Status ThreadTest(void *data) {
    Status status;

    status = BufblkPoolInit();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolInit fail")

    status = ThreadInit();
    UTLT_Assert(status == STATUS_OK, return status, "ThreadInit fail");

    status = TestThread_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestThread_1 fail");

    status = ThreadFinal();
    UTLT_Assert(status == STATUS_OK, return status, "ThreadFinal fail");

    status = BufblkPoolFinal();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolFinal fail");

    return STATUS_OK;
}
