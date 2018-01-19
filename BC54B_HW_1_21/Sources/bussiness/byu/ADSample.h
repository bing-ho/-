/**
*
* Copyright (c) 2011 Ligoo Inc. 
* 
* @file     ADSample.h
* @brief    ��ز���ͷ�ļ�
* @note 
* @version 1.00
* @author   ��־��
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
#define LTC6803_INIT_CNT         3              //ltc��ʼ����
#define VOL_REORGANIZE_DATA_LEN  6            //��ѹ�������鳤��
#define LTC_COM_ERR_CNT          config_get(kCommFstAlarmDlyIndex) * 10           //ͨ�ż���
#define T_FILTER_CNT             3           //�¶��˲�����
#define V_FILTER_CNT             20           //��ѹ�˲�����
#define OPEN_STOWAD_VOL_CHANGE   645         //200mv ��������Դʱǰ���ѹ��ֵ
#define CHANGED_VOL              30          //��ѹ�쳣�迪������������ǰ��ɼ���ѹ��ֵ30mv
#define FIRST_BAT_MIN_VOL        778     //400mv
#define LTC_TEMP_CHANGE_MIN      4
#define LTC_TEMP_CHANGE_MAX      10
#define NEGATIVE_CRITICAL_POINT  0x8000       //��ֵ�ٽ��
#define TEMP_NEGATIVE_POINT      40       //��ֵ�ٽ��
typedef struct 
{
   INT8U   vfiltercnt[MAX_LTC6803_NUM][LTC_CELLV_NUM];  //��ѹ�˲�����
   INT16U  cellA[MAX_LTC6803_NUM][LTC_CELLV_NUM];       //��һ�ο�������Դ�õ���ѹ����
   INT16U  cellB[MAX_LTC6803_NUM][LTC_CELLV_NUM];       //�ڶ��ο�������Դ�õ���ѹ����
   INT16U  cellVBuf[MAX_LTC6803_NUM][LTC_CELLV_NUM];    //���յ�ѹ����
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


