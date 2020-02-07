#include <stdio.h>

#include "test_utlt.h"
#include "utlt_debug.h"
#include "utlt_3gppTypes.h"

int TestPlmnId_1() {
    uint16_t mcc = 208, mnc = 93, mncLen = 2;
    plmnId_t plmnId;
    SetPlmnId(&plmnId, mcc, mnc, mncLen);
    UTLT_Assert(plmnId.octet[0] == 0x02, return STATUS_ERROR, "The value of plmn octet 1 is 0x02, not %d", plmnId.octet[0]);
    UTLT_Assert(plmnId.octet[1] == 0xF8, return STATUS_ERROR, "The value of plmn octet 1 is 0xF8, not %d", plmnId.octet[1]);
    UTLT_Assert(plmnId.octet[2] == 0x39, return STATUS_ERROR, "The value of plmn octet 1 is 0x39, not %d", plmnId.octet[2]);
    
    UTLT_Assert(GetMcc(&plmnId) == mcc, return STATUS_ERROR, "The value of MCC getting from plmn id is %d, not %d", mcc, GetMcc(&plmnId));
    UTLT_Assert(GetMnc(&plmnId) == mnc, return STATUS_ERROR, "The value of MNC getting from plmn id is %d, not %d", mnc, GetMnc(&plmnId));

    return STATUS_OK;
}

Status _3gppTypesTest(void *data) {
    Status status;

    status = TestPlmnId_1();
    UTLT_Assert(status == STATUS_OK, return status, "TestPlmnId_1 fail");

    return STATUS_OK;
}
