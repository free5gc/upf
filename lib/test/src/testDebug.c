#include <stdio.h>
#include <string.h>

#include "test_utlt.h"
#include "utlt_debug.h"

Status TestDebug_1() {
    /*
    // Have Output
    UTLT_Info("Debug Test");
    */
    // No Output
    UTLT_Error("");
    return STATUS_OK;
}

Status TestDebug_2() {
    const char *needOK = UTLT_StrStatus(STATUS_OK);
    UTLT_Assert(strcmp(needOK, "status OK") == 0, return STATUS_ERROR,
                "UTLT_StrStatus fail : need 'status OK', not %s", needOK);

    const char *needError = UTLT_StrStatus(STATUS_ERROR);
    UTLT_Assert(strcmp(needError, "status error") == 0, return STATUS_ERROR,
                "UTLT_StrStatus fail : need 'status error', not %s", needError);

    const char *needEagain = UTLT_StrStatus(STATUS_EAGAIN);
    UTLT_Assert(strcmp(needEagain, "status eagain") == 0, return STATUS_ERROR,
                "UTLT_StrStatus fail : need 'status eagain', not %s", needEagain);

    const char *needUnknown = UTLT_StrStatus(87);
    UTLT_Assert(strcmp(needUnknown, "status unknown") == 0, return STATUS_ERROR,
                "UTLT_StrStatus fail : need 'status unknown', not %s", needUnknown);

    return STATUS_OK;
}

Status DebugTest(void *data) {
    Status status;

    status = TestDebug_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestDebug_1 fail");

    status = TestDebug_2();
    UTLT_Assert(status == STATUS_OK, return status, "TestDebug_2 fail");

    return STATUS_OK;
}
