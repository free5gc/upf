#include "utlt_3gppTypes.h"

#include <stdio.h>

#define PlmnIdDigit1(x) ((x / 100) % 10)
#define PlmnIdDigit2(x) ((x / 10) % 10)
#define PlmnIdDigit3(x) (x % 10)

void SetPlmnId(plmnId_t *plmnId, uint16_t mcc, uint16_t mnc, uint8_t mncLen) {
    plmnId->octet[0] = (PlmnIdDigit2(mcc) << 4) + PlmnIdDigit1(mcc);
    plmnId->octet[1] = (mncLen == 2 ? 0xF0 : PlmnIdDigit1(mnc) << 4) + PlmnIdDigit3(mcc);
    plmnId->octet[2] = (PlmnIdDigit3(mnc) << 4) + PlmnIdDigit2(mnc);
}

uint16_t GetMcc(plmnId_t *plmnId) {
    return (plmnId->octet[0] & 0x0F) * 100 + (plmnId->octet[0] >> 4) * 10 + (plmnId->octet[1] & 0x0F);
}

uint16_t GetMnc(plmnId_t *plmnId) {
    return (plmnId->octet[1] >> 4 == 0xF ? 0 : (plmnId->octet[1] >> 4) * 100) + (plmnId->octet[2] & 0x0F) * 10 + (plmnId->octet[2] >> 4);
}

uint16_t GetMncLen(plmnId_t *plmnId) {
    return (plmnId->octet[1] ^ 0xF0) ? 3 : 2;
}
