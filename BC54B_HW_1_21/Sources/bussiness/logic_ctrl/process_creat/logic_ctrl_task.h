/**
  * @file       logic_ctrl_task.h
  * @brief      逻辑控制任务
  *	@copyright	Ligoo Inc.
  *	@date       2017.03.17
  *	@author     Ligoo软件逻辑组
  */
#ifndef	_LOGIC_CTRL_TASK_H_
#define	_LOGIC_CTRL_TASK_H_

#include "includes.h"
#include "bms_business_defs.h"
//#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
//extern RelayState discharge_relay_state_last_value1;
//extern RelayState charge_relay_state_last_value1;
//extern int flag;
//#pragma DATA_SEG DEFAULT
/**
  * @brief  初始化启动流程
  */
void logic_ctrl_init(void);



#endif