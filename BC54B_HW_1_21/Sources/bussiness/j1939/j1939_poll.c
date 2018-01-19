/*
 * j1939_poll.c
 *
 *  Created on: 2012-9-19
 *      Author: Administrator
 */
#ifndef J1939_POLL_C_
#define J1939_POLL_C_

#include "j1939_poll.h"
#include "run_mode.h"

#if BMS_SUPPORT_J1939_COMM

OS_STK g_j1939_poll_task_stack[J1939_POLL_TASK_STK_SIZE];

void j1939_poll_task_run(void *pdata);

void j1939_poll_task_create(void)
{
    OSTaskCreate(j1939_poll_task_run, (void *) NULL,
            (OS_STK *) &g_j1939_poll_task_stack[J1939_POLL_TASK_STK_SIZE - 1], J1939_POLL_TASK_PRIO);
}

void j1939_poll_task_run(void *pdata)
{
  UNUSED(pdata);
  
  for(;;)
	{
    	J1939CanPoll(g_slaveCanContextPtr);
    	J1939CanPoll(g_chargerCanContextPtr);
#if BMS_SUPPORT_DTU == BMS_DTU_CAN
    	J1939CanPoll(g_dtuContext.can_cxt);
#endif
      if(mode == RUN_MODE_READ)
      {
          sleep(1);
      } else
      {
    	sleep(2);
	}
    	
	}
}

#endif

#endif
