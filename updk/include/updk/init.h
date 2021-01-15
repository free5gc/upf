#ifndef __UPDK_INIT_H__
#define __UPDK_INIT_H__

#include "updk/env.h"

/*
 * These functions shall be customized by kinds of device.
 * You can create a directory and put all customized function
 * in there, like "device.c" under "updk/src/kernel/"
 */

/**
 * Gtpv1EnvInit - Init Environment by EnvParams,
 *                like tuntap created, socket created
 *
 * @env: EnvParams pointer which shall be initialized first
 * @return: fd of the UDP socket for buffering passing up, 0 if it is not used or -1 if error
 */
int Gtpv1EnvInit(EnvParams *env);

/**
 * Gtpv1EnvTerm - Terminate and clean up Environment by EnvParams,
 *                like tuntap delete, socket close
 *
 * @env: EnvParams pointer which shall be initialized first
 * @return: 0 or -1 if one of parts terminates/cleans up failed
 */
int Gtpv1EnvTerm(EnvParams *env);

#endif /* __UPDK_INIT_H__ */