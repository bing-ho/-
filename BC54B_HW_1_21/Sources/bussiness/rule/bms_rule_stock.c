/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_rule_stock.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-21
*
*/
#include "bms_rule_stock.h"
#include "gpio_interface.h"


#pragma MESSAGE DISABLE C5919 // Conversion of floating to unsigned integral
#pragma MESSAGE DISABLE C12056

/**
 * 10 + 29 * (sizeof(HigherLevelPriorityAlarmContext)/sizeof(INT16U)) = 619
 */

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_RULE
static INT32U g_rule_engine_check_keep_items[RULE_ENGINE_MAX_RULE_NUM + 1] = {0};
static INT16U g_rule_engine_custom_variables[RULE_ENGINE_MAX_CUSTOM_VAR_NUM] = {0};
#pragma pop

RULE_STOCK_HIGHER_LEVEL_PRIORITY_CHECK_ALARM(first, First, first, First);
RULE_STOCK_HIGHER_LEVEL_PRIORITY_CHECK_ALARM(second, Second, first, First);
RULE_STOCK_HIGHER_LEVEL_PRIORITY_CHECK_ALARM(third, Third, second, Second);
#if BMS_FORTH_ALARM_SUPPORT
RULE_STOCK_HIGHER_LEVEL_PRIORITY_CHECK_ALARM(forth, Forth, third, Third);
#endif

INT8U rule_stock_check_keep(INT16U id, INT8U status, INT32U keep_time)
{
    INT32U now_tick;
    if (id == 0 || id > RULE_ENGINE_MAX_RULE_NUM) return 0;

    if (!status)
    {
        g_rule_engine_check_keep_items[id] = 0;
        return 0;
    }

    now_tick = get_tick_count();
    if (now_tick == 0) now_tick = 1;

    if (g_rule_engine_check_keep_items[id] == 0)
    {
        g_rule_engine_check_keep_items[id] = now_tick;
    }

    if (get_interval_by_tick(g_rule_engine_check_keep_items[id], now_tick) >= keep_time)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

Result rule_stock_rule_check_keep(INT16U status, INT32U keep_time)
{
    return rule_stock_check_keep(rule_engine_get_current_rule_id(), (INT8U)status, keep_time) ? RES_TRUE : RES_FALSE;
}

void rule_stock_charger_set_exp_current(void)
{
    INT16U charger_current;
    //INT16U full_charger_current;
    INT16U current=0;
    charger_current = config_get(kChargerCurIndex);
    if (charger_current > g_config_attribute[kChargerCurIndex].config_max) charger_current = g_config_attribute[kChargerCurIndex].config_max;
    //TODO：此处全速充电电压已经修改为满充释放总压，此规则需要修改
    //current = (INT16U) (charger_current * exp(0.008 * (INT16S) (config_get(kFullChgReleaseIndex) - bcu_get_high_voltage())));

    charger_set_current(current);
}

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
extern const struct hcs12_gpio_detail *beep_output_pins;
#pragma pop
void rule_stock_test_beep(INT16U value)
{
    //PORTA_PA0 = (INT8U)value;
    if(beep_output_pins != NULL)
    {
        hcs12_gpio_set_output(&beep_output_pins[0].io, (INT8U)value);
    }
}

void rule_stock_dump_cpu_usage(void)
{
    //DEBUG("performance", "current cpu usage:%d", OSCPUUsage);
}

Result rule_stock_set_var(INT16U index, INT16U value)
{
    if (index >= RULE_ENGINE_MAX_CUSTOM_VAR_NUM) return RES_ERROR;
    g_rule_engine_custom_variables[index] = value;
    return RES_OK;
}

INT16U rule_stock_get_var(INT16U index)
{
    if (index >= RULE_ENGINE_MAX_CUSTOM_VAR_NUM) return 0;
    return g_rule_engine_custom_variables[index];
}

INT16U rule_stock_get_var_buff_size(void)
{
    return RULE_ENGINE_MAX_CUSTOM_VAR_NUM;
}

void rule_stock_higher_level_priority_check_alarm(HigherLevelPriorityAlarmContext* _PAGED ctx)
{   
    INT16U alarm_level = kAlarmNone;
    INT8U flag = 0;
    if(ctx == NULL) return;
    
    
#if BMS_FORTH_ALARM_SUPPORT
    rule_stock_higher_level_priority_check_forth_alarm(ctx);
#endif
    rule_stock_higher_level_priority_check_third_alarm(ctx);
    rule_stock_higher_level_priority_check_second_alarm(ctx);
    rule_stock_higher_level_priority_check_first_alarm(ctx);
    
#if BMS_FORTH_ALARM_SUPPORT
    if(ctx->forth_info.bits.state == kAlarmState_alarm || ctx->forth_info.bits.state == kAlarmState_release) {
        alarm_level = kAlarmForthLevel;
        flag = 1;
    }   
#endif
    if(flag == 0)
    {
        if(ctx->third_info.bits.state == kAlarmState_alarm || ctx->third_info.bits.state == kAlarmState_release) {
            alarm_level = kAlarmThirdLevel;
        }
        else if(ctx->second_info.bits.state == kAlarmState_alarm || ctx->second_info.bits.state == kAlarmState_release) {
            alarm_level = kAlarmSecondLevel;
        }
        else if(ctx->first_info.bits.state == kAlarmState_alarm || ctx->first_info.bits.state == kAlarmState_release) {
            alarm_level = kAlarmFirstLevel;
        }
    }
    
    if(ctx->cur_state < alarm_level)
    {
        if(ctx->cur_state % 2) ctx->cur_state = ctx->cur_state + 2;
        else ctx->cur_state = ctx->cur_state + 1;
    }
    else if(ctx->cur_state > alarm_level)
    {
        switch(ctx->cur_state)
        {
            #if BMS_FORTH_ALARM_SUPPORT
            case kAlarmForthLevel:
                ctx->cur_state = kAlarmThirdLevel;
                break;
            #endif
            case kAlarmThirdLevel:
                ctx->cur_state = kAlarmSecondLevel;
                break;
            case kAlarmSecondLevel:
                ctx->cur_state = kAlarmFirstLevel;
                break;
            case kAlarmFirstLevel:
                ctx->cur_state = kAlarmNone;
                break;
        }     
    }
}

void rule_stock_higher_level_priority_alarm_run(INT16U pos)
{
    if(pos >= RULE_ENGINE_MAX_CUSTOM_VAR_NUM) return;
    
    rule_stock_higher_level_priority_check_alarm((HigherLevelPriorityAlarmContext* _PAGED)(g_rule_engine_custom_variables + pos));
}

void rule_stock_test(INT16U value)
{
	UNUSED(value);
    //DEBUG("test", "discharge_exception_flag:%d, high_temp:%d, insu:%d, voltage:%d", bcu_get_discharge_exception_flag(), bcu_get_high_temperature(),
    //        bcu_get_system_insulation_resistance(), bcu_get_total_voltage());
}

