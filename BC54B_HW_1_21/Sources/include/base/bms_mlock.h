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
Result chglock_config_set(ChgLock_ConfigType config); //设置电子锁参数配置
ChgLock_ConfigType chglock_config_get(void);//得到电子锁配置参数
Result ChargerLock_Set(INT8U set_state);//设置电子锁锁止还是解锁
ChgLock_StateType chglock_get_logic_state(void);//得到电子锁的逻辑状态值

#endif    /* BMS_MLOCK_H_ */