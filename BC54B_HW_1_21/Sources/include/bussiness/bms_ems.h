/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file bms_ems.h
* @brief 整车控制系统交互
* @note  
* @author 
* @date 2012-5-24  
*  
*/

#ifndef BMS_EMS_H_
#define BMS_EMS_H_

#include "app_cfg.h"
#include "bms_buffer.h"
#include "includes.h"
#include "bms_system.h"
#include "bms_util.h"
#include "bms_relay.h"
#include "bms_can.h"
#include "bms_config.h"

/**********************************************
 * Functions
 ***********************************************/
void ems_init(void);
void ems_uninit(void);
void detect_ems_init(void);
void detect_ems_uninit(void);

#endif /* BMS_EMS_H_ */
