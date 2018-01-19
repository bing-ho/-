/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_rule_engine_timer_checker.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-14
*
*/

#ifndef BMS_RULE_ENGINE_TIMER_CHECKER_H_
#define BMS_RULE_ENGINE_TIMER_CHECKER_H_
#include "bms_defs.h"
#include "bms_rule_engine_impl.h"
#include "bms_log.h"
#include "bms_system.h"

typedef struct
{
    INT16U id;
    INT16U interval;
    INT32U tick;
}RuleTimerItem;

/** Timer Rule Management */
Result rule_engine_timer_checker_check(void);
Result rule_engine_timer_checker_init(void);
Result rule_engine_timer_checker_uninit(void);

Result rule_engine_timer_checker_add(RuleTimerItem* item);




#endif /* BMS_RULE_ENGINE_TIMER_CHECKER_H_ */
