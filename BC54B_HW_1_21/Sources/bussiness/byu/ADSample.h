/**
*
* Copyright (c) 2011 Ligoo Inc. 
* 
* @file     ADSample.h
* @brief    电池采样头文件
* @note 
* @version 1.00
* @author   曹志勇
* @date 2012/10/6 
* 
*/
#ifndef BMSMAIN_H
#define BMSMAIN_H

#include "includes.h"
#include "bms_byu.h"
#include "LTC6803.h"
#if BMS_SUPPORT_HARDWARE_LTC6803 == 1 
#define LTC_COMERR_CNT_SUPPORT   1 

#define CLR_LTC6803_REG_PERIOD   10
#define LTC6803_INIT_CNT         3              //ltc初始化数
#define VOL_REORGANIZE_DATA_LEN  6            //电压数据重组长度
#define LTC_COM_ERR_CNT          config_get(kCommFstAlarmDlyIndex) * 10           //通信计数
#define T_FILTER_CNT             3           //温度滤波计数
#define V_FILTER_CNT             20           //电压滤波计数
#define OPEN_STOWAD_VOL_CHANGE   645         //200mv 开启恒流源时前后电压差值
#define CHANGED_VOL              30          //电压异常需开启排线脱落检测前后采集电压差值30mv
#define FIRST_BAT_MIN_VOL        778     //400mv
#define LTC_TEMP_CHANGE_MIN      4
#define LTC_TEMP_CHANGE_MAX      10
#define NEGATIVE_CRITICAL_POINT  0x8000       //负值临界点
#define TEMP_NEGATIVE_POINT      40       //负值临界点
typedef struct 
{
   INT8U   vfiltercnt[MAX_LTC6803_NUM][LTC_CELLV_NUM];  //电压滤波计数
   INT16U  cellA[MAX_LTC6803_NUM][LTC_CELLV_NUM];       //第一次开启恒流源得到电压缓存
   INT16U  cellB[MAX_LTC6803_NUM][LTC_CELLV_NUM];       //第二次开启恒流源得到电压缓存
   INT16U  cellVBuf[MAX_LTC6803_NUM][LTC_CELLV_NUM];    //最终电压缓存
}VOL_BUF_GROP;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BYU_SAMPLE
extern BMS_BYU_CXT     g_byuCxt;
#if LTC_COMERR_CNT_SUPPORT
    extern INT8U g_ltcComErrorCnt[MAX_LTC6803_NUM];
#endif
extern  LTC_REG_GROP   g_ltcRegGrop; 
#pragma DATA_SEG DEFAULT

extern void ADSampleInit(void);
extern INT8U get_ltc_num(void);

extern OS_EVENT* g_LTCSPISem;


#if  LTC6803_INIT_CNT >255
#error "LTC6803_INIT_CNT must < 255"
#endif 
#if  VOL_REORGANIZE_DATA_LEN !=6
#error "VOL_REORGANIZE_DATA_LEN must == 6"
#endif 

#if  T_FILTER_CNT >255
#error "T_FILTER_CNT must < 255"
#endif 
#if  V_FILTER_CNT >255
#error "V_FILTER_CNT must < 255"
#endif 

#endif

#endif

/***********END OF FILE***********/


