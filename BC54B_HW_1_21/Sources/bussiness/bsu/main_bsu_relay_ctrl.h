/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:mian_bsu_relay_ctrl.h
 **��������:2015.10.15
 **�ļ�˵��:���ɼ���̵�������
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef   _MIAN_BSU_RELAY_CTRL_H_ 
#define   _MIAN_BSU_RELAY_CTRL_H_

#include "includes.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

#define BSU_RELAY_MAX_COUNT  2  //�ӻ��̵���������
#define BSU_COOL_ANALYSE_DLY 5000//ms/bit
#define BSU_HEAT_ANALYSE_DLY 5000//ms/bit

//�̵�������map
typedef enum 
{
    kBsuRelayTypeUnknown = 0,
    kBsuRelayTypeCool,
    kBsuRelayTypeHeat,
    kBsuRelayTypeMaxCount,
}BsuRelayControlType;

//����
typedef enum 
{
   kNoNeedCool,
   kNeedCool
}BsuCoolStatus;

//����
typedef enum 
{
   kNoNeedHeat,
   kNeedHeat
}BsuHotStatus;

 //�̵���ǿ��״̬
typedef enum
{
	kBsuRelayForceNone,
	kBsuRelayForceOff,
    kBsuRelayForceOn
} BsuRelayForceStatus;

 //�̵���״̬
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
     INT8U relay_control_map[kBsuRelayTypeMaxCount]; //����map
     BsuRelayForceStatus relay_force_command[BSU_RELAY_MAX_COUNT + 1];//ǿ������
     BsuCoolStatus bsu_cool_status[BMU_MAX_SLAVE_COUNT];//����״̬
     INT32U bsu_cool_ticks[BMU_MAX_SLAVE_COUNT];//����tick
     BsuHotStatus bsu_heat_status[BMU_MAX_SLAVE_COUNT];//����״̬
     INT32U bsu_heat_ticks[BMU_MAX_SLAVE_COUNT];//����tick
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
#error "BSU_RELAY_MAX_COUNT ����С��3"
#endif

#endif
#endif