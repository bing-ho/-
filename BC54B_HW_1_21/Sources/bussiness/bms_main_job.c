/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_main_job.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-9-28
*
*/
#include "bms_main_job.h"
#include "includes.h"
#include "app_cfg.h"
#include "bms_system.h"
#include "bms_job.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C4001 // Condition always FALSE

OS_STK g_bms_main_job_task_stack[BMS_MAIN_JOB_TASK_STK_SIZE] = { 0 }; //电池分析任务堆栈


void bms_main_job_task_run(void* pdata);
void bms_main_job_task_create(void);

void bms_main_job_init(void)
{
    job_init();
    bms_main_job_task_create();
}

void bms_main_job_task_create(void)
{
    OSTaskCreate(bms_main_job_task_run, (void *) 0,
            (OS_STK *) &g_bms_main_job_task_stack[BMS_MAIN_JOB_TASK_STK_SIZE - 1],
            BMS_MAIN_JOB_TASK_PRIO);
}

//volatile INT16U diff = 0;
void bms_main_job_task_run(void* pdata)
{
    
    //INT32U last_tick = 0;
    UNUSED(pdata);
    for (;;)
    {
      //last_tick = OSTimeGet();
      job_heartbeat(MAIN_JOB_GROUP);
      sleep(MAIN_JOB_CHECK_PERIODIC);
      //diff = get_elapsed_tick_count(last_tick);
      
    }
}
