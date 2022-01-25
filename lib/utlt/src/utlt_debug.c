#include "utlt_debug.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include "logger.h"
#include "utlt_lib.h"

// TODO : Need to use buffer written by ourself
#define MAX_SIZE_OF_BUFFER 32768

unsigned int reportCaller = 0;
pthread_mutex_t UTLT_logBufLock;
static int logLevel = LOG_INFO;

static void __lowerString(char *output, const char *input) {
    size_t n = strlen(input);
    size_t i;

    for (i = 0; i < n; i++) {
        output[i] = tolower(input[i]);
    }
    output[i] = '\0';
}

Status UTLT_SetLogLevel(const char *level) {
    if (UpfUtilLog_SetLogLevel(UTLT_CStr2GoStr(level))) {
        char *lwrLevel = malloc(strlen(level)+1);
        if (!lwrLevel)
            return STATUS_ERROR;
        __lowerString(lwrLevel, level);

        if (!strcmp(lwrLevel, "panic"))
            logLevel = LOG_PANIC;
        else if (!strcmp(lwrLevel, "fatal"))
            logLevel = LOG_FATAL;
        else if (!strcmp(lwrLevel, "error"))
            logLevel = LOG_ERROR;
        else if (!strcmp(lwrLevel, "warning"))
            logLevel = LOG_WARNING;
        else if (!strcmp(lwrLevel, "info"))
            logLevel = LOG_INFO;
        else if (!strcmp(lwrLevel, "debug"))
            logLevel = LOG_DEBUG;
        else if (!strcmp(lwrLevel, "trace"))
            logLevel = LOG_TRACE;
        else {
            free(lwrLevel);
            return STATUS_ERROR;
        }

        free(lwrLevel);
        return STATUS_OK;
    }
    else
        return STATUS_ERROR;
}

Status UTLT_SetReportCaller(unsigned int flag) {
    if (reportCaller >= REPORTCALLER_MAX) {
        reportCaller = 0;
        return STATUS_ERROR;
    }

    reportCaller = flag;
    return STATUS_OK;
}

Status UTIL_LogFileHook(const char *nfPath, const char *free5gcPath) {
    if (UpfUtilLog_FileHook(UTLT_CStr2GoStr(nfPath), UTLT_CStr2GoStr(free5gcPath))) {
        return STATUS_OK;
    } else {
        return STATUS_ERROR;
    }
}

int UTLT_LogPrint(int level, const char *filename, const int line, 
                  const char *funcname, const char *fmt, ...) {
    char buffer[MAX_SIZE_OF_BUFFER];

    unsigned int cnt, vspCnt;
    Status status = STATUS_OK;
    if (level > logLevel) return status;
    va_list vl;
    va_start(vl, fmt);
    pthread_mutex_lock(&UTLT_logBufLock);
    vspCnt = vsnprintf(buffer, sizeof(buffer), fmt, vl);
    if (vspCnt < 0) {
        fprintf(stderr, "vsnprintf in UTLT_LogPrint error : %s\n", strerror(errno));
        status = STATUS_ERROR;
    } else if (vspCnt == 0) {
        status = STATUS_OK;
    }
    va_end(vl);
    if (status != STATUS_OK) goto unlockReturn;

    if (reportCaller == REPORTCALLER_TRUE) {
        cnt = snprintf(buffer + vspCnt, sizeof(buffer) - vspCnt, " (%s:%d %s)", filename, line, funcname);
        if (cnt < 0) {
            fprintf(stderr, "sprintf in UTLT_LogPrint error : %s\n", strerror(errno));
            status = STATUS_ERROR;
            goto unlockReturn;
        }
    }

    switch(level) {
        case 0 :
            UpfUtilLog_Panicln(UTLT_CStr2GoStr(buffer));
            break;
        case 1 :
            UpfUtilLog_Fatalln(UTLT_CStr2GoStr(buffer));
            break;
        case 2 :
            UpfUtilLog_Errorln(UTLT_CStr2GoStr(buffer));
            break;
        case 3 :
            UpfUtilLog_Warningln(UTLT_CStr2GoStr(buffer));
            break;
        case 4 :
            UpfUtilLog_Infoln(UTLT_CStr2GoStr(buffer));
            break;
        case 5 :
            UpfUtilLog_Debugln(UTLT_CStr2GoStr(buffer));
            break;
        case 6 :
            UpfUtilLog_Traceln(UTLT_CStr2GoStr(buffer));
            break;
        default :
            fprintf(stderr, "The log level %d is out of range.\n", level);
            status = STATUS_ERROR;
    }

unlockReturn:
    pthread_mutex_unlock(&UTLT_logBufLock);
    return status;
}

const char *UTLT_StrStatus(Status status) {
    switch(status) {
        case STATUS_OK :
            return "status OK";
        case STATUS_ERROR :
            return "status error";
        case STATUS_EAGAIN :
            return "status eagain";
        default :
            return "status unknown";
    }
}
