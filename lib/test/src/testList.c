#include <stdio.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_list.h"

#define SIZE_OF_LIST 6

typedef struct _ListType {
    ListHead node;
    int cmpItem;
} ListType;

ListHead testList ={NULL, NULL};

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

    ListHeadInit(&testList);

    ListInsert(&testNode[0], &testList);

    iter = ListFirst(&testList);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List append error : need %d, not %d", &testNode[0], iter);
   
    ListInsert(&testNode[1], &testList);
    iter = ListFirst(&testList);
    UTLT_Assert(iter == &testNode[1], return STATUS_ERROR, "List append error : need %d, not %d", &testNode[1], iter);
    
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List append error : need %d, not %d", &testNode[0], iter);

    ListInsert(&testNode[2], &testNode[0]);
    iter = ListFirst(&testList);
    iter = ListNext(iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[2], return STATUS_ERROR, "List append error : need %d, not %d", &testNode[2], iter);
    iter = ListPrev(iter);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List append error : need %d, not %d", &testNode[0], iter);

    ListInsert(&testNode[3], &testNode[0]);
    ListInsert(&testNode[4], &testNode[1]);
    ListInsert(&testNode[5], &testNode[2]);

    iter = ListFirst(&testList);
    UTLT_Assert(iter == &testNode[1], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[1], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[4], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[4], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[3], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[3], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[2], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[2], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[5], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[5], iter);
    
    ListRemove(&testNode[3]);
    iter = ListFirst(&testList);
    UTLT_Assert(iter == &testNode[1], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[1], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[4], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[4], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[2], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[2], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[5], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[5], iter);
    
    ListRemove(&testNode[4]);
    iter = ListFirst(&testList);
    UTLT_Assert(iter == &testNode[1], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[1], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[2], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[2], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[5], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[5], iter);
    
    ListRemove(&testNode[5]);
    iter = ListFirst(&testList);
    UTLT_Assert(iter == &testNode[1], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[1], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[0], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[0], iter);
    iter = ListNext(iter);
    UTLT_Assert(iter == &testNode[2], return STATUS_ERROR, "List insert to previous error : need %d, not %d", &testNode[2], iter);

    ListRemove(&testNode[0]);
    ListRemove(&testNode[1]);
    ListRemove(&testNode[2]);
    UTLT_Assert( testList.prev == testList.next, return STATUS_ERROR, "The list is not NULL");

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
        ListHeadInit(&testList);

        for (j = 0; j < 4; j++)
            ListInsertSorted(&testNode[order[i][j]], &testList, &listCompare);
        j = 0;
        iter = ListFirst(&testList);
        while (iter) {
            UTLT_Assert(iter->cmpItem == j, return STATUS_ERROR, "List insert sorted error : need %d, not %d", j, iter->cmpItem);
            j++;
            iter = ListNext(iter);
            if (testList.next == (ListHead *)iter->node.next) {
                break;
            }
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
