/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_current_impl.h
* @brief
* @note
* @author
* @date 2012-5-9
*
*/

#ifndef BMS_CURRENT_IMPL_H_
#define BMS_CURRENT_IMPL_H_
#include "bms_current.h"

#define USE_TBL_INSTEAD_OF_MACRO 1


#define CURRENT_REF_VOLTAGE_BEFORE_V120_5000mV      5000
#define CURRENT_REF_VOLTAGE_V120_8096mV             8096     //power supply +/-4000mV + 4096mV

/** ADC Power Supply Volt */
#define CURRENT_SAMPLE_ADC_SUPPLY_VOLT_CNT      10

/** ADC timeout */
#define CURRENT_SAMPLE_ADC_TIMEOUT              50

#define CURRENT_SUPPLY_VOLT_DEFAULT             5000//mv
#define CURRENT_SUPPLY_VOLT_MIN                 1500
#define CURRENT_SUPPLY_VOLT_MAX                 5500
#define CURRENT_SUPPLY_VOLT_IS_VALID(VOLT)  (VOLT >= CURRENT_SUPPLY_VOLT_MIN && VOLT <= CURRENT_SUPPLY_VOLT_MAX)

#define CURRENT_REFERENCE_VOLT                  5000//mv
#define CURRENT_DOUBLE_SENSOR_OUTPUT_VOLT_MIN   450//200L //
#define CURRENT_DOUBLE_SENSOR_OUTPUT_VOLT_MAX   4092//4850 //
#define CURRENT_SINGLE_SENSOR_OUTPUT_VOLT_MIN   40 //电流传感器输出最小电压
#define CURRENT_SINGLE_SENSOR_OUTPUT_VOLT_MAX   3600//4000 电流传感器输出最大电压
//适合1.20版本的
#define CURRENT_DOUBLE_SENSOR_OUTPUT_VOLT_MIN_1   180//RENT_DOUBLE_SENSOR_OUTPUT_VOLT_MIN*10+5)/11) //200//电流传感器输出最小电压 mv
//#define CURRENT_DOUBLE_SENSOR_OUTPUT_VOLT_MAX_1   ((CURRENT_DOUBLE_SENSOR_OUTPUT_VOLT_MAX+4096)/2.26)//4850 //电流传感器输出最大电压
//#define CURRENT_SINGLE_SENSOR_OUTPUT_VOLT_MIN_1   ((CURRENT_SINGLE_SENSOR_OUTPUT_VOLT_MIN+4096)/2.26) //500//电流传感器输出最小电压
//#define CURRENT_SINGLE_SENSOR_OUTPUT_VOLT_MAX_1   ((CURRENT_SINGLE_SENSOR_OUTPUT_VOLT_MAX+4096)/2.26)//4500 //电流传感器输出最大电压
#define CURRENT_ALLOW_OVER_PERCENT              0.05  // 5%

#define CURRENT_NORMAL_TEMPERATURE              TEMPERATURE_FROM_C(25)

#define FEI_XUAN_VOLT_TO_CURRENT_IN_100MA(VOLT, REF_VOLT, RANGE) \
        ((float)(VOLT - REF_VOLT / 2) * RANGE / 200.0)
#if !USE_TBL_INSTEAD_OF_MACRO        
#define CURRENT_GET_LOW_OFFSET_VOLT(RANGE)  CURRENT_GET_LOW_OFFSET_VOLT_EX(RANGE)
#define CURRENT_GET_LOW_OFFSET_VOLT_EX(RANGE) CURRENT_##RANGE##A_LOW_OFFSET_VOLT  

#define CURRENT_GET_LOW_SENSITIVITY(RANGE)  CURRENT_GET_LOW_SENSITIVITY_EX(RANGE)
#define CURRENT_GET_LOW_SENSITIVITY_EX(RANGE) CURRENT_##RANGE##A_LOW_SENSITIVITY

#define CURRENT_GET_HIGH_OFFSET_VOLT(RANGE)  CURRENT_GET_HIGH_OFFSET_VOLT_EX(RANGE)
#define CURRENT_GET_HIGH_OFFSET_VOLT_EX(RANGE) CURRENT_##RANGE##A_HIGH_OFFSET_VOLT  
  
#define CURRENT_GET_HIGH_SENSITIVITY(RANGE)  CURRENT_GET_HIGH_SENSITIVITY_EX(RANGE)
#define CURRENT_GET_HIGH_SENSITIVITY_EX(RANGE) CURRENT_##RANGE##A_HIGH_SENSITIVITY

#define CURRENT_GET_LOW_RANGE_MIN(RANGE)  CURRENT_GET_LOW_RANGE_MIN_EX(RANGE)
#define CURRENT_GET_LOW_RANGE_MIN_EX(RANGE) CURRENT_##RANGE##A_LOW_RANGE_MIN

#define CURRENT_GET_LOW_RANGE_MAX(RANGE)  CURRENT_GET_LOW_RANGE_MAX_EX(RANGE)
#define CURRENT_GET_LOW_RANGE_MAX_EX(RANGE) CURRENT_##RANGE##A_LOW_RANGE_MAX

#define CURRENT_GET_HIGH_RANGE_MIN(RANGE)  CURRENT_GET_HIGH_RANGE_MIN_EX(RANGE)
#define CURRENT_GET_HIGH_RANGE_MIN_EX(RANGE) CURRENT_##RANGE##A_HIGH_RANGE_MIN

#define CURRENT_GET_HIGH_RANGE_MAX(RANGE)  CURRENT_GET_HIGH_RANGE_MAX_EX(RANGE)
#define CURRENT_GET_HIGH_RANGE_MAX_EX(RANGE) CURRENT_##RANGE##A_HIGH_RANGE_MAX
#endif

#define CURRENT_150A_LOW_OFFSET_VOLT     2500 //mv
#define CURRENT_150A_LOW_SENSITIVITY     667  //0.1mv/A
#define CURRENT_150A_HIGH_OFFSET_VOLT    3300 //mv
#define CURRENT_150A_HIGH_SENSITIVITY    80   //0.1mv/A
#define CURRENT_150A_LOW_RANGE_MIN       -30
#define CURRENT_150A_LOW_RANGE_MAX       30
#define CURRENT_150A_HIGH_RANGE_MIN      -350
#define CURRENT_150A_HIGH_RANGE_MAX      150

#define CURRENT_350A_LOW_OFFSET_VOLT     2500 //mv
#define CURRENT_350A_LOW_SENSITIVITY     667  //0.1mv/A
#define CURRENT_350A_HIGH_OFFSET_VOLT    2500 //mv
#define CURRENT_350A_HIGH_SENSITIVITY    57   //0.1mv/A
#define CURRENT_350A_LOW_RANGE_MIN       -30
#define CURRENT_350A_LOW_RANGE_MAX       30
#define CURRENT_350A_HIGH_RANGE_MIN      -350
#define CURRENT_350A_HIGH_RANGE_MAX      350

#define CURRENT_600A_LOW_OFFSET_VOLT     2500 //mv
#define CURRENT_600A_LOW_SENSITIVITY     333  //0.1mv/A
#define CURRENT_600A_HIGH_OFFSET_VOLT    2500 //mv
#define CURRENT_600A_HIGH_SENSITIVITY    33   //0.1mv/A
#define CURRENT_600A_LOW_RANGE_MIN       -60
#define CURRENT_600A_LOW_RANGE_MAX       60
#define CURRENT_600A_HIGH_RANGE_MIN      -600
#define CURRENT_600A_HIGH_RANGE_MAX      600

#define CURRENT_500A_LOW_OFFSET_VOLT     2500 //mv
#define CURRENT_500A_LOW_SENSITIVITY     267  //0.1mv/A
#define CURRENT_500A_HIGH_OFFSET_VOLT    2500 //mv
#define CURRENT_500A_HIGH_SENSITIVITY    40   //0.1mv/A
#define CURRENT_500A_LOW_RANGE_MIN       -75
#define CURRENT_500A_LOW_RANGE_MAX       75
#define CURRENT_500A_HIGH_RANGE_MIN      -500
#define CURRENT_500A_HIGH_RANGE_MAX      500

#define CURRENT_200A_LOW_OFFSET_VOLT     2500 //mv
#define CURRENT_200A_LOW_SENSITIVITY     800  //0.1mv/A
#define CURRENT_200A_HIGH_OFFSET_VOLT    2500 //mv
#define CURRENT_200A_HIGH_SENSITIVITY    100  //0.1mv/A
#define CURRENT_200A_LOW_RANGE_MIN       -25
#define CURRENT_200A_LOW_RANGE_MAX       25
#define CURRENT_200A_HIGH_RANGE_MIN      -200
#define CURRENT_200A_HIGH_RANGE_MAX      200

#define CURRENT_START_CHECK_TO_ZERO_LOW_CURRENT 20  // 2A
#define CURRENT_CHECK_TO_ZERO_DELAY      5000    // 5s
#define CURRENT_CHECK_TO_ZERO_AVR_NUM    5    // 平均次数
#define CURRENT_CHECK_TO_ZERO_CHARGE_CUR_MIN    100     // 充电最小电流10A
#define CURRENT_CHECK_TO_ZERO_CHARGE_TIME       10      // 充电时间10min

#if !USE_TBL_INSTEAD_OF_MACRO  
#define CURRENT_PARA_GET_DISPATCH(TYPE, VALUE, FUN)   \
    if(TYPE == kCurrent150Type) VALUE = FUN(150);\
    else if(TYPE == kCurrent200Type) VALUE = FUN(200);\
    else if(TYPE == kCurrent350Type) VALUE = FUN(350);\
    else if(TYPE == kCurrent500Type) VALUE = FUN(500);\
    else if(TYPE == kCurrent600Type) VALUE = FUN(600);
#endif

/** ADC SWITCH */
#define CURRENT_SW_A0_DIR_REG       DDRD_DDRD0
#define CURRENT_SW_A0_DAT_REG       PORTD_PD0
#define CURRENT_SW_A1_DIR_REG       DDRD_DDRD1
#define CURRENT_SW_A1_DAT_REG       PORTD_PD1

#define CURRENT_SW_ON_LOW_CUR()     CURRENT_SW_A0_DIR_REG = 1; CURRENT_SW_A0_DAT_REG = 1; 
#define CURRENT_SW_ON_HIGH_CUR()    CURRENT_SW_A0_DIR_REG = 1; CURRENT_SW_A0_DAT_REG = 0; 
#define CURRENT_SW_ON_POWER()  

#define CURRENT_SAMPLE_AD_PORT1      2     
#define CURRENT_SAMPLE_AD_PORT2      3

#define CURRENT_NORMAL      0x00
#define CURRENT_NO_CONNECT  0x01
#define CURRENT_OVER_RANGE  0x02

/** TYPE DECLEAR */
typedef struct
{
    INT32U adc_data0;
    INT32U adc_data1;
    OS_EVENT* event;
    INT16S* _PAGED low_buffer;
    INT16S* _PAGED high_buffer;
    INT16S* _PAGED single_buffer;
    INT8U  sample_count;
    INT16U sample_peroid;
    INT8U  min_sample_count;
    INT8U  filter_num;
    INT16U current_senor_range; // 量程
    INT16U current_senor_range1; // 量程
    CURRENT_RANGE_TYPE range_type;//单量程or双量程
    INT8U signal_invalid_flag; //传感器信号无效状态
    INT8U signal_invalid_flag1; //传感器信号无效状态
    Result (*get_current_type_func)(Current_platform platform);
    Result (*current_auto_check_func)(void);
    float (*CurrentCalibration_func)(float current);
}CurrentSampleContext;

/** FUNCTION DECLEAR */

#pragma INLINE
void current_sample_adc_init(void);

#pragma INLINE
INT16U current_sample_average_filter(INT16S* _PAGED voltages,INT8U num);
Result get_current_range_type_double(Current_platform platform);
Result get_current_range_type_single(Current_platform platform);
Result get_current_range_type_twosingle(Current_platform platform); 

Result current_sample_auto_check_single(void); 
Result current_sample_auto_check_double(void); 
Result current_sample_auto_check_twosingle(void); 

float CurrentCalibration_double(float current);
float CurrentCalibration_single(float current); 
#endif /* BMS_CURRENT_IMPL_H_ */
