#ifndef __UPF_CONFIG_H__
#define __UPF_CONFIG_H__

#define UPF_EXPECTED_CONFIG_VERSION "1.0.0"

#include "utlt_debug.h"

Status UpfLoadConfigFile(const char *configFilePath);

Status UpfConfigParse();

#endif /* __UPF_CONFIG_H__ */
