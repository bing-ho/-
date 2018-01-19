/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_stat_system_time.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-10-18
*
*/
#include "app_cfg.h"
#include "bms_stat.h"
#include "bms_stat_chg_time.h"
#include "bms_stat_dchg_time.h"
#include "bms_stat_debug.h"
#include "bms_job.h"
#include "bms_clock.h"
#include "bms_bcu.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C4001 // Condition always FALSE

#define BCU_SYSTEM_TIME_CLOCK_GET_INTERVAL  300000 //ms
#define MS_FROM_S_GAIN                      1000

INT8U g_bcu_system_time_upate_flag = 1; //上电从时钟芯片更新系统时间标志

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
static INT32U g_bcu_system_time_start_tm_t = 0;
static INT32U g_bcu_system_time_start_tick = 0;
static INT32U g_bcu_system_time_clock_get_tick = 0;
#pragma DATA_SEG DEFAULT


void statistic_system_time_update(void)
{
    DateTime time;
    unsigned long tm_t;
    INT8U flag = 1;
    INT32U diff = 0, tick;
    
    tick = get_tick_count();
    if(g_bcu_system_time_upate_flag == 1 ||
        get_interval_by_tick(g_bcu_system_time_clock_get_tick, tick) >= BCU_SYSTEM_TIME_CLOCK_GET_INTERVAL) 
    {
        if(clock_get(&time) == RES_OK)
        {
            flag = 0;
            g_bcu_system_time_upate_flag = 0;
            g_bcu_system_time_start_tick = tick;
            g_bcu_system_time_clock_get_tick = tick;
            g_bcu_system_time_start_tm_t = clock_to_time_t(&time);
        }
    }
    if(flag)
    {
        tm_t = g_bcu_system_time_start_tm_t;
        diff = get_interval_by_tick(g_bcu_system_time_start_tick, tick);
        tm_t += (diff + MS_FROM_S_GAIN / 2) / MS_FROM_S_GAIN;
        clock_from_time_t(tm_t, &time);
        if(diff >= MS_FROM_S_GAIN)
        {
            g_bcu_system_time_start_tick += MS_FROM_S_GAIN;
            g_bcu_system_time_start_tm_t += 1;
        }
    }
    
    bcu_set_system_time(&time);
}


