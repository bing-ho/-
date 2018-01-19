/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:ActiveBalance.h
 **��������:2017.04.27
 **�ļ�˵��:��������������
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/

#ifndef   _ACTIVE_BALANCE_H_ 
#define   _ACTIVE_BALANCE_H_

#include "includes.h"
#include "bms_config.h"
#include "bsu_passive_balance.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1


//12V  9V~14.4V
//24V  18~28.8V
typedef enum
{
    kSystemPowerVolt_Unknow = 0,
    kSystemPowerVolt_12V,
    kSystemPowerVolt_24V
}SystemPowerVoltType;//ϵͳ�����ѹ����(12V or 24V)

typedef enum
{
    kPowerVolt_Error = 0,//����
    kPowerVolt_Normal, //����
    kPowerVolt_OverChg, //����
    kPowerVolt_OverDisChg,//����
    kPowerVolt_Charging, //����
}SystemPowerVoltStatus;


typedef enum
{
	kActiveBalanceVoltDiffNormal = 0, //ѹ������
	kActiveBalanceVoltDiffBig = 1, //ѹ�����Ҫ����
} ActiveBalanceVoltDiffStatus; //ѹ��״̬

typedef union {
  byte pos_byte;
  struct {
    byte pos                :7;                                       
    byte is_max_volt_diff   :1; //�Ƿ�����ߵ�ѹλ��                                      
  } pos_bits;
} VoltDiffPos; 

typedef struct
{
	VoltDiffPos pos; //��ص�λ��
	INT8U  bsu_index;
	INT16U volt_diff; //��ص�ѹ
}VoltDiffItem; //�����ѹ�ṹ��
#define VOLT_DIFF_SORT_NUM    (PARAM_BSU_NUM_MAX/2)
typedef struct
{
	ActiveBalanceVoltDiffStatus volt_diff_status; //ѹ��״̬
	INT16U status_dly; //��ʱ
	BalanceItem min_volt_item[PARAM_BSU_NUM_MAX]; //��ѹ��λ��
	INT16U balance_control_word[PARAM_BSU_NUM_MAX]; //��������ֽ�, ���λ����������� 0-������ 1-�ŵ����
	INT8U balance_current[PARAM_BSU_NUM_MAX];
	VoltDiffItem volt_diff_item[VOLT_DIFF_SORT_NUM];
	INT32U balance_error;
} BsuActiveBalanceContext;

#define DSP_IIC_ADDR           0x50


#define BMS_12V_SYSTEM_VOLT_MIN       10800
#define BMS_12V_SYSTEM_VOLT_MID       13000
#define BMS_12V_SYSTEM_VOLT_MAX       14000
#define BMS_24V_SYSTEM_VOLT_MIN       21000
#define BMS_24V_SYSTEM_VOLT_MID       26000
#define BMS_24V_SYSTEM_VOLT_MAX       28000



#define ACTIVE_BALANCE_0_5A   1//0.5A
#define ACTIVE_BALANCE_1_0A   2//1A
#define ACTIVE_BALANCE_1_5A   3//1.5A
#define ACTIVE_BALANCE_2_0A   4//2A
#define ACTIVE_BALANCE_2_5A   5//2.5A
#define ACTIVE_BALANCE_3_0A   6//3A

#define ACTIVE_BALANCE_CURRENT  ACTIVE_BALANCE_2_0A

void clr_balance_min_volt_item(void);
void bsu_statistic_min_volt(INT8U bsu_index, INT8U pos, INT16U volt);
void bsu_active_balance_ctrl(void);
INT16U get_bsu_active_balance_word(INT8U bsu_index);
void send_active_balance_cmd_to_ltc6804(INT8U bsu_num);
void read_active_balance_current(INT8U bsu_num);
INT8U get_bsu_active_balance_current(INT8U bsu_num);
INT8U is_active_balance_error(INT8U bsu_index);
#endif
#endif
