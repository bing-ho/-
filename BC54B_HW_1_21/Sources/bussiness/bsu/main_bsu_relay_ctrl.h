/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:mian_bsu_relay_ctrl.h
 **创建日期:2015.10.15
 **文件说明:主采集板继电器控制
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef   _MIAN_BSU_RELAY_CTRL_H_ 
#define   _MIAN_BSU_RELAY_CTRL_H_

#include "includes.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

#define BSU_RELAY_MAX_COUNT  2  //从机继电器最大个数
#define BSU_COOL_ANALYSE_DLY 5000//ms/bit
#define BSU_HEAT_ANALYSE_DLY 5000//ms/bit

//继电器功能map
typedef enum 
{
    kBsuRelayTypeUnknown = 0,
    kBsuRelayTypeCool,
    kBsuRelayTypeHeat,
    kBsuRelayTypeMaxCount,
}BsuRelayControlType;

//制冷
typedef enum 
{
   kNoNeedCool,
   kNeedCool
}BsuCoolStatus;

//制热
typedef enum 
{
   kNoNeedHeat,
   kNeedHeat
}BsuHotStatus;

 //继电器强控状态
typedef enum
{
	kBsuRelayForceNone,
	kBsuRelayForceOff,
    kBsuRelayForceOn
} BsuRelayForceStatus;

 //继电器状态
typedef enum
{
	kBsuRelayOff,
    kBsuRelayOn
} BsuRelayStatus;

typedef union
{
	INT8U value;
	struct
	{
		INT8U relay1 :1;
		INT8U relay2 :1; 
		INT8U NA :6;
	} Bits;
} BsuRelayItem;

typedef struct 
{
     INT8U relay_control_map[kBsuRelayTypeMaxCount]; //功能map
     BsuRelayForceStatus relay_force_command[BSU_RELAY_MAX_COUNT + 1];//强控命令
     BsuCoolStatus bsu_cool_status[BMU_MAX_SLAVE_COUNT];//制冷状态
     INT32U bsu_cool_ticks[BMU_MAX_SLAVE_COUNT];//制冷tick
     BsuHotStatus bsu_heat_status[BMU_MAX_SLAVE_COUNT];//制热状态
     INT32U bsu_heat_ticks[BMU_MAX_SLAVE_COUNT];//制热tick
 }BsuTempCtrlStatus;
 
 
void main_bsu_relay_control_load_config(void);
void bsu_cool_analyse(void* data);
void bsu_heat_analyse(void* data);
BsuRelayForceStatus get_main_bsu_relay_force_cmd(INT8U id);
void main_bsu_relay_force_control_on(INT8U id);
void main_bsu_relay_force_control_off(INT8U id);
void main_bsu_relay_force_control_cancle(INT8U id);
BsuRelayStatus bmu_get_bsu_relay_status(INT8U slave_id,INT8U relay_id);
BsuRelayStatus get_bsu_relay1_status(INT8U bsu_index);
BsuRelayStatus get_bsu_relay2_status(INT8U bsu_index);
BsuRelayItem *get_bsu_relay_item_ptr(void);

#if BSU_RELAY_MAX_COUNT >= 3
#error "BSU_RELAY_MAX_COUNT 必须小于3"
#endif

#endif
#endif