/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:PassiveBalance.h
 **��������:2015.12.24
 **�ļ�˵��:��ر����������
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/

#ifndef   _PASSIVE_BALANCE_H_ 
#define   _PASSIVE_BALANCE_H_

#include "includes.h"
#include "bms_config.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

#define MULTI_MAX_VOLT_NUM      BYU_BAL_NUM_MAX_DEF     // ��ߵ����ѹͳ�Ƹ���

//��ʱ
#define BALANCE_DLAY_2S  2000//2S
#define BALANCE_DLAY_3S  3000//5S 
#define BALANCE_DLAY_5S  5000//5S 
#define BALANCE_DLAY_10S 10000//10S 
#define BALANCE_DLAY_30S 30000//30S
#define BALANCE_DLAY_1MIN 60000//1min

typedef enum
{
	kPassiveBalanceVoltDiffNormal = 0, //ѹ������
	kPassiveBalanceVoltDiffBig = 1, //ѹ�����Ҫ����
} BalanceVoltDiffStatus; //ѹ��״̬

typedef struct
{
	INT8U pos; //��ص�λ��
	INT16U volt; //��ص�ѹ
} BalanceItem; //�����ѹ�ṹ��

typedef struct
{
	BalanceVoltDiffStatus max_volt_diff_status; //ѹ��״̬
	INT16U status_dly; //��ʱ
	BalanceItem volt_item[PARAM_BSU_NUM_MAX][MULTI_MAX_VOLT_NUM]; //��߼�·��ѹ��λ��
	INT16U balance_control_word[PARAM_BSU_NUM_MAX]; //��������ֽ�
} BsuPassiveBalanceContext;

void clr_bsu_passive_balance_multi_volt_item(void);
void bsu_statistic_multi_max_volt(INT8U bsu_index, INT8U pos, INT16U volt);
void bsu_passive_balance_ctrl(void);
INT16U get_bsu_passive_balance_word(INT8U bsu_index);
INT16U get_bsu_max_cell_volt(INT8U bsu_index);
INT8U get_bsu_max_cell_volt_pos(INT8U bsu_index);

#if   MULTI_MAX_VOLT_NUM <  1
#error "MULTI_MAX_VOLT_NUM �������1"
#endif
#endif
#endif
