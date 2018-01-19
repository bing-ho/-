/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_rule_engine_default.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-17
*
*/

#ifndef BMS_RULE_ENGINE_DEFAULT_H_
#define BMS_RULE_ENGINE_DEFAULT_H_
#include "bms_rule_engine_impl.h"

//#define DEBUG_EXPR


#ifdef DEBUG_EXPR
#define RULE_EXPR(NAME) NAME##_expr
#define RULE_FUNC(NAME) NULL
#else
#define RULE_EXPR(NAME) NAME##_expr
#define RULE_FUNC(NAME) NAME##_func
#endif

#define RULE_PARAMS(NAME) RULE_EXPR(NAME##_cond), RULE_EXPR(NAME##_action), \
    RULE_FUNC(NAME##_cond), RULE_FUNC(NAME##_action)

#define RULE_COND(EXPR) ((EXPR) ? RES_TRUE : RES_FALSE)

#define RULE_DEFINE_FUNC(NAME, EXPR) \
const char NAME##_expr[] = EXPR; \
Result NAME##_func(void)


#define RULE_DEFINE_COND(NAME, EXPR) RULE_DEFINE_FUNC(NAME##_cond, EXPR)
#define RULE_DEFINE_ACTION(NAME, EXPR) RULE_DEFINE_FUNC(NAME##_action, EXPR)



void rule_engine_create_default_rules(void);

void rule_engine_add_rule_demo(void);
void rule_engine_add_rule_charge(void);

Result rule_engine_add_timer_express_rule(INT16U interval, PCSTR condition, PCSTR expression);
Result rule_engine_add_event_express_rule(INT16U event, PCSTR condition, PCSTR expression);

Result rule_engine_add_timer_rule(INT16U interval, PCSTR name, PCSTR description,
        PCSTR condition, PCSTR action,
        RuleFunction cond_func, RuleFunction action_func);

Result rule_engine_add_event_rule(INT16U event, PCSTR name, PCSTR description,
        PCSTR condition, PCSTR action,
        RuleFunction cond_func, RuleFunction action_func);
#endif /* BMS_RULE_ENGINE_DEFAULT_H_ */
