/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:PassiveBalance.h
 **创建日期:2015.12.24
 **文件说明:电池被动均衡控制
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/

#ifndef   _PASSIVE_BALANCE_H_ 
#define   _PASSIVE_BALANCE_H_

#include "includes.h"
#include "bms_config.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

#define MULTI_MAX_VOLT_NUM      BYU_BAL_NUM_MAX_DEF     // 最高单体电压统计个数

//延时
#define BALANCE_DLAY_2S  2000//2S
#define BALANCE_DLAY_3S  3000//5S 
#define BALANCE_DLAY_5S  5000//5S 
#define BALANCE_DLAY_10S 10000//10S 
#define BALANCE_DLAY_30S 30000//30S
#define BALANCE_DLAY_1MIN 60000//1min

typedef enum
{
	kPassiveBalanceVoltDiffNormal = 0, //压差正常
	kPassiveBalanceVoltDiffBig = 1, //压差大需要均衡
} BalanceVoltDiffStatus; //压差状态

typedef struct
{
	INT8U pos; //电池的位置
	INT16U volt; //电池电压
} BalanceItem; //均衡电压结构体

typedef struct
{
	BalanceVoltDiffStatus max_volt_diff_status; //压差状态
	INT16U status_dly; //延时
	BalanceItem volt_item[PARAM_BSU_NUM_MAX][MULTI_MAX_VOLT_NUM]; //最高几路电压及位置
	INT16U balance_control_word[PARAM_BSU_NUM_MAX]; //均衡控制字节
} BsuPassiveBalanceContext;

void clr_bsu_passive_balance_multi_volt_item(void);
void bsu_statistic_multi_max_volt(INT8U bsu_index, INT8U pos, INT16U volt);
void bsu_passive_balance_ctrl(void);
INT16U get_bsu_passive_balance_word(INT8U bsu_index);
INT16U get_bsu_max_cell_volt(INT8U bsu_index);
INT8U get_bsu_max_cell_volt_pos(INT8U bsu_index);

#if   MULTI_MAX_VOLT_NUM <  1
#error "MULTI_MAX_VOLT_NUM 必须大于1"
#endif
#endif
#endif
