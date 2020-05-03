#ifndef __UPF_INIT_H__
#define __UPF_INIT_H__

#include "utlt_debug.h"
#include "utlt_lib.h"

/**
 * Init & Term - Function Pointer for UpfOps to do initialization and termination
 * 
 * @data: parameter used by self function pointer
 * @return: STATUS_OK or STATUS_ERROR if the functions exec fail
 */
typedef Status (*Init)(void *data);
typedef Status (*Term)(void *data);

/**
 * UpfOps - Structure for UpfInit and UpfTerm
 * 
 * @name: used for print log
 * 
 * @init: NULL or function pointer that it will exec when UPF initialization
 * @initData: parameter for function "init"
 * 
 * @term: NULL or function pointer that it will exec when UPF termination
 * @termData: parameter for function "init"
 */
typedef struct {
    char *name;
    
    Init init;
    void *initData;

    Term term;
    void *termData;
} UpfOps;

/**
 * UpfInit - Do the initialization for UPF by UpfOpsList in "upf_init.c"
 * 
 * @configPath: Path for set parameters when UPF init
 * @return: STATUS_OK or STATUS_ERROR if one part of init is failed
 */
Status UpfInit();

/**
 * UpfTerm - Do the termination for UPF by UpfOpsList in "upf_init.c"
 * 
 * @return: STATUS_OK or STATUS_ERROR if one part of init is failed
 */
Status UpfTerm();

/**
 * UpfSetConfigPath - Set config path for init
 */

Status UpfSetConfigPath(char *path);

#endif /* __UPF_INIT_H__ */
