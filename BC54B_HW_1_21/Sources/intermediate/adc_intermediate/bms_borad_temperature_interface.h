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

#ifndef __BMS_BOARD_TEMPERATURE_INTERFACE_H__
#define __BMS_BOARD_TEMPERATURE_INTERFACE_H__

#include "bms_defs.h"
#include "app_cfg.h"
//#include "adc.h"
#include "bms_job.h"
#include "NTC.h"
#include "bms_util.h"
#include "bms_bmu.h"

#define inline

#define NCP18XH103_MAX          34
#define NCP18XH103_TEMP_FACTOR  5 //温度分辨率为5
#define NCP18XH103_TEMP_OFFSET  -40
#define NCP18XH103_TEMP_MIN     -40
#define NCP18XH103_TEMP_MAX     125

#define BCU_BOARD_TEMPERATURE_JOB_PERIODIC  1000

#define BCU_BOARD_TEMPERATURE_AD_CHANNEL    0 //BCU板载温度检测AD通道
#define BCU_BOARD_TEMPERATURE_BUFF_MAX      4

#define BCU_BOARD_TEMPERATURE2_AD_CHANNEL    12 //BCU板载温度检测AD通道
#define BCU_BOARD_TEMPERATURE2_BUFF_MAX      4


/***********************************************************************
  * @brief           板载温度初始化函数
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
void board_temperature_init(void);

/***********************************************************************
  * @brief           获取板载温度1大小
  * @param[in]       无  
  * @return          温度值
***********************************************************************/ 
inline INT8U board_temperature_get(void); 

/***********************************************************************
  * @brief           获取板载温度2大小
  * @param[in]       无  
  * @return          温度值
***********************************************************************/ 
inline INT8U board_temperature2_get(void); 

/***********************************************************************
  * @brief           获取板载温度值
  * @param[in]       which   温度序号  
  * @param[in]       *temp   返回温度值 
  * @return          1：ok  0：err
***********************************************************************/ 
INT8U bcu_calc_board_temperature(INT8U which, INT8U *temp);

/***********************************************************************
  * @brief           获取板载温度值
  * @param[in]       data   未使用  
  * @return          无
***********************************************************************/ 
void bcu_board_temperature_detect(void* data);

/***********************************************************************
  * @brief           返回有效温度的平均值
  * @param[in]       无  
  * @return          温度值
***********************************************************************/ 
INT8U board_average_temperature_get(void);



#endif /* BMS_SYSTEM_VOLTAGE_IMPL_H_ */
