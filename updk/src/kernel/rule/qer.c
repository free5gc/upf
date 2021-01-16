#include "updk/rule.h"

#include "updk/rule_qer.h"

/*
 * These functions shall be customized by kinds of device.
 * You can create a directory and put all customized function
 * in there, like "device.c" under "updk/src/kernel/include"
 *
 * Note:
 * 1. Parameter in each Gtpv1Tunnel* function may not be the same.
 * Therefore, please do not use memory copy function to handle in different type.
 * 2. Function "Gtpv1Tunnel*" how to work is dependent on the kind of device.
 * It is up to you can set the real rule into your device or just treat it
 * as a notification.
 */

// Kernel don't support QER yet, so these functions are only usesd to test
int Gtpv1TunnelCreateQER(UPDK_QER *qer) {
    return 0;
}

// Kernel don't support QER yet, so these functions are only usesd to test
int Gtpv1TunnelUpdateQER(UPDK_QER *qer) {
    return 0;
}

// Kernel don't support QER yet, so these functions are only usesd to test
int Gtpv1TunnelRemoveQER(UPDK_QER *qer) {
    return 0;
}