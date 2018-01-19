/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_rule_engine_event_checker.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-14
*
*/
#include "bms_rule_engine_event_checker.h"
#include "bms_rule_engine_impl.h"

#pragma MESSAGE DISABLE C1420  //Result of function-call is ignored
#pragma MESSAGE DISABLE C4001  //Condition always FALSE

#define RULE_ENGINE_MAX_EVENT_ITEM_NUM 200
#define RULE_ENGINE_MAX_EVENT   100
#define RULE_ENGINE_MAX_EVENT_QUEUE_NUM 100

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
/*
static RuleEventItem g_rule_engine_event_items[RULE_ENGINE_MAX_EVENT_ITEM_NUM] = {0};
static int g_rule_engine_event_item_num = 0;
static RuleEventGroup g_rule_engine_event_groups[RULE_ENGINE_MAX_EVENT] = {0};
static int g_rule_egnine_event_group_num = 0;
*/

static INT16U g_rule_engine_rule_queue[RULE_ENGINE_MAX_EVENT_QUEUE_NUM] = 0;
static INT16U g_rule_engine_rule_queue_begin = 0;
static INT16U g_rule_engine_rule_queue_num = 0;

#pragma DATA_SEG DEFAULT


/** Event Rule Management */
Result rule_engine_event_checker_check(void)
{
    INT16U rule;
    while (rule_engine_checker_pop_rule(&rule) == RES_OK)
    {
        //rule_engine_checker_dispatch_event(event);
        rule_engine_run_rule(rule);
    }

    return RES_OK;
}

Result rule_engine_event_checker_init(void)
{
    INT16U index, rule_num = rule_engine_get_rule_num();
    RuleItem* _PAGED rule_item;

    for (index = 0; index < rule_num; ++index)
    {
        rule_item = rule_engine_get_rule(index + 1);
        if (rule_item == NULL || rule_item->tigger.type != kRuleTriggerTypeEvent
                || rule_item->status == kRuleDisable) continue;

        rule_engine_event_checker_add(rule_item->tigger.data.event, rule_item->id);
    }

    return RES_OK;
}

Result rule_engine_event_checker_uninit(void)
{
    return RES_OK;
}

#if 0

INT16U rule_engine_event_checker_find_tail(INT16U group)
{
    INT16U id = g_rule_engine_event_groups[group].next;
    while (id > 0)
    {
        if (g_rule_engine_event_items[id].next == 0)
            break;
        id = g_rule_engine_event_items[id].next;
    }
    return id;
}

INT16U rule_engine_event_checker_find_availabled(void)
{
    INT16U index;

    for (index = 1; index < RULE_ENGINE_MAX_EVENT_ITEM_NUM; ++index)
    {
        if (g_rule_engine_event_items[index].rule_id == 0)
            return index;
    }

    return 0;
}
#endif

Result rule_engine_event_checker_on_event(EventTypeId event, void* event_data, void* user_data)
{
    UNUSED(event);
    UNUSED(event_data);
    rule_engine_checker_post_rule((INT16U)user_data);
    return RES_OK;
}


Result rule_engine_event_checker_add(INT16U event, RuleID rule)
{
/*
    RuleEventItem item;
    INT16U event_id, last_event_id;

    if (event >= RULE_ENGINE_MAX_EVENT_ITEM_NUM) return RES_ERROR;
*/
    return event_observe((EventTypeId)event, rule_engine_event_checker_on_event, (void*)rule);
/*
    event_id = rule_engine_event_checker_find_availabled();
    if (event_id == 0) return RES_ERROR;

    g_rule_engine_event_items[event_id].next = 0;
    g_rule_engine_event_items[event_id].rule_id = rule;

    last_event_id = rule_engine_event_checker_find_tail(event);
    if (last_event_id == 0)
    {
        g_rule_engine_event_groups[event].next = event_id;
    }
    else
    {
        g_rule_engine_event_items[last_event_id].next = event_id;
    }
    return RES_OK;
*/
}

Result rule_engine_checker_post_rule(INT16U event)
{
    INT16U id;
    if (g_rule_engine_rule_queue_num >= RULE_ENGINE_MAX_EVENT_QUEUE_NUM) return RES_ERROR;

    id = (g_rule_engine_rule_queue_begin + g_rule_engine_rule_queue_num) % RULE_ENGINE_MAX_EVENT_QUEUE_NUM;

    g_rule_engine_rule_queue[id] = event;

    ++g_rule_engine_rule_queue_num;

    return RES_OK;
}

Result rule_engine_checker_pop_rule(INT16U* event)
{
    if (g_rule_engine_rule_queue_num == 0) return RES_ERROR;

    --g_rule_engine_rule_queue_num;
    if (event) *event = g_rule_engine_rule_queue[g_rule_engine_rule_queue_begin];

    g_rule_engine_rule_queue_begin = (g_rule_engine_rule_queue_begin + 1) % RULE_ENGINE_MAX_EVENT_QUEUE_NUM;

    return RES_OK;
}

#if 0
Result rule_engine_checker_dispatch_rule(INT16U event)
{

    INT16U id;
    if (event >= RULE_ENGINE_MAX_EVENT) return RES_ERROR;

    id = g_rule_engine_event_groups[event].next;

    while (id > 0)
    {
        rule_engine_run_rule(g_rule_engine_event_items[id].rule_id);
        id = g_rule_engine_event_items[id].next;
    }

    return RES_OK;
}
#endif




