#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_buff.h"
#include "utlt_network.h"

char tcpServerIP[] = "127.0.0.10";
int tcpServerPort = 12345;
char tcpwelstr[] = "Welcome to TCP server and type something\n";
char tcpclientstr[] = "Hello TCP server\n";

Status TestTCPServer_1() {
//    printf("[Testing] TCP Server start\n");
    Status status;
    char buffer[0x40];
    
    Sock *tcpSock = SockCreate(AF_INET, SOCK_STREAM, 0);
    UTLT_Assert(tcpSock, return STATUS_ERROR, "SockCreate tcpSock fail");

    SockAddr addr;
    status = SockSetAddr(&addr, AF_INET, tcpServerIP, tcpServerPort);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    // printf("Get L3 Info : IP[%s], Port[%d]\n", GetIP(&addr), GetPort(&addr));
    UTLT_Assert(memcmp(GetIP(&addr), tcpServerIP, sizeof(tcpServerIP)) == 0, return STATUS_ERROR, 
                "[Server] L3 Info is wrong : need IP[%s], not IP[%s]", tcpServerIP, GetIP(&addr));
    UTLT_Assert(GetPort(&addr) == tcpServerPort, return STATUS_ERROR,
                "[Server] L3 Info is wrong : need Port[%d], not Port[%d]", tcpServerPort, GetPort(&addr));

    int opt = SO_REUSEADDR;
    SockSetOpt(tcpSock, SOL_SOCKET, SO_REUSEADDR, &opt);

    status = SockBind(tcpSock, &addr);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    // printf("[Server] Get Socket Bind Info : IP[%s], Port[%d]\n", GetIP(&tcpSock->localAddr), GetPort(&tcpSock->localAddr));
    UTLT_Assert(memcmp(GetIP(&tcpSock->localAddr), tcpServerIP, sizeof(tcpServerIP)) == 0, return STATUS_ERROR, 
                "[Server] L3 Info is wrong : need IP[%s], not IP[%s]", tcpServerIP, GetIP(&tcpSock->localAddr));
    UTLT_Assert(GetPort(&tcpSock->localAddr) == tcpServerPort, return STATUS_ERROR,
                "[Server] L3 Info is wrong : need Port[%d], not Port[%d]", tcpServerPort, GetPort(&tcpSock->localAddr));

    status = SockListen(tcpSock, 5);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "")

    // printf("[Server] Please connect to server : IP[%s], Port[%d]\n", tcpServerIP, tcpServerPort);

    Sock *tcpAC = SockCreate(AF_INET, SOCK_STREAM, 0);
    UTLT_Assert(tcpSock, return STATUS_ERROR, "SockCreate tcpAC fail");

    status = SockAccept(tcpSock, tcpAC);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    status = SockWrite(tcpAC, tcpwelstr, sizeof(tcpwelstr));
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    int readNum = SockRead(tcpAC, buffer, sizeof(buffer));
    UTLT_Assert(readNum >= 0, return STATUS_ERROR, "");
    UTLT_Assert(memcmp(buffer, tcpclientstr, sizeof(tcpclientstr)) == 0, return STATUS_ERROR, "SockRead msg is not match");
    

    SockFree(tcpAC);
    SockFree(tcpSock);

//    printf("[Testing] TCP Server end\n");
    return STATUS_OK;
}

Status TestTCPClient_1() {
//    printf("[Testing] TCP Client start\n");
    Status status;
    char buffer[0x40];

    Sock *tcpSock = SockCreate(AF_INET, SOCK_STREAM, 0);
    UTLT_Assert(tcpSock, return STATUS_ERROR, "SockCreate tcpSock fail");

    SockAddr addr;
    status = SockSetAddr(&addr, AF_INET, tcpServerIP, tcpServerPort);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    // printf("[Client] Get L3 Info : IP[%s], Port[%d]\n", GetIP(&addr), GetPort(&addr));
    UTLT_Assert(memcmp(GetIP(&addr), tcpServerIP, sizeof(tcpServerIP)) == 0, return STATUS_ERROR, 
                "[Client] L3 Info is wrong : need IP[%s], not IP[%s]", tcpServerIP, GetIP(&addr));
    UTLT_Assert(GetPort(&addr) == tcpServerPort, return STATUS_ERROR,
                "[Client] L3 Info is wrong : need Port[%d], not Port[%d]", tcpServerPort, GetPort(&addr));

    // printf("[Client] Please open the TCP server first : IP[%s], Port[%d]\n", tcpServerIP, tcpServerPort);

    sleep(3);

    status = SockConnect(tcpSock, &addr);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    status = SockSendTo(tcpSock, tcpclientstr, sizeof(tcpclientstr));
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "");

    int readNum = SockRecvFrom(tcpSock, buffer, sizeof(buffer));
    UTLT_Assert(readNum >= 0 , return STATUS_ERROR, "");
    UTLT_Assert(memcmp(buffer, tcpwelstr, sizeof(tcpwelstr)) == 0, return STATUS_ERROR, "SockRead msg is not match");

    SockFree(tcpSock);

//    printf("[Testing] TCP Client end\n");
    return STATUS_OK;
}

Status TestTCP_1() {
    pid_t pid = fork();
    UTLT_Assert(pid >= 0, return STATUS_ERROR, "Fork fail");
    
    if (pid == 0) {
        sleep(1);
        TestTCPClient_1();
        exit(0);
    } else {
        TestTCPServer_1();
    }

    return STATUS_OK;
}

const char udpServerIP[] = "127.0.0.127";
int udpServerPort = 12345;

char udpwelstr[] = "Welcome to UDP server and bye bye\n";
char udpclientstr[] = "Hello UDP server\n";

Status TestUDPServer_1() {
//    printf("[Testing] UDP Server start\n");
    Status status;
    char buffer[0x40];

    Sock *udpSock = UdpServerCreate(AF_INET, udpServerIP, udpServerPort);
    UTLT_Assert(udpSock, return STATUS_ERROR, "UdpServerCreate Fail")

    // printf("[Server] Please connect to server : IP[%s], Port[%d]\n", udpServerIP, udpServerPort);

    while (1) {
        int readNum = UdpRecvFrom(udpSock, buffer, sizeof(buffer));
        UTLT_Assert(readNum >= 0, return STATUS_ERROR, "UdpRecvFrom fail");

        if (readNum > 0) {
            // printf("[Server] UdpRecvFrom : %s", buffer);
            UTLT_Assert(memcmp(buffer, udpclientstr, readNum) == 0, return STATUS_ERROR, "String is not the same");

            status = UdpSendTo(udpSock, udpwelstr, sizeof(udpwelstr));
            UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "UdpSendTo fail");
            break;
        }
    }

    UdpFree(udpSock);

//    printf("[Testing] UDP Server end\n");
    return STATUS_OK;
}

Status TestUDPClient_1() {
//    printf("[Testing] UDP Client start\n");
    Status status;
    char buffer[0x40];

    // printf("[Client] Please open the UDP server first : IP[%s], Port[%d]\n", udpServerIP, udpServerPort);
    sleep(1);

    Sock *sock = UdpClientCreate(AF_INET, udpServerIP, udpServerPort);
    UTLT_Assert(sock, return STATUS_ERROR, "");

    status = UdpSendTo(sock, udpclientstr, sizeof(udpclientstr));
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "UdpSendTo fail");

    usleep(500);

    int readNum = UdpRecvFrom(sock, buffer, sizeof(buffer));
    UTLT_Assert(readNum >= 0, return STATUS_ERROR, "UdpRecvFrom fail");

    UTLT_Assert(memcmp(buffer, udpwelstr, readNum) == 0, return STATUS_ERROR, "String is not the same");
    
    // printf("[Client] UdpRecvFrom : %s", buffer);

    UdpFree(sock);

//    printf("[Testing] UDP Client end\n");

    return STATUS_OK;
}


Status TestUDP_1() {
    pid_t pid = fork();

    UTLT_Assert(pid >= 0, return STATUS_ERROR, "Fork Error");

    if (pid == 0) {
        sleep(1);
        TestUDPClient_1();
        exit(0);
    } else {
        TestUDPServer_1();
    }

    return STATUS_OK;
}

#define NUM_OF_SOCK 5
const char epollIP[] = "127.0.0.87";
int epollPort = 10000;
char exitStr[] = "exit";

const char prefix[] = "[Server] ^_^ -> ";

static Status TestCallBackFunc(Sock *sock, void *data) {
    Status status;
    Bufblk *buflk = data;
    char buffer[4096];

    int readNum = UdpRecvFrom(sock, buffer, sizeof(buffer));
    UTLT_Assert(readNum >= 0, return STATUS_ERROR, "UdpRecvFrom fail");

    buffer[readNum] = '\0';

    // printf("[Server] Recv from IP[%s], Port[%d] : length[%d]\n",
            // GetIP(&sockptr->remoteAddr), GetPort(&sockptr->remoteAddr), readNum);

    if (memcmp(buffer, exitStr, sizeof(exitStr) - 1) == 0) {
        // printf("[Server] Bye Bye\n");
        return 87;
    }

    BufblkClear(buflk);
    BufblkFmt(buflk, "%s%s", prefix, buffer);

    status = UdpSendTo(sock, buflk->buf, buflk->len);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "UdpSendTo fail");

    return STATUS_OK;
}

Status TestEpollServer_1() {
//    printf("[Testing] Epoll Server Test start\n");
    Status status;

    Bufblk *buflk = BufblkAlloc(4096, sizeof(char));
    Sock *sock[NUM_OF_SOCK], *sockptr;
    for (int i = 0; i < NUM_OF_SOCK; i++) {
        sock[i] = UdpServerCreate(AF_INET, epollIP, epollPort + i);
        UTLT_Assert(sock[i], return STATUS_ERROR, "");

        status = SockRegister(sock[i], TestCallBackFunc, buflk);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR,
                    "Socket[%d] register fail", i);
    }

    int epfd = EpollCreate(); 
    for (int i = 0; i < NUM_OF_SOCK; i++) {
        status = EpollRegisterEvent(epfd, sock[i]);
        UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, 
                    "The %d of EpollRegisterEvent fail", i);
    }


    // printf("[Server] Servers IP[%s], Port[%d - %d]\n", epollIP, epollPort, epollPort + NUM_OF_SOCK);

    struct epoll_event events[NUM_OF_SOCK];
    while(1) {
        int nfds = EpollWait(epfd, events, 3);
        UTLT_Assert(nfds >= 0, return STATUS_ERROR, "Epoll Wait error : %s", strerror(errno));
        
        for (int i = 0; i < nfds; i++) {
            sockptr = events[i].data.ptr;
            status = sockptr->handler(sockptr, sockptr->data);
            if (status != STATUS_OK) {
                if (status == 87) {
                    goto CLOSE;
                } else {
                    UTLT_Assert(status == STATUS_OK, goto CLOSE, "Socket handler fail");
                }
            }
        }
    }

CLOSE :
    for (int i = 0; i < NUM_OF_SOCK; i++) {
        status = EpollDeregisterEvent(epfd, sock[i]);
        UTLT_Assert(status == STATUS_OK, ,
                    "The %d of EpollDeregisterEvent fail", i);
        status = SockUnregister(sock[i]);
        UTLT_Assert(status == STATUS_OK, ,
                    "The %d of Socket unregister fail", i);

        UdpFree(sock[i]);
    }

//    printf("[Testing] Epoll Server Test end\n");
    return STATUS_OK;
}

Status TestEpollClient_1() {
//    printf("[Testing] Epoll Client Test start\n"); 
    Status status;
    char buffer[0x40], sbuf[0x50];

    sleep(3);
    Sock *sock[NUM_OF_SOCK];
    for (int i = 0; i < NUM_OF_SOCK; i++) {
        sock[i] = UdpClientCreate(AF_INET, epollIP, epollPort + i);
        UTLT_Assert(sock[i], return STATUS_ERROR, "");
    }

    usleep(500);

    char str1[] = "I am Connection One\n";
    status = UdpSendTo(sock[0], str1, sizeof(str1));
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "UdpSendTo fail");

    char str2[] = "What is my !@#$%^&\n";
    status = UdpSendTo(sock[1], str2, sizeof(str2));
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "UdpSendTo fail");

    char str3[] = "dfdfgdfgweebvxcasreghgfgd";
    status = UdpSendTo(sock[2], str3, sizeof(str3));

    usleep(500);

    int readNum = UdpRecvFrom(sock[0], buffer, sizeof(buffer));
    UTLT_Assert(readNum >= 0, return STATUS_ERROR, "UdpRecvFrom fail");
    sprintf(sbuf, "%s%s", prefix, str1);
    UTLT_Assert(memcmp(buffer, sbuf, readNum) == 0, return STATUS_ERROR, "String 1 recv from server is not match");

    readNum = UdpRecvFrom(sock[1], buffer, sizeof(buffer));
    UTLT_Assert(readNum >= 0, return STATUS_ERROR, "UdpRecvFrom fail");
    sprintf(sbuf, "%s%s", prefix, str2);
    UTLT_Assert(memcmp(buffer, sbuf, readNum) == 0, return STATUS_ERROR, "String 2 recv from server is not match");

    readNum = UdpRecvFrom(sock[2], buffer, sizeof(buffer));
    UTLT_Assert(readNum >= 0, return STATUS_ERROR, "UdpRecvFrom fail");
    sprintf(sbuf, "%s%s", prefix, str3);
    UTLT_Assert(memcmp(buffer, sbuf, readNum) == 0, return STATUS_ERROR, "String 3 recv from server is not match");

    status = UdpSendTo(sock[3], exitStr, sizeof(exitStr));

    for (int i = 0; i < NUM_OF_SOCK; i++) {
        UdpFree(sock[i]);
    }

//    printf("[Testing] Epoll Client Test end\n"); 
    return STATUS_OK;
}


Status TestEpoll_1() {
    pid_t pid = fork();
    UTLT_Assert(pid >= 0, return STATUS_ERROR, "Fork fail");

    if (pid == 0) {
        sleep(1);
        TestEpollClient_1();
        exit(0);
    } else {
        TestEpollServer_1();
    }

    return STATUS_OK;
}

Status TestGetAddr_1() {
    char addr[INET6_ADDRSTRLEN];

    GetAddrFromHost(addr, "ns1.NCTU.edu.tw", INET6_ADDRSTRLEN);
    UTLT_Assert(!strcmp(addr, "140.113.1.1"), return STATUS_ERROR, "");

    return STATUS_OK;
}

Status TestGetAddr_2() {
    char addr[INET6_ADDRSTRLEN];

    GetAddrFromHost(addr, "140.113.1.1", INET6_ADDRSTRLEN);
    UTLT_Assert(!strcmp(addr, "140.113.1.1"), return STATUS_ERROR, "");

    return STATUS_OK;
}

Status NetworkTest(void *data) {
    Status status;

    status = BufblkPoolInit();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolInit fail");

    status = SockPoolInit();
    UTLT_Assert(status == STATUS_OK, return status, "SockPoolInit fail");

    status = TestTCP_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestTCP_1 fail");

    status = TestUDP_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestUDP_1 fail");
    
    status = TestEpoll_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestEpoll_1 fail");

    status = TestGetAddr_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestGetAddr_1 fail");

    status = TestGetAddr_2();
    UTLT_Assert(status == STATUS_OK, return status, "TestGetAddr_2 fail");
    
    status = SockPoolFinal();
    UTLT_Assert(status == STATUS_OK, return status, "SockPoolFinal fail");

    status = BufblkPoolFinal();
    UTLT_Assert(status == STATUS_OK, return status, "BufblkPoolFinal fail");
 
    return STATUS_OK;
}
