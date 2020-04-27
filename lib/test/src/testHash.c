#include <stdio.h>
#include <string.h>

#include "test_utlt.h"
#include "utlt_hash.h"
#include "utlt_buff.h"

Status TestHash_1() {
    Hash *h = NULL;
    char *result = NULL;

    h = HashMake();
    UTLT_Assert(h, return STATUS_ERROR, "The hash table is NULL");
//    printf("[Testing] Hash table ok\n");
    fflush(stdout);

    HashSet(h, "key", HASH_KEY_STRING, "value");
    result = HashGet(h, "key", HASH_KEY_STRING);
//    printf("[Testing] Hash value = %s\n", result);
    fflush(stdout);
    UTLT_Assert(strcmp("value", result) == 0, return STATUS_ERROR, "Hash set error: need %s, not %s\n", "value", result);

    HashDestroy(h);

    return STATUS_OK;
}

Status TestHash_2() {
    Hash *h;
    int cnt;
    HashIndex *hi;
    
    h = HashMake();
    UTLT_Assert(h, return STATUS_ERROR, "The hash table is NULL");

    HashSet(h, "key_1", HASH_KEY_STRING, "key_1");
    HashSet(h, "key_2", HASH_KEY_STRING, "key_2");
    HashSet(h, "key_3", HASH_KEY_STRING, "key_3");
    HashSet(h, "key_4", HASH_KEY_STRING, "key_4");
    HashSet(h, "key_5", HASH_KEY_STRING, "key_5");

    cnt = HashCount(h);
    // printf("[Testing] Hash count = %d\n", cnt);
    UTLT_Assert(cnt == 5, return STATUS_ERROR, "Hash count error: need %d, not %d", 5, cnt);

    for (hi = HashFirst(h); hi; hi = HashNext(hi)) {
        const char *key = HashThisKey(hi);
        int len = HashThisKeyLen(hi);
        char *val = HashThisVal(hi);

        // printf("[Testing] key = %s, klen = %d, value = %s\n", key, len, val);
        UTLT_Assert(strcmp(key, val) == 0, return STATUS_ERROR,
                    "Hash this key, key len, val error: %s, %d, %s", key, len, val);
    }

    HashClear(h);
    cnt = HashCount(h);
    UTLT_Assert(cnt == 0, return STATUS_ERROR, "Hash count error: need %d, not %d", 0, cnt);

    HashDestroy(h);

    return STATUS_OK;
}

Status TestHash_3() {
    Hash *h = NULL;
    char *result = NULL;

    h = HashMake();
    UTLT_Assert(h, return STATUS_ERROR, "The hash table is NULL");

    result = HashGetOrSet(h, "key", HASH_KEY_STRING, "value");
    UTLT_Assert(strcmp("value", result) == 0, return STATUS_ERROR, "Hash get or set error: need %s, not %s\n", "value", result);

    result = HashGetOrSet(h, "key", HASH_KEY_STRING, "other");
    UTLT_Assert(strcmp("value", result) == 0, return STATUS_ERROR, "Hash get or set error: need %s, not %s\n", "value", result);

    HashSet(h, "key", HASH_KEY_STRING, NULL);
    result = HashGet(h, "key", HASH_KEY_STRING);
    UTLT_Assert(!result, return STATUS_ERROR, "Hash get or set error: need NULL, not %s\n", result);

    result = HashGetOrSet(h, "key", HASH_KEY_STRING, NULL);
    UTLT_Assert(!result, return STATUS_ERROR, "Hash get or set error: need NULL, not %s\n", result);

    result = HashGetOrSet(h, "key", HASH_KEY_STRING, "other");
    UTLT_Assert(strcmp("other", result) == 0, return STATUS_ERROR, "Hash get or set error: need %s, not %s\n", "other", result);

    HashDestroy(h);

    return STATUS_OK;
}

Status HashTest(void *data) {
    Status status;

    status = BufblkPoolInit();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolInit fail");

    status = TestHash_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestHash_1 fail");

    status = TestHash_2();
    UTLT_Assert(status == STATUS_OK, return status, "TestHash_2 fail");

    status = TestHash_3();
    UTLT_Assert(status == STATUS_OK, return status, "TestHash_3 fail");

    status = BufblkPoolFinal();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolFinal fail");

    return STATUS_OK;
}
