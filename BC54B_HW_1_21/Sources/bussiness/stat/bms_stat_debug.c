/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_stat_debug.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-10-17
*
*/

#ifndef BMS_STAT_DEBUG_C_
#define BMS_STAT_DEBUG_C_
#include "bms_defs.h"
#include "includes.h"
#include "bms_util.h"
#include "bms_memory.h"
#include "bms_log.h"
#include "bms_vm_engine.h"
#include "bms_rule_engine.h"

#define STAT_INTERVAL 5000

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
INT32U g_stat_debug_last_tick = 0;
#pragma DATA_SEG DEFAULT

void statistic_debug(void)
{
    INT32U tick = get_tick_count();

    if (get_interval_by_tick(g_stat_debug_last_tick, tick) <= STAT_INTERVAL) return;
    g_stat_debug_last_tick = tick;

    //DEBUG("stat", "cpu:%d", OSCPUUsage);
    DEBUG("stat", "rule_num:%d", rule_engine_get_rule_num());
    DEBUG("stat", "vm_var_num:%d", vm_get_variable_num());
    DEBUG("stat", "vm_func_num:%d", vm_get_function_num());
    DEBUG("stat", "vm_const_num:%d", vm_get_const_num());
    DEBUG("stat", "malloc_size:%d", bms_malloc_size());
}


#endif /* BMS_STAT_DEBUG_C_ */
