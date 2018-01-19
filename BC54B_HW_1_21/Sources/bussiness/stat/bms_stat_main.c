/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_stat_main.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-10-8
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
#include "bms_stat_system_time.h"


void stat_refresh(void* data)
{
    UNUSED(data);

    statistic_charge_time();
    statistic_discharge_time();
    statistic_system_time_update();

#if BMS_SUPPORT_STAT_DEBUG_INFO
    statistic_debug();
#endif
}

void stat_init(void)
{
    job_schedule(MAIN_JOB_GROUP, STAT_REFRESH_PERIDOIC, stat_refresh, NULL);
}


