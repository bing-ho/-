/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_unit.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-7-3
*
*/

#ifndef BMS_UNIT_H_
#define BMS_UNIT_H_
#include "bms_defs.h"

/****************************************
 *  SOC
****************************************/
#define SOC_TO_INT8U(VALUE) ((INT8U)(((INT32U)(VALUE) * 250 + 5000) / 10000))
#define SOC_TO_PERCENT(VALUE) ((INT8U)((VALUE+50) / 100))
#define PERCENT_TO_SOC(VALUE) ((INT16U)(VALUE) * 100)
#define SOC_TO_THOUSAND(VALUE) (((VALUE)+5)/10)
/****************************************
 *  SOH
****************************************/
#define SOH_TO_INT8U(VALUE) ((INT8U)(((INT32U)(VALUE) * 250 +5000) / 10000))
#define SOH_TO_PERCENT(VALUE) ((INT8U)(((VALUE)+50) / 100))
#define PERCENT_TO_SOH(VALUE) ((INT16U)(VALUE) * 100)
#define SOH_TO_THOUSAND(VALUE) ((VALUE+5)/10)
/****************************************
 *  Cap
****************************************/
/** mAS -> mAH */
#define CAP_MAS_TO_AH(VALUE) (INT16U)(((VALUE) + 1800000) / 3600000)
#define CAP_MAS_TO_TENFOLD_AH(VALUE) (INT16U)(((VALUE) + 180000) / 360000)
/** AH -> mAH */
#define CAP_AH_TO_MAS(VALUE) ((CapAms_t)(VALUE) * 3600000)
#define CAP_TENFOLD_AH_TO_MAS(VALUE) ((CapAms_t)(VALUE) * 360000)

/****************************************
 *  Voltage
****************************************/
#define V_TO_MV(VALUE) ((VALUE) * 1000)
#define MV_TO_V(VALUE) (((VALUE) + 500) / 1000)
#define MV_TO_100MV(VALUE)  (((VALUE) + 50) / 100)
#define TV_100_MV_TO_V(VALUE)   ((VALUE) / 10)
#define TV_100_MV_FROM_V(VALUE)   ((VALUE) * 10)
#define VOLT_4_DISPLAY(VALUE) low_voltage_for_display(VALUE)
#define MV_TO_10MV(VALUE)  (((VALUE) + 5) / 10) 
#define MV_FROM_10MV(VALUE) ((VALUE) * 10)
/****************************************
 *  Current
****************************************/
#define CURRENT_100_MA_TO_A(VALUE) ((VALUE) / 10)
#define CURRENT_100_MA_FROM_A(VALUE)   ((VALUE) * 10)

/****************************************
 * Power
****************************************/
#define POWER_100_W_TO_W(VALUE)  ((VALUE) * 100)
#define POWER_100_W_FROM_W(VALUE)    ((VALUE) / 100)

/****************************************
 *  Temperature
****************************************/
#define TEMPERATURE_TO_C(VALUE) ((INT16S)(VALUE) - 50)
#define TEMPERATURE_FROM_C(VALUE) ((VALUE) + 50)

#define TEMP_TO_40_OFFSET(VALUE)  temp_to_40_offset(VALUE)
#define TEMP_4_DISPLAY(VALUE) low_temp_for_display(VALUE)

#define TEMP_C_FROM_40_OFFSET(VALUE)   ((INT16S)(VALUE) - 40)
#define TEMP_C_TO_40_OFFSET(VALUE) ((VALUE) + 40)
/****************************************
 *  Normal
****************************************/
#define GAIN(VALUE,TIMES) ((INT32U)(VALUE) * (TIMES))
#define OFF_SET(VALUE,OFFSET) ((INT32U)(VALUE) - (OFFSET))
#define DIVISION(DIVIDEND, DIVISOR) (((INT32U)(DIVIDEND) + (DIVISOR) / 2) / (DIVISOR))
#define DIVISION_F(DIVIDEND, DIVISOR) (((double)(DIVIDEND) + (DIVISOR) / 2) / (DIVISOR))

INT8U temp_to_40_offset(INT8U temp);
INT8U low_temp_for_display(INT8U value);
INT16U low_voltage_for_display(INT16U value);

/****************************************
 *  Time
****************************************/
#define MS_TO_HOUR(VALUE)   (INT32U)(VALUE)/3600000
#define MS_FROM_HOUR(VALUE) ((INT32U)(VALUE) * 3600000)

#define S_TO_HOUR(VALUE)   (INT32U)(VALUE)/3600
#define S_FROM_HOUR(VALUE) ((INT32U)(VALUE) * 3600)

#define MS_TO_S(VALUE)      ((INT32U)(VALUE) / 1000)
#define MS_FROM_S(VALUE)    ((INT32U)(VALUE) * 1000)

#endif /* BMS_UNIT_H_ */
