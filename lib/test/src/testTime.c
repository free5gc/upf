#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_time.h"

#define STR_SIZE 40

/* time now = 1566455087445585
 * GMT = 2019-08-22 06:24:47.445585 [234 Thu] (month is the actual month)
 * localtime = 2019-08-22 14:24:47.445585 [234 Thu] (month is the actual month)
 */
static utime_t now = INT64_C(1566455087445585);

Status TestTime_1() {
    utime_t timediff, current;
    time_t OSnow;

    current = TimeNow();
    time(&OSnow);

    timediff = OSnow - (current / USEC_PER_SEC);
    UTLT_Assert((timediff > -2) && (timediff < 2), return STATUS_ERROR, "TimeNow and OS time do not agree");

    return STATUS_OK;
}

Status TestTime_2() {
    Status status;
    TimeTM timeTM;
    TimeTM *tmPtr = NULL;
    char str[STR_SIZE+1];

    status = TimeConvert(&timeTM, now, TIME_USE_GMT);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "TimeConvert GMT error");
    
    tmPtr = &timeTM;
    sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d.%06d [%d %s]%s",
                 tmPtr->tm_ptr->tm_year + 1900,
                 tmPtr->tm_ptr->tm_mon + 1,
                 tmPtr->tm_ptr->tm_mday,
                 tmPtr->tm_ptr->tm_hour,
                 tmPtr->tm_ptr->tm_min,
                 tmPtr->tm_ptr->tm_sec,
                 tmPtr->tm_usec,
                 tmPtr->tm_ptr->tm_yday + 1,
                 day_snames[tmPtr->tm_ptr->tm_wday],
                 (tmPtr->tm_ptr->tm_isdst ? " DST" : ""));
    
    UTLT_Assert(strcmp("2019-08-22 06:24:47.445585 [234 Thu]", str) == 0, 
                return STATUS_ERROR, 
                "TimeConvert GMT error: need %s, not %s\n", 
                "2019-08-22 06:24:47.445585 [234 Thu]", 
                str);

    return STATUS_OK;
}

Status TestTime_3() {
    Status status;
    TimeTM timeTM;
    TimeTM *tmPtr = NULL;
    char str[STR_SIZE+1];

    status = TimeConvert(&timeTM, now, TIME_USE_LOCAL);
    UTLT_Assert(status == STATUS_OK, return STATUS_ERROR, "TimeConvert localtime error");
    
    tmPtr = &timeTM;
    sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d.%06d [%d %s]%s",
                 tmPtr->tm_ptr->tm_year + 1900,
                 tmPtr->tm_ptr->tm_mon + 1,
                 tmPtr->tm_ptr->tm_mday,
                 tmPtr->tm_ptr->tm_hour,
                 tmPtr->tm_ptr->tm_min,
                 tmPtr->tm_ptr->tm_sec,
                 tmPtr->tm_usec,
                 tmPtr->tm_ptr->tm_yday + 1,
                 day_snames[tmPtr->tm_ptr->tm_wday],
                 (tmPtr->tm_ptr->tm_isdst ? " DST" : ""));
    
    UTLT_Assert(strcmp("2019-08-22 14:24:47.445585 [234 Thu]", str) == 0, 
                return STATUS_ERROR, 
                "TimeConvert GMT error: need %s, not %s\n", 
                "2019-08-22 14:24:47.445585 [234 Thu]", 
                str);

    return STATUS_OK;
}

Status TimeTest(void *data) {
    Status status;

    status = TestTime_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestTime_1 fail");

    status = TestTime_2();
    UTLT_Assert(status == STATUS_OK, return status, "TestTime_2 fail");

    status = TestTime_3();
    UTLT_Assert(status == STATUS_OK, return status, "TestTime_3 fail");

    return STATUS_OK;
}
