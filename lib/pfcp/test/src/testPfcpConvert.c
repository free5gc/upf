#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utlt_debug.h"
#include "utlt_buff.h"
#include "pfcp_types.h"
#include "pfcp_convert.h"

Status TestPfcpConvert_1() {
    const char ipv4[] = "172.1.0.1";
    PfcpFSeid fseidv4 = {
        .v4 = 1,
        .seid = 87,
    };
    inet_pton(AF_INET, ipv4, &fseidv4.addr4);

    const char ipv6[] = "fd00:dead:beef::1234";
    PfcpFSeid fseidv6 = {
        .v6 = 1,
        .seid = 0x87,
    };
    inet_pton(AF_INET6, ipv6, &fseidv6.addr6);

    PfcpFSeid fseidBoth = {
        .v4 = 1,
        .v6 = 1,
        .seid = 0x17,
    };
    inet_pton(AF_INET, ipv4, &fseidBoth.dualStack.addr4);
    inet_pton(AF_INET6, ipv6, &fseidBoth.dualStack.addr6);

    SockAddr *sockAddrPtr = NULL;
    int port = 1721;

    // Test only IPv4
    UTLT_Assert(PfcpFSeidToSockaddr(&fseidv4, port, &sockAddrPtr) == STATUS_OK,
                return STATUS_ERROR, "PfcpFSeidToSockaddr fail");

    UTLT_Assert(sockAddrPtr->_family == AF_INET, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need Version[%d], not %d", AF_INET, sockAddrPtr->_family);
    UTLT_Assert(strcmp(GetIP(sockAddrPtr), ipv4) == 0, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need IP[%s], not IP[%s]", ipv4, GetIP(sockAddrPtr));
    UTLT_Assert(GetPort(sockAddrPtr) == port, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need Port[%d], not Port[%d]", port, GetPort(sockAddrPtr));

    // Maybe have memory leak if forget to free
    UTLT_Free(sockAddrPtr);
    
    // Test only IPv6
    UTLT_Assert(PfcpFSeidToSockaddr(&fseidv6, port, &sockAddrPtr) == STATUS_OK,
                return STATUS_ERROR, "PfcpFSeidToSockaddr fail");

    UTLT_Assert(sockAddrPtr->_family == AF_INET6, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need Version[%d], not %d", AF_INET6, sockAddrPtr->_family);
    UTLT_Assert(strcmp(GetIP(sockAddrPtr), ipv6) == 0, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need IP[%s], not IP[%s]", ipv6, GetIP(sockAddrPtr));
    UTLT_Assert(GetPort(sockAddrPtr) == port, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need Port[%d], not Port[%d]", port, GetPort(sockAddrPtr));

    // Maybe have memory leak if forget to free
    UTLT_Free(sockAddrPtr);

    // Test IPv4 and IPv6
    UTLT_Assert(PfcpFSeidToSockaddr(&fseidBoth, port, &sockAddrPtr) == STATUS_OK,
                return STATUS_ERROR, "PfcpFSeidToSockaddr fail");

    UTLT_Assert(sockAddrPtr->_family == AF_INET, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need Version[%d], not %d", AF_INET, sockAddrPtr->_family);
    UTLT_Assert(strcmp(GetIP(sockAddrPtr), ipv4) == 0, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need IP[%s], not IP[%s]", ipv4, GetIP(sockAddrPtr));
    UTLT_Assert(GetPort(sockAddrPtr) == port, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need Port[%d], not Port[%d]", port, GetPort(sockAddrPtr));

    UTLT_Assert(sockAddrPtr->next->_family == AF_INET6, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need Version[%d], not %d", AF_INET6, sockAddrPtr->next->_family);
    UTLT_Assert(strcmp(GetIP(sockAddrPtr->next), ipv6) == 0, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need IP[%s], not IP[%s]", ipv6, GetIP(sockAddrPtr->next));
    UTLT_Assert(GetPort(sockAddrPtr->next) == port, return STATUS_ERROR,
                "PfcpFSeidToSockaddr fail : need Port[%d], not Port[%d]", port, GetPort(sockAddrPtr->next));

    // Maybe have memory leak if forget to free
    UTLT_Free(sockAddrPtr->next);
    UTLT_Free(sockAddrPtr);

    return STATUS_OK;
}

// TODO : Need another to check IPv6 and both v4 and v6
Status TestPfcpConvert_2() {
    int port = 1684;

    const char ipv4[] = "192.168.0.4";
    SockAddr addrv4;
    UTLT_Assert(SockSetAddr(&addrv4, AF_INET, ipv4, port) == STATUS_OK, return STATUS_ERROR,
                "SockSetAddr fail : IP[%s], Port[%d]", ipv4, port);

    const char ipv6[] = "face:b006::9987";
    SockAddr addrv6;
    UTLT_Assert(SockSetAddr(&addrv6, AF_INET6, ipv6, port) == STATUS_OK, return STATUS_ERROR,
                "SockSetAddr fail : IP[%s], Port[%d]", ipv6, port);

    int ipLen;
    PfcpFSeid fseid;
    char str[INET6_ADDRSTRLEN];

    // Test only IPv4
    UTLT_Assert(PfcpSockaddrToFSeid(&addrv4, NULL, &fseid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpSockaddrToFSeid fail");
    UTLT_Assert(fseid.v4 == 1, return STATUS_ERROR, "The v4 value of FSeid error : need 1, not %d", fseid.v4);
    UTLT_Assert(fseid.v6 == 0, return STATUS_ERROR, "The v6 value of FSeid error : need 0, not %d", fseid.v6);
    UTLT_Assert(inet_ntop(AF_INET, &fseid.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpSockaddrToFSeid fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(ipLen == PFCP_F_SEID_IPV4_LEN, return STATUS_ERROR,
                "PfcpSockaddrToFSeid fail : need len[%d], not len[%d]", PFCP_F_SEID_IPV4_LEN, ipLen);

    // Test only IPv6
    UTLT_Assert(PfcpSockaddrToFSeid(NULL, &addrv6, &fseid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpSockaddrToFSeid fail");
    UTLT_Assert(fseid.v4 == 0, return STATUS_ERROR, "The v4 value of FSeid error : need 0, not %d", fseid.v4);
    UTLT_Assert(fseid.v6 == 1, return STATUS_ERROR, "The v6 value of FSeid error : need 1, not %d", fseid.v6);
    UTLT_Assert(inet_ntop(AF_INET6, &fseid.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpSockaddrToFSeid fail : need IP[%s], not IP[%s]", ipv6, str);
    UTLT_Assert(ipLen == PFCP_F_SEID_IPV6_LEN, return STATUS_ERROR,
                "PfcpSockaddrToFSeid fail : need len[%d], not len[%d]", PFCP_F_SEID_IPV6_LEN, ipLen);

    // Test IPv4 and IPv6
    UTLT_Assert(PfcpSockaddrToFSeid(&addrv4, &addrv6, &fseid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpSockaddrToFSeid fail");
    UTLT_Assert(fseid.v4 == 1, return STATUS_ERROR, "The v4 value of FSeid error : need 1, not %d", fseid.v4);
    UTLT_Assert(fseid.v6 == 1, return STATUS_ERROR, "The v6 value of FSeid error : need 1, not %d", fseid.v6);
    UTLT_Assert(inet_ntop(AF_INET, &fseid.dualStack.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpSockaddrToFSeid fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(inet_ntop(AF_INET6, &fseid.dualStack.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpSockaddrToFSeid fail : need IP[%s], not IP[%s]", ipv6, str);
    UTLT_Assert(ipLen == PFCP_F_SEID_IPV4V6_LEN, return STATUS_ERROR,
                "PfcpSockaddrToFSeid fail : need len[%d], not len[%d]", PFCP_F_SEID_IPV4V6_LEN, ipLen);

    return STATUS_OK;
}

Status TestPfcpConvert_3() {
    const char ipv4[] = "172.1.0.3";
    PfcpFSeid fseidv4 = {
        .v4 = 1,
        .seid = 87,
    };
    inet_pton(AF_INET, ipv4, &fseidv4.addr4);

    const char ipv6[] = "fd00:dead:beef::3127";
    PfcpFSeid fseidv6 = {
        .v6 = 1,
        .seid = 0x87,
    };
    inet_pton(AF_INET6, ipv6, &fseidv6.addr6);

    PfcpFSeid fseidBoth = {
        .v4 = 1,
        .v6 = 1,
        .seid = 0x17,
    };
    inet_pton(AF_INET, ipv4, &fseidBoth.dualStack.addr4);
    inet_pton(AF_INET6, ipv6, &fseidBoth.dualStack.addr6);
    
    Ip ip;
    char str[INET6_ADDRSTRLEN];

    // Test only IPv4
    UTLT_Assert(PfcpFSeidToIp(&fseidv4, &ip) == STATUS_OK, return STATUS_ERROR,
                "PfcpFSeidToIp fail");
    UTLT_Assert(ip.ipv4 == 1, return STATUS_ERROR, "The ipv4 value of IP error : need 1, not %d", ip.ipv4);
    UTLT_Assert(ip.ipv6 == 0, return STATUS_ERROR, "The ipv6 value of IP error : need 0, not %d", ip.ipv6);
    UTLT_Assert(ip.len == IPV4_LEN, return STATUS_ERROR,
                "The len of IP error : need %d, not %d", IPV4_LEN, ip.len);
    UTLT_Assert(inet_ntop(AF_INET, &ip.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpFSeidToIp fail : need IP[%s], not IP[%s]", ipv4, str);

    // Test only IPv6
    UTLT_Assert(PfcpFSeidToIp(&fseidv6, &ip) == STATUS_OK, return STATUS_ERROR,
                "PfcpFSeidToIp fail");
    UTLT_Assert(ip.ipv4 == 0, return STATUS_ERROR, "The ipv4 value of IP error : need 0, not %d", ip.ipv4);
    UTLT_Assert(ip.ipv6 == 1, return STATUS_ERROR, "The ipv6 value of IP error : need 1, not %d", ip.ipv6);
    UTLT_Assert(ip.len == IPV6_LEN, return STATUS_ERROR,
                "The len of IP error : need %d, not %d", IPV6_LEN, ip.len);
    UTLT_Assert(inet_ntop(AF_INET6, &ip.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpFSeidToIp fail : need IP[%s], not IP[%s]", ipv6, str);

    // Test IPv4 and IPv6
    UTLT_Assert(PfcpFSeidToIp(&fseidBoth, &ip) == STATUS_OK, return STATUS_ERROR,
                "PfcpFSeidToIp fail");
    UTLT_Assert(ip.ipv4 == 1, return STATUS_ERROR, "The ipv4 value of IP error : need 1, not %d", ip.ipv4);
    UTLT_Assert(ip.ipv6 == 1, return STATUS_ERROR, "The ipv6 value of IP error : need 1, not %d", ip.ipv6);
    UTLT_Assert(ip.len == IPV4V6_LEN, return STATUS_ERROR,
                "The len of IP error : need %d, not %d", IPV4V6_LEN, ip.len);
    UTLT_Assert(inet_ntop(AF_INET, &ip.dualStack.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpFSeidToIp fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(inet_ntop(AF_INET6, &ip.dualStack.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpFSeidToIp fail : need IP[%s], not IP[%s]", ipv6, str);

    return STATUS_OK;
}

Status TestPfcpConvert_4() {
    const char ipv4[] = "87.87.78.78";
    Ip ipStructv4 = {
        .ipv4 = 1,
        .len = IPV4_LEN,
    };
    inet_pton(AF_INET, ipv4, &ipStructv4.addr4);

    const char ipv6[] = "1234:5678::2346";
    Ip ipStructv6 = {
        .ipv6 = 1,
        .len = IPV6_LEN,
    };
    inet_pton(AF_INET6, ipv6, &ipStructv6.addr6);

    Ip ipStructBoth = {
        .ipv4 = 1,
        .ipv6 = 1,
        .len = IPV6_LEN,
    };
    inet_pton(AF_INET, ipv4, &ipStructBoth.dualStack.addr4);
    inet_pton(AF_INET6, ipv6, &ipStructBoth.dualStack.addr6);

    int ipLen;
    PfcpFSeid fseid;
    char str[INET6_ADDRSTRLEN];
    
    // Test only IPv4
    UTLT_Assert(PfcpIpToFSeid(&ipStructv4, &fseid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpIpToFSeid fail");
    UTLT_Assert(fseid.v4 == 1, return STATUS_ERROR, "The v4 value of FSeid error : need 1, not %d", fseid.v4);
    UTLT_Assert(fseid.v6 == 0, return STATUS_ERROR, "The v6 value of FSeid error : need 0, not %d", fseid.v6);
    UTLT_Assert(inet_ntop(AF_INET, &fseid.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpIpToFSeid fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(ipLen == PFCP_F_SEID_IPV4_LEN, return STATUS_ERROR,
                "PfcpIpToFSeid fail : need len[%d], not len[%d]", PFCP_F_SEID_IPV4_LEN, ipLen);

    // Test only IPv6
    UTLT_Assert(PfcpIpToFSeid(&ipStructv6, &fseid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpIpToFSeid fail");
    UTLT_Assert(fseid.v4 == 0, return STATUS_ERROR, "The v4 value of FSeid error : need 0, not %d", fseid.v4);
    UTLT_Assert(fseid.v6 == 1, return STATUS_ERROR, "The v6 value of FSeid error : need 1, not %d", fseid.v6);
    UTLT_Assert(inet_ntop(AF_INET6, &fseid.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpIpToFSeid fail : need IP[%s], not IP[%s]", ipv6, str);
    UTLT_Assert(ipLen == PFCP_F_SEID_IPV6_LEN, return STATUS_ERROR,
                "PfcpIpToFSeid fail : need len[%d], not len[%d]", PFCP_F_SEID_IPV6_LEN, ipLen);

    // Test IPv4 and IPv6
    UTLT_Assert(PfcpIpToFSeid(&ipStructBoth, &fseid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpIpToFSeid fail");
    UTLT_Assert(fseid.v4 == 1, return STATUS_ERROR, "The v4 value of FSeid error : need 1, not %d", fseid.v4);
    UTLT_Assert(fseid.v6 == 1, return STATUS_ERROR, "The v6 value of FSeid error : need 1, not %d", fseid.v6);
    UTLT_Assert(inet_ntop(AF_INET, &fseid.dualStack.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpIpToFSeid fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(inet_ntop(AF_INET6, &fseid.dualStack.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpIpToFSeid fail : need IP[%s], not IP[%s]", ipv6, str);
    UTLT_Assert(ipLen == PFCP_F_SEID_IPV4V6_LEN, return STATUS_ERROR,
                "PfcpIpToFSeid fail : need len[%d], not len[%d]", PFCP_F_SEID_IPV4V6_LEN, ipLen);

    return STATUS_OK;
}

Status TestPfcpConvert_5() {
    const char ipv4[] = "150.151.152.153";
    Ip ipStructv4 = {
        .ipv4 = 1,
        .len = IPV4_LEN,
    };
    inet_pton(AF_INET, ipv4, &ipStructv4.addr4);

    const char ipv6[] = "1223:4556:7789::aabc";
    Ip ipStructv6 = {
        .ipv6 = 1,
        .len = IPV6_LEN,
    };
    inet_pton(AF_INET6, ipv6, &ipStructv6.addr6);

    Ip ipStructBoth = {
        .ipv4 = 1,
        .ipv6 = 1,
        .len = IPV6_LEN,
    };
    inet_pton(AF_INET, ipv4, &ipStructBoth.dualStack.addr4);
    inet_pton(AF_INET6, ipv6, &ipStructBoth.dualStack.addr6);

    int ipLen;
    PfcpFTeid fteid;
    char str[INET6_ADDRSTRLEN];

    // Test only IPv4
    UTLT_Assert(PfcpIpToFTeid(&ipStructv4, &fteid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpIpToFTeid fail");
    UTLT_Assert(fteid.v4 == 1, return STATUS_ERROR, "The v4 value of FTeid error : need 1, not %d", fteid.v4);
    UTLT_Assert(fteid.v6 == 0, return STATUS_ERROR, "The v6 value of FTeid error : need 0, not %d", fteid.v6);
    UTLT_Assert(inet_ntop(AF_INET, &fteid.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpIpToFTeid fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(ipLen == PFCP_F_TEID_IPV4_LEN, return STATUS_ERROR,
                "PfcpIpToFTeid fail : need len[%d], not len[%d]", PFCP_F_TEID_IPV4_LEN, ipLen);

    // Test only IPv6
    UTLT_Assert(PfcpIpToFTeid(&ipStructv6, &fteid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpIpToFTeid fail");
    UTLT_Assert(fteid.v4 == 0, return STATUS_ERROR, "The v4 value of FTeid error : need 0, not %d", fteid.v4);
    UTLT_Assert(fteid.v6 == 1, return STATUS_ERROR, "The v6 value of FTeid error : need 1, not %d", fteid.v6);
    UTLT_Assert(inet_ntop(AF_INET6, &fteid.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpIpToFTeid fail : need IP[%s], not IP[%s]", ipv6, str);
    UTLT_Assert(ipLen == PFCP_F_TEID_IPV6_LEN, return STATUS_ERROR,
                "PfcpIpToFTeid fail : need len[%d], not len[%d]", PFCP_F_TEID_IPV6_LEN, ipLen);
    
    // Test IPv4 and IPv6
    UTLT_Assert(PfcpIpToFTeid(&ipStructBoth, &fteid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpIpToFTeid fail");
    UTLT_Assert(fteid.v4 == 1, return STATUS_ERROR, "The v4 value of FTeid error : need 1, not %d", fteid.v4);
    UTLT_Assert(fteid.v6 == 1, return STATUS_ERROR, "The v6 value of FTeid error : need 1, not %d", fteid.v6);
    UTLT_Assert(inet_ntop(AF_INET, &fteid.dualStack.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpIpToFTeid fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(inet_ntop(AF_INET6, &fteid.dualStack.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpIpToFTeid fail : need IP[%s], not IP[%s]", ipv6, str);
    UTLT_Assert(ipLen == PFCP_F_TEID_IPV4V6_LEN, return STATUS_ERROR,
                "PfcpIpToFTeid fail : need len[%d], not len[%d]", PFCP_F_TEID_IPV4V6_LEN, ipLen);
 
    return STATUS_OK;
}

Status TestPfcpConvert_6() {
    const char ipv4[] = "150.151.152.153";
    Ip ipStructv4 = {
        .ipv4 = 1,
        .len = IPV4_LEN,
    };
    inet_pton(AF_INET, ipv4, &ipStructv4.addr4);

    const char ipv6[] = "1223:4556:7789::aabc";
    Ip ipStructv6 = {
        .ipv6 = 1,
        .len = IPV6_LEN,
    };
    inet_pton(AF_INET6, ipv6, &ipStructv6.addr6);

    Ip ipStructBoth = {
        .ipv4 = 1,
        .ipv6 = 1,
        .len = IPV6_LEN,
    };
    inet_pton(AF_INET, ipv4, &ipStructBoth.dualStack.addr4);
    inet_pton(AF_INET6, ipv6, &ipStructBoth.dualStack.addr6);

    int ipLen;
    PfcpUeIpAddr ueIp;
    char str[INET6_ADDRSTRLEN];

    // Test only IPv4
    UTLT_Assert(PfcpIpToUeIpAddr(&ipStructv4, &ueIp, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpIpToUeIpAddr fail");
    UTLT_Assert(ueIp.v4 == 1, return STATUS_ERROR, "The v4 value of UEIP error : need 1, not %d", ueIp.v4);
    UTLT_Assert(ueIp.v6 == 0, return STATUS_ERROR, "The v6 value of UEIP error : need 0, not %d", ueIp.v6);
    UTLT_Assert(inet_ntop(AF_INET, &ueIp.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpIpToUeIpAddr fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(ipLen == PFCP_UE_IP_ADDR_IPV4_LEN, return STATUS_ERROR,
                "PfcpIpToUeIpAddr fail : need len[%d], not len[%d]", PFCP_UE_IP_ADDR_IPV4_LEN, ipLen);

    // Test only IPv6
    UTLT_Assert(PfcpIpToUeIpAddr(&ipStructv6, &ueIp, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpIpToUeIpAddr fail");
    UTLT_Assert(ueIp.v4 == 0, return STATUS_ERROR, "The v4 value of UEIP error : need 0, not %d", ueIp.v4);
    UTLT_Assert(ueIp.v6 == 1, return STATUS_ERROR, "The v6 value of UEIP error : need 1, not %d", ueIp.v6);
    UTLT_Assert(inet_ntop(AF_INET6, &ueIp.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpIpToUeIpAddr fail : need IP[%s], not IP[%s]", ipv6, str);
    UTLT_Assert(ipLen == PFCP_UE_IP_ADDR_IPV6_LEN, return STATUS_ERROR,
                "PfcpIpToUeIpAddr fail : need len[%d], not len[%d]", PFCP_UE_IP_ADDR_IPV6_LEN, ipLen);

    // Test IPv4 and IPv6
     UTLT_Assert(PfcpIpToUeIpAddr(&ipStructBoth, &ueIp, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpIpToUeIpAddr fail");
    UTLT_Assert(ueIp.v4 == 1, return STATUS_ERROR, "The v4 value of UEIP error : need 1, not %d", ueIp.v4);
    UTLT_Assert(ueIp.v6 == 1, return STATUS_ERROR, "The v6 value of UEIP error : need 1, not %d", ueIp.v6);
    UTLT_Assert(inet_ntop(AF_INET, &ueIp.dualStack.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpIpToUeIpAddr fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(inet_ntop(AF_INET6, &ueIp.dualStack.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpIpToUeIpAddr fail : need IP[%s], not IP[%s]", ipv6, str);
    UTLT_Assert(ipLen == PFCP_UE_IP_ADDR_IPV4V6_LEN, return STATUS_ERROR,
                "PfcpIpToUeIpAddr fail : need len[%d], not len[%d]", PFCP_UE_IP_ADDR_IPV4V6_LEN, ipLen);

    return STATUS_OK;
}

Status TestPfcpConvert_7() {
    const char ipv4[] = "177.176.178.174";
    PfcpOuterHdr outerHdrv4 = {
        .gtpuIpv4 = 1,
    };
    inet_pton(AF_INET, ipv4, &outerHdrv4.addr4);

    const char ipv6[] = "ffff:eeee:aaaa:bbbb::cccc";
    PfcpOuterHdr outerHdrv6 = {
        .gtpuIpv6 = 1,
    };
    inet_pton(AF_INET6, ipv6, &outerHdrv6.addr6);

    PfcpOuterHdr outerHdrBoth = {
        .gtpuIpv4 = 1,
        .gtpuIpv6 = 1,
    };
    inet_pton(AF_INET, ipv4, &outerHdrBoth.dualStack.addr4);
    inet_pton(AF_INET6, ipv6, &outerHdrBoth.dualStack.addr6);

    Ip ip;
    char str[INET6_ADDRSTRLEN];

    // Test only IPv4
    UTLT_Assert(PfcpOuterHdrToIp(&outerHdrv4, &ip) == STATUS_OK, return STATUS_ERROR,
                "PfcpOuterHdrToIp fail");
    UTLT_Assert(ip.ipv4 == 1, return STATUS_ERROR, "The ipv4 value of IP error : need 1, not %d", ip.ipv4);
    UTLT_Assert(ip.ipv6 == 0, return STATUS_ERROR, "The ipv4 value of IP error : need 0, not %d", ip.ipv6);
    UTLT_Assert(ip.len == IPV4_LEN, return STATUS_ERROR,
                "The len of IP error : need %d, not %d", IPV4_LEN, ip.len);
    UTLT_Assert(inet_ntop(AF_INET, &ip.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpOuterHdrToIp fail : need IP[%s], not IP[%s]", ipv4, str);

    // Test only IPv6
    UTLT_Assert(PfcpOuterHdrToIp(&outerHdrv6, &ip) == STATUS_OK, return STATUS_ERROR,
                "PfcpOuterHdrToIp fail");
    UTLT_Assert(ip.ipv4 == 0, return STATUS_ERROR, "The ipv4 value of IP error : need 0, not %d", ip.ipv4);
    UTLT_Assert(ip.ipv6 == 1, return STATUS_ERROR, "The ipv4 value of IP error : need 1, not %d", ip.ipv6);
    UTLT_Assert(ip.len == IPV6_LEN, return STATUS_ERROR,
                "The len of IP error : need %d, not %d", IPV6_LEN, ip.len);
    UTLT_Assert(inet_ntop(AF_INET6, &ip.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpOuterHdrToIp fail : need IP[%s], not IP[%s]", ipv6, str);

    // Test IPv4 and IPv6
    UTLT_Assert(PfcpOuterHdrToIp(&outerHdrBoth, &ip) == STATUS_OK, return STATUS_ERROR,
                "PfcpOuterHdrToIp fail");
    UTLT_Assert(ip.ipv4 == 1, return STATUS_ERROR, "The ipv4 value of IP error : need 1, not %d", ip.ipv4);
    UTLT_Assert(ip.ipv6 == 1, return STATUS_ERROR, "The ipv4 value of IP error : need 1, not %d", ip.ipv6);
    UTLT_Assert(ip.len == IPV4V6_LEN, return STATUS_ERROR,
                "The len of IP error : need %d, not %d", IPV4V6_LEN, ip.len);
    UTLT_Assert(inet_ntop(AF_INET, &ip.dualStack.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpOuterHdrToIp fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(inet_ntop(AF_INET6, &ip.dualStack.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpOuterHdrToIp fail : need IP[%s], not IP[%s]", ipv6, str);

    return STATUS_OK;
};

Status TestPfcpConvert_8() {
    int port = 8787;

    const char ipv4[] = "88.89.90.91";
    SockAddr addrv4;
    UTLT_Assert(SockSetAddr(&addrv4, AF_INET, ipv4, port) == STATUS_OK, return STATUS_ERROR,
                "SockSetAddr fail : IP[%s], Port[%d]", ipv4, port);

    const char ipv6[] = "face:b006:600b:ecaf::9870";
    SockAddr addrv6;
    UTLT_Assert(SockSetAddr(&addrv6, AF_INET6, ipv6, port) == STATUS_OK, return STATUS_ERROR,
                "SockSetAddr fail : IP[%s], Port[%d]", ipv6, port);

    int ipLen;
    PfcpFTeid fteid;
    char str[INET6_ADDRSTRLEN];
    
    // Test only IPv4
    UTLT_Assert(PfcpSockaddrToFTeid(&addrv4, NULL, &fteid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpSockaddrToFTeid fail");
    UTLT_Assert(fteid.v4 == 1, return STATUS_ERROR, "The v4 value of FTeid error : need 1, not %d", fteid.v4);
    UTLT_Assert(fteid.v6 == 0, return STATUS_ERROR, "The v6 value of FTeid error : need 0, not %d", fteid.v6);
    UTLT_Assert(inet_ntop(AF_INET, &fteid.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpSockaddrToFTeid fail : need IP[%s], not IP[%s]", ipv4, str);
    UTLT_Assert(ipLen == PFCP_F_TEID_IPV4_LEN, return STATUS_ERROR,
                "PfcpSockaddrToFTeid fail : need len[%d], not len[%d]", PFCP_F_TEID_IPV4_LEN, ipLen);

    // Test only IPv6
    UTLT_Assert(PfcpSockaddrToFTeid(NULL, &addrv6, &fteid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpSockaddrToFTeid fail");
    UTLT_Assert(fteid.v4 == 0, return STATUS_ERROR, "The v4 value of FTeid error : need 0, not %d", fteid.v4);
    UTLT_Assert(fteid.v6 == 1, return STATUS_ERROR, "The v6 value of FTeid error : need 1, not %d", fteid.v6);
    UTLT_Assert(inet_ntop(AF_INET6, &fteid.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpSockaddrToFTeid fail : need IP[%s], not IP[%s]", ipv6, str);
    UTLT_Assert(ipLen == PFCP_F_TEID_IPV6_LEN, return STATUS_ERROR,
                "PfcpSockaddrToFTeid fail : need len[%d], not len[%d]", PFCP_F_TEID_IPV6_LEN, ipLen);
    
    // Test IPv4 and IPv6
     UTLT_Assert(PfcpSockaddrToFTeid(&addrv4, &addrv6, &fteid, &ipLen) == STATUS_OK, return STATUS_ERROR,
                "PfcpSockaddrToFTeid fail");
    UTLT_Assert(fteid.v4 == 1, return STATUS_ERROR, "The v4 value of FTeid error : need 1, not %d", fteid.v4);
    UTLT_Assert(fteid.v6 == 1, return STATUS_ERROR, "The v6 value of FTeid error : need 1, not %d", fteid.v6);
    UTLT_Assert(inet_ntop(AF_INET, &fteid.dualStack.addr4, str, INET_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv4, str) == 0, return STATUS_ERROR,
                "PfcpSockaddrToFTeid fail : need IP[%s], not IP[%s]", ipv4, str);
     UTLT_Assert(inet_ntop(AF_INET6, &fteid.dualStack.addr6, str, INET6_ADDRSTRLEN), return STATUS_ERROR,
                "inet_ntop fail");
    UTLT_Assert(strcmp(ipv6, str) == 0, return STATUS_ERROR,
                "PfcpSockaddrToFTeid fail : need IP[%s], not IP[%s]", ipv6, str);
     UTLT_Assert(ipLen == PFCP_F_TEID_IPV4V6_LEN, return STATUS_ERROR,
                "PfcpSockaddrToFTeid fail : need len[%d], not len[%d]", PFCP_F_TEID_IPV4V6_LEN, ipLen);

    return STATUS_OK;
}

int main() {
    BufblkPoolInit();

    TestPfcpConvert_1();
    TestPfcpConvert_2();
    TestPfcpConvert_3();
    TestPfcpConvert_4();
    TestPfcpConvert_5();
    TestPfcpConvert_6();
    TestPfcpConvert_7();
    TestPfcpConvert_8();

    return STATUS_OK;
}
