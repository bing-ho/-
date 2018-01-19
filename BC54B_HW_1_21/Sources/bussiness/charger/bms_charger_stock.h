/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_charger_stock.h
* @brief
* @note
* @author
* @date 2015-6-3
*
*/
#ifndef BMS_CHARGER_STOCK_H_
#define BMS_CHARGER_STOCK_H_

#include "includes.h"
#include "bms_charger.h"

/***********************************************************************************
** �������������Ʋ���
***********************************************************************************/
/*
ע��������ʹ�ܺ󵱽�����������CHARGER_STOCK_REDUCE_CUR_NUMʱ��
����Ĭ��CHARGER_STOCK_FULL_CHARGE_VOLT_DEFAULT��CHARGER_STOCK_REDUCE_CUR_DEFAULT
*/
#define CHARGER_STOCK_USE_TRICKLE_CUR_EN    0 //������ʹ��
#define SLOW_CHARGER_STOCK_TRICKLE_EN       1 //���併������ʹ��


typedef enum
{
    kReduceTypePercent,// 1%/bit ������ǰ�����İٷֱ�
    kReduceTypeCurrent // 0.1A ��������ֵ
}ReduceCurType;

typedef struct
{
    ReduceCurType type;
    INT16U value;
}ChgCurCtlCxt;

#define CHARGER_STOCK_CURRENT_RESET_AUTO_EN     0  //�����ͷź��Զ��ָ�������ʹ��
#define CHARGER_STOCK_TRICKLE_CURRENT           60 //0.1A/bit ���������
#define CHARGER_STOCK_FULL_CHARGE_VOLT_DEFAULT  3650 //mv/bit ����ȫ�ٳ���ѹ����
#define CHARGER_STOCK_TRICKLE_CHARGE_TIME       5000 //ms/bit ������ʱ��
#define CHARGER_STOCK_CURRENT_MIN               60 //0.1A/bit ������������С��Ч����
#define CHARGER_STOCK_REDUCE_CUR_DEFAULT        (current / 2) //���μ��ٵĵ���ֵ

void charger_stock_current_reset(void);
void charger_stock_current_update(void* pdata);

/***********************************************************************************
** ������繦��
***********************************************************************************/

INT16U charger_curr_in_temper(void);
INT8U ems_chg_cur_decrease_is_50_percent(void);
INT8U ems_chg_cur_decrease_is_100_percent(void);

/***********************************************************************************
** �����ȿ�������
***********************************************************************************/

#define CHARGER_HEAT_RELAY_ON_INDEX         0 //����״̬
#define CHARGER_CHARGE_RELAY_ON_INDEX       1 //���״̬
#define CHARGER_HEAT_FAULT_BUFF_INDEX       2 //����״̬�쳣

#define CHARGER_HEAT_LOW_TEMPERATURE        -5 //���¼�����ֵ
#define CHARGER_HEAT_NORMAL_TEMPERATURE     0 //�����¶���ֵ
#define CHARGER_HEAT_TEMPERATURE_MAX        5 //�������¶�

#define CHARGER_HEAT_NORMAL_VOLTAGE         3680 //0.1V �������ȵ�ѹ

#define CHARGER_HEAT_CURRENT_MIN            20 //0.1A ��С���ȵ���
#define CHARGER_HEAT_LT_CURRENT             40 //0.1A ���¼��ȵ���
#define CHARGER_HEAT_NORMAL_CURRENT         60 //0.1A �������ȵ���

#define CHARGER_HEATER_FAULT_TEMPERATURE    80 //�����������¶�
#define CHARGER_HEATER_NORMAL_TEMP_MAX      60 //�����������¶����ֵ
#define CHARGER_HEATER_STOP_HEAT_TEMP       75 //������ֹͣ�����¶�

#define CHARGER_LT_HEAT_ON_DELAY            1000 //1ms ���¼��ȼ̵����պ���ʱ
#define CHARGER_CHG_RELAY_OFF_DELAY_IN_HEAT 10000 // 1ms ���¼���״̬���̵����Ͽ���ʱ

typedef enum
{
    kChgHeatNone=0,
    kChgHeatInit, //���ȳ�ʼ��
    kChgHeatSelect, //��ʼ�������ѡ��
    kChgHeatLTHeat, //���¼���,<-5
    kChgHeatNormalHeat,//��������,>0
    kChgHeatNormalChg,//�������
    kChgHeatChgFinish, //������
    kChgHeatFault //���ȹ���
}ChgHeatState;

ChgHeatState charger_get_heat_state(void);
void charger_heat_state_update(void);
void charger_heat_state_reset(void);
void charger_heat_init_state_process(void);
void charger_heat_select_state_process(void);
void charger_heat_lt_heat_state_process(void);
void charger_heat_normal_heat_state_process(void);
void charger_heat_normal_charge_state_process(void);
void charger_heat_charge_finish_state_process(void);
void charger_heat_Fault_state_process(void);
INT8U charger_check_heat_relay_fault(void);

#endif