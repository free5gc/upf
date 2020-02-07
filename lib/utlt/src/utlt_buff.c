#include "utlt_buff.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "utlt_debug.h"
#include "utlt_pool.h"

#define MAX_NUM_OF_BUFBLK_POOL       256

#define MAX_NUM_OF_BUFBLK_POOL_64    128
#define MAX_NUM_OF_BUFBLK_POOL_128   128
#define MAX_NUM_OF_BUFBLK_POOL_256   128
#define MAX_NUM_OF_BUFBLK_POOL_512   64
#define MAX_NUM_OF_BUFBLK_POOL_1024  64
#define MAX_NUM_OF_BUFBLK_POOL_2048  32
#define MAX_NUM_OF_BUFBLK_POOL_4096  32
#define MAX_NUM_OF_BUFBLK_POOL_8192  16
#define MAX_NUM_OF_BUFBLK_POOL_16384 8
#define MAX_NUM_OF_BUFBLK_POOL_32768 8
#define MAX_NUM_OF_BUFBLK_POOL_65536 4

#define SIZE_OF_BUF_RESERVED     4
#define SIZE_OF_BUF_64           64
#define SIZE_OF_BUF_128          128
#define SIZE_OF_BUF_256          256
#define SIZE_OF_BUF_512          512
#define SIZE_OF_BUF_1024         1024
#define SIZE_OF_BUF_2048         2048
#define SIZE_OF_BUF_4096         4096
#define SIZE_OF_BUF_8192         8192
#define SIZE_OF_BUF_16384        16384
#define SIZE_OF_BUF_32768        36728
#define SIZE_OF_BUF_65536        65536

typedef uint8_t bufPool64_t[SIZE_OF_BUF_64 + SIZE_OF_BUF_RESERVED];
typedef uint8_t bufPool128_t[SIZE_OF_BUF_128 + SIZE_OF_BUF_RESERVED];
typedef uint8_t bufPool256_t[SIZE_OF_BUF_256 + SIZE_OF_BUF_RESERVED];
typedef uint8_t bufPool512_t[SIZE_OF_BUF_512 + SIZE_OF_BUF_RESERVED];
typedef uint8_t bufPool1024_t[SIZE_OF_BUF_1024 + SIZE_OF_BUF_RESERVED];
typedef uint8_t bufPool2048_t[SIZE_OF_BUF_2048 + SIZE_OF_BUF_RESERVED];
typedef uint8_t bufPool4096_t[SIZE_OF_BUF_4096 + SIZE_OF_BUF_RESERVED];
typedef uint8_t bufPool8192_t[SIZE_OF_BUF_8192 + SIZE_OF_BUF_RESERVED];
typedef uint8_t bufPool16384_t[SIZE_OF_BUF_16384 + SIZE_OF_BUF_RESERVED];
typedef uint8_t bufPool32768_t[SIZE_OF_BUF_32768 + SIZE_OF_BUF_RESERVED];
typedef uint8_t bufPool65536_t[SIZE_OF_BUF_65536 + SIZE_OF_BUF_RESERVED];

PoolDeclare(bufblkPool, Bufblk, MAX_NUM_OF_BUFBLK_POOL);

PoolDeclare(bufPool64, bufPool64_t, MAX_NUM_OF_BUFBLK_POOL_64);
PoolDeclare(bufPool128, bufPool128_t, MAX_NUM_OF_BUFBLK_POOL_128);
PoolDeclare(bufPool256, bufPool256_t, MAX_NUM_OF_BUFBLK_POOL_256);
PoolDeclare(bufPool512, bufPool512_t, MAX_NUM_OF_BUFBLK_POOL_512);
PoolDeclare(bufPool1024, bufPool1024_t, MAX_NUM_OF_BUFBLK_POOL_1024);
PoolDeclare(bufPool2048, bufPool2048_t, MAX_NUM_OF_BUFBLK_POOL_2048);
PoolDeclare(bufPool4096, bufPool4096_t, MAX_NUM_OF_BUFBLK_POOL_4096);
PoolDeclare(bufPool8192, bufPool8192_t, MAX_NUM_OF_BUFBLK_POOL_8192);
PoolDeclare(bufPool16384, bufPool16384_t, MAX_NUM_OF_BUFBLK_POOL_16384);
PoolDeclare(bufPool32768, bufPool32768_t, MAX_NUM_OF_BUFBLK_POOL_32768);
PoolDeclare(bufPool65536, bufPool65536_t, MAX_NUM_OF_BUFBLK_POOL_65536);

Status SelectBufblkOption(Bufblk *bufblk, int opt);
Status BufAlloc(Bufblk *bufblk, uint32_t num, uint32_t size);
Status BufFree(Bufblk *bufblk);

int BufIsNotEnough(Bufblk *bufblk, uint32_t num, uint32_t size);

#define BUF_ALLOC 1
#define BUF_FREE  2
#define BufblkOptionSelector(__bufblk, __sizeNum, __opt) \
    switch(__opt) { \
        case BUF_ALLOC : \
            PoolAlloc(&bufPool##__sizeNum, (__bufblk)->buf); \
            UTLT_Assert((__bufblk)->buf, (__bufblk)->size = (__bufblk)->len = 0; return STATUS_ERROR, "bufPool"#__sizeNum" is empty"); \
            (__bufblk)->size = __sizeNum; \
            (__bufblk)->len = 0; \
            UTLT_Trace("Buffer Alloc Size["#__sizeNum"]"); \
            break; \
        case BUF_FREE : \
            PoolFree(&bufPool##__sizeNum, (bufPool##__sizeNum##_t *)(__bufblk)->buf); \
            (__bufblk)->size = (__bufblk)->len = 0; \
            UTLT_Trace("Buffer Free Size["#__sizeNum"]"); \
            break; \
        default : \
            (__bufblk)->size = (__bufblk)->len = 0; \
            UTLT_Assert(0, return STATUS_ERROR, "BufblkOptionSelector Option error"); \
    }

Status SelectBufblkOption(Bufblk *bufblk, int opt) {
    if (bufblk->size <= 64) {
        BufblkOptionSelector(bufblk, 64, opt);
    } else if (bufblk->size <= 128) {
        BufblkOptionSelector(bufblk, 128, opt); 
    } else if (bufblk->size <= 256) {
        BufblkOptionSelector(bufblk, 256, opt);
    } else if (bufblk->size <= 512) {
        BufblkOptionSelector(bufblk, 512, opt);
    } else if (bufblk->size <= 1024) {
        BufblkOptionSelector(bufblk, 1024, opt);
    } else if (bufblk->size <= 2048) {
        BufblkOptionSelector(bufblk, 2048, opt);
    } else if (bufblk->size <= 4096) {
        BufblkOptionSelector(bufblk, 4096, opt);
    } else if (bufblk->size <= 8192) {
        BufblkOptionSelector(bufblk, 8192, opt);
    } else if (bufblk->size <= 16384) {
        BufblkOptionSelector(bufblk, 16384, opt);
    } else if (bufblk->size <= 32768) {
        BufblkOptionSelector(bufblk, 32768, opt);
    } else if (bufblk->size <= 65536) {
        BufblkOptionSelector(bufblk, 65536, opt);
    } else {
        UTLT_Error("The size for Buffer block is too big : size[%d]", bufblk->size);
        bufblk->size = bufblk->len = 0;
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

Status BufblkPoolInit() {
    PoolInit(&bufblkPool, MAX_NUM_OF_BUFBLK_POOL);
    PoolInit(&bufPool64, MAX_NUM_OF_BUFBLK_POOL_64);
    PoolInit(&bufPool128, MAX_NUM_OF_BUFBLK_POOL_128);
    PoolInit(&bufPool256, MAX_NUM_OF_BUFBLK_POOL_256);
    PoolInit(&bufPool512, MAX_NUM_OF_BUFBLK_POOL_512);
    PoolInit(&bufPool1024, MAX_NUM_OF_BUFBLK_POOL_1024);
    PoolInit(&bufPool2048, MAX_NUM_OF_BUFBLK_POOL_2048);
    PoolInit(&bufPool4096, MAX_NUM_OF_BUFBLK_POOL_4096);
    PoolInit(&bufPool8192, MAX_NUM_OF_BUFBLK_POOL_8192);
    PoolInit(&bufPool16384, MAX_NUM_OF_BUFBLK_POOL_16384);
    PoolInit(&bufPool32768, MAX_NUM_OF_BUFBLK_POOL_32768);
    PoolInit(&bufPool65536, MAX_NUM_OF_BUFBLK_POOL_65536);

    return STATUS_OK;
}

Status BufblkPoolFinal() {
    PoolTerminate(&bufblkPool);
    PoolTerminate(&bufPool64);
    PoolTerminate(&bufPool128);
    PoolTerminate(&bufPool256);
    PoolTerminate(&bufPool512);
    PoolTerminate(&bufPool1024);
    PoolTerminate(&bufPool2048);
    PoolTerminate(&bufPool4096);
    PoolTerminate(&bufPool8192);
    PoolTerminate(&bufPool16384);
    PoolTerminate(&bufPool32768);
    PoolTerminate(&bufPool65536);

    return STATUS_OK;
}

#define BufferCheck(__sizeNum) \
    if (PoolSize(&bufPool##__sizeNum) != PoolCap(&bufPool##__sizeNum)) \
        UTLT_Warning("Memory leak happens in Bufblk"#__sizeNum" , need %d but only %d", \
            PoolCap(&bufPool##__sizeNum), PoolSize(&bufPool##__sizeNum))

void BufblkPoolCheck(const char *showInfo) {
    UTLT_Debug("Memory leak check start: %s", showInfo);

    UTLT_Assert(PoolSize(&bufblkPool) == PoolCap(&bufblkPool),
        , "Memory leak happens in BufblkPool, need %d but only %d",
        PoolCap(&bufblkPool), PoolSize(&bufblkPool));

    BufferCheck(64);
    BufferCheck(128);
    BufferCheck(256);
    BufferCheck(512);
    BufferCheck(1024);
    BufferCheck(2048);
    BufferCheck(4096);
    BufferCheck(8192);
    BufferCheck(16384);
    BufferCheck(32768);
    BufferCheck(65536);

    UTLT_Debug("Memory leak check end");
}

Bufblk *BufblkAlloc(uint32_t num, uint32_t size) {
    Status status;
    Bufblk *bufblk = NULL;

    PoolAlloc(&bufblkPool, bufblk);
    UTLT_Assert(bufblk, return NULL, "The pool of Buffer is empty");
    
    status = BufAlloc(bufblk, num, size);
    UTLT_Assert(status == STATUS_OK, 
                PoolFree(&bufblkPool, bufblk); bufblk = NULL, "");

    return bufblk;
}

Status BufAlloc(Bufblk *bufblk, uint32_t num, uint32_t size) {

    bufblk->size = num * size;
    UTLT_Assert(SelectBufblkOption(bufblk, BUF_ALLOC) == STATUS_OK, 
                return STATUS_ERROR, "");

    return STATUS_OK;
}

Status BufblkResize(Bufblk *bufblk, uint32_t num, uint32_t size) {
    Bufblk tmpBufblk;

    UTLT_Assert(BufAlloc(&tmpBufblk, num, size) == STATUS_OK, 
                return STATUS_ERROR, "Buffer Resize fail"); 
    
    tmpBufblk.len = (bufblk->len > tmpBufblk.size ? tmpBufblk.size : bufblk->len);
    memcpy(tmpBufblk.buf, bufblk->buf, tmpBufblk.len);

    UTLT_Assert(BufFree(bufblk) == STATUS_OK, 
                BufFree(&tmpBufblk); return STATUS_ERROR, 
                "Buffer Resize fail"); 
    
    bufblk->size = tmpBufblk.size;
    bufblk->len = tmpBufblk.len;
    bufblk->buf = tmpBufblk.buf;

    return STATUS_OK;
}

Status BufblkClear(Bufblk *bufblk) {
    char *ptr = bufblk->buf;
    bufblk->len = 0;
    ptr[bufblk->len] = '\0';

    return STATUS_OK;
}

Status BufFree(Bufblk *bufblk) {

    if (bufblk->buf) {
        UTLT_Assert(SelectBufblkOption(bufblk, BUF_FREE) == STATUS_OK, 
                    return STATUS_ERROR, "Buffer Pool Free fail");
        bufblk->buf = NULL;
    }

    return STATUS_OK;
}

Status BufblkFree(Bufblk *bufblk) {
    
    UTLT_Assert(BufFree(bufblk) == STATUS_OK, return STATUS_ERROR, 
                "Buffer Free fail");
    PoolFree(&bufblkPool, bufblk);

    return STATUS_OK;
}

int BufIsNotEnough(Bufblk *bufblk, uint32_t num, uint32_t size) {
    return ((bufblk->size - bufblk->len >= num * size) ? 0 : 1);
}

Status BufblkStr(Bufblk *bufblk, const char *str) {
    return BufblkBytes(bufblk, str, strlen(str));
}

int BufblkBuf(Bufblk *bufblk, const Bufblk *bufblk2) {
    return BufblkBytes(bufblk, (const char *) bufblk2->buf, bufblk2->len);
}

Status BufblkFmt(Bufblk *bufblk, const char *fmt, ...) {
    Status status;
    char *ptr = NULL;
    va_list vl;

    do {
        ptr = bufblk->buf;
        va_start(vl, fmt);
        status = vsnprintf(&ptr[bufblk->len], 
            bufblk->size + SIZE_OF_BUF_RESERVED - bufblk->len, fmt, vl);
        va_end(vl);
        if (status < 0) {
            ptr[bufblk->len] = '\0';
            UTLT_Assert(0, return STATUS_ERROR, "Buffer Fmt vsnprintf error : %s", strerror(errno));
        }

        if (status + bufblk->len < bufblk->size) {
            bufblk->len += status;
            ptr[bufblk->len] = '\0';
            return STATUS_OK;
        }
        ptr[bufblk->len] = '\0';
        status = BufblkResize(bufblk, 1, bufblk->size << 1);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");
    } while(1);
}

Status BufblkBytes(Bufblk *bufblk, const char *str, uint32_t size) {
    if (BufIsNotEnough(bufblk, 1, size)) {
        UTLT_Assert(BufblkResize(bufblk, 1, bufblk->size + size) == STATUS_OK, return STATUS_ERROR, "");
    }
    
    char *ptr = bufblk->buf;
    memcpy(&ptr[bufblk->len], str, size);
    bufblk->len += size;
    ptr[bufblk->len] = '\0';

    return STATUS_OK;
}

Status BufblkAppend(Bufblk *bufblk, uint32_t num, uint32_t size) {
    uint32_t totalSize = num * size;

    if (BufIsNotEnough(bufblk, num, size)) {
        UTLT_Assert(BufblkResize(bufblk, 1, bufblk->size + totalSize) == STATUS_OK, return STATUS_ERROR, "");
    }

    char *ptr = bufblk->buf;
    memset(&ptr[bufblk->len], 0, totalSize);
    bufblk->len += totalSize;
    ptr[bufblk->len] = '\0';

    return STATUS_OK;
}

void *UTLT_Malloc(uint32_t size) {
    Bufblk tmpBufblk;
    uint32_t realSize = size + sizeof(uint32_t);
    UTLT_Assert(BufAlloc(&tmpBufblk, 1, realSize) == STATUS_OK,
                return NULL, "UTLT_Malloc fail");

    ((uint32_t *) tmpBufblk.buf)[0] = realSize;

    return tmpBufblk.buf + sizeof(uint32_t);
}

void *UTLT_Calloc(uint32_t num, uint32_t size) {
    Bufblk tmpBufblk;
    uint32_t realSize = num * size + sizeof(uint32_t);
    UTLT_Assert(BufAlloc(&tmpBufblk, 1, realSize) == STATUS_OK,
                return NULL, "UTLT_Malloc fail");

    ((uint32_t *) tmpBufblk.buf)[0] = realSize;

    return tmpBufblk.buf + sizeof(uint32_t);
}

Status UTLT_Free(void *buf) {
    Bufblk tmpBufblk;
    tmpBufblk.buf = buf - sizeof(uint32_t);
    tmpBufblk.size = *(uint32_t *)(buf - sizeof(uint32_t));

    UTLT_Assert(BufFree(&tmpBufblk) == STATUS_OK, return STATUS_ERROR,
                "UTLT_Free fail")
    buf = NULL;
    return STATUS_OK;
}

Status UTLT_Resize(void *buf, uint32_t size) {
    Bufblk tmpBufblk;

    tmpBufblk.buf = buf - sizeof(uint32_t);
    tmpBufblk.size = *(uint32_t *)(buf - sizeof(uint32_t));
    tmpBufblk.len = tmpBufblk.size;

    UTLT_Assert(BufblkResize(&tmpBufblk, 1, size + sizeof(uint32_t)) == STATUS_OK,
                return STATUS_ERROR, "UTLT_Resize fail");

    buf = tmpBufblk.buf + sizeof(uint32_t);

    return STATUS_OK;
}
