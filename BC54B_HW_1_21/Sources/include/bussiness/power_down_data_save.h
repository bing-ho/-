/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file power_down_data_save
* @brief 
* @note  
* @author 
* @date 2012-5-24  
*  
*/

#ifndef POWER_DOWN_DATA_SAVE_H_
#define POWER_DOWN_DATA_SAVE_H_

#include "app_cfg.h"
#include "bms_buffer.h"
#include "includes.h"
#include "bms_system.h"
#include "bms_util.h"

/**********************************************
 * Functions
 ***********************************************/
void power_down_data_save_init(void);
char power_down_store_data(void);
char power_down_restore_left_cap(void);
void power_down_eeeprom_store_valid_check(void);

#endif /* POWER_DOWN_DATA_SAVE_H_ */