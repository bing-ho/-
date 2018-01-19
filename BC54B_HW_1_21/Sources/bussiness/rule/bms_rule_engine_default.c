/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_rule_engine_default.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-17
*
*/
#include "bms_rule_engine_default.h"
#include "bms_bcu.h"
#include "bms_system.h"
#include "bms_rule_engine_impl.h"
#include "bms_charger.h"
#include "bms_log.h"
#include "bms_rule_stock.h"

#pragma MESSAGE DISABLE C1420  //Result of function-call is ignored
#pragma MESSAGE DISABLE C4001  //Condition always FALSE

extern void default_rule_load(void);

void rule_engine_create_default_rules(void)
{
    //rule_engine_add_rule_demo();

    default_rule_load();
}

/***************************************************************
 *
 *
 ****************************************************************/
Result rule_engine_action_show_cpu_usage(void)
{
    //DEBUG("performance", "current cpu usage:%d", OSCPUUsage);
    
    return RES_OK;
}

void rule_engine_add_rule_demo(void)
{
    rule_engine_add_timer_rule(5000, NULL, NULL, NULL, NULL, NULL, rule_engine_action_show_cpu_usage);
    //rule_engine_add_timer_express_rule(5000, NULL, "post_event(7)");
    //rule_engine_add_event_express_rule(7, NULL, "beep_on();sleep(100);beep_off()");
    //rule_engine_add_event_express_rule(7, NULL, "beep_on();sleep(100);");
    //rule_engine_add_event_rule(7, NULL, NULL, NULL, NULL, NULL, rule_engine_rule_beep_off);
}

Result rule_engine_add_timer_express_rule(INT16U interval, PCSTR condition, PCSTR expression)
{
    RuleItem rule;

    memset(&rule, 0, sizeof(rule));
    rule.tigger.type = kRuleTriggerTypeTimer;
    rule.tigger.data.interval = interval;

    if (condition)
    {
        rule.condition.type = kRuleActionTypeExpression;
        rule.condition.expression = condition;
    }

    rule.action.type = kRuleActionTypeExpression;
    rule.action.expression = expression;

    return rule_engine_add_rule(&rule);
}

Result rule_engine_add_event_express_rule(INT16U event, PCSTR condition, PCSTR expression)
{
    RuleItem rule;

    memset(&rule, 0, sizeof(rule));
    rule.tigger.type = kRuleTriggerTypeEvent;
    rule.tigger.data.event = event;

    if (condition)
    {
        rule.condition.type = kRuleActionTypeExpression;
        rule.condition.expression = condition;
    }

    rule.action.type = kRuleActionTypeExpression;
    rule.action.expression = expression;

    return rule_engine_add_rule(&rule);
}

Result rule_engine_add_timer_rule(INT16U interval, PCSTR name, PCSTR description,
        PCSTR condition, PCSTR action,
        RuleFunction cond_func, RuleFunction action_func)
{
    RuleItem rule;       
    
    UNUSED(description);

    memset(&rule, 0, sizeof(rule));
    rule.tigger.type = kRuleTriggerTypeTimer;
    rule.tigger.data.interval = interval;
    rule.name = name;
    //rule.description = description;
    rule.type = kRuleTypeDefault;
    rule.status = kRuleEnabled;

    rule.condition.expression = condition;

    if (cond_func)
    {
        rule.condition.type = kRuleConditionTypeFunction;
        rule.condition.data.function.func = cond_func;
    }
    else
    {
        rule.condition.type = kRuleConditionTypeExpression;
    }

    rule.action.expression = action;
    if (action_func)
    {
        rule.action.type = kRuleActionTypeFunction;
        rule.action.data.function.func = action_func;
    }
    else
    {
        rule.action.type = kRuleActionTypeExpression;
    }

    return rule_engine_add_rule(&rule);
}

Result rule_engine_add_event_rule(INT16U event, PCSTR name, PCSTR description,
        PCSTR condition, PCSTR action,
        RuleFunction cond_func, RuleFunction action_func)
{
    RuleItem rule;
    
    UNUSED(description);    

    memset(&rule, 0, sizeof(rule));
    rule.tigger.type = kRuleTriggerTypeEvent;
    rule.tigger.data.event = event;
    rule.name = name;
    //rule.description = description;
    rule.type = kRuleTypeDefault;
    rule.status = kRuleEnabled;

    rule.condition.expression = condition;

    if (cond_func)
    {
        rule.condition.type = kRuleConditionTypeFunction;
        rule.condition.data.function.func = cond_func;
    }
    else
    {
        rule.condition.type = kRuleConditionTypeExpression;
    }

    rule.action.expression = action;
    if (action_func)
    {
        rule.action.type = kRuleActionTypeFunction;
        rule.action.data.function.func = action_func; 
    }
    else
    {
        rule.action.type = kRuleActionTypeExpression;
    }

    return rule_engine_add_rule(&rule);
}
