#ifndef __KERNEL_GTP5G_BUFFER_H__
#define __KERNEL_GTP5G_BUFFER_H__

/*
 * These functions shall be customized by kinds of device.
 * You can create a directory and put all customized function
 * in there, like "device.c" under "updk/src/kernel/".
 *
 * This file would be included when the UP part runs with
 * Linux kernel module "gtp5g", otherwise it will not be used.
 */


#include "utlt_debug.h"

/**
 * BufferServerInit - Initialize buffering server for GTP5G
 * 
 * @return: STATUS_OK or STATUS_ERROR if there is any error in initialization
 */
Status BufferServerInit();

/**
 * BufferServerTerm - Terminate buffering server for GTP5G
 * 
 * @return: STATUS_OK or STATUS_ERROR if there is any error in termination
 */
Status BufferServerTerm();

#endif /* __KERNEL_GTP5G_BUFFER_H__ */