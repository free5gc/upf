#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utlt_debug.h"
#include "utlt_buff.h"
#include "utlt_list.h"
#include "knet_route.h"

Status testKnet_1() {
    printf("[Sudo Testing] testKnet 1\n");
    Status status = STATUS_OK;
    
    ListHead *routeEntries = KnetGetRoutes(AF_INET, RT_TABLE_MAIN);
    UTLT_Assert(routeEntries, return STATUS_ERROR, "");
    
    UTLT_Info("Printing IPv4 main routing table:");
    UTLT_Assert(KnetPrintRoutes(routeEntries) == STATUS_OK, status = STATUS_ERROR, "");

    KnetRtListFree(routeEntries);
    return status;
}

Status testKnet_2() {
    printf("[Sudo Testing] testKnet 2\n");
    Status status = STATUS_OK;
    
    ListHead *routeEntries = KnetGetRoutes(AF_INET6, RT_TABLE_MAIN);
    UTLT_Assert(routeEntries, return STATUS_ERROR, "");
    
    UTLT_Info("Printing IPv6 main routing table:");
    UTLT_Assert(KnetPrintRoutes(routeEntries) == STATUS_OK, status = STATUS_ERROR, "");

    KnetRtListFree(routeEntries);
    return status;
}

Status testKnet_3() {
    printf("[Sudo Testing] testKnet 3\n");
    Status status;

    printf("Adding route 100.100.0.0/24 dev lo via 127.0.0.1: ");
    status = KnetAddRoute("lo", "100.100.0.0", 24, "127.0.0.1", 123);
    printf(status == STATUS_OK ? "OK\n" : "failed\n");

    // Print IPv4 main routing table
    ListHead *routeEntries = KnetGetRoutes(AF_INET, RT_TABLE_MAIN);
    UTLT_Assert(routeEntries, return STATUS_ERROR, "");
    UTLT_Assert(KnetPrintRoutes(routeEntries) == STATUS_OK, status = STATUS_ERROR, "");
    KnetRtListFree(routeEntries);
    return status;
}

Status testKnet_4() {
    printf("[Sudo Testing] testKnet 4\n");
    Status status;

    printf("Deleting route 100.100.0.0/24 dev lo via 127.0.0.1: ");
    status = KnetDelRoute("lo", "100.100.0.0", 24, "127.0.0.1", 123);
    printf(status == STATUS_OK ? "OK\n" : "failed\n");

    // Print IPv4 main routing table
    ListHead *routeEntries = KnetGetRoutes(AF_INET, RT_TABLE_MAIN);
    UTLT_Assert(routeEntries, return STATUS_ERROR, "");
    UTLT_Assert(KnetPrintRoutes(routeEntries) == STATUS_OK, status = STATUS_ERROR, "");
    KnetRtListFree(routeEntries);
    return status;
}

int main() {
    fprintf(stderr, "**********************************************************************************************\n");
    fprintf(stderr, "[Warning] This test will need privilege user to run because it will modify the network setting\n");
    fprintf(stderr, "[Warning] Please open the new terminal tab to do this test\n");
    fprintf(stderr, "**********************************************************************************************\n");

    BufblkPoolInit();

    testKnet_1();
    testKnet_2();
    testKnet_3();
    testKnet_4();

    BufblkPoolFinal();
    return 0;
}
