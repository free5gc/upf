#include "utlt_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utlt_debug.h"
#include "utlt_list.h"

TestContext testSelf;

Status TestInit() {
    testSelf.totalCase = 0;
    testSelf.finishCase = 0;
    
    ListHeadInit(&testSelf.node);

    return STATUS_OK;
}

Status TestTerminate() {

    for (TestNode *it = ListFirst(&testSelf.node); it != NULL; it = ListNext(it)) {
        ListRemove(it);
        free(it);
    }

    testSelf.finishCase = 0;
    testSelf.totalCase = 0;

    return STATUS_OK;
}

Status TestAdd(TestCase *testCase) {
    UTLT_Assert(testCase, return STATUS_ERROR, "TestNode is NULL");

    TestNode *newNode = malloc(sizeof(TestNode));
    memcpy(&newNode->test, testCase, sizeof(TestCase));

    ListInsert(newNode, &testSelf.node);
    testSelf.totalCase += 1;

    return STATUS_OK;
}

Status TestAddList(TestCase *testList, int size) {
    UTLT_Assert(testList, return STATUS_ERROR, "TestNode is NULL");
    UTLT_Assert(size >= 0, return STATUS_ERROR, "Size of test list must be non-negative");

    for (int i = 0; i < size; i++) {
        UTLT_Assert(TestAdd(&testList[i]) == STATUS_OK, return STATUS_ERROR, "TestAdd fail");
    }

    return STATUS_OK;
}

Status TestRun() {
    Status status;
    TestNode *it, *nextIt = NULL;
    ListForEachSafe(it, nextIt, &testSelf.node) {
        printf("[%d/%d][%s] : ", ++testSelf.finishCase, testSelf.totalCase, it->test.name);
        fflush(stdout);

        status = it->test.FuncPtr(it->test.data);

        printf("%s\n", UTLT_StrStatus(status));
        fflush(stdout);
    }

    return STATUS_OK;
}

int TestCaseArrayFindByName(TestCase *array, int size, const char *targetName) {
    for (int i = 0; i < size; i++) {
        if (strcmp(array[i].name, targetName) == 0) {
            return i;
        }
    }

    return -1;
}
