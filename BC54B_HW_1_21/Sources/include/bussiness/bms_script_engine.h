/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_script_engine.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-10
*
*/

#ifndef BMS_SCRIPT_ENGINE_H_
#define BMS_SCRIPT_ENGINE_H_
#include "bms_defs.h"
#include "bms_vm_engine.h"


void script_engine_init(void);
void script_engine_uninit(void);

Result script_engine_run_expr(PCSTR expr, VmValue* value);


#endif /* BMS_SCRIPT_ENGINE_H_ */
