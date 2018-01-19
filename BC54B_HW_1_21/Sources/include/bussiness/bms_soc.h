/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_soc.h
* @brief
* @note
* @author
* @date 2012-5-12
*
*/

#ifndef BMS_SOC_H_
#define BMS_SOC_H_

#include "bms_defs.h"
#include "bms_current.h"
#include "bms_bcu.h"
#include "bms_system.h"
#include "bms_charger_common.h"
#include "bms_wdt.h"
#include "bms_event.h"
#include "power_down_data_save.h"

#define SOC_NOT_100_PERCENT_FOR_CHARGING_EN     1  //充电未充满时，SOC最大值只能为99%
#define SOC_MAX_FOR_CHARGER_CHARGING            PERCENT_TO_SOC(99) //0.01%/bit
/**
 * SOC模块初始化函数
 */
void soc_init(void);
void soc_unit(void);
INT8U soc_is_support(void);

void soc_daemon_run(void);

/**
 * Get the SOC value
 * @return the SOC value, unit:1mAh
 */
INT32U soc_get(void);
INT32U soc_get_total_delta_time(void);

/**
 * 刷新SOC数值
 */
void soc_refresh(void);
void soc_empty_calib_check(void);
void soc_update_left_cap_by_tenfold_ah(INT16U cap);//void soc_update_left_cap_by_ah(INT16U cap);
void soc_update_tenfold_left_cap_by_ams(CapAms_t cap);//void soc_update_left_cap_by_ams(CapAms_t cap);
/**
 * 计算SOC的值
 * @param left_cap 剩余容量
 * @param total_cap 总容量
 * @return
 */
INT16U soc_from_cap(INT16U left_cap, INT16U total_cap);
INT16U soc_from_cap2(CapAms_t left_cap, INT16U total_cap);

void soc_fix_as_empty(void);
void soc_fix_as_full(void);
void soc_check_and_fix_as_full(void);
void soc_check_and_fix_as_empty(void);
void soc_reset_charging_checked_flags(void);
void soc_reset_discharging_checked_flags(void);

#endif /* BMS_SOC_H_ */
