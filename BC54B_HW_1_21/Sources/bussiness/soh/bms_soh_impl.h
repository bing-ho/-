/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file soh_impl.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-7-2
*
*/

#ifndef BMS_SOH_IMPL_H_
#define BMS_SOH_IMPL_H_
#include "bms_soh.h"
#include "bms_config.h"

#define SOH_70_PERCENT_CYCLE_MIN 3500
#define SOH_70_PERCENT_CYCLE_MAX 4000
#define SOH_70_PERCENT_CYCLE     3500 //循环充放电量3500C，认为SOH为70%(只针对铁锂的电池)。这个3500可根据不同的电池进行修改。

void soh_refersh(INT16U total_cap, INT16U max_cap);
Result soh_releated_config_changed(ConfigIndex index, INT16U new_value);
void soh_fix_as_totalcap(void);

#endif /* SOH_IMPL_H_ */
