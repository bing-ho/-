/**
*
* Copyright (c) 2011 Ligoo Inc. 
* 
* @file     bms_insulation_impl.h
* @brief    电池分析,及参数更新,控制 头文件
* @note 
* @version 1.00
* @author 
* @date 2011/11/30 
* 
*/
#ifndef BMSMAIN_H
#define BMSMAIN_H

#include "bms_config.h"
//#include "ads1015.h"
#include "ads1015_intermediate.h"
#include "adc.h"
#include "bms_insulation.h"
#include "bms_util.h"

/*变量类型定义*/

typedef struct{
    INT16U positive_res;  //正极绝缘电阻
    INT16U negative_res;  //负极绝缘电阻
    INT16U insu_sys_res;
    INT16U positive_res_diff; //正极绝缘阻值最大差值
    INT16U negative_res_diff; //负极绝缘阻值最大差值
    INT8U  insu_status; //工作状态
    INT8U  is_precharging; //预充电状态
    INT8U  is_charging;  //正在充电
    INT16U total_volt;//系统总压
    INT16U pre_volt;
    INT16U pch_volt;
    INT16U bat_volt;
    INT16U hv3_volt;
    INT16U ext_positive_res;  //正极绝缘电阻
    INT16U ext_negative_res;  //负极绝缘电阻
    INT16U ext_insu_sys_res;
    INT8U  ext_insu_status; //工作状态
    INT8U test_mode;//实时检测阻值标志
}InsulationInfo;

typedef enum
{
    kTVRange400V = 1,
    kTVRange1000V,
    kTVRange72V,
}TVRange;


#define POWER_ON_CALC_RES_DELAY     100
#define MOS_ON_DELAY                3

#define CD74HC4052_ON_DELAY         2

#define AD_REF_VOLT                 5000
#define AD_TO_VOLT_FACTOR           4096

#define EXTEND_TV_AD_CNT            10//20 高压检测的滤波次数由20改为10

#define AD_CNT                      30//AD采样次数

#define AD_CHN_P                    7
#define AD_CHN_N                    14
#define AD_CHN_Z                    8

#define INSULATION_SWITCH_ON_PRE_CFG    5
#define INSULATION_SWITCH_ON_PCH_CFG    4
#define INSULATION_SWITCH_ON_BHV_CFG    6

#define ALARM_RES_BUFF_NUM          5//10
#define RES_FILTER_BUFF_NUM         3

#define TV_FACTOR_CHG_THRESHOLD     1900//1960 TODO:数值保守（偏小），主要考虑400V总压时的误差
#define TV_HIGHER_THAN_400_FLAG     1
#define TV_LOWER_THAN_400_FLAG      0

#define POS_FEN_YA_RES_IN_400V      200//正极分压电阻,单位kohm
#define POS_CAI_YANG_RES_IN_400V    0.1//正极采样电阻,单位kohm
#define POS_FEN_YA_RES_IN_1000V     1000//正极分压电阻,单位kohm
#define POS_CAI_YANG_RES_IN_1000V   0.34//正极采样电阻,单位kohm
#define POS_FEN_YA_RES_IN_72V       25//正极分压电阻,单位kohm
#define POS_CAI_YANG_RES_IN_72V     0.05//正极采样电阻,单位kohm

#define TV_RANGE_CALC_FACTOR        1.015

#define TOTAL_VOLT_MIN_DEFAULT      50
#define TOTAL_VOLT_MIN_TEST_MODE      360



#define RES_GAIN_FACTOR             19.6 //阻值电压放大倍数

/*Pre volt*/
#define PRE_TV_FULL_VALUE           10
#define PRE_TV_GAIN_FACTOR          16.08
#define PCH_TV_GAIN_FACTOR          16.08
#define BAT_TV_GAIN_FACTOR          16.08

#define SYS_SHORT_CIRCUIT_RES           10  //系统短路阻值,单位0.1kohm

#define SYS_LEAK_RES_MIN_IN_72V         5   //系统最小漏电阻值,单位kohm
#define SYS_LEAK_RES_MIN_IN_400V        10   //系统最小漏电阻值,单位kohm
#define SYS_LEAK_RES_MIN_IN_1000V       25   //系统最小漏电阻值,单位kohm

#define PRE_TV_CAI_YANG_RES_IN_72V      0.2
#define PRE_TV_FEN_YA_RES_IN_400V       200
#define PRE_TV_FEN_YA_RES_IN_1000V      2000

#define PRE_TV_FEN_YA_RES_IN_72V        200
#define PRE_TV_CAI_YANG_RES_IN_400V     0.1
#define PRE_TV_CAI_YANG_RES_IN_1000V    0.47

#if BMS_SUPPORT_BY5248D == 1
  #if BMS_SUPPORT_BY5248D_1_1A
    #define PRE_TV_CAI_YANG_RES_COMMON      10
    #define PRE_TV_FEN_YA_RES_COMMON        511
    #define PCH_TV_CAI_YANG_RES_COMMON      10
    #define PCH_TV_FEN_YA_RES_COMMON        511
    #define BAT_TV_CAI_YANG_RES_COMMON      10
    #define BAT_TV_FEN_YA_RES_COMMON        511
    #define HV3_TV_CAI_YANG_RES_COMMON      10
    #define HV3_TV_FEN_YA_RES_COMMON        511
  #else  
    #define PRE_TV_CAI_YANG_RES_COMMON      10
    #define PRE_TV_FEN_YA_RES_COMMON        200
    #define PCH_TV_CAI_YANG_RES_COMMON      10
    #define PCH_TV_FEN_YA_RES_COMMON        200
    #define BAT_TV_CAI_YANG_RES_COMMON      10
    #define BAT_TV_FEN_YA_RES_COMMON        200
    #define HV3_TV_CAI_YANG_RES_COMMON      10
    #define HV3_TV_FEN_YA_RES_COMMON        200
  #endif
#else 
  #define PRE_TV_CAI_YANG_RES_COMMON      10
  #define PRE_TV_FEN_YA_RES_COMMON        510
  #define PCH_TV_CAI_YANG_RES_COMMON      10
  #define PCH_TV_FEN_YA_RES_COMMON        510
  #define BAT_TV_CAI_YANG_RES_COMMON      10
  #define BAT_TV_FEN_YA_RES_COMMON        510
  #define HV3_TV_CAI_YANG_RES_COMMON      10
  #define HV3_TV_FEN_YA_RES_COMMON        510
#endif

/*Pch volt*/
#define PCH_TV_FULL_VALUE           10
#define PCH_TV_FEN_YA_RES_IN_72V        200
#define PCH_TV_FEN_YA_RES_IN_400V       200
#define PCH_TV_FEN_YA_RES_IN_1000V      2000

#define PCH_TV_CAI_YANG_RES_IN_72V      0.2
#define PCH_TV_CAI_YANG_RES_IN_400V     0.1
#define PCH_TV_CAI_YANG_RES_IN_1000V    0.47

/* Delay */
#define RES_CALC_CYCLE_FAST             50
#define RES_CALC_CYCLE_SLOW             10000//60000


//extern ISO_DATA g_isoInfo;
#define HV3_TV_OPTOCOUPLER_RES_COMMON   0.5     //1.23中光耦等效电阻0.5kohm






void reset_mos_status(void);//复位正负极开关



#endif

/***********END OF FILE***********/