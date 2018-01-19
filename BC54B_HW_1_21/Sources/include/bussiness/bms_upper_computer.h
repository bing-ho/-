/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file bms_upper_computer.h
* @brief 
* @note  
* @author 
* @date 2012-5-24  
*  
*/

#ifndef BMS_UPPER_COMPUTER_H_
#define BMS_UPPER_COMPUTER_H_

#include "bms_defs.h"
#include "app_cfg.h"
#include "bms_clock.h"
#include "bms_can.h"
#include "j1939_cfg.h"
#include "bms_bcu.h"
#include "bms_buffer.h"
#include "bms_rule_engine.h"
#include "bms_rule_engine_persistence.h"
#include "bms_vm_engine.h"
#include "bms_util.h"
#include "bms_relay.h"
#include "bms_soc.h"
#include "bms_charger_common.h"
#include "bms_input_signal.h"
#include "bms_byu.h"
#include "bms_charger_gb.h"
#include "bms_system_voltage.h"
#include "bms_board_temperature.h"
#include "bms_diagnosis_relay.h"
#include "bms_job.h"
#include "bms_data_save.h"

void upper_computer_init(void);
void upper_computer_uninit(void);

BOOLEAN upper_computer_is_connected(void);


#endif /* BMS_COMPUTER_H_ */
