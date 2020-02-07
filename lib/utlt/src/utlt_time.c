#include "utlt_time.h"

utime_t TimeNow() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * USEC_PER_SEC + tv.tv_usec;
}

Status TimeConvert(TimeTM *timeTM, utime_t ut, int timeType) {
    struct tm *tm;
    time_t tt = (ut / USEC_PER_SEC);
    timeTM->tm_usec = ut % USEC_PER_SEC;

    if (timeType == TIME_USE_LOCAL)
        tm = localtime(&tt);
    else
        tm = gmtime(&tt);

    timeTM->tm_ptr = tm;

    return STATUS_OK;
}