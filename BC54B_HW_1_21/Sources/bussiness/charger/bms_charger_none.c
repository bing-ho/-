/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_charger_none.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-6-25
*
*/
#include "bms_defs.h"
#include "bms_charger.h"
#include "app_cfg.h"
#include "bms_charger_common.h"

#pragma MESSAGE DISABLE  C5703

void none_charger_init(void)
{
    
}

void none_charger_uninit(void)
{
}

Result none_charger_on(void)
{
    return RES_ERROR;
}

Result none_charger_off(void)
{
    return RES_ERROR;
}

BOOLEAN none_charger_is_charging(void)
{
    return FALSE;
}

/** 充电机是否连接工作 */
Result none_charger_is_connected(void)
{
    return RES_FALSE;
}

Result none_charger_get_status(INT8U* status)
{
    if (status) *status = 0;
    return RES_ERROR;
}

Result none_charger_get_device_info(ChargerDeviceInfo* info)
{
    return RES_ERROR;
}

Result none_charger_set_current(INT16U current)
{
    return RES_ERROR;
}

Result none_charger_get_current(INT16U* current)
{
    return RES_ERROR;
}

Result none_charger_set_voltage(INT16U voltage)
{
    return RES_ERROR;
}

Result none_charger_get_voltage(INT16U* voltage)
{
    return RES_ERROR;
}

Result none_charger_get_output_voltage(INT16U* voltage)
{
    return RES_ERROR;
}

Result none_charger_get_output_current(INT16U* current)
{
    return RES_ERROR;
}

Result none_charger_set_pulse_current(INT16U current)
{
    return RES_ERROR;
}

Result none_charger_get_pulse_current(INT16U* current)
{
    return RES_ERROR;
}

Result none_charger_set_pulse_charge_time(INT16U time)
{
    return RES_ERROR;
}

Result none_charger_get_pulse_charge_time(INT16U* time)
{
    return RES_ERROR;
}

Result none_charger_constant_current_voltage_ctl(void)
{
    return RES_ERROR;
}

Result none_charger_get_charge_voltage_max(INT16U* voltage)
{
    return RES_ERROR;
}

Result none_charger_get_charge_voltage_min(INT16U* voltage)
{
    return RES_ERROR;
}

Result none_charger_get_charge_current_max(INT16U* current)
{
    return RES_ERROR;
}

Result none_charger_get_charge_current_min(INT16U* current)
{
    return RES_ERROR;
}

Result none_charger_get_charge_pulse_current_max(INT16U* current)
{
    return RES_ERROR;
}

Result none_charger_get_charged_energy(INT16U* energy)
{
    return RES_ERROR;
}

Result none_charger_get_charge_time_eclipse(INT16U* time)
{
    return RES_ERROR;
}

Result none_charger_get_charg_time_require(INT16U* time)
{
    return RES_ERROR;
}

Result none_charger_get_charge_cycle(INT16U* cycle)
{
    return RES_ERROR;
}




