/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:main_bsu_relay_ctrl.c
 **��������:2015.10.15
 **�ļ�˵��:���ɼ���̵�������
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/

#include "includes.h"
#include "bsu_sample.h"
#include "bms_bsu.h"
#include "bms_bmu.h"
#include "bms_config.h"
#include "main_bsu_relay_ctrl.h"

#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

static BsuRelayItem bsu_relay_item[PARAM_BSU_NUM_MAX];//bit0  ����1�ż̵���״̬�� bit1  ����2�ż̵���״̬��

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU_RELAY_CTRL 
static BsuTempCtrlStatus bsu_temp_ctrl_status;
#pragma DATA_SEG DEFAULT

/*****************************************************************************
 *��������:get_bsu_relay_item_ptr
 *��������:��ȡ bsu_relay_item��ַָ��
 *��    ��:
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
BsuRelayItem *get_bsu_relay_item_ptr(void)
{
    return  bsu_relay_item;
}
/*****************************************************************************
 *��������:set_bsu_relay1_on
 *��������:����bsu�ɼ���1�ż̵���������Ӳ����ӦGPIO2����
 *��    ��:bsu_index�ɼ�����
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
void set_bsu_relay1_on(INT8U bsu_index)
{
	if (bsu_index >= get_bsu_num())
	{
		return;
	}
	if (get_bsu_n_board_type(bsu_index) != kBoardMain) //���ɼ�����м̵���
	{
		return;
	}
	bsu_relay_item[bsu_index].Bits.relay1 = kBsuRelayOn;
}
/*****************************************************************************
 *��������:set_bsu_relay1_off
 *��������:����bsu�ɼ���1�ż̵����رգ�Ӳ����ӦGPIO2����
 *��    ��:bsu_index�ɼ�����
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
void set_bsu_relay1_off(INT8U bsu_index)
{
	if (bsu_index >= get_bsu_num())
	{
		return;
	}
	if (get_bsu_n_board_type(bsu_index) != kBoardMain)//���ɼ�����м̵���
	{
		return;
	}
	bsu_relay_item[bsu_index].Bits.relay1 = kBsuRelayOff;
}
/*****************************************************************************
 *��������:get_bsu_relay1_status
 *��������:��ȡbsu�ɼ���1�ż̵���״̬��Ӳ����ӦGPIO2����
 *��    ��:bsu_index�ɼ�����
 *�� �� ֵ:1�պ� 0�Ͽ�
 *�޶���Ϣ:
 ******************************************************************************/
BsuRelayStatus get_bsu_relay1_status(INT8U bsu_index)
{
	if (bsu_index >= get_bsu_num())
	{
		return 0;
	}
	if (get_bsu_n_board_type(bsu_index) != kBoardMain) //���ɼ�����м̵���
	{
		return 0;
	}
	return bsu_relay_item[bsu_index].Bits.relay1;
}
/*****************************************************************************
 *��������:set_bsu_relay2_on
 *��������:����bsu�ɼ���2�ż̵���������Ӳ����ӦGPIO1����
 *��    ��:bsu_index�ɼ�����
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void set_bsu_relay2_on(INT8U bsu_index)
{
    if (bsu_index >= get_bsu_num())
    {
        return;
    }
	if (get_bsu_n_board_type(bsu_index) != kBoardMain)//���ɼ�����м̵���
    {
        return;
    }
    bsu_relay_item[bsu_index].Bits.relay2 = kBsuRelayOn;
}
/*****************************************************************************
 *��������:set_bsu_relay2_off
 *��������:����bsu�ɼ���2�ż̵����Ͽ���Ӳ����ӦGPIO1����
 *��    ��:bsu_index�ɼ�����
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
void set_bsu_relay2_off(INT8U bsu_index)
{
    if (bsu_index >= get_bsu_num())
    {
        return;
    }
	if (get_bsu_n_board_type(bsu_index) != kBoardMain) //���ɼ�����м̵���
    {
        return;
    }
    bsu_relay_item[bsu_index].Bits.relay2 = kBsuRelayOff;
}
/*****************************************************************************
 *��������:get_bsu_relay2_status
 *��������:��ȡbsu�ɼ���2�ż̵���״̬��Ӳ����ӦGPIO1����
 *��    ��:bsu_index�ɼ�����
 *�� �� ֵ:1�պ� 0�Ͽ�
 *�޶���Ϣ:
 ******************************************************************************/
BsuRelayStatus get_bsu_relay2_status(INT8U bsu_index)
{
	if (bsu_index >= get_bsu_num())
	{
		return 0;
	}
	if (get_bsu_n_board_type(bsu_index) != kBoardMain) //���ɼ�����м̵���
	{
		return 0;
	}
	return bsu_relay_item[bsu_index].Bits.relay2;
}
/*****************************************************************************
 *��������:bmu_set_bsu_relay
 *��������:���ôӻ��̵���״̬
 *��    ��:slave_id�ӻ���,relay_id�̵������,status����״̬
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bmu_set_bsu_relay(INT8U slave_id,INT8U relay_id,BsuRelayStatus status)
{
    INT8U bsu_pos = 0;
    
    if(slave_id >= BMU_MAX_SLAVE_COUNT)
        return;	
    if ((relay_id == 0) || (relay_id > BSU_RELAY_MAX_COUNT))
		return;
    bsu_pos = get_main_bsu_pos(slave_id);//��ȡ���ɼ���ı��
    if (bsu_pos == 0xFF)
		return;
    
    if (relay_id == 1) //1�ż̵���
	{
		if(status == kBsuRelayOff)
		    set_bsu_relay1_off(bsu_pos);
	    else
	        set_bsu_relay1_on(bsu_pos);
	}
	else //2��
	{
		if(status == kBsuRelayOff)
		    set_bsu_relay2_off(bsu_pos);
	    else
	        set_bsu_relay2_on(bsu_pos);
	}
}
/*****************************************************************************
 *��������:bmu_get_bsu_relay_status
 *��������:��ȡ�ӻ��̵���״̬
 *��    ��:slave_id�ӻ���,relay_id�̵������
 *�� �� ֵ:1�պ� 0�Ͽ�
 *�޶���Ϣ:
 ******************************************************************************/
BsuRelayStatus bmu_get_bsu_relay_status(INT8U slave_id,INT8U relay_id)
{
    INT8U bsu_pos = 0;
    
    if(slave_id >= BMU_MAX_SLAVE_COUNT)
        return 0;	
    if ((relay_id == 0) || (relay_id > BSU_RELAY_MAX_COUNT))
		return 0;
    bsu_pos = get_main_bsu_pos(slave_id);//��ȡ���ɼ���ı��
    if (bsu_pos == 0xFF)
		return 0;
    
    if (relay_id == 1) //1�ż̵���
	{
    	return get_bsu_relay1_status(bsu_pos);
	}
	else //2��
	{
		return get_bsu_relay2_status(bsu_pos);
	}
}
/*****************************************************************************
 *��������:get_main_bsu_relay_id_by_type
 *��������:ͨ�����ͻ�ȡ�̵���ID��
 *��    ��:BsuRelayControlType type�̵�������
 *�� �� ֵ:�̵���ID��
 *�޶���Ϣ:
 ******************************************************************************/
INT8U get_main_bsu_relay_id_by_type(BsuRelayControlType type)
{
	INT8U id = 0;
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;
	
	if (type <= kBsuRelayTypeUnknown || type >= kBsuRelayTypeMaxCount)
		return 0;

	id = ctrl_status->relay_control_map[type];

	if (id > BSU_RELAY_MAX_COUNT)
		return 0;

	return id;
}
/*****************************************************************************
 *��������:get_main_bsu_relay_force_cmd
 *��������:��ȡ�̵���ǿ�ƿ�������
 *��    ��:�̵���ID��
 *�� �� ֵ:ǿ�ƿ���״̬����
 *�޶���Ϣ:
 ******************************************************************************/
BsuRelayForceStatus get_main_bsu_relay_force_cmd(INT8U id)
{
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;

	if ((id == 0) || (id > BSU_RELAY_MAX_COUNT))
		return kBsuRelayForceNone;

	return ctrl_status->relay_force_command[id];
}
/*****************************************************************************
 *��������:main_bsu_relay_force_control_on
 *��������:ǿ�ƿ����̵���
 *��    ��:�̵���ID��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void main_bsu_relay_force_control_on(INT8U id)
{
	INT8U slave_index = 0;
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;
	if ((id == 0) || (id > BSU_RELAY_MAX_COUNT))
		return;
	ctrl_status->relay_force_command[id] = kBsuRelayForceOn;
	for (slave_index = 0; slave_index < BMU_MAX_SLAVE_COUNT; slave_index++)
	{
		bmu_set_bsu_relay(slave_index,id,kBsuRelayOn);
	}
}
/*****************************************************************************
 *��������:main_bsu_relay_force_control_off
 *��������:ǿ�ƹرռ̵���
 *��    ��:�̵���ID��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void main_bsu_relay_force_control_off(INT8U id)
{
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;
	INT8U slave_index = 0;
	if ((id == 0) || (id > BSU_RELAY_MAX_COUNT))
		return;

	ctrl_status->relay_force_command[id] = kBsuRelayForceOff;
	for (slave_index = 0; slave_index < BMU_MAX_SLAVE_COUNT; slave_index++)
	{
	   bmu_set_bsu_relay(slave_index,id,kBsuRelayOff);
	}
}
/*****************************************************************************
 *��������:main_bsu_relay_force_control_cancle
 *��������:ȡ��ǿ�ƿ��Ƽ̵���
 *��    ��:�̵���ID��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void main_bsu_relay_force_control_cancle(INT8U id)
{
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;

	if ((id == 0) || (id > BSU_RELAY_MAX_COUNT))
		return;

	ctrl_status->relay_force_command[id] = kBsuRelayForceNone;
}

/*****************************************************************************
 *��������:main_bsu_relay_control_load_config
 *��������:���ؼ̵�������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void main_bsu_relay_control_load_config(void)
{
	INT8U type;
	ConfigIndex index;
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;
	for (index = kBsuRelayCfgTypeStartIndex; index <= kBsuRelayCfgTypeEndIndex; ++index)
	{
		type = (INT8U) config_get(index);
		if (type <= 0 || type >= kBsuRelayTypeMaxCount) continue;

		ctrl_status->relay_control_map[type] = (INT8U)(index - kBsuRelayCfgTypeStartIndex + 1);
	}
}

/*****************************************************************************
 *��������:main_bsu_relay_control_on
 *��������:�̵����պ�
 *��    ��:slave_index�ӻ��� type�̵�������
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void main_bsu_relay_control_on(INT8U slave_index, RelayControlType type)
{
	INT8U id = 0xFF;
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;
	if (type <= kBsuRelayTypeUnknown || type >= kBsuRelayTypeMaxCount)
		return;
    if(slave_index >= BMU_MAX_SLAVE_COUNT)
        return;
	id = ctrl_status->relay_control_map[type];

	if ((id == 0) || (id > BSU_RELAY_MAX_COUNT))
		return;

	if (ctrl_status->relay_force_command[id] == kBsuRelayForceNone)
	{   //��û��ǿ�ƿ��������½�����������
		bmu_set_bsu_relay(slave_index,id,kBsuRelayOn);
	}
	else if(ctrl_status->relay_force_command[id] == kBsuRelayForceOn)
	{
	    bmu_set_bsu_relay(slave_index,id,kBsuRelayOn);
	}
	else if(ctrl_status->relay_force_command[id] == kBsuRelayForceOff)
	{
	    bmu_set_bsu_relay(slave_index,id,kBsuRelayOff);
	}
}
/*****************************************************************************
 *��������:main_bsu_relay_control_off
 *��������:�̵����Ͽ�
 *��    ��:slave_index�ӻ��� type�̵�������
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void main_bsu_relay_control_off(INT8U slave_index, RelayControlType type)
{
	INT8U id = 0xFF;
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;

	if (type < kBsuRelayTypeUnknown || type >= kBsuRelayTypeMaxCount)
		return;
    if(slave_index >= BMU_MAX_SLAVE_COUNT)
        return;
    
	id = ctrl_status->relay_control_map[type];

	if ((id == 0) || (id > BSU_RELAY_MAX_COUNT))
		return;

	if (ctrl_status->relay_force_command[id] == kBsuRelayForceNone)
	{   //��û��ǿ�ƿ��������½�����������
		bmu_set_bsu_relay(slave_index,id,kBsuRelayOff);
	}
	else if(ctrl_status->relay_force_command[id] == kBsuRelayForceOn)
	{
	    bmu_set_bsu_relay(slave_index,id,kBsuRelayOn);
	}
	else if(ctrl_status->relay_force_command[id] == kBsuRelayForceOff)
	{
	    bmu_set_bsu_relay(slave_index,id,kBsuRelayOff);
	}
}
/*****************************************************************************
 *��������:bsu_cool_analyse
 *��������:�������
 *��    ��:void* data
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_cool_analyse(void* data)
{
	INT8U index = 0;
	INT8U salve_num = 0;
	INT32U now_tick = OSTimeGet();
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;
    (void)data;
    
	salve_num = (INT8U) config_get(kSlaveNumIndex);
	for (index = 0; index < salve_num; index++)
	{
		switch (ctrl_status->bsu_cool_status[index])
		{
			case kNoNeedCool://����Ҫ����״̬
				if (bmu_get_max_temp_from_bsu(index) >= config_get(kCoolOnTemperature))
				{   //�¶ȴﵽ���俪���¶�
					if (get_interval_by_tick(ctrl_status->bsu_cool_ticks[index], now_tick) >= BSU_COOL_ANALYSE_DLY)
					{   //ά��5S����״̬Ϊ��Ҫ����
						ctrl_status->bsu_cool_ticks[index] = now_tick;
						ctrl_status->bsu_cool_status[index] = kNeedCool;
					}
				}
				else
				{
					ctrl_status->bsu_cool_ticks[index] = now_tick;
				}
			break;
			case kNeedCool:
				if (bmu_get_max_temp_from_bsu(index) <= config_get(kCoolOffTemperature))
				{   //�¶ȴﵽ����ر��¶�
					if (get_interval_by_tick(ctrl_status->bsu_cool_ticks[index], now_tick) >= BSU_COOL_ANALYSE_DLY)
					{   //ά��5S����״̬Ϊ��������
						ctrl_status->bsu_cool_ticks[index] = now_tick;
						ctrl_status->bsu_cool_status[index] = kNoNeedCool;
					}
				}
				else
				{
					ctrl_status->bsu_cool_ticks[index] = now_tick;
				}
			break;
			default:
				ctrl_status->bsu_cool_ticks[index] = now_tick;
				ctrl_status->bsu_cool_status[index] = kNoNeedCool;
			break;
		}
		//����Ҫ��������̵���״̬��������
		if (ctrl_status->bsu_cool_status[index] == kNeedCool)
		{
			bmu_set_cool_status(index, 1);
		}
		else
		{
			bmu_set_cool_status(index, 0);
		}
		#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN
            bmu_slave_cool_control_update(index, 0);
        #endif
		//���Ƽ̵���    �ﵽ����״̬���������������־ʹ��
		if ((ctrl_status->bsu_cool_status[index] == kNeedCool) 
		#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN
		    && (bmu_cool_control_is_on(index))
		#endif
		)
		{
			main_bsu_relay_control_on(index, kBsuRelayTypeCool);
		} 
		else 
		{
		    main_bsu_relay_control_off(index, kBsuRelayTypeCool);
		}
	}
}
/*****************************************************************************
 *��������:bsu_hot_analyse
 *��������:���ȷ���
 *��    ��:void* data
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_heat_analyse(void* data)
{
	INT8U index = 0;
	INT8U salve_num = 0;
	INT32U now_tick = OSTimeGet();
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;
    (void)data;
    //�ӻ���
	salve_num = (INT8U) config_get(kSlaveNumIndex);
	for (index = 0; index < salve_num; index++)
	{
		switch (ctrl_status->bsu_heat_status[index])
		{
			case kNoNeedHeat: //����Ҫ����
				if ((bmu_get_min_temp_from_bsu(index) != 0) && (bmu_get_min_temp_from_bsu(index) <= config_get(kHeatOnTemperature)))
				{    //����¶ȴﵽ��������
					if (get_interval_by_tick(ctrl_status->bsu_heat_ticks[index], now_tick) >= BSU_HEAT_ANALYSE_DLY)
					{   //ά��5S������Ҫ����״̬
						ctrl_status->bsu_heat_ticks[index] = now_tick;
						ctrl_status->bsu_heat_status[index] = kNeedHeat;
					}
				}
				else
				{
					ctrl_status->bsu_heat_ticks[index] = now_tick;
				}
			break;
			case kNeedHeat://��Ҫ����
				if ((bmu_get_min_temp_from_bsu(index) >= config_get(kHeatOffTemperature)) || (bmu_get_min_temp_from_bsu(index) == 0))
				{   //����¶ȴ��ڵ��ڼ��ȹر�����
					if (get_interval_by_tick(ctrl_status->bsu_heat_ticks[index], now_tick) >= BSU_HEAT_ANALYSE_DLY)
					{   //ά��5S���ò���Ҫ����״̬
						ctrl_status->bsu_heat_ticks[index] = now_tick;
						ctrl_status->bsu_heat_status[index] = kNoNeedHeat;
					}
				}
				else
				{
					ctrl_status->bsu_heat_ticks[index] = now_tick;
				}
			break;
			default:
				ctrl_status->bsu_heat_ticks[index] = now_tick;
				ctrl_status->bsu_heat_status[index] = kNoNeedHeat;
			break;
		}
		//����Ҫ�������ȼ̵���״̬��������
		if (ctrl_status->bsu_heat_status[index] == kNeedHeat)
		{
			bmu_set_heat_status(index, 1);
		}
		else
		{
			bmu_set_heat_status(index, 0);
		}
		#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN
            bmu_slave_heat_control_update(index, 0);
        #endif
		//���Ƽ̵���   �ﵽ����״̬���������Ƽ��ȱ�־ʹ��
		if ((ctrl_status->bsu_heat_status[index] == kNeedHeat) 
		#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN
		    && (bmu_heat_control_is_on(index))
		#endif
		)
		{
			main_bsu_relay_control_on(index, kBsuRelayTypeHeat);
		} 
		else 
		{
			main_bsu_relay_control_off(index, kBsuRelayTypeHeat);
		}
	}
}

#endif
