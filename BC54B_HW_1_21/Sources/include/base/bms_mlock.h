                                     /**  
*  
* Copyright (c) 2017 Ligoo Inc.  
*  
* @file bms_mlock
* @brief 
* @note  
* @author 
* @date 2017-4-26  
*  
*/

#ifndef BMS_MLOCK_H_
#define BMS_MLOCK_H_

#include "bms_mlock_impl.h"

/**********************************************
 * Functions
 ***********************************************/
Result chglock_config_set(ChgLock_ConfigType config); //���õ�������������
ChgLock_ConfigType chglock_config_get(void);//�õ����������ò���
Result ChargerLock_Set(INT8U set_state);//���õ�������ֹ���ǽ���
ChgLock_StateType chglock_get_logic_state(void);//�õ����������߼�״ֵ̬

#endif    /* BMS_MLOCK_H_ */