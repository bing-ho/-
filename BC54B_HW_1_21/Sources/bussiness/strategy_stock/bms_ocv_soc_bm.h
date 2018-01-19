/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_ocv_soc_calibration.h
 * @brief
 * @note
 * @date 2017-11-17
 *
 */

#ifndef BMS_OCV_SOC_BM_H_
#define BMS_OCV_SOC_BM_H_

#include "bms_ocv_soc.h"

INT8U get_delta_battery_cap_sign(void);
void set_delta_battery_cap_sign(INT8U value);


/**
* @brief   电量累计变化量外部访问接口
*/
INT32U get_delta_battery_cap_ams(void);

/**
* @brief   显示SOC外部访问接口， 惰性更新
*/
float get_soc_display(void);

/**
* @brief   soc_ah_calib外部访问接口， 惰性更新
*/
float get_soc_ah_calib(void) ;

/**
* @brief   delta_battery_cap_ams外部写接口
*/
void set_delta_battery_cap_ams(INT32U value);

/**
* @brief   显示SOC对应剩余电量外部访问接口
*/  
INT32U get_left_cap_ams_display(void);

/**
* @brief   left_cap_ams_display外部写接口
*/ 
void set_left_cap_ams_display(INT32U value);

/**
* @brief   OCV校准， 查表更新soc_bm_high, soc_bm_low, delta_battery_cap_ams
*/ 
void update_soc_bm_by_ocv_soc_table(void);

/**
* @brief   更新显示SOC对应的剩余容量
*/
INT32U update_left_cap_ams_display_with_factor(INT32U value, INT8S sign);

/**
* @brief   参数初始化
*/
void ocv_soc_calib_params_init(void) ;

/**
* @brief   soc_bm_high外部写接口
*/ 
void set_soc_bm_high (float value);

/*
* @brief   soc_bm_low外部写接口
*/ 
void set_soc_bm_low (float value);

/**
* @brief   根据电流方向， 充电取soc_bm_high,放电取soc_bm_low作为基准SOC
*/ 
float get_soc_bm(void);

/**
* @brief   计算显示SOC对应的剩余容量，相对安时积分SOC对应的剩余容量变化倍率
*/ 
float get_ah_calib_factor(float current_soc);

/**
* @brief   更新bmsinfo中的剩余容量
*/
void update_left_cap_by_soc_display(void);

/**
* @brief   left_cap_ams_display存储到Eeeprom中
*/
void save_left_cap_ams_display_to_config(void);

/**
* @brief   delta_battery_cap_ams存储到Eeeprom中
*/
void save_delta_battery_cap_ams_to_config(void);
#endif /* BMS_OCV_SOC_CALIBRATION_H_ */
