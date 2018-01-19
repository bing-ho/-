/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_rule_engine_timer_checker.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-14
*
*/
#include "bms_rule_engine_timer_checker.h"


#pragma MESSAGE DISABLE C1420  //Result of function-call is ignored
#pragma MESSAGE DISABLE C4001  //Condition always FALSE

#define RULE_ENGINE_MAX_TIMER_ITEM_NUM 140
#define RULE_ENGINE_MAX_INTERVAL    60000

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_RULE_TIMER

static RuleTimerItem g_rule_engine_timer_items[RULE_ENGINE_MAX_TIMER_ITEM_NUM] = {0};
INT16U g_rule_engine_timer_item_num = 0;
INT32U g_rule_engine_timer_last_tick = 0;
INT32U g_rule_engine_timer_last_interval = RULE_ENGINE_MAX_INTERVAL;

#pragma DATA_SEG DEFAULT



/** Timer Rule Management */
Result rule_engine_timer_checker_check(void)
{
    INT16U index;
    INT32U tick = get_tick_count();
    INT32U diff, interval;
    static INT16U count = 0;
    INT16U num = 0;

    //DEBUG("timer", "check:%d", ++count);

    if (get_interval_by_tick(g_rule_engine_timer_last_tick, tick) < g_rule_engine_timer_last_interval)
        return RES_OK;

    g_rule_engine_timer_last_interval = RULE_ENGINE_MAX_INTERVAL;
    g_rule_engine_timer_last_tick = tick;

    // check every timer item
    for (index = 0; index < g_rule_engine_timer_item_num; ++index)
    {
        diff = get_interval_by_tick(g_rule_engine_timer_items[index].tick, tick);
        if (diff >= g_rule_engine_timer_items[index].interval)
        {
            g_rule_engine_timer_items[index].tick = tick;
            interval = g_rule_engine_timer_items[index].interval;

            rule_engine_run_rule(g_rule_engine_timer_items[index].id);
            ++num;
        }
        else
        {
            interval = g_rule_engine_timer_items[index].interval - diff;
        }

        if (interval < g_rule_engine_timer_last_interval) g_rule_engine_timer_last_interval = interval;
    }

    if (num > 0)
    {
      INT32U time = get_tick_count() - g_rule_engine_timer_last_tick;
      //DEBUG("rule", "count:%d, time:%d", num, (INT16U)time);
    }

    return RES_OK;
}

Result rule_engine_timer_checker_init(void)
{
    INT16U index, rule_num = rule_engine_get_rule_num();
    RuleTimerItem item;
    RuleItem* _PAGED rule_item;
    INT32U tick = get_tick_count();
    g_rule_engine_timer_last_tick = tick;

    g_rule_engine_timer_last_interval = RULE_ENGINE_MAX_INTERVAL;

    for (index = 0; index < rule_num; ++index)
    {
        rule_item = rule_engine_get_rule(index + 1);
        if (rule_item == NULL || rule_item->tigger.type != kRuleTriggerTypeTimer
            || rule_item->status == kRuleDisable)
            continue;

        item.id = index + 1;
        item.interval = rule_item->tigger.data.interval;
        item.tick = tick;

        if (g_rule_engine_timer_last_interval > item.interval)
        {
            g_rule_engine_timer_last_interval = item.interval;
        }

        rule_engine_timer_checker_add(&item);
    }

    return RES_OK;
}

Result rule_engine_timer_checker_uninit(void)
{
    return RES_OK;
}

Result rule_engine_timer_checker_add(RuleTimerItem* item)
{
    if (g_rule_engine_timer_item_num >= RULE_ENGINE_MAX_TIMER_ITEM_NUM)
        return RES_ERROR;

    g_rule_engine_timer_items[g_rule_engine_timer_item_num++] = *item;

    return RES_OK;
}


