/**
  * @file       logic_ctrl_task.c
  * @brief      逻辑控制任务
  *	@copyright	Ligoo Inc.
  *	@date       2017.03.17
  *	@author     Ligoo软件逻辑组
  */
#include "process_mgmt.h"
#include "bms_system.h"
#include "relay_fault_check.h"
#include "logic_ctrl_task.h"
#include "logic_ctrl_process_start.h"
#include "bms_rule_stock.h"
#include "rule_check.h"

/*#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
RelayState discharge_relay_state_last_value1 = kRelayStatusOff;
RelayState charge_relay_state_last_value1 = kRelayStatusOff;

int flag=0;
#pragma DATA_SEG DEFAULT */

#define  LOGIC_CTRL_TASK_PRODIC         50//任务周期

static OS_STK logic_control_task_stack[RULE_ENGINE_STK_SIZE];
/**  
 *  @brief       逻辑任务控制函数
 *  @param[in]	 pdata    任务参数
 *  @return      无
 */
static void logic_ctrl_task(void* pdata);

/**
  * @brief  创建逻辑控制任务
  */
void logic_ctrl_task_create(void)
{
    (void)OSTaskCreate(logic_ctrl_task, (void *) NULL,
                (OS_STK *) &logic_control_task_stack[RULE_ENGINE_STK_SIZE - 1], RULE_ENGINE_TASK_PRIO);

}


/**
  * @brief  初始化启动流程
  */
void logic_ctrl_init(void)
{
    process_mgmt_init();
    ctrl_mgmt_init();
    start_all_logic_ctrl_process();

    logic_ctrl_task_create();
}
/**  
 *  @brief       任务休眠，用于任务周期控制
 *  @param[in]	 start_tick 任务起始时刻
 *  @param[in]	 task_prodic 任务周期 
 *  @return      无
 */
static void task_sleep(INT32U start_tick, INT16U task_prodic)
{
	INT32U tick_diff = 0;
	tick_diff = get_elapsed_tick_count(start_tick);
	if (tick_diff < task_prodic)
	{
		sleep(task_prodic - tick_diff);
	}
	else
	{
		sleep(5);
	}
}


/*
1?slave communication abort?comm??????????
RuleTrigger(Type: kRuleTriggerTypeEvent,Timer_trigger_interval: 0,Event_trigger_event_id: 50)
?1	??
$comm_state == ALARM_NORMAL&&disable_alarm(COMM_ALARM);
$comm_state == ALARM_NORMAL&&enable_charger(COMM_ALARM);
$comm_state == FIRST_ALARM&&enable_alarm(COMM_ALARM);
$comm_state == FIRST_ALARM&&disable_charger(COMM_ALARM);

void  slave_communication_abort_comm_rule(void) 
{
  if(bcu_get_slave_communication_state() == kAlarmNone) 
  {
    bcu_alarm_disable(SELFCHECK_COMM);
    bcu_charger_enable(SELFCHECK_COMM);
  }
  else if(bcu_get_slave_communication_state() == kAlarmFirstLevel)
  {
    bcu_alarm_enable(SELFCHECK_COMM);
    bcu_charger_disable(SELFCHECK_COMM);
  }
}
*/


/**  
 *  @brief       逻辑任务控制函数
 *  @param[in]	 pdata    任务参数
 *  @return      无
 */
static void logic_ctrl_task(void* pdata)
{
    INT8U timeout = 0;
    void* p_arg = pdata;

    INT32U start_tick = 0;//任务起始时刻
    do
    {
        OSTimeDly(50);
        //watch_dog_feed(WDT_RULE_ID);
        timeout++;
        if(timeout >= 20) break;              //超时1S
    }while((bcu_get_chg_start_failure_state() == kStartFailureOccure)||(bcu_get_dchg_start_failure_state() == kStartFailureOccure));
    for(;;)
    {
    
       start_tick = OSTimeGet();
       rule_check();
       ctrl_process_mgmt_run();
       basic_logic_control_run();
       relay_fault_check_run();
       
       task_sleep(start_tick, LOGIC_CTRL_TASK_PRODIC);
    }
}
