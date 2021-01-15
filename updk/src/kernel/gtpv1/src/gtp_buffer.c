#include "gtp_buffer.h"

Sock *BufferServerCreate(int type, const char *path, SockHandler handler, void *data) {
    Status status;  
    
    Sock *sock = UnixServerCreate(type, path);
    UTLT_Assert(sock, return NULL, "BufferServerCreate fail");

    status = SockRegister(sock, handler, data);
    UTLT_Assert(status == STATUS_OK, UnixFree(sock); return NULL,
                "SockRegister fail");

    return sock;
}

Status BufferServerFree(Sock *sock) {
    UTLT_Assert(sock, return STATUS_OK, "");
    Status status = STATUS_OK;

    UTLT_Assert(SockUnregister(sock) == STATUS_OK, status |= STATUS_ERROR,
                "Unix socket unregister fail");

    UTLT_Assert(UnixFree(sock) == STATUS_OK, status |= STATUS_ERROR,
                "Unix socket free fail");

    return status;
}

// TODO: Need new bufer to handle packet struct
int BufferRecv(Sock *sock, Bufblk *pktbuf, uint16_t *pdrId, uint8_t *farAction) {
    UTLT_Assert(sock && pktbuf, return -1, "Socket or pktbuf pointer is NULL");

    if (pktbuf->size != MAX_OF_BUFFER_PACKET_SIZE) {
        UTLT_Assert(BufblkResize(pktbuf, 1, MAX_OF_BUFFER_PACKET_SIZE) == STATUS_OK,
                    return -1, "Buffer is not enough");
    }

    int readNum = UnixRecv(sock, pktbuf->buf, MAX_OF_BUFFER_PACKET_SIZE);
    UTLT_Assert(readNum >= sizeof(uint32_t), return -1, "BufferRecv fail");

    // Handle self-defined protocol here
    uint16_t *pU16 = pktbuf->buf, realBufferSize = readNum - sizeof(uint32_t);
    *pdrId = pU16[0];
    *farAction = pU16[1];

    memcpy(pktbuf->buf, pktbuf->buf + sizeof(uint32_t), realBufferSize);

    pktbuf->len = realBufferSize;
    return pktbuf->len;
}

Status BufferEpollRegister(int epfd, Sock *sock) {
    return EpollRegisterEvent(epfd, sock);
}

Status BufferEpollDeregister(int epfd, Sock *sock) {
    return EpollDeregisterEvent(epfd, sock);
}