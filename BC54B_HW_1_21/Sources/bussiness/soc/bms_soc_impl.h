/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file soc.h
* @brief 利用积分法实现SOC的计算
* @note
* @author
* @date 2012-5-9
*
*/

#ifndef BMS_SOC_IMPL_H_
#define BMS_SOC_IMPL_H_
#include "bms_soc.h"

#define SOC_INIT_DISPLAY_DELAY 2000  //SOC初始化延时刷新时间

#define LEFTSOC0FLAGE 0XFF

void soc_daemon_run(void);
void soc_daemon_check(void);

#pragma INLINE
INT16U soc_get_max_cap(void);

Result soc_on_voltage_charging_normal(EventTypeId event, void* event_data, void* user_data);
Result soc_on_voltage_charging_2nd_normal(EventTypeId event, void* event_data, void* user_data);
Result soc_on_voltage_discharging_normal(EventTypeId event, void* event_data, void* user_data);
Result soc_on_voltage_discharging_3rd_alarm(EventTypeId event, void* event_data, void* user_data);

INT16U bms_get_ad1_value_h(void);
INT16U bms_get_ad1_value_l(void);
INT16U bms_get_ad1_value_single(void) ;
Current_platform bms_get_current_platform(void);
void bms_set_current_platform(Current_platform platform); 
#endif /* SOC_DAEMON_H_ */