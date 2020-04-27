#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_pool.h"
#include "utlt_buff.h"

Status TestBuff_1() {
    Status status;

    Bufblk *buffer = BufblkAlloc(50, sizeof(char));
//    printf("[Testing] Buffer size = %d, len = %d, buf addr = %p\n", buffer->size, buffer->len, buffer->buf);
    UTLT_Assert(buffer, return STATUS_ERROR, "The buffer is NULL");

    BufblkStr(buffer, "Hello World!");

    BufblkStr(buffer, " Bye Bye!");

    UTLT_Assert(memcmp(buffer->buf, "Hello World! Bye Bye!", buffer->len) == 0, return STATUS_ERROR, "Buffer concate fail");

    BufblkAppend(buffer, 5, sizeof(char));

    memcpy(&((char *)buffer->buf)[21], "#$%&", 5);

    UTLT_Assert(memcmp(buffer->buf, "Hello World! Bye Bye!#$%&", buffer->len) == 0,
                return STATUS_ERROR, "Buffer concate fail");

    status = BufblkFree(buffer);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufferFree error");

    return STATUS_OK;
}

Status TestBuff_2() {
    Status status;

    Bufblk *buffer = BufblkAlloc(50, sizeof(char));
//    printf("[Testing] Buffer size = %d, len = %d, buf addr = %p\n", buffer->size, buffer->len, buffer->buf);
    UTLT_Assert(buffer->buf, return STATUS_ERROR, "The buffer is NULL");

    status = BufblkStr(buffer, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufblkStr fail");
    UTLT_Assert(buffer->size == 64, return STATUS_ERROR, "Buffer size error : need %d, not %d", 64, buffer->size);
    UTLT_Assert(buffer->len == 36, return STATUS_ERROR, "Buffer Concate Fist fail : need %d, not %d", 36, buffer->len);

    status = BufblkFmt(buffer, "[FMT Testing] Num = %d, Str = %s", 87, "WTF");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufferFmt fail");
    UTLT_Assert(buffer->size == 128, return STATUS_ERROR, "Buffer size error : need %d, not %d", 128, buffer->size);
    UTLT_Assert(buffer->len == (36 + 33), return STATUS_ERROR, "Buffer Concate Second fail : need %d, not %d", 69, buffer->len);

    const char testStr[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ[FMT Testing] Num = 87, Str = WTF";
    UTLT_Assert(memcmp(buffer->buf, testStr, buffer->len) == 0, return STATUS_ERROR, "Buffer concate fail");

    status = BufblkFree(buffer);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufferFree error");

    return STATUS_OK;
}

Status TestBuff_3() {
    Status status;

    Bufblk *buffer = BufblkAlloc(50, sizeof(char));
    UTLT_Assert(buffer->buf, return STATUS_ERROR, "The buffer is NULL");

    status = BufblkStr(buffer, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufblkStr fail");
    UTLT_Assert(buffer->size == 64, return STATUS_ERROR, "Buffer size error : need %d, not %d", 64, buffer->size);
    UTLT_Assert(buffer->len == 36, return STATUS_ERROR, "Buffer Concate Fist fail : need %d, not %d", 36, buffer->len);

    status = BufblkStr(buffer, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufblkStr fail");
    UTLT_Assert(buffer->size == 128, return STATUS_ERROR, "Buffer size error : need %d, not %d", 128, buffer->size);
    UTLT_Assert(buffer->len == (36 * 2), return STATUS_ERROR, "Buffer Concate Second fail : need %d, not %d", 72, buffer->len);

    const char testStr[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    UTLT_Assert(memcmp(buffer->buf, testStr, buffer->len) == 0, return STATUS_ERROR, "Buffer concate fail");

    status = BufblkFree(buffer);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufferFree error");

    return STATUS_OK;
}

Status TestBuff_4() {
    Status status;
    Bufblk *buffer = BufblkAlloc(50, sizeof(char));
    UTLT_Assert(buffer->buf, return STATUS_ERROR, "The buffer is NULL");
    Bufblk *buffer2 = BufblkAlloc(50, sizeof(char));
    UTLT_Assert(buffer2->buf, return STATUS_ERROR, "The buffer is NULL");

    status = BufblkStr(buffer, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufblkStr fail");
    UTLT_Assert(buffer->size == 64, return STATUS_ERROR, "Buffer size error : need %d, not %d", 64, buffer->size);
    UTLT_Assert(buffer->len == 36, return STATUS_ERROR, "Buffer Concate Fist fail : need %d, not %d", 36, buffer->len);

    status = BufblkStr(buffer2, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufblkStr2 fail");
    UTLT_Assert(buffer2->size == 64, return STATUS_ERROR, "Buffer2 size error : need %d, not %d", 64, buffer2->size);
    UTLT_Assert(buffer2->len == 36, return STATUS_ERROR, "Buffer2 Concate Fist fail : need %d, not %d", 36, buffer2->len);

    status = BufblkBuf(buffer, buffer2);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufblkBuf fail");
    UTLT_Assert(buffer->size == 128, return STATUS_ERROR, "Buffer size error : need %d, not %d", 128, buffer->size);
    UTLT_Assert(buffer->len == (36 * 2), return STATUS_ERROR, "Buffer Concate Second fail : need %d, not %d", 72, buffer->len);

    const char testStr[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    UTLT_Assert(memcmp(buffer->buf, testStr, buffer->len) == 0, return STATUS_ERROR, "Buffer concate fail");

    BufblkClear(buffer2);
    UTLT_Assert(buffer2->len == 0, return STATUS_ERROR, "Buffer2 Clear Fail, len need %d, not %d", 0, buffer2->len);
    UTLT_Assert(buffer2->size == 64, return STATUS_ERROR, "Buffer2 Clear Fail, size need %d, not %d", 64, buffer2->size);

    status = BufblkFree(buffer);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufferFree error");
    
    status = BufblkFree(buffer2);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufferFree error");

    return STATUS_OK;
}

typedef struct _paper_t {
    char *title;
    Bufblk *word;
    int pageNum;
} paper_t;

Status TestBuff_5() {
    Status status;
    // paper_t Alloc
    paper_t *paper = UTLT_Malloc(sizeof(paper_t));
    paper->title = UTLT_Calloc(0x10, sizeof(char));
    paper->word = BufblkAlloc(0x100, sizeof(char));
    //
    
    strcpy(paper->title, "Paper Test");
    BufblkFmt(paper->word, "%s 01234567890123456789", paper->title);
    BufblkStr(paper->word, " 0xabcdef1234");
    paper->pageNum = 1;

    Bufblk *outputStr = BufblkAlloc(1, sizeof(char));
    BufblkStr(outputStr, paper->title);
    BufblkBuf(outputStr, paper->word);
    BufblkBuf(outputStr, paper->word);
    BufblkBuf(outputStr, paper->word);
    BufblkBuf(outputStr, paper->word);
    BufblkBuf(outputStr, paper->word);
    
    const char testStr[] = "Paper TestPaper Test 01234567890123456789 0xabcdef1234Paper Test 01234567890123456789 0xabcdef1234Paper Test 01234567890123456789 0xabcdef1234Paper Test 01234567890123456789 0xabcdef1234Paper Test 01234567890123456789 0xabcdef1234";
    UTLT_Assert(memcmp(outputStr->buf, testStr, sizeof(testStr)) == 0, return STATUS_ERROR, "BufblkBuf concate fail");

    status = BufblkFree(outputStr);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufferFree 'outputStr' error");

    status = BufblkFree(paper->word);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufferFree 'paper->word' error");

    status = UTLT_Free(paper->title);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufferFree 'paper->title' error");

    status = UTLT_Free(paper);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "BufferFree 'paper' error");

    return STATUS_OK;
}

Status BuffTest(void *data) {
    Status status;

    status = BufblkPoolInit();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolInit fail");

    status = TestBuff_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestBuff_1 fail");

    status = TestBuff_2();
    UTLT_Assert(status == STATUS_OK, return status, "TestBuff_2 fail");

    status = TestBuff_3();
    UTLT_Assert(status == STATUS_OK, return status, "TestBuff_3 fail");

    status = TestBuff_4();
    UTLT_Assert(status == STATUS_OK, return status, "TestBuff_4 fail");

    status = TestBuff_5();
    UTLT_Assert(status == STATUS_OK, return status, "TestBuff_5 fail");

    status = BufblkPoolFinal();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolFinal fail");

    return STATUS_OK;
}
