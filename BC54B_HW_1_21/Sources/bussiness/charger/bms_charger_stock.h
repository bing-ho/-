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
** 充电机充电电流控制参数
***********************************************************************************/
/*
注：涓流充电使能后当降流次数超过CHARGER_STOCK_REDUCE_CUR_NUM时，
启用默认CHARGER_STOCK_FULL_CHARGE_VOLT_DEFAULT和CHARGER_STOCK_REDUCE_CUR_DEFAULT
*/
#define CHARGER_STOCK_USE_TRICKLE_CUR_EN    0 //涓流充电使能
#define SLOW_CHARGER_STOCK_TRICKLE_EN       1 //慢充降流策略使能


typedef enum
{
    kReduceTypePercent,// 1%/bit 降至当前电流的百分比
    kReduceTypeCurrent // 0.1A 降至电流值
}ReduceCurType;

typedef struct
{
    ReduceCurType type;
    INT16U value;
}ChgCurCtlCxt;

#define CHARGER_STOCK_CURRENT_RESET_AUTO_EN     0  //故障释放后自动恢复充电电流使能
#define CHARGER_STOCK_TRICKLE_CURRENT           60 //0.1A/bit 涓流充电电流
#define CHARGER_STOCK_FULL_CHARGE_VOLT_DEFAULT  3650 //mv/bit 单体全速充电电压上限
#define CHARGER_STOCK_TRICKLE_CHARGE_TIME       5000 //ms/bit 涓流充电时间
#define CHARGER_STOCK_CURRENT_MIN               60 //0.1A/bit 充电电流控制最小有效电流
#define CHARGER_STOCK_REDUCE_CUR_DEFAULT        (current / 2) //单次减少的电流值

void charger_stock_current_reset(void);
void charger_stock_current_update(void* pdata);

/***********************************************************************************
** 充电机充电功率
***********************************************************************************/

INT16U charger_curr_in_temper(void);
INT8U ems_chg_cur_decrease_is_50_percent(void);
INT8U ems_chg_cur_decrease_is_100_percent(void);

/***********************************************************************************
** 充电加热控制流程
***********************************************************************************/

#define CHARGER_HEAT_RELAY_ON_INDEX         0 //加热状态
#define CHARGER_CHARGE_RELAY_ON_INDEX       1 //充电状态
#define CHARGER_HEAT_FAULT_BUFF_INDEX       2 //加热状态异常

#define CHARGER_HEAT_LOW_TEMPERATURE        -5 //低温加热阈值
#define CHARGER_HEAT_NORMAL_TEMPERATURE     0 //正常温度阈值
#define CHARGER_HEAT_TEMPERATURE_MAX        5 //最大加热温度

#define CHARGER_HEAT_NORMAL_VOLTAGE         3680 //0.1V 正常加热电压

#define CHARGER_HEAT_CURRENT_MIN            20 //0.1A 最小加热电流
#define CHARGER_HEAT_LT_CURRENT             40 //0.1A 低温加热电流
#define CHARGER_HEAT_NORMAL_CURRENT         60 //0.1A 正常加热电流

#define CHARGER_HEATER_FAULT_TEMPERATURE    80 //加热器故障温度
#define CHARGER_HEATER_NORMAL_TEMP_MAX      60 //加热器正常温度最大值
#define CHARGER_HEATER_STOP_HEAT_TEMP       75 //加热器停止加热温度

#define CHARGER_LT_HEAT_ON_DELAY            1000 //1ms 低温加热继电器闭合延时
#define CHARGER_CHG_RELAY_OFF_DELAY_IN_HEAT 10000 // 1ms 低温加热状态充电继电器断开延时

typedef enum
{
    kChgHeatNone=0,
    kChgHeatInit, //加热初始化
    kChgHeatSelect, //起始加热入口选择
    kChgHeatLTHeat, //低温加热,<-5
    kChgHeatNormalHeat,//正常加热,>0
    kChgHeatNormalChg,//正常充电
    kChgHeatChgFinish, //充电完成
    kChgHeatFault //加热故障
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