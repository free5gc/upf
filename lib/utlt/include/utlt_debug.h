#ifndef __UTLT_DEBUG_H__
#define __UTLT_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <string.h>

typedef int Status;
#define STATUS_ERROR -1
#define STATUS_OK 0
#define STATUS_EAGAIN 1

Status UTLT_SetLogLevel(const char *level);
Status UTLT_SetReportCaller(unsigned int reportCaller);

const char *UTLT_StrStatus(Status status);

int UTLT_LogPrint(int level, const char *filename, const int line,
                  const char *funcname, const char *fmt, ...);

enum LogLevel {
    LOG_PANIC = 0,
    LOG_FATAL,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TRACE,
};

enum ReportCaller {
    REPORTCALLER_FALSE = 0,
    REPORTCALLER_TRUE,

    REPORTCALLER_MAX,
};

#define __FILENAME__ (strstr(__FILE__, "/gofree5gc/src/upf/") ? strstr(__FILE__, "/gofree5gc/src/upf/") + 11 : __FILE__)

#define UTLT_Panic(fmt, ...) \
    UTLT_LogPrint(LOG_PANIC, __FILENAME__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define UTLT_Fatal(fmt, ...) \
    UTLT_LogPrint(LOG_FATAL, __FILENAME__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define UTLT_Error(fmt, ...) \
    UTLT_LogPrint(LOG_ERROR, __FILENAME__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define UTLT_Warning(fmt, ...) \
    UTLT_LogPrint(LOG_WARNING, __FILENAME__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define UTLT_Info(fmt, ...) \
    UTLT_LogPrint(LOG_INFO, __FILENAME__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define UTLT_Debug(fmt, ...) \
    UTLT_LogPrint(LOG_DEBUG, __FILENAME__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define UTLT_Trace(fmt, ...) \
    UTLT_LogPrint(LOG_TRACE, __FILENAME__, __LINE__, __func__, fmt, ## __VA_ARGS__)

#define UTLT_Assert(cond, expr, fmt, ...) \
    if (!(cond)) { \
        UTLT_Error(fmt, ## __VA_ARGS__); \
        expr; \
    }

#define UTLT_Level_Assert(level, cond, expr, fmt, ...) \
    if (!(cond)) { \
        UTLT_LogPrint(level, __FILENAME__, __LINE__, __func__, fmt, ## __VA_ARGS__); \
        expr; \
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __UTLT_DEBUG_H__ */
