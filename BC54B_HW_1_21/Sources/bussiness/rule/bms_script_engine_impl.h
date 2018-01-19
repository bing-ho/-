/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_script_engine_impl.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-10
*
*/

#ifndef BMS_SCRIPT_ENGINE_IMPL_H_
#define BMS_SCRIPT_ENGINE_IMPL_H_
#include "bms_defs.h"
#include "bms_vm_engine.h"
#include "bms_script_engine.h"
#include "bms_config.h"
#include "bms_bcu.h"
#include "bms_system.h"
#include "bms_rule_stock.h"
#include "bms_relay.h"
#include "bms_hardware.h"
#include "bms_charger.h"
#include "bms_input_signal.h"
#include "bms_diagnosis_relay.h"

void script_engine_set_config(INT32U value, void* user_data);
void script_engine_get_config(INT32U* value, void* user_data);


Result script_engine_register_constants(void);
Result script_engine_register_variables(void);
Result script_engine_register_functions(void);

#endif /* BMS_SCRIPT_ENGINE_H_ */
