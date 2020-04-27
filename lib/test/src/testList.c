#include <stdio.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_list.h"

#define SIZE_OF_LIST 6

typedef struct _ListType {
    ListNode node;
    int cmpItem;
} ListType;

ListNode testList;

int listCompare(ListType *pNode1, ListType *pNode2) {
    if (pNode1->cmpItem == pNode2->cmpItem)
        return 0;
    else if (pNode1->cmpItem < pNode2->cmpItem)
        return -1;
    else
        return 1;
}

Status TestList_1() {
    int i;
    ListType *iter, testNode[SIZE_OF_LIST];

    for (i = 0; i < SIZE_OF_LIST; i++)
        testNode[i].cmpItem = i;

    ListInit(&testList);
    UTLT_Assert(ListIsEmpty(&testList), return STATUS_ERROR, "The list is not NULL");

    iter = ListFirst(&testList);
    UTLT_Assert(iter == NULL, return STATUS_ERROR, "The first node of list is not NULL");

    ListAppend(&testList, &testNode[0]);

    iter = ListFirst(&testList);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List append error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == NULL, return STATUS_ERROR, "The next node of list is not NULL");

    iter = ListLast(&testList);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List last error : need %d, not %d", &testNode[0], iter);
    iter = ListPrev(iter);
    UTLT_Assert(iter == NULL, return STATUS_ERROR, "The previous node of list is not NULL");

    ListAppend(&testList, &testNode[1]);
    ListAppend(&testList, &testNode[2]);

    ListInsertToPrev(&testList, &testNode[0], &testNode[3]);
    ListInsertToPrev(&testList, &testNode[1], &testNode[4]);
    ListInsertToPrev(&testList, &testNode[2], &testNode[5]);

    iter = ListFirst(&testList);
    UTLT_Assert(iter == &testNode[3], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[4], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[1], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[5], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[2], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == NULL, return STATUS_ERROR, "The next node of list is not NULL");

    ListRemove(&testList, &testNode[3]);
    ListRemove(&testList, &testNode[4]);
    ListRemove(&testList, &testNode[5]);

    iter = ListFirst(&testList);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List remove error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[1], return STATUS_ERROR, "List remove error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[2], return STATUS_ERROR, "List remove error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == NULL, return STATUS_ERROR, "The next node of list is not NULL");

    ListRemove(&testList, &testNode[0]);
    ListRemove(&testList, &testNode[1]);
    ListRemove(&testList, &testNode[2]);
    UTLT_Assert(ListIsEmpty(&testList), return STATUS_ERROR, "The list is not NULL");

    return STATUS_OK;
}

Status TestList_2() {
    int i, j;
    ListType *iter, testNode[SIZE_OF_LIST];

    int order[24][4] = {
        {0,1,2,3}, {0,1,3,2}, {0,2,1,3}, {0,2,3,1}, {0,3,1,2}, {0,3,2,1},
        {1,0,2,3}, {1,0,3,2}, {1,2,0,3}, {1,2,3,0}, {1,3,0,2}, {1,3,2,0},
        {2,1,0,3}, {2,1,3,0}, {2,0,1,3}, {2,0,3,1}, {2,3,1,0}, {2,3,0,1},
        {3,1,2,0}, {3,1,0,2}, {3,2,1,0}, {3,2,0,1}, {3,0,1,2}, {3,0,2,1}
    };

    for (i = 0; i < SIZE_OF_LIST; i++)
        testNode[i].cmpItem = i;

    for (i = 0; i < 24; i++) {
        ListInit(&testList);
        UTLT_Assert(ListIsEmpty(&testList), return STATUS_ERROR, "The list is not NULL");

        for (j = 0; j < 4; j++)
            ListInsertSorted(&testList, &testNode[order[i][j]], &listCompare);

        j = 0;
        iter = ListFirst(&testList);
        while (iter) {
            UTLT_Assert(iter->cmpItem == j, return STATUS_ERROR, "List insert sorted error : need %d, not %d", j, iter->cmpItem);
            j++;
            iter = ListNext(iter);
        }
    }

    return STATUS_OK;
}

Status ListTest(void *data) {
    Status status;

    status = TestList_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestList_1 fail");

    status = TestList_2();
    UTLT_Assert(status == STATUS_OK, return status, "TestList_2 fail");

    return STATUS_OK;
}
