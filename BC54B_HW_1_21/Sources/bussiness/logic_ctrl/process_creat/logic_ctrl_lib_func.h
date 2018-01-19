#ifndef	_LOGIC_CTRL_LIB_FUNC_H_
#define	_LOGIC_CTRL_LIB_FUNC_H_

#include "includes.h"



#define BMS_PRECHARGE_RETRY_CNT                 2 //预充失败重试次数
#define BMS_PRECHARGE_RETRY_INTERVAL            3000 //预充失败重试间隔

#define BMS_PRECHARGE_VALID_TV_MIN_ON_MCU       30 // V/bit MCU端最小有效电压
#define BMS_PRECHARGE_TV_ON_MCU                 (MV_TO_V(bcu_get_insulation_pre_total_voltage())) //V/bit MCU端总压值
#define BMS_PRECHARGE_TIMEOUT                   1000 //ms/bit 预充电超时时间
#define BMS_PRECHARGE_TIME_MIN                  500 //ms/bit 预充电最小预充时间
#define BMS_PRECHARGE_FINISH_DELTA_VOLT         15 // V/bit 预充完成压差


#endif