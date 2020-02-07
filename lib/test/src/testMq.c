#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_buff.h"
#include "utlt_mq.h"

static void *TestMQ_4_thread1(void *mqIdAddr);
static void *TestMQ_5_thread1(void *mqIdAddr);

// Single mq open close
Status TestMQ_1() {
    Status status;

    MQId mqId = MQCreate(O_RDWR);
    UTLT_Assert(mqId > 0, return STATUS_ERROR, "");

    status = MQDelete(mqId);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    return STATUS_OK;
}

// Multiple mq open close
Status TestMQ_2() {
    Status status;
    int i;
    MQId mqId[8];

    for (i = 0; i < 8; i++) {
        mqId[i] = MQCreate(O_RDWR);
        UTLT_Assert(mqId[i] > 0, return STATUS_ERROR, "Failed to create mq #%d", i + 1);
    }

    for (i = 0; i < 8; i++) {
        status = MQDelete(mqId[i]);
        UTLT_Assert(status == STATUS_OK,, "");
    }

    return STATUS_OK;
}

// Single mq send recv msg
Status TestMQ_3() {
    Status status;
    char *sendBuf = "Hello@#$%^World!)(*^&";

    MQId mqId = MQCreate(O_RDWR);
    UTLT_Assert(mqId > 0, return STATUS_ERROR, "");

    status = MQSend(mqId, sendBuf, strlen(sendBuf));
    UTLT_Assert(status == STATUS_OK,, "");
    
    // Test insufficient buffer size (smaller than msgsize of the queue)
    // char recvBuf1[10];
    // status = MQRecv(mqId, recvBuf1, 10);
    // UTLT_Assert(status == STATUS_ERROR,, "");

    int msgSize = MQGetMsgSize(mqId);
    char *recvBuf2 = UTLT_Malloc(msgSize);
    status = MQRecv(mqId, recvBuf2, msgSize);
    UTLT_Assert(status == STATUS_OK,, "");
    UTLT_Assert(strcmp(sendBuf, recvBuf2) == 0,, "Received message is not equal to the message sent");
    UTLT_Free(recvBuf2);

    status = MQDelete(mqId);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    return STATUS_OK;
}

// Single blocking mq send recv msg simultaneously
Status TestMQ_4() {
    Status status;
    int i;
    char sendBuf[50];
    pthread_t receiverThread;

    MQId mqId = MQCreate(O_RDWR);
    UTLT_Assert(mqId > 0, return STATUS_ERROR, "");

    pthread_create(&receiverThread, NULL, TestMQ_4_thread1, (void*) &mqId);

    for (i = 0; i < 10; i++) {
        sprintf(sendBuf, "msg_%d", i);
        status = MQSend(mqId, sendBuf, strlen(sendBuf));
        UTLT_Assert(status == STATUS_OK,, "");
        usleep(10 * 1000);
    }
    
    pthread_join(receiverThread, NULL);

    status = MQDelete(mqId);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    return STATUS_OK;
}

// Queue receiver thread of TestMQ_4()
void *TestMQ_4_thread1(void *mqIdAddr) {
    MQId *mqId = (MQId*) mqIdAddr;
    Status status;
    int i;
    char expectedMsg[50];
    int msgSize = MQGetMsgSize(*mqId);
    char *recvBuf = UTLT_Malloc(msgSize);

    for (i = 0; i < 10; i++) {
        status = MQRecv(*mqId, recvBuf, msgSize);
        UTLT_Assert(status != STATUS_ERROR,, "");

        sprintf(expectedMsg, "msg_%d", i);
        // printf("thread, %d, %s\n", i, recvBuf);
        UTLT_Assert(strcmp(expectedMsg, recvBuf) == 0,, "Received message is not equal to the message sent");
    }
    
    UTLT_Free(recvBuf);

    return NULL;
}

// Single nonblocking mq send recv msg simultaneously
Status TestMQ_5() {
    Status status;
    int i;
    char sendBuf[50];
    pthread_t receiverThread;

    MQId mqId = MQCreate(O_RDWR | O_NONBLOCK);
    UTLT_Assert(mqId > 0, return STATUS_ERROR, "");

    pthread_create(&receiverThread, NULL, TestMQ_5_thread1, (void*) &mqId);

    for (i = 0; i < 10; i++) {
        sprintf(sendBuf, "msg_%d", i);
        status = MQSend(mqId, sendBuf, strlen(sendBuf));
        UTLT_Assert(status == STATUS_OK,, "");
        usleep(10 * 1000);
    }
    
    pthread_join(receiverThread, NULL);

    status = MQDelete(mqId);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    return STATUS_OK;
}

// Queue receiver thread of TestMQ_5()
void *TestMQ_5_thread1(void *mqIdAddr) {
    MQId *mqId = (MQId*) mqIdAddr;
    Status status = STATUS_OK;
    int i = 0;
    char expectedMsg[50];
    int msgSize = MQGetMsgSize(*mqId);
    char *recvBuf = UTLT_Malloc(msgSize);

    usleep(5 * 1000);

    while (1) {
        usleep(10 * 1000);
        status = MQRecv(*mqId, recvBuf, msgSize);
        UTLT_Assert(status != STATUS_ERROR,, "");
        if (status == STATUS_EAGAIN)
            break;

        // printf("thread, %d, %s\n", i, recvBuf);
        sprintf(expectedMsg, "msg_%d", i++);
        UTLT_Assert(strcmp(expectedMsg, recvBuf) == 0,, "Received message is not equal to the message sent");
    }
    
    UTLT_Free(recvBuf);

    return NULL;
}

// Single nonblocking mq send msg until queue is full
Status TestMQ_6() {
    Status status;
    long i;

    MQId mqId = MQCreate(O_RDWR | O_NONBLOCK);
    UTLT_Assert(mqId > 0, return STATUS_ERROR, "");

    int msgSize = MQGetMsgSize(mqId);
    char *sendBuf = UTLT_Malloc(msgSize);

    for (i = 0; i < LONG_MAX; i++) {
        status = MQSend(mqId, sendBuf, msgSize);
        
        UTLT_Assert(status != STATUS_ERROR,, "");
        if (status == STATUS_EAGAIN) {
            // printf("Queue is full\n");
            UTLT_Free(sendBuf);
            UTLT_Assert(MQDelete(mqId) == STATUS_OK, return STATUS_ERROR, "");
            return STATUS_OK;
        }
    }
    
    UTLT_Free(sendBuf);
    UTLT_Assert(MQDelete(mqId) == STATUS_OK, return STATUS_ERROR, "");
    return STATUS_ERROR;
}

Status MqTest(void *data) {
    Status status;

    status = BufblkPoolInit();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolInit fail");

    status = TestMQ_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestMQ_1 fail");

    status = TestMQ_2();
    UTLT_Assert(status == STATUS_OK, return status, "TestMQ_2 fail");

    status = TestMQ_3();
    UTLT_Assert(status == STATUS_OK, return status, "TestMQ_3 fail");

    status = TestMQ_4();
    UTLT_Assert(status == STATUS_OK, return status, "TestMQ_2 fail");

    status = TestMQ_5();
    UTLT_Assert(status == STATUS_OK, return status, "TestMQ_5 fail");

    status = TestMQ_6();
    UTLT_Assert(status == STATUS_OK, return status, "TestMQ_6 fail");

    status = BufblkPoolFinal();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolFinal fail");

    return STATUS_OK;
}
