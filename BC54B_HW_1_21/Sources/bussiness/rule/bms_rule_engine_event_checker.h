/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_rule_engine_event_checker.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-14
*
*/

#ifndef BMS_RULE_ENGINE_EVENT_CHECKER_H_
#define BMS_RULE_ENGINE_EVENT_CHECKER_H_

#include "bms_defs.h"
#include "bms_rule_engine_impl.h"
#include "bms_event.h"

typedef struct
{
    RuleID rule_id;
    INT16U next;
}RuleEventItem;

typedef struct
{
    INT16U next;
}RuleEventGroup;

/** Event Rule Management */
Result rule_engine_event_checker_check(void);
Result rule_engine_event_checker_init(void);
Result rule_engine_event_checker_uninit(void);


Result rule_engine_checker_post_rule(INT16U event);
Result rule_engine_checker_pop_rule(INT16U* event);
//Result rule_engine_checker_dispatch_rule(INT16U event);

Result rule_engine_event_checker_add(INT16U event, RuleID rule);
Result rule_engine_event_checker_on_event(EventTypeId event, void* event_data, void* user_data);
/*
INT16U rule_engine_event_checker_find_tail(INT16U group);
INT16U rule_engine_event_checker_find_availabled(void);
*/

#endif /* BMS_RULE_ENGINE_EVENT_CHECKER_H_ */
