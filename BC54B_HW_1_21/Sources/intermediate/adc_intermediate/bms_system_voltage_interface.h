         /**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_system_voltage_impl.h
* @brief
* @note
* @author Liwei Dong
* @date 2014-9-23
*
*/

#ifndef BMS_SYSTEM_VOLTAGE_INTERFACE_H_
#define BMS_SYSTEM_VOLTAGE_INTERFACE_H_

#include "bms_defs.h"
#include "app_cfg.h"
#include "ads1013.h"
#include "bms_bcu.h"
#include "bms_job.h"
//#include "adc.h"

#define inline

#define BMS_SYSTEM_VOLTAGE_MIN              9000
#define BMS_SYSTEM_VOLTAGE_MAX              36000
#define BMS_SYSTEM_VOLTAGE_LOW_THRESHOLD    5000

#define BMS_24V_DC_VOLTAGE_MIN              9000
#define BMS_24V_DC_VOLTAGE_MAX              36000
#define BMS_24V_DC_VOLTAGE_LOW_THRESHOLD    5000

#define BMS_BAT_DET_VOLTAGE_MIN             9000
#define BMS_BAT_DET_VOLTAGE_MAX             36000
#define BMS_BAT_DET_VOLTAGE_LOW_THRESHOLD   5000

#define BCU_SYSTEM_VOLTAGE_JOB_PERIODIC     1000

#define BCU_SYSTEM_VOLT_BUFF_MAX            3
#define BCU_BATTERY_V_VOLT_MAGNIFICATION    11//12.8
#define BCU_24V_IN_VOLT_MAGNIFICATION       11

#if BMS_SUPPORT_BY5248D   ==  0 
#define BCU_BATTERY_V_VOLT_OFFSET           300 //mv 修正二极管压降
#else
#define BCU_BATTERY_V_VOLT_OFFSET           400 //mv 修正二极管压降
#endif
#define BCU_SYSTEM_VOLT_AD_PORT             15       
#define BCU_DC_VOLT_AD_PORT_V114            13
#define BCU_DC_VOLT_AD_PORT_V116            12

/***********************************************************************
  * @brief           bms工作电压检测初始化
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
void system_voltage_init(void);


/***********************************************************************
  * @brief           返回系统电压值
  * @param[in]       无  
  * @return          电压值
***********************************************************************/ 
inline INT16U system_voltage_get(void);

/***********************************************************************
  * @brief           判断系统供电电压是否有效
  * @param[in]       无  
  * @return          1：ok  0：err
***********************************************************************/
INT8U system_voltage_is_usful(void); 

/***********************************************************************
  * @brief           判断系统电压是否有效
  * @param[in]       无  
  * @return          1：ok  0：err
***********************************************************************/
INT8U system_voltage_is_valid(void);

/***********************************************************************
  * @brief           获取系统供电电压值
  * @param[in]       *__far volt  系统供电电压地址
  * @return          1：ok  0：err
***********************************************************************/
INT8U bcu_calc_system_voltage(INT16U *__far volt);

/***********************************************************************
  * @brief           获取系统供电电压值
  * @param[in]       *data  未使用
  * @return          无
***********************************************************************/
void bcu_system_voltage_detect(void* data);//系统供电电压检测

/***********************************************************************
  * @brief           获取24V供电电压值大小
  * @param[in]       无
  * @return          返回电压值
***********************************************************************/
INT16U dc_24V_voltage_get(void);

/***********************************************************************
  * @brief           判断24v电压值是否有效
  * @param[in]       无
  * @return          1：ok  0：err
***********************************************************************/
INT8U dc_24V_voltage_is_usful(void);

/***********************************************************************
  * @brief           判断24v电压值是否有效
  * @param[in]       无
  * @return          1：ok  0：err
***********************************************************************/
INT8U dc_24V_voltage_is_valid(void);

/***********************************************************************
  * @brief           充电直流低压检测
  * @param[in]       *volt  返回电压值地址
  * @return          1：ok  0：err
***********************************************************************/
INT8U bcu_calc_24V_dc_voltage(INT16U *volt);

/***********************************************************************
  * @brief           返回蓄电池的电压值
  * @param[in]       无
  * @return          电压值
***********************************************************************/
INT16U bcu_get_lead_acid_volt(void);

/***********************************************************************
  * @brief           返回2.5V基准电压值
  * @param[in]       无
  * @return          电压值
***********************************************************************/
INT16U bat_det_voltage_get(void);






#endif /* BMS_SYSTEM_VOLTAGE_IMPL_H_ */
