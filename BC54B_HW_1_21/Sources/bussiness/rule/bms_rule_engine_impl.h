/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_rule_engine_impl.h
 * @brief
 * @note
 * @author Xiangyong Zhang
 * @date 2012-8-10
 *
 */

#ifndef BMS_RULE_ENGINE_IMPL_H_
#define BMS_RULE_ENGINE_IMPL_H_
#include "bms_defs.h"
#include "bms_rule_engine.h"  
#include "bms_vm_engine.h"
#include "app_cfg.h"
#include "bms_rule_engine_event_checker.h"
#include "bms_rule_engine_timer_checker.h"
#include "bms_script_engine.h"
#include "bms_rule_engine_persistence.h"
#include "bms_wdt.h"
#include "bms_rule_engine_default.h"

/** rule engine task */
void rule_engine_task_create(void);
void rule_engine_task_run(void* data);

/** rule executor */
Result rule_engine_load_config(void);

Result rule_engine_load_config_rule(void);
Result rule_engine_load_config_status(void);

//Result rule_engine_run_rule(RuleItem* item);
Result rule_engine_run_rule(RuleID id);

Result rule_engine_check_condition(RuleCondition* _PAGED condition);
Result rule_engine_execute_action(RuleAction* _PAGED action);



#endif /* BMS_RULE_ENGINE_H_ */
