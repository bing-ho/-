/**
  * @file       relay_fault_check.c
  * @brief      �̵������ϼ��
  *	@copyright	Ligoo Inc.
  *	@date       2017.03.17
  *	@author     Ligoo����߼���
  */
#include "relay_fault_check.h"
#include "bms_relay.h"
#include "bms_system_impl.h"

/***************************************************************
�ڲ����ݽṹ����
***************************************************************/

/**
  * @struct   RelayFaultCheckItem
  * @brief    �̵������ϼ��
  */
typedef struct
{
	RelayFaultCheckStatus status; //���״̬
	RelayFaultCheckParam check_param; //������
	INT32U check_tick; //���tick
	INT32U confirm_tick; //ȷ��tick
} RelayFaultCheckItem;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_LOGIC_CTRL
static RelayFaultCheckItem relay_adhesion_fault_check_item[BMS_RELAY_MAX_COUNT + 1]; //ճ��
static RelayFaultCheckItem relay_open_circuit_fault_check_item[BMS_RELAY_MAX_COUNT + 1]; //��·
#pragma DATA_SEG DEFAULT


/***************************************************************
����ʵ��
***************************************************************/
/**  
 *  @brief       �����̵������ϼ��
 *  @param[in]	 relay_id    �̵������
 *  @param[in]   check_param ������
 *  @return      ��
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

	if((check_param->confirm_time == 0) //����ȷ��ʱ��
			||(check_param->check_time == 0)//���ʱ��
			||(check_param->check_time < check_param->confirm_time)//���ʱ���ȹ���ȷ��ʱ��С
			||(check_param->cond_func ==NULL))//��⺯��Ϊ��
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
 *  @brief       ��ȡ�̵������ϼ��״̬,���ݼ̵�����ŷ�����Ӧ��״̬
 *  @param[in]	 relay_id �̵������
 *  @param[in]	 type �������
 *  @return      kRelayFaultCheck_Nomal ����,kRelayFaultCheck_Checking ����У�kRelayFaultCheck_Fault����
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
 *  @brief     �����̵������ϼ�����
 *  @param[in] relay_id �̵�����ţ�
 *  @param[in] type ������ͣ�
 *  @param[in] item�����ϸ����
 *  @return    ��
 */
static void relay_fault_check_diagnose(INT8U relay_id,RelayFaultCheckType type,RelayFaultCheckItem * far item)
{
	INT16U tick_diff = 0;
	OS_CPU_SR cpu_sr = 0;

	if(relay_id > BMS_RELAY_MAX_COUNT) //ID�ų���
		return;
	if (item == NULL)
		return;
	if(item[relay_id].status != kRelayFaultCheck_Checking)//���ڼ����
		return;

	if(type == kRelayFaultCheckAdhesion)//ճ�����
	{
		if(relay_is_on(relay_id))
		{ //�̵������ڱպϣ�����ճ�����
			item[relay_id].status = kRelayFaultCheck_Nomal;
			return;
		}
	}
	else //��·���
	{
		if(!relay_is_on(relay_id))
		{ //�̵������ڶϿ���������·���
			item[relay_id].status = kRelayFaultCheck_Nomal;
			return;
		}
	}
	OS_ENTER_CRITICAL();
	tick_diff = (INT16U)get_elapsed_tick_count(item[relay_id].check_tick);
	if(tick_diff > item[relay_id].check_param.check_time) //��ʱ
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

	if(item[relay_id].check_param.cond_func()) //�й���
	{
		tick_diff = (INT16U)get_elapsed_tick_count(item[relay_id].confirm_tick);
		if(tick_diff >= item[relay_id].check_param.confirm_time) //����ȷ��ʱ�䵽��
		{
			item[relay_id].status = kRelayFaultCheck_Fault; //����
		}
	}
	else
	{
		item[relay_id].confirm_tick = get_tick_count(); //������ʼʱ��
	}
	OS_EXIT_CRITICAL();
}
/**  
 *  @brief       �̵������ϼ������,�ú��������м̵������м�⣬��Ҫ���������ڵ��øú��������ڽ���С��50ms��
 *  @param[in]	 ��
 *  @return      ��
 */
void relay_fault_check_run(void)
{
	INT8U relay_index = 0;
	for (relay_index = 1; relay_index <= BMS_RELAY_MAX_COUNT; relay_index++)
	{
		relay_fault_check_diagnose(relay_index, kRelayFaultCheckAdhesion, relay_adhesion_fault_check_item); //ճ�����
		relay_fault_check_diagnose(relay_index, kRelayFaultCheckOpenCircuit, relay_open_circuit_fault_check_item); //��·���
	}
}
