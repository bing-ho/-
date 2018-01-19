/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:main_bsu_relay_ctrl.c
 **创建日期:2015.10.15
 **文件说明:主采集板继电器控制
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/

#include "includes.h"
#include "bsu_sample.h"
#include "bms_bsu.h"
#include "bms_bmu.h"
#include "bms_config.h"
#include "main_bsu_relay_ctrl.h"

#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

static BsuRelayItem bsu_relay_item[PARAM_BSU_NUM_MAX];//bit0  代表1号继电器状态； bit1  代表2号继电器状态；

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU_RELAY_CTRL 
static BsuTempCtrlStatus bsu_temp_ctrl_status;
#pragma DATA_SEG DEFAULT

/*****************************************************************************
 *函数名称:get_bsu_relay_item_ptr
 *函数功能:获取 bsu_relay_item地址指针
 *参    数:
 *返 回 值:
 *修订信息:
 ******************************************************************************/
BsuRelayItem *get_bsu_relay_item_ptr(void)
{
    return  bsu_relay_item;
}
/*****************************************************************************
 *函数名称:set_bsu_relay1_on
 *函数功能:控制bsu采集板1号继电器开启，硬件对应GPIO2引脚
 *参    数:bsu_index采集板编号
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_bsu_relay1_on(INT8U bsu_index)
{
	if (bsu_index >= get_bsu_num())
	{
		return;
	}
	if (get_bsu_n_board_type(bsu_index) != kBoardMain) //主采集板才有继电器
	{
		return;
	}
	bsu_relay_item[bsu_index].Bits.relay1 = kBsuRelayOn;
}
/*****************************************************************************
 *函数名称:set_bsu_relay1_off
 *函数功能:控制bsu采集板1号继电器关闭，硬件对应GPIO2引脚
 *参    数:bsu_index采集板编号
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_bsu_relay1_off(INT8U bsu_index)
{
	if (bsu_index >= get_bsu_num())
	{
		return;
	}
	if (get_bsu_n_board_type(bsu_index) != kBoardMain)//主采集板才有继电器
	{
		return;
	}
	bsu_relay_item[bsu_index].Bits.relay1 = kBsuRelayOff;
}
/*****************************************************************************
 *函数名称:get_bsu_relay1_status
 *函数功能:获取bsu采集板1号继电器状态，硬件对应GPIO2引脚
 *参    数:bsu_index采集板编号
 *返 回 值:1闭合 0断开
 *修订信息:
 ******************************************************************************/
BsuRelayStatus get_bsu_relay1_status(INT8U bsu_index)
{
	if (bsu_index >= get_bsu_num())
	{
		return 0;
	}
	if (get_bsu_n_board_type(bsu_index) != kBoardMain) //主采集板才有继电器
	{
		return 0;
	}
	return bsu_relay_item[bsu_index].Bits.relay1;
}
/*****************************************************************************
 *函数名称:set_bsu_relay2_on
 *函数功能:控制bsu采集板2号继电器开启，硬件对应GPIO1引脚
 *参    数:bsu_index采集板编号
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void set_bsu_relay2_on(INT8U bsu_index)
{
    if (bsu_index >= get_bsu_num())
    {
        return;
    }
	if (get_bsu_n_board_type(bsu_index) != kBoardMain)//主采集板才有继电器
    {
        return;
    }
    bsu_relay_item[bsu_index].Bits.relay2 = kBsuRelayOn;
}
/*****************************************************************************
 *函数名称:set_bsu_relay2_off
 *函数功能:控制bsu采集板2号继电器断开，硬件对应GPIO1引脚
 *参    数:bsu_index采集板编号
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_bsu_relay2_off(INT8U bsu_index)
{
    if (bsu_index >= get_bsu_num())
    {
        return;
    }
	if (get_bsu_n_board_type(bsu_index) != kBoardMain) //主采集板才有继电器
    {
        return;
    }
    bsu_relay_item[bsu_index].Bits.relay2 = kBsuRelayOff;
}
/*****************************************************************************
 *函数名称:get_bsu_relay2_status
 *函数功能:获取bsu采集板2号继电器状态，硬件对应GPIO1引脚
 *参    数:bsu_index采集板编号
 *返 回 值:1闭合 0断开
 *修订信息:
 ******************************************************************************/
BsuRelayStatus get_bsu_relay2_status(INT8U bsu_index)
{
	if (bsu_index >= get_bsu_num())
	{
		return 0;
	}
	if (get_bsu_n_board_type(bsu_index) != kBoardMain) //主采集板才有继电器
	{
		return 0;
	}
	return bsu_relay_item[bsu_index].Bits.relay2;
}
/*****************************************************************************
 *函数名称:bmu_set_bsu_relay
 *函数功能:设置从机继电器状态
 *参    数:slave_id从机号,relay_id继电器编号,status开关状态
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bmu_set_bsu_relay(INT8U slave_id,INT8U relay_id,BsuRelayStatus status)
{
    INT8U bsu_pos = 0;
    
    if(slave_id >= BMU_MAX_SLAVE_COUNT)
        return;	
    if ((relay_id == 0) || (relay_id > BSU_RELAY_MAX_COUNT))
		return;
    bsu_pos = get_main_bsu_pos(slave_id);//获取主采集板的编号
    if (bsu_pos == 0xFF)
		return;
    
    if (relay_id == 1) //1号继电器
	{
		if(status == kBsuRelayOff)
		    set_bsu_relay1_off(bsu_pos);
	    else
	        set_bsu_relay1_on(bsu_pos);
	}
	else //2号
	{
		if(status == kBsuRelayOff)
		    set_bsu_relay2_off(bsu_pos);
	    else
	        set_bsu_relay2_on(bsu_pos);
	}
}
/*****************************************************************************
 *函数名称:bmu_get_bsu_relay_status
 *函数功能:获取从机继电器状态
 *参    数:slave_id从机号,relay_id继电器编号
 *返 回 值:1闭合 0断开
 *修订信息:
 ******************************************************************************/
BsuRelayStatus bmu_get_bsu_relay_status(INT8U slave_id,INT8U relay_id)
{
    INT8U bsu_pos = 0;
    
    if(slave_id >= BMU_MAX_SLAVE_COUNT)
        return 0;	
    if ((relay_id == 0) || (relay_id > BSU_RELAY_MAX_COUNT))
		return 0;
    bsu_pos = get_main_bsu_pos(slave_id);//获取主采集板的编号
    if (bsu_pos == 0xFF)
		return 0;
    
    if (relay_id == 1) //1号继电器
	{
    	return get_bsu_relay1_status(bsu_pos);
	}
	else //2号
	{
		return get_bsu_relay2_status(bsu_pos);
	}
}
/*****************************************************************************
 *函数名称:get_main_bsu_relay_id_by_type
 *函数功能:通过类型获取继电器ID号
 *参    数:BsuRelayControlType type继电器类型
 *返 回 值:继电器ID号
 *修订信息:
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
 *函数名称:get_main_bsu_relay_force_cmd
 *函数功能:获取继电器强制控制命令
 *参    数:继电器ID号
 *返 回 值:强制控制状态命令
 *修订信息:
 ******************************************************************************/
BsuRelayForceStatus get_main_bsu_relay_force_cmd(INT8U id)
{
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;

	if ((id == 0) || (id > BSU_RELAY_MAX_COUNT))
		return kBsuRelayForceNone;

	return ctrl_status->relay_force_command[id];
}
/*****************************************************************************
 *函数名称:main_bsu_relay_force_control_on
 *函数功能:强制开启继电器
 *参    数:继电器ID号
 *返 回 值:无
 *修订信息:
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
 *函数名称:main_bsu_relay_force_control_off
 *函数功能:强制关闭继电器
 *参    数:继电器ID号
 *返 回 值:无
 *修订信息:
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
 *函数名称:main_bsu_relay_force_control_cancle
 *函数功能:取消强制控制继电器
 *参    数:继电器ID号
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void main_bsu_relay_force_control_cancle(INT8U id)
{
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;

	if ((id == 0) || (id > BSU_RELAY_MAX_COUNT))
		return;

	ctrl_status->relay_force_command[id] = kBsuRelayForceNone;
}

/*****************************************************************************
 *函数名称:main_bsu_relay_control_load_config
 *函数功能:加载继电器配置
 *参    数:无
 *返 回 值:无
 *修订信息:
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
 *函数名称:main_bsu_relay_control_on
 *函数功能:继电器闭合
 *参    数:slave_index从机号 type继电器类型
 *返 回 值:无
 *修订信息:
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
	{   //在没有强制控制条件下进行正常控制
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
 *函数名称:main_bsu_relay_control_off
 *函数功能:继电器断开
 *参    数:slave_index从机号 type继电器类型
 *返 回 值:无
 *修订信息:
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
	{   //在没有强制控制条件下进行正常控制
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
 *函数名称:bsu_cool_analyse
 *函数功能:制冷分析
 *参    数:void* data
 *返 回 值:无
 *修订信息:
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
			case kNoNeedCool://不需要制冷状态
				if (bmu_get_max_temp_from_bsu(index) >= config_get(kCoolOnTemperature))
				{   //温度达到制冷开启温度
					if (get_interval_by_tick(ctrl_status->bsu_cool_ticks[index], now_tick) >= BSU_COOL_ANALYSE_DLY)
					{   //维持5S，置状态为需要制冷
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
				{   //温度达到制冷关闭温度
					if (get_interval_by_tick(ctrl_status->bsu_cool_ticks[index], now_tick) >= BSU_COOL_ANALYSE_DLY)
					{   //维持5S，置状态为无需制冷
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
		//将需要开启制冷继电器状态传给主机
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
		//控制继电器    达到制冷状态且主机控制制冷标志使能
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
 *函数名称:bsu_hot_analyse
 *函数功能:制热分析
 *参    数:void* data
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_heat_analyse(void* data)
{
	INT8U index = 0;
	INT8U salve_num = 0;
	INT32U now_tick = OSTimeGet();
	BsuTempCtrlStatus *far ctrl_status = &bsu_temp_ctrl_status;
    (void)data;
    //从机数
	salve_num = (INT8U) config_get(kSlaveNumIndex);
	for (index = 0; index < salve_num; index++)
	{
		switch (ctrl_status->bsu_heat_status[index])
		{
			case kNoNeedHeat: //不需要加热
				if ((bmu_get_min_temp_from_bsu(index) != 0) && (bmu_get_min_temp_from_bsu(index) <= config_get(kHeatOnTemperature)))
				{    //最低温度达到加热条件
					if (get_interval_by_tick(ctrl_status->bsu_heat_ticks[index], now_tick) >= BSU_HEAT_ANALYSE_DLY)
					{   //维持5S，置需要加热状态
						ctrl_status->bsu_heat_ticks[index] = now_tick;
						ctrl_status->bsu_heat_status[index] = kNeedHeat;
					}
				}
				else
				{
					ctrl_status->bsu_heat_ticks[index] = now_tick;
				}
			break;
			case kNeedHeat://需要加热
				if ((bmu_get_min_temp_from_bsu(index) >= config_get(kHeatOffTemperature)) || (bmu_get_min_temp_from_bsu(index) == 0))
				{   //最低温度大于等于加热关闭条件
					if (get_interval_by_tick(ctrl_status->bsu_heat_ticks[index], now_tick) >= BSU_HEAT_ANALYSE_DLY)
					{   //维持5S，置不需要加热状态
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
		//将需要开启制热继电器状态传给主机
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
		//控制继电器   达到加热状态且主机控制加热标志使能
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
