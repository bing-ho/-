/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_alert_impl.c
 * @brief
 * @note
 * @author
 * @date 2012-5-31
 *
 */
#include "bms_alert_impl.h"

#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C5703 // Parameter is not used

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

INT8U g_alert_is_enabled = 1; //开启和使用报警功能

INT32U g_alert_disabled_tick = 0; //报警超时定时器，如果定时器超时则重新启动报警功能

//INT16U g_alert_beep_mask = 0xFFFF; // TODO, 应添加相应配置项
INT32U g_alert_beep_bits = 0;
INT32U g_alert_beep_last_tick = 0;

#pragma DATA_SEG DEFAULT


void alert_init(void)
{
    beep_init();

    g_alert_beep_bits = 0;

    job_schedule(MAIN_JOB_GROUP, ALERT_JOB_PERIODIC, alert_heart_beat, NULL);
}

void alert_unit(void)
{
}

void alert_enable(void)
{
    g_alert_is_enabled = 1;
    g_alert_beep_last_tick = 0;

    g_alert_disabled_tick = 0;
}

void alert_disable(void)
{
    g_alert_disabled_tick = 0;
}

INT8U alert_is_enabled(void)
{
    return g_alert_is_enabled;
}

void alert_long_beep(INT16U time)
{
    if (time == 0) return;

    beep_on();
    sleep(time);
    beep_off();
}

void alert_beep_on(INT32U type)
{
    g_alert_beep_bits |= type;
}

void alert_beep_off(INT32U type)
{
    g_alert_beep_bits &= (~type);
}

void alert_heart_beat(void* data)
{
    INT32U tick;
    INT32U alert_beep_mask;

    if (!g_alert_is_enabled) //报警功能重置
    {
        if (get_elapsed_tick_count_with_init(&g_alert_disabled_tick) >= ALERT_DISABLE_TIMEOUT)
        {
            g_alert_disabled_tick = 0;
            g_alert_is_enabled = 1;
        }
        else
        {
            return;
        }
    }

    alert_beep_mask = (((INT32U)ALARM_BEEP_MASK_HIGH_DEF) << 16) + ALARM_BEEP_MASK_LOW_DEF;//config_get(kAlarmBeepMaskLow)//config_get(kAlarmBeepMaskHigh)
    if ((g_alert_beep_bits & alert_beep_mask) == 0) return;

    tick = get_tick_count();
    if (beep_is_on() == 1)
    {
        if (get_interval_by_tick(g_alert_beep_last_tick, tick) >= ALERT_BEEP_ON_PERIODIC)
        {
            g_alert_beep_last_tick = tick;
            beep_off();
        }
    }
    else
    {
        if (get_interval_by_tick(g_alert_beep_last_tick, tick) >= ALARM_BEEP_INTERVAL_DEF)//config_get(kAlarmBeepInterval)
        {
            g_alert_beep_last_tick = tick;
            beep_on();
        }
    }
}
