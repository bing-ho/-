#ifndef __BMS_DTU_BUILDIN_H__
#define __BMS_DTU_BUILDIN_H__

#include "app_cfg.h"

#if defined(BMS_SUPPORT_DTU) && (BMS_SUPPORT_DTU == BMS_DTU_BUILDIN)
void dtu_init(void);
#endif

#endif
