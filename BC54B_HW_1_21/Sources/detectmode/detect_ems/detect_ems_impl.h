/**  
*  
* Copyright (c) 2017 Ligoo Inc.  
*  
* @file detect_ems_impl.h
* @brief 
* @note  
* @author
* @date 2017-11-1  
*  
*/

#ifndef DETECT_EMS_IMPL_H_
#define DETECT_EMS_IMPL_H_
#include "bms_ems.h"


/**********************************************
 * Functions
 ***********************************************/
void detect_ems_task_create(void);

INT16U detect_ems_charger_continue_current_update(void);  //��ȡ���ɳ���������
INT16U detect_ems_discharge_continue_current_update(void); //��ȡ���ɳ����ŵ����
INT16U detect_ems_charge_current_update(void);  //��ȡ����ʱ������
INT16U detect_ems_discharge_current_update(void); //��ȡ����ʱ�ŵ����
AlarmLevel detect_ems_dchg_fault_level_update(void); //��ȡ�ŵ���ϵȼ�
AlarmLevel detect_ems_chg_fault_level_update(void); //��ȡ�����ϵȼ�
AlarmLevel detect_ems_bms_alarm_level_update(void); //��ȡBMS���ϵȼ�
INT8U detect_ems_get_Lv4_from_alarm(AlarmLevel alarm_level); //���ݹ��ϵȼ���ȡ������(0:kAlarmNone; 1:kAlarmFirstLevel; 2:kAlarmSecondLevel; 3:kAlarmThirdLevel; 4:kAlarmForthLevel)
INT8U detect_ems_get_Lv3_from_alarm(AlarmLevel alarm_level); //���ݹ��ϵȼ���ȡ������(0:kAlarmNone,kAlarmFirstLevel; 1:kAlarmSecondLevel; 2:kAlarmThirdLevel; 3:kAlarmForthLevel)
INT8U detect_ems_total_fault_num_update(void); //��ȡϵͳ�ܹ�����
AlarmLevel detect_ems_max_fault_level_update(void); //��ȡϵͳ��߹��ϵȼ�

//����������������ƺ���
INT8U detect_ems_power_control_command_is_enable(void);
void detect_ems_power_on(void);
void detect_ems_power_off(void);

//self-check
void detect_ems_task_self_check_run(void *pdata);

#endif /* DETECT_EMS_IMPL_H_ */
