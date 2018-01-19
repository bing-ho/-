/**
*
* Copyright (c) 2012 Ligoo Inc. 
* 
* @file     BatBalance.h
* @brief    电池均衡头文件 
* @note 
* @version 1.00
* @author   曹志勇
* @date 2012/10/6 
* 
*/
#ifndef BAT_BALANCE_H
#define BAT_BALANCE_H

#include "LTC6803.h"
#include "ADSample.h"
#if BMS_SUPPORT_HARDWARE_LTC6803 == 1 
#define BL_OFF_VOLT            2700                   //均衡开启最小电压
#define MAX_BLANCE_BUF_LEN     2 
#define MIN_INIT_VOL           6000

#define LTC_HALF_VOL_NUM                  6
#define LTC_AFTER_HALF_VOL_INIT_NO        6

#define PASSIVE_BALANCE_ON_TIME     95 //+5 为真实开启时间
#define PASSIVE_BALANCE_OFF_TIME    80 //+20为真实关闭时间

#define PASSIVE_BALANCE_TYPE    1  //被动均衡
#define INITIATIVE_BALANCE_TYPE 2  //主动均衡
#define BALANCE_TYPE_NUM 2

typedef struct
{
    INT16U   before_six_low_volt[MAX_LTC6803_NUM];
    INT8U    before_six_low_num[MAX_LTC6803_NUM];
    INT16U   after_six_low_volt[MAX_LTC6803_NUM];       
    INT8U    after_six_low_num[MAX_LTC6803_NUM];
    INT16U   before_six_high_volt[MAX_LTC6803_NUM];
    INT8U    before_six_high_num[MAX_LTC6803_NUM];
    INT16U   after_six_high_volt[MAX_LTC6803_NUM];       
    INT8U    after_six_high_num[MAX_LTC6803_NUM];
}BALANCE_VOL_INFO;

typedef  struct  
{ 	   	
    INT8U           open_dly;                                    
    INT8U           close_dly; 
    INT8U           flag;     //0代表关，1代表开                     
}BalancePower;

extern void BatBalanceInit(void );
extern void ControlLtcIO(LTC_REG_GROP * _PAGED ltcRegGrop);


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BYU_SAMPLE
extern BalancePower blance_power;
#pragma DATA_SEG DEFAULT

#if  MAX_BLANCE_BUF_LEN !=2
#error "MAX_BLANCE_BUF_LEN must ==2"
#endif 
#if  MIN_INIT_VOL < 5000
#error "MIN_INIT_VOL must >5000"
#endif
#endif 
#endif

/***********END OF FILE***********/