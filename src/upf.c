#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "upf_init.h"
#include "utlt_debug.h"
#include "utlt_event.h"
#include "utlt_network.h"
#include "upf_context.h"
#include "n4/n4_dispatcher.h"

static Status parseArgs(int argc, char *argv[]);
static Status checkPermission();
static void eventConsumer();
static void showHelp(_Bool breakLine);

int main(int argc, char *argv[]) {
    Status status, returnStatus = STATUS_OK;
    pthread_mutex_init(&UTLT_logBufLock, 0);

    UTLT_Assert(parseArgs(argc, argv) == STATUS_OK, return STATUS_ERROR, 
                "Error parsing args");

    if (checkPermission() != STATUS_OK) {
        return STATUS_ERROR;
    }

    status = UpfInit();
    UTLT_Assert(status == STATUS_OK, returnStatus = STATUS_ERROR,
                "UPF failed to initialize");

    if (status == STATUS_OK) {
        eventConsumer();
    }

    status = UpfTerm();
    UTLT_Assert(status == STATUS_OK, returnStatus = STATUS_ERROR,
                "UPF terminate error");

    pthread_mutex_destroy(&UTLT_logBufLock);
    return returnStatus;
}

static Status parseArgs(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "l:g:c:h")) != -1) {
        switch (opt) {
            case 'l':
                if (UpfSetNfLogPath(optarg) != STATUS_OK) {
                    return STATUS_ERROR;
                }
                break;

            case 'g':
                if (UpfSetFree5gcLogPath(optarg) != STATUS_OK) {
                    return STATUS_ERROR;
                }
                break;

            case 'c':
                if (UpfSetConfigPath(optarg) != STATUS_OK) {
                    return STATUS_ERROR;
                }
                break;

            case 'h':
                showHelp(0);
                exit(0);

            case '?':
                showHelp(1);
                UTLT_Error("Illigal option: %c", optopt);
                return STATUS_ERROR;

            default:
                showHelp(1);
                UTLT_Error("Not supported option: %c", optopt);
                return STATUS_ERROR;
        }
    }

    return STATUS_OK;
}

static Status checkPermission() {
    if (geteuid() != 0) {
        UTLT_Error("Please run UPF as root in order to enable route management "
                   "and communication with gtp5g kernel module.");
        return STATUS_ERROR;
    }
    return STATUS_OK;
}


static void eventConsumer() {
    Status status;
    Event event;

    while (1) {
        status = EventRecv(Self()->eventQ, &event);
        if (status != STATUS_OK) {
            if (status == STATUS_EAGAIN) {
                continue;
            } else {
                UTLT_Assert(0, break, "Event receive fail");
            }
        }

        UpfDispatcher(&event);
    }
}

static void showHelp(_Bool breakLine) {
    if (breakLine) {
        printf("\n");
    }

    printf("NAME:\n   upf - 5G User Plane Function (UPF)\n\n");
    printf("USAGE:\n   main [global options] command [command options] [arguments...]\n\n");
    printf("COMMANDS:\n   help, h  Shows a list of commands or help for one command\n\n");
    printf("GLOBAL OPTIONS:\n");
    printf("   -c FILE  Load configuration from FILE\n");
    printf("   -l FILE  Output NF log to FILE\n");
    printf("   -g FILE  Output free5gc log to FILE\n");
    printf("   -h       show help\n");
}

