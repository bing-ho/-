/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_charger_stock.c
 * @brief
 * @note
 * @author
 * @date 2015-6-3
 *
 */
#ifndef BMS_CHARGER_STOCK_C
#define BMS_CHARGER_STOCK_C

#include "bms_charger_stock.h"

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER

INT8U g_charger_stock_current_control_state = 0; //状态
INT32U g_charger_stock_current_control_tick = 0; //计时
INT16U g_charger_stock_last_current = 0; //保存上一次的有效充电电流值

#define CHARGER_STOCK_REDUCE_CUR_NUM    2 //降电流次数
INT8U g_charger_current_reduce_cnt = 0; //当前降电流次数
const INT16U g_charger_stock_full_charge_volt_buff[CHARGER_STOCK_REDUCE_CUR_NUM] = 
{
3600, 3650
};
const ChgCurCtlCxt g_fast_charger_reduce_current_buff[CHARGER_STOCK_REDUCE_CUR_NUM] = 
{
kReduceTypePercent, 50,
kReduceTypeCurrent, 72
};
#if SLOW_CHARGER_STOCK_TRICKLE_EN
const ChgCurCtlCxt g_slow_charger_reduce_current_buff[CHARGER_STOCK_REDUCE_CUR_NUM] = 
{
kReduceTypePercent, 50,
kReduceTypeCurrent, 60
};
#endif
#pragma DATA_SEG DEFAULT

/**
充电机充电电流控制：
1.	首先充电机以恒流充电电流进行恒流充电；
2.	如果电池组中有最高单体到达g_charger_stock_full_charge_volt_buff定义电压值后，控制充电电流以涓流充电电流充单次涓流充电时间（如果涓流充电电流使能）；
3.	然后当前电流（上一次涓流前的充电电流）减少到g_fast_charger_reduce_current_buff和g_slow_charger_reduce_current_buff定义的值，如果降电流后的当前电流
    大于最小充电电流，则跳转到步骤2重复此降流过程，否则以最小充电电流持续充电，直到达到充电过充保护电压（校正SOC）。
注：CHARGER_STOCK_USE_TRICKLE_CUR_EN == 1 
    降流次数不超过CHARGER_STOCK_REDUCE_CUR_NUM定义值，即只能使用g_fast_charger_reduce_current_buff和g_slow_charger_reduce_current_buff定义的电流值。
    CHARGER_STOCK_USE_TRICKLE_CUR_EN == 0 
    降流次数不受CHARGER_STOCK_REDUCE_CUR_NUM定义值限制，超过后使用CHARGER_STOCK_FULL_CHARGE_VOLT_DEFAULT和CHARGER_STOCK_REDUCE_CUR_DEFAULT定义的值继续降流。
    
*/
void charger_stock_current_reset(void)
{
    g_charger_stock_current_control_state = 0;
}

void charger_stock_current_update(void* pdata)
{
    INT16U temp, current = 0;
    INT32U now_tick = 0;
    
    UNUSED(pdata);
    if(charger_is_connected() == 0) 
    {
        charger_stock_current_reset();
        return ; //充电机未连接
    }
    if(RES_OK != charger_get_current(&current)) return; //无法获取充电电流

#if CHARGER_STOCK_CURRENT_RESET_AUTO_EN
    if(current == 0) //充满电后再次启动检查
    {
        if(bcu_get_charge_state() == kPackStateNormal &&
            bcu_get_total_voltage_state() == kPackStateNormal &&
            bcu_get_full_charge_flag == 0)
        {
            charger_stock_current_reset();
        }
    }
#endif

    switch(g_charger_stock_current_control_state)
    {
        case 0: //恢复充电电流默认值
            g_charger_current_reduce_cnt = 0;
            g_charger_stock_current_control_state = 1;
            break;
        case 1: //达到单体全速充电电压上限，控制电流为涓流充电电流
            if (g_charger_current_reduce_cnt == 0)
            {
                g_charger_stock_last_current = charger_curr_in_temper();//config_get(kChargerCurIndex);
                charger_set_current(g_charger_stock_last_current);
            }
            if (g_charger_current_reduce_cnt >= CHARGER_STOCK_REDUCE_CUR_NUM) temp = CHARGER_STOCK_FULL_CHARGE_VOLT_DEFAULT;
            else temp = g_charger_stock_full_charge_volt_buff[g_charger_current_reduce_cnt];
            
            if(bcu_get_high_voltage() >= temp) 
            {
#if CHARGER_STOCK_USE_TRICKLE_CUR_EN
                charger_set_current(CHARGER_STOCK_TRICKLE_CURRENT);
#endif
                g_charger_stock_current_control_tick = get_tick_count();
                g_charger_stock_current_control_state = 2;
            }
            break;
        case 2: //涓流充电持续设定时间
            now_tick = get_tick_count();
#if CHARGER_STOCK_USE_TRICKLE_CUR_EN
            if(get_interval_by_tick(g_charger_stock_current_control_tick, now_tick) >= CHARGER_STOCK_TRICKLE_CHARGE_TIME)
#endif
            {
                ChgCurCtlCxt * _PAGED ptr;
                
                g_charger_stock_current_control_tick = now_tick;
                current = g_charger_stock_last_current;
                if(g_charger_current_reduce_cnt < CHARGER_STOCK_REDUCE_CUR_NUM)
                {
#if SLOW_CHARGER_STOCK_TRICKLE_EN
                    if(bms_slow_charger_is_connected()) // 慢充
                    {
                        ptr = g_slow_charger_reduce_current_buff;
                        
                    }
                    else // 快充
#endif
                    {
                        ptr = g_fast_charger_reduce_current_buff;
                    }
                    if(ptr[g_charger_current_reduce_cnt].type == kReduceTypePercent)
                    {
                        current = (INT16U)((INT32U)current * ptr[g_charger_current_reduce_cnt].value / 100);
                    }
                    else
                    {
                        current = ptr[g_charger_current_reduce_cnt].value;
                    }
#if CHARGER_STOCK_USE_TRICKLE_CUR_EN
                    g_charger_current_reduce_cnt++;
#else
                    if(g_charger_current_reduce_cnt+1 < CHARGER_STOCK_REDUCE_CUR_NUM) g_charger_current_reduce_cnt++;
#endif
                }
                else //启用默认降流策略
                {
                    if(current > CHARGER_STOCK_REDUCE_CUR_DEFAULT) current -= CHARGER_STOCK_REDUCE_CUR_DEFAULT;
                    else current = 0;
                }
                if(current > CHARGER_STOCK_CURRENT_MIN)
                {
                    g_charger_stock_last_current = current;
                    charger_set_current(current);
                    g_charger_stock_current_control_state = 1; //进入下一循环
                }
                else
                {
                    g_charger_stock_last_current = CHARGER_STOCK_CURRENT_MIN;
                    charger_set_current(CHARGER_STOCK_CURRENT_MIN);
                    g_charger_stock_current_control_state = 3; 
                }
            }
            break;
        case 3:
            //等待充电完成
            break;
    }
}

/***********************************************************************************
** 充电机充电功率
***********************************************************************************/

#define GB_CHARGER_CUR_TEMPER_BUFF_NUM  8
const INT8U g_charge_temper_buff[GB_CHARGER_CUR_TEMPER_BUFF_NUM] = 
{
TEMPERATURE_FROM_C(-50),TEMPERATURE_FROM_C(-5),TEMPERATURE_FROM_C(0),TEMPERATURE_FROM_C(5),TEMPERATURE_FROM_C(10),
TEMPERATURE_FROM_C(50),TEMPERATURE_FROM_C(55),TEMPERATURE_FROM_C(120)
};

const INT16U g_gb_charge_cur_buff[GB_CHARGER_CUR_TEMPER_BUFF_NUM-1] = 
{
60,60,144,288,432,216,0
};

const INT16U g_slow_charge_cur_buff[GB_CHARGER_CUR_TEMPER_BUFF_NUM-1] = 
{
60,60,120,120,120,120,0
};

INT16U charger_curr_in_temper(void)
{
    INT8U high_temp_index = 0, low_temp_index = 0;
    INT16U current = 0, temp = 0;
    
    low_temp_index =  (INT8U)bms_get_byte_range_index(g_charge_temper_buff, GB_CHARGER_CUR_TEMPER_BUFF_NUM, (INT8U)bcu_get_low_temperature());
    high_temp_index = (INT8U)bms_get_byte_range_index(g_charge_temper_buff, GB_CHARGER_CUR_TEMPER_BUFF_NUM, (INT8U)bcu_get_high_temperature());
    
    if(high_temp_index > GB_CHARGER_CUR_TEMPER_BUFF_NUM-1 && low_temp_index  > GB_CHARGER_CUR_TEMPER_BUFF_NUM-1) return 0;
    if(high_temp_index > GB_CHARGER_CUR_TEMPER_BUFF_NUM-1) high_temp_index = low_temp_index;
    if(low_temp_index  > GB_CHARGER_CUR_TEMPER_BUFF_NUM-1) low_temp_index = high_temp_index;
    
    if(bms_slow_charger_is_connected()) //慢充
    {
        temp = g_slow_charge_cur_buff[high_temp_index];
        current = g_slow_charge_cur_buff[low_temp_index];
    }
    else if(charger_is_connected()) //快充
    {
        temp = g_gb_charge_cur_buff[high_temp_index];
        current = g_gb_charge_cur_buff[low_temp_index];
    }
    if(current > temp) current = temp;
    
    return current;
}

/***********************************************************************************
** 充电加热控制流程
***********************************************************************************/

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER

ChgHeatState g_chargeHeatState = kChgHeatInit;
ChgHeatState g_chargeHeatStateLast = kChgHeatNone;
static INT8U g_chargeHeatFaultStartTemp = 0; //加热故障时加热片温度
static INT8U g_chargeHeat75TFlag = 0;
static INT32U g_chargeHeatLastTick = 0;
static INT32U g_chargeHeat60TLastTick = 0;
static INT32U g_chargeHeat75TLastTick = 0;
static INT32U g_chargeHeat80TLastTick = 0;

#pragma DATA_SEG DEFAULT

ChgHeatState charger_get_heat_state(void)
{
    return g_chargeHeatState;
}
//注：charger_heat_state_update不允许仅在充电阶段更新，防止出现充电继电器无法闭合和充电机检测不到电池电压无法充电准备就绪互锁。
void charger_heat_state_update(void)
{
    //允许充电检查
    if (charger_is_connected() == FALSE ||
        charger_charging_is_enable() == FALSE)
    {
        charger_heat_state_reset();
        return;
    }
    switch(g_chargeHeatState)
    {
        case kChgHeatInit:
            charger_heat_init_state_process();
            break;
        case kChgHeatSelect:
            charger_heat_select_state_process();
            break;
        case kChgHeatLTHeat:
            charger_heat_lt_heat_state_process();
            break;
        case kChgHeatNormalHeat:
            charger_heat_normal_heat_state_process();
            break;
        case kChgHeatNormalChg:
            charger_heat_normal_charge_state_process();
            break;
        case kChgHeatChgFinish:
            charger_heat_charge_finish_state_process();
            break;
        case kChgHeatFault:
            charger_heat_Fault_state_process();
            break;
        default:
            g_chargeHeatState = kChgHeatInit;
            break;
    }
}

void charger_heat_state_reset(void)
{
    if(g_chargeHeatState == kChgHeatFault) return; //故障不允许恢复
    
    g_chargeHeatState = kChgHeatInit;
    g_chargeHeatStateLast = kChgHeatNone;
    //charger_control_enable_charger(SELFCHECK_CHG_HEAT_TEMP);
    rule_stock_set_var(CHARGER_HEAT_FAULT_BUFF_INDEX, FALSE);
    rule_stock_set_var(CHARGER_HEAT_RELAY_ON_INDEX, FALSE);
}

void charger_heat_init_state_process(void)
{
    rule_stock_set_var(CHARGER_HEAT_RELAY_ON_INDEX, FALSE);
    if(rule_stock_get_var(CHARGER_HEAT_FAULT_BUFF_INDEX) == FALSE &&  //加热片未故障
        bcu_get_high_heat_temperature() < TEMPERATURE_FROM_C(CHARGER_HEATER_STOP_HEAT_TEMP)) //当前加热片温度正常
    {
        charger_heat_state_reset();
        g_chargeHeatStateLast = g_chargeHeatState;
        g_chargeHeatState = kChgHeatSelect; //进入加热选择状态
    }
    else
    {
        g_chargeHeatState = kChgHeatFault;
    }
}

void charger_heat_select_state_process(void)
{
    INT8S low_temp = 0;
    
    low_temp = TEMPERATURE_TO_C(TEMP_4_DISPLAY((INT8S)bcu_get_low_temperature()));
    g_chargeHeatStateLast = g_chargeHeatState;
    if(low_temp < CHARGER_HEAT_LOW_TEMPERATURE)
    {
        g_chargeHeatState = kChgHeatLTHeat; //低温加热
    }
    else if(low_temp < CHARGER_HEAT_NORMAL_TEMPERATURE)
    {
        rule_stock_set_var(CHARGER_HEAT_RELAY_ON_INDEX, TRUE);
        rule_stock_set_var(CHARGER_CHARGE_RELAY_ON_INDEX, TRUE);
        g_chargeHeatState = kChgHeatNormalHeat; //正常加热
    }
    else
    {
        g_chargeHeatState = kChgHeatNormalChg; //正常充电
    }
}

void charger_heat_lt_heat_state_process(void)
{
    INT16S current = 0;
    INT32U now_tick = 0;
    
    current = bcu_get_current();
    now_tick = get_tick_count();
    //state init
    if(g_chargeHeatStateLast != g_chargeHeatState)
    {
        charger_set_current(CHARGER_HEAT_LT_CURRENT);
        charger_set_voltage(CHARGER_HEAT_NORMAL_VOLTAGE);
        rule_stock_set_var(CHARGER_CHARGE_RELAY_ON_INDEX, TRUE);
        g_chargeHeatStateLast = g_chargeHeatState;
    }
    else //state wait for checking
    {
        if(relay_control_is_on(kRelayTypeHeating) == 0) //加热继电器未闭合
        {
            if(current > CHARGER_HEAT_CURRENT_MIN)
            {
                if(get_interval_by_tick(g_chargeHeatLastTick, now_tick) >= CHARGER_LT_HEAT_ON_DELAY)
                {
                    rule_stock_set_var(CHARGER_HEAT_RELAY_ON_INDEX, TRUE); //加热继电器闭合
                    g_chargeHeatLastTick = now_tick;
                }
            }
            else
            {
                g_chargeHeatLastTick = now_tick;
            }
        }
        else
        {
            if(current > CHARGER_HEAT_CURRENT_MIN)
            {
                if(get_interval_by_tick(g_chargeHeatLastTick, now_tick) >= CHARGER_CHG_RELAY_OFF_DELAY_IN_HEAT)
                {
                    rule_stock_set_var(CHARGER_CHARGE_RELAY_ON_INDEX, FALSE);
                    if(bcu_get_charge_relay_state() == kRelayStatusOff) //等待充电继电器断开
                    {
                        g_chargeHeatLastTick = now_tick;
                        g_chargeHeatState = kChgHeatNormalHeat;
                    }
                }
            }
            else
            {
                charger_heat_state_reset();
            }
        }
    }
}

void charger_heat_normal_heat_state_process(void)
{
    INT8S high_heat_temp = 0;
    INT16S current = 0;
    INT16U charge_current = 0;
    INT32U now_tick = 0;
    
    high_heat_temp = TEMPERATURE_TO_C(bcu_get_high_heat_temperature());
    current = bcu_get_current();
    now_tick = get_tick_count();
    //state init
    if(g_chargeHeatStateLast != g_chargeHeatState)
    {
        charger_set_current(CHARGER_HEAT_NORMAL_CURRENT);
        g_chargeHeat60TLastTick = now_tick;
        g_chargeHeat75TLastTick = now_tick;
        g_chargeHeat80TLastTick = now_tick;
        g_chargeHeat75TFlag = 0;
        g_chargeHeatStateLast = g_chargeHeatState;
    }
    else //state wait for checking
    {
        if(current < -30) //存在放电电流
        {
            charger_heat_state_reset();
            return;
        }
        if(high_heat_temp > CHARGER_HEATER_FAULT_TEMPERATURE)// >80度
        {
            g_chargeHeat60TLastTick = now_tick;
            if(get_interval_by_tick(g_chargeHeat80TLastTick, now_tick) >= 10000)
            {
                g_chargeHeatState = kChgHeatFault;
            }
        }
        else if(high_heat_temp > CHARGER_HEATER_STOP_HEAT_TEMP) //>75度
        {
            g_chargeHeat60TLastTick = now_tick;
            g_chargeHeat80TLastTick = now_tick;
            charger_get_current(&charge_current);
            if(charge_current > 0)
            {
                if(get_interval_by_tick(g_chargeHeat75TLastTick, now_tick) >= 10000)
                {
                    g_chargeHeat75TLastTick = now_tick;
                    g_chargeHeat75TFlag = 1;
                    charger_set_current(0);
                }
            }
        }
        else
        {
            g_chargeHeat75TLastTick = now_tick;
            g_chargeHeat80TLastTick = now_tick;
            charger_get_current(&charge_current);
            if(g_chargeHeat75TFlag == 1) //已经出现超过75度
            {
                if(high_heat_temp < CHARGER_HEATER_NORMAL_TEMP_MAX)
                {
                    if(get_interval_by_tick(g_chargeHeat60TLastTick, now_tick) >= 10000)
                    {
                        g_chargeHeat75TFlag = 0;
                        charger_set_current(CHARGER_HEAT_NORMAL_CURRENT);
                    }
                }
                else
                {
                    g_chargeHeat60TLastTick = now_tick;
                }
            }
            else
            {
                if(bcu_get_low_temperature() > TEMPERATURE_FROM_C(CHARGER_HEAT_TEMPERATURE_MAX))
                {
                    g_chargeHeatState = kChgHeatNormalChg;
                }
            }
        }
    }
}

void charger_heat_normal_charge_state_process(void)
{
    INT32U now_tick;
    
    now_tick = get_tick_count();
    //state init
    if(g_chargeHeatStateLast != g_chargeHeatState)
    {
        charger_stock_current_reset();
        charger_set_voltage(config_get(kChargerVoltIndex));
        rule_stock_set_var(CHARGER_CHARGE_RELAY_ON_INDEX, TRUE);
        g_chargeHeatLastTick = now_tick;
        g_chargeHeatStateLast = g_chargeHeatState;
    }
    else
    {
        charger_stock_current_update(NULL); //正常充电
        if(relay_control_is_on(kRelayTypeHeating) == 0) return;
        if(bcu_get_charge_relay_state() == kRelayStatusOn)
        {
            if(get_interval_by_tick(g_chargeHeatLastTick, now_tick) >= 1000)
            {
                rule_stock_set_var(CHARGER_HEAT_RELAY_ON_INDEX, FALSE);
            }
        }
        else
        {
            g_chargeHeatLastTick = now_tick;
        }
    }
}

void charger_heat_charge_finish_state_process(void)
{
    
}

INT8U charger_check_heat_relay_fault(void)
{
    if(g_chargeHeatState != kChgHeatFault) return FALSE;
    if(rule_stock_get_var(CHARGER_HEAT_FAULT_BUFF_INDEX)) return TRUE;
    
    if(bcu_get_high_heat_temperature() > g_chargeHeatFaultStartTemp + 2) //超过2度认为仍在加热，加热异常
    {
        rule_stock_set_var(CHARGER_HEAT_FAULT_BUFF_INDEX, TRUE);
        return TRUE;
    }
    return FALSE;
}

void charger_heat_Fault_state_process(void)
{
    //state init
    if(g_chargeHeatStateLast != g_chargeHeatState)
    {
        charger_set_current(0);
        charger_control_disable_charger(SELFCHECK_CHG_HEAT_TEMP);
        rule_stock_set_var(CHARGER_HEAT_RELAY_ON_INDEX, FALSE);
        rule_stock_set_var(CHARGER_CHARGE_RELAY_ON_INDEX, FALSE);
        g_chargeHeatFaultStartTemp = bcu_get_high_heat_temperature();
        g_chargeHeatStateLast = g_chargeHeatState;
    }
    else //state wait for checking
    {
        charger_check_heat_relay_fault();
    }
}

#endif