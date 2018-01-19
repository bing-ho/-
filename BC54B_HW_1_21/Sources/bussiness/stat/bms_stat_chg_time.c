/*
 * bms_stat_chg_time.c
 *
 *  Created on: 2012-9-19
 *      Author: Administrator
 */

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler
#pragma MESSAGE DISABLE C2705 // Possible loss of data
#pragma MESSAGE DISABLE  C5919 // Conversion of floating to unsigned integral

#ifndef BMS_DIAGNOSE_CHARGE_TIME_C
#define BMS_DIAGNOSE_CHARGE_TIME_C

#include "includes.h"
#include "stdtypes.h"
#include "bms_bcu.h"
#include "bms_stat_chg_time.h"
#include "bms_config.h"

#define BMS_CHARGE_REQUIRE_TIME_MAX         0xFFFFFFFF

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
INT32U g_bms_charge_time_last=0;
INT32U g_bms_charge_time_eclipse=0;//已充电时间 单位：s
INT32U g_bms_charge_time_require=0;//剩余充电时间 单位:s
#pragma DATA_SEG DEFAULT

BOOLEAN battery_is_charging(INT16U current)
{
    return ((current > 0) && (current < 0x8000));
}

INT32U update_bms_charge_time_require(void)
{
    OS_CPU_SR cpu_sr=0;
    INT16U cur;
    FP32 time, mdata;

    cur = bcu_get_current();
    if(!battery_is_charging(cur))
        return BMS_CHARGE_REQUIRE_TIME_MAX;
    if(config_get(kTotalCapIndex) < config_get(kLeftCapIndex))
        return BMS_CHARGE_REQUIRE_TIME_MAX;
    time = DIVISION((config_get(kTotalCapIndex) - config_get(kLeftCapIndex)),10);//time = config_get(kTotalCapIndex) - config_get(kLeftCapIndex);
    mdata = cur;
    
    OS_ENTER_CRITICAL();
    time = time*3600/CURRENT_100_MA_TO_A(mdata);
    OS_EXIT_CRITICAL();
    return (INT32U)time;
}

void update_bms_charge_para(void)
{
    g_bms_charge_time_eclipse = get_elapsed_tick_count(g_bms_charge_time_last)/1000;
    g_bms_charge_time_require = update_bms_charge_time_require();
}

void update_bms_uncharge_para(void)
{
    g_bms_charge_time_last = get_tick_count();
    g_bms_charge_time_eclipse=0;
    g_bms_charge_time_require=BMS_CHARGE_REQUIRE_TIME_MAX;
}

void statistic_charge_time(void)
{
    if(bcu_is_charging())
        update_bms_charge_para();
    else
        update_bms_uncharge_para();
}

INT32U get_bms_charge_eclipse_time_s(void)
{
    return g_bms_charge_time_eclipse;
}

INT32U get_bms_charge_require_time_s(void)
{
   static INT32U time = 0;
    INT16U cur;
    
    cur = bcu_get_current();
    if(battery_is_charging(cur) && guobiao_charger_get_CommStage() == kChargeStage)
        time = time < g_bms_charge_time_require ? time : g_bms_charge_time_require;
    else
        time = g_bms_charge_time_require;
    
    return time;
    //return g_bms_charge_time_require;
}

#endif



