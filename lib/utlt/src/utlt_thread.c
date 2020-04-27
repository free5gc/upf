#include "utlt_thread.h"

#include <string.h>
#include <stdio.h>

struct ThreadInfo {
    pthread_t thread;
    sem_t *semaphore;
};

PoolDeclare(threadPool, Thread, MAX_NUM_OF_THREAD);

static struct ThreadInfo threadStopCheck;

int ThreadStop() {
    return (threadStopCheck.thread == pthread_self());
}

Status ThreadInit() {
    PoolInit(&threadPool, MAX_NUM_OF_THREAD);

    memset(&threadStopCheck, 0, sizeof(threadStopCheck));

    // semaphore create
    sem_t *sem = NULL;
    sem = sem_open("semThreadInit", O_CREAT | O_EXCL, 0644, 0);
    UTLT_Assert(sem != SEM_FAILED, return STATUS_ERROR, "Thread initialize sem_open error");
    sem_unlink("semThreadInit");
    threadStopCheck.semaphore = sem;

    return STATUS_OK;
}

Status ThreadFinal() {
    Status status;

    PoolTerminate(&threadPool);

    status = sem_close(threadStopCheck.semaphore);
    UTLT_Assert(status == 0, return STATUS_ERROR, "Thread final sem_post error");

    return STATUS_OK;
}

static void *StartRoutine(void *th) {
    Status status;
    ThreadFuncType func;
    Thread *thread = (Thread *)th;

    thread->tid = pthread_self();
    status = sem_post(thread->semaphore);
    UTLT_Assert(status == STATUS_OK, return (void *)STATUS_ERROR, "Thread start routine sem_post error");

    if (!ThreadStop()) {
        func = thread->func;
        func((ThreadID)thread, thread->data);
    }
    status = sem_post(threadStopCheck.semaphore);
    UTLT_Assert(status == STATUS_OK, return (void *)STATUS_ERROR, "Thread start routine sem_post error");

    return func;
}

Status ThreadCreate(ThreadID *id, ThreadFuncType func, void *data) {
    Status status;
    Thread *thread = NULL;
    PoolAlloc(&threadPool, thread);
    UTLT_Assert(thread, return STATUS_ERROR, "Thread pool is empty");

    memset(thread, 0, sizeof(ThreadID));
    thread->func = func;
    thread->data = data;

    // semaphore create
    sem_t *sem = NULL;
    sem = sem_open("semThreadCreate", O_CREAT | O_EXCL, 0644, 0);
    UTLT_Assert(sem != SEM_FAILED, return STATUS_ERROR, "Thread create sem_open error");
    sem_unlink("semThreadCreate");
    thread->semaphore = sem;

    status = pthread_create(&thread->tid, NULL, StartRoutine, thread);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Thread create pthread_create error");

    status = sem_wait(thread->semaphore);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Thread create sem_wait error");

    *id = (ThreadID)thread;

    return STATUS_OK;
}

Status ThreadDelete(ThreadID id) {
    Status status;
    Thread *thread = (Thread *)id;

    UTLT_Assert(thread, return STATUS_ERROR, "");

    threadStopCheck.thread = thread->tid;
    
    status = sem_wait(thread->semaphore);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Thread delete sem_wait error");

    threadStopCheck.thread = 0;

    status = pthread_join(thread->tid, 0);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "Thread delete pthread_join error");

    sem_close(thread->semaphore);
    PoolFree(&threadPool, thread);

    return STATUS_OK;
}

Status ThreadJoin(ThreadID id) {
    Status status;
    Thread *thread = (Thread *)id;

    status = pthread_join(thread->tid, NULL);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "pthread_join error");
    
    sem_close(thread->semaphore);
    PoolFree(&threadPool, thread);

    return STATUS_OK;
}
