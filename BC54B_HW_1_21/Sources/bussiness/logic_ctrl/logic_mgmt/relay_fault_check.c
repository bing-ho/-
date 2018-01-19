/**
  * @file       relay_fault_check.c
  * @brief      继电器故障检测
  *	@copyright	Ligoo Inc.
  *	@date       2017.03.17
  *	@author     Ligoo软件逻辑组
  */
#include "relay_fault_check.h"
#include "bms_relay.h"
#include "bms_system_impl.h"

/***************************************************************
内部数据结构定义
***************************************************************/

/**
  * @struct   RelayFaultCheckItem
  * @brief    继电器故障检测
  */
typedef struct
{
	RelayFaultCheckStatus status; //检测状态
	RelayFaultCheckParam check_param; //检测参数
	INT32U check_tick; //检测tick
	INT32U confirm_tick; //确认tick
} RelayFaultCheckItem;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_LOGIC_CTRL
static RelayFaultCheckItem relay_adhesion_fault_check_item[BMS_RELAY_MAX_COUNT + 1]; //粘连
static RelayFaultCheckItem relay_open_circuit_fault_check_item[BMS_RELAY_MAX_COUNT + 1]; //开路
#pragma DATA_SEG DEFAULT


/***************************************************************
函数实现
***************************************************************/
/**  
 *  @brief       启动继电器故障检测
 *  @param[in]	 relay_id    继电器编号
 *  @param[in]   check_param 检测参数
 *  @return      无
 */
void start_relay_fault_check(INT8U relay_id,RelayFaultCheckParam *far check_param)
{
	OS_CPU_SR cpu_sr = 0;
	RelayFaultCheckItem *far item = NULL;
	if(relay_id > BMS_RELAY_MAX_COUNT)
		return;
	if(check_param == NULL)
		return;
	if(check_param->type == kRelayFaultCheckAdhesion)
	{
		item = relay_adhesion_fault_check_item;
	}
	else
	{
		item = relay_open_circuit_fault_check_item;
	}

	if((check_param->confirm_time == 0) //故障确认时间
			||(check_param->check_time == 0)//检测时长
			||(check_param->check_time < check_param->confirm_time)//检测时长比故障确认时间小
			||(check_param->cond_func ==NULL))//检测函数为空
	{
		return;
	}
	OS_ENTER_CRITICAL();
	item[relay_id].status = kRelayFaultCheck_Checking;
	item[relay_id].check_param.cond_func = check_param->cond_func;
	item[relay_id].check_param.confirm_time = check_param->confirm_time;
	item[relay_id].confirm_tick = get_tick_count();
	item[relay_id].check_param.check_time = check_param->check_time;
	item[relay_id].check_tick = get_tick_count();
	OS_EXIT_CRITICAL();
}
/**  
 *  @brief       获取继电器故障检测状态,根据继电器编号返回相应的状态
 *  @param[in]	 relay_id 继电器编号
 *  @param[in]	 type 检测类型
 *  @return      kRelayFaultCheck_Nomal 正常,kRelayFaultCheck_Checking 检测中，kRelayFaultCheck_Fault故障
 *  @note    
 */
RelayFaultCheckStatus get_relay_fault_status(INT8U relay_id, RelayFaultCheckType type)
{
	RelayFaultCheckItem * far
	item = NULL;

	if (relay_id > BMS_RELAY_MAX_COUNT)
		return kRelayFaultCheck_Nomal;

	if (type == kRelayFaultCheckAdhesion)
	{
		item = relay_adhesion_fault_check_item;
	}
	else
	{
		item = relay_open_circuit_fault_check_item;
	}

	return item[relay_id].status;
}

/**  
 *  @brief     单个继电器故障检测诊断
 *  @param[in] relay_id 继电器编号，
 *  @param[in] type 检测类型，
 *  @param[in] item检测详细内容
 *  @return    无
 */
static void relay_fault_check_diagnose(INT8U relay_id,RelayFaultCheckType type,RelayFaultCheckItem * far item)
{
	INT16U tick_diff = 0;
	OS_CPU_SR cpu_sr = 0;

	if(relay_id > BMS_RELAY_MAX_COUNT) //ID号超限
		return;
	if (item == NULL)
		return;
	if(item[relay_id].status != kRelayFaultCheck_Checking)//不在检测中
		return;

	if(type == kRelayFaultCheckAdhesion)//粘连检测
	{
		if(relay_is_on(relay_id))
		{ //继电器处于闭合，不做粘连检测
			item[relay_id].status = kRelayFaultCheck_Nomal;
			return;
		}
	}
	else //开路检测
	{
		if(!relay_is_on(relay_id))
		{ //继电器处于断开，不做开路检测
			item[relay_id].status = kRelayFaultCheck_Nomal;
			return;
		}
	}
	OS_ENTER_CRITICAL();
	tick_diff = (INT16U)get_elapsed_tick_count(item[relay_id].check_tick);
	if(tick_diff > item[relay_id].check_param.check_time) //超时
	{
		item[relay_id].status = kRelayFaultCheck_Nomal;
		OS_EXIT_CRITICAL();
		return;
	}
	if(item[relay_id].check_param.cond_func == NULL)
	{
		item[relay_id].status = kRelayFaultCheck_Nomal;
		OS_EXIT_CRITICAL();
		return;
	}

	if(item[relay_id].check_param.cond_func()) //有故障
	{
		tick_diff = (INT16U)get_elapsed_tick_count(item[relay_id].confirm_tick);
		if(tick_diff >= item[relay_id].check_param.confirm_time) //故障确认时间到达
		{
			item[relay_id].status = kRelayFaultCheck_Fault; //故障
		}
	}
	else
	{
		item[relay_id].confirm_tick = get_tick_count(); //设置起始时间
	}
	OS_EXIT_CRITICAL();
}
/**  
 *  @brief       继电器故障检测运行,该函数对所有继电器进行检测，需要无条件周期调用该函数，周期建议小于50ms。
 *  @param[in]	 无
 *  @return      无
 */
void relay_fault_check_run(void)
{
	INT8U relay_index = 0;
	for (relay_index = 1; relay_index <= BMS_RELAY_MAX_COUNT; relay_index++)
	{
		relay_fault_check_diagnose(relay_index, kRelayFaultCheckAdhesion, relay_adhesion_fault_check_item); //粘连检测
		relay_fault_check_diagnose(relay_index, kRelayFaultCheckOpenCircuit, relay_open_circuit_fault_check_item); //开路检测
	}
}
