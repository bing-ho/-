/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:ActiveBalance.h
 **创建日期:2017.04.27
 **文件说明:电池主动均衡控制
 **修改记录:
 **版    本:V1.0
 **备    注:
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
}SystemPowerVoltType;//系统供电电压类型(12V or 24V)

typedef enum
{
    kPowerVolt_Error = 0,//故障
    kPowerVolt_Normal, //正常
    kPowerVolt_OverChg, //过充
    kPowerVolt_OverDisChg,//过放
    kPowerVolt_Charging, //浮充
}SystemPowerVoltStatus;


typedef enum
{
	kActiveBalanceVoltDiffNormal = 0, //压差正常
	kActiveBalanceVoltDiffBig = 1, //压差大需要均衡
} ActiveBalanceVoltDiffStatus; //压差状态

typedef union {
  byte pos_byte;
  struct {
    byte pos                :7;                                       
    byte is_max_volt_diff   :1; //是否是最高电压位置                                      
  } pos_bits;
} VoltDiffPos; 

typedef struct
{
	VoltDiffPos pos; //电池的位置
	INT8U  bsu_index;
	INT16U volt_diff; //电池电压
}VoltDiffItem; //均衡电压结构体
#define VOLT_DIFF_SORT_NUM    (PARAM_BSU_NUM_MAX/2)
typedef struct
{
	ActiveBalanceVoltDiffStatus volt_diff_status; //压差状态
	INT16U status_dly; //延时
	BalanceItem min_volt_item[PARAM_BSU_NUM_MAX]; //电压及位置
	INT16U balance_control_word[PARAM_BSU_NUM_MAX]; //均衡控制字节, 最高位代表均衡类型 0-充电均衡 1-放电均衡
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
