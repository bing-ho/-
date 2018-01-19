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
#define SOH_70_PERCENT_CYCLE     3500 //ѭ����ŵ���3500C����ΪSOHΪ70%(ֻ�����﮵ĵ��)�����3500�ɸ��ݲ�ͬ�ĵ�ؽ����޸ġ�

void soh_refersh(INT16U total_cap, INT16U max_cap);
Result soh_releated_config_changed(ConfigIndex index, INT16U new_value);
void soh_fix_as_totalcap(void);

#endif /* SOH_IMPL_H_ */
