/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_rule_engine.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-10
*
*/

#ifndef BMS_RULE_ENGINE_H_
#define BMS_RULE_ENGINE_H_
#include "bms_defs.h"

typedef Result (*RuleFunction)(void);

typedef enum
{
    kRuleTypeDefault = 0, //
    kRuleTypeCustom, //
} RuleType;

typedef enum
{
    kRuleEnabled = 0, //
    kRuleDisable = 1, //
} RuleStatus;

typedef enum
{
    kRuleTriggerTypeNone = 0, //
    kRuleTriggerTypeTimer, //
    kRuleTriggerTypeEvent, //
} RuleTriggerType;

typedef enum
{
    kRuleConditionTypeNone = 0, //
    kRuleConditionTypeFunction, //
    kRuleConditionTypeExpression, //
} RuleConditionType;

typedef enum
{
    kRuleActionTypeNone = 0, //
    kRuleActionTypeFunction, //
    kRuleActionTypeExpression, //
} RuleActionType;

typedef struct
{
    INT8U type;
    union
    {
        INT16U interval;
        INT16U event;
    } data;
} RuleTrigger;

typedef struct
{
    INT8U type;
    PCSTR expression;
    union
    {
        struct
        {
            PINT8U vm_buffer;
            INT16U vm_length;
        } express;
        struct
        {
            RuleFunction func;
        } function;
    } data;
} RuleCondition;

typedef struct
{
    INT8U type;
    PCSTR expression;
    union
    {
        struct
        {
            PINT8U vm_buffer;
            INT16U vm_length;
        } express;
        struct
        {
            RuleFunction func;
        } function;
    } data;
} RuleAction;

typedef INT16U RuleID;

typedef struct
{
    RuleID id;
    PCSTR name;
    //PCSTR description;
    INT8U type;
    INT8U status;
    RuleTrigger tigger;
    RuleCondition condition;
    RuleAction action;
} RuleItem;


void rule_engine_init(void);
void rule_engine_uninit(void);
void rule_engine_stop(void);

/** rule management */
Result rule_engine_add_rule(RuleItem* _PAGED rule);
RuleItem* _PAGED rule_engine_get_rule(RuleID id);
INT16U rule_engine_get_rule_num(void);

RuleID rule_engine_get_current_rule_id(void);



#endif /* BMS_RULE_ENGINE_H_ */
