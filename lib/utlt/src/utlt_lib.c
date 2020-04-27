#include "utlt_lib.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

GoString UTLT_CStr2GoStr(const char *str) {
    GoString goStr = {str, strlen(str)};
    return goStr;
}

Status GetAbsPath(char *str) {
    UTLT_Assert(str, return STATUS_ERROR, "null string");
    UTLT_Assert(strlen(str) < MAX_FILE_PATH_STRLEN, return STATUS_ERROR, "string to long");
    
    char absPath[MAX_FILE_PATH_STRLEN];
    UTLT_Assert(realpath(str, absPath), return STATUS_ERROR, "realpath fail : %s", strerror(errno));
    strcpy(str, absPath);

    return STATUS_OK;
}
