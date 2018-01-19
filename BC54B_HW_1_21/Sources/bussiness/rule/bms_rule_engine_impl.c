/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_rule_engine_impl.c
 * @brief
 * @note
 * @author Xiangyong Zhang
 * @date 2012-8-10
 *
 */
#include "bms_rule_engine_impl.h"


#pragma MESSAGE DISABLE  C4000 // Condition always TRUE
#pragma MESSAGE DISABLE C1420  //Result of function-call is ignored
#pragma MESSAGE DISABLE C4001  //Condition always FALSE

//OS_STK g_rule_engine_tx_task_stack[RULE_ENGINE_STK_SIZE] = { 0 };

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_RULE2
static RuleItem g_rule_engine_rules[RULE_ENGINE_MAX_RULE_NUM];
//#pragma DATA_SEG DEFAULT

INT16U g_rule_engine_rule_num = 0;

volatile INT16U g_rule_engine_current_rule = 0;
volatile INT8U  g_rule_engine_running_flag = 1;

#pragma DATA_SEG DEFAULT

#if 0
static INT8U g_tmp_buffer[512];
static INT16U g_tmp_buffer_len = 0;
void rule_engine_test_serial(RuleItem* rule)
{
#if 1
    volatile Result res;
    RuleType type;
    res = rule_config_writer_open();
    res = rule_config_writer_write_rule(rule);
    //res = rule_config_writer_write_rule(rule);
    res = rule_config_writer_close();

    memset(rule, 0, sizeof(RuleItem));

    res = rule_config_reader_open();
    res = rule_config_reader_next_type(&type);
    res = rule_config_reader_read_rule(rule);
    res = rule_config_reader_close();
#endif
    //rule_serial_to(rule, g_tmp_buffer, 512, &g_tmp_buffer_len);
    //memset(rule, 0, sizeof(rule));
    //rule_serial_from(rule, g_tmp_buffer, g_tmp_buffer_len, NULL);
}
#endif

void rule_engine_init(void)
{
    watch_dog_register(WDT_RULE_ID, WDT_RULE_TIME);
    script_engine_init();

    rule_engine_task_create();
}

void rule_engine_uninit(void)
{
    rule_engine_event_checker_uninit();
    rule_engine_timer_checker_uninit();
}

void rule_engine_stop(void)
{
    g_rule_engine_running_flag = 0;
}

void rule_engine_task_create(void)
{
    /*OSTaskCreate(rule_engine_task_run, (void *) NULL,
            (OS_STK *) &g_rule_engine_tx_task_stack[RULE_ENGINE_STK_SIZE - 1],
            RULE_ENGINE_TASK_PRIO);*/
}

void rule_engine_task_run(void* data)
{  
    INT8U timeout = 0;
    UNUSED(data);
    do
     {
        OSTimeDly(50);
        watch_dog_feed(WDT_RULE_ID);
        timeout++;
        if(timeout >= 20) break;              //³¬Ê±1S
     }while((bcu_get_chg_start_failure_state() == kStartFailureOccure)||(bcu_get_dchg_start_failure_state() == kStartFailureOccure));

    rule_engine_create_default_rules();
    rule_engine_load_config();

    rule_engine_timer_checker_init();
    rule_engine_event_checker_init();
       
    while(1)
    {   
        watch_dog_feed(WDT_RULE_ID);
        if(g_rule_engine_running_flag)
        {
          rule_engine_timer_checker_check();
          rule_engine_event_checker_check();
        }

        sleep(10);
    }
}

Result rule_engine_check_condition(RuleCondition* _PAGED condition)
{
    switch (condition->type)
    {
    case kRuleConditionTypeFunction:
        if (condition->data.function.func == NULL) return RES_TRUE;
        return condition->data.function.func();
        break;
    case kRuleConditionTypeExpression:
        {
            VmValue value;
            Result res;

            if (condition->data.express.vm_buffer == NULL
                    && condition->expression == NULL) return RES_TRUE;

            if (condition->data.express.vm_buffer)
                res = vm_run(condition->data.express.vm_buffer, condition->data.express.vm_length, &value);
            else
                res = vm_interpret(condition->expression, &value);
            if (res != RES_OK)
            {
                ERROR("rule", "fail to rule the condition %d, res:%d", g_rule_engine_current_rule, res);
                return RES_FALSE;
            }

            return (vm_value_to_int16u(&value) ? RES_TRUE : RES_FALSE);
        }
        break;
    default:
        return RES_TRUE;
    }
}

Result rule_engine_execute_action(RuleAction* _PAGED action)
{
    Result res;
    VmValue value;
    //if (action->expression) DEBUG("action", action->expression);
    //if (action->expression) debug_far("action", action->expression);

    switch (action->type)
    {
    case kRuleActionTypeFunction:
        action->data.function.func();
        return RES_OK;
        break;
    case kRuleActionTypeExpression:
        if (action->data.express.vm_buffer)
        {
            //log_buffer("serial", action->data.express.vm_buffer, action->data.express.vm_length);
            return vm_run(action->data.express.vm_buffer, action->data.express.vm_length, &value);
        }
        else if (action->expression)
        {
            res = vm_interpret(action->expression, &value);
            if (res != RES_OK)
            ERROR("rule", "fail to rule the action: %d, res:%d", g_rule_engine_current_rule, res);
        }
        else
            return RES_OK;
        break;
    default:
        return RES_ERROR;
    }
}

Result rule_engine_run_rule(RuleID id)
{
    Result res;
    RuleItem* _PAGED item = rule_engine_get_rule(id);
    if (item == NULL) return RES_ERROR;

    if (item->status == kRuleDisable) return RES_OK;

    g_rule_engine_current_rule = id;

    res = rule_engine_check_condition(&item->condition);
    if (res == RES_TRUE)
    {
        //DEBUG("action", "id:%d", item->id);
        res = rule_engine_execute_action(&item->action);
    }
    else
    {
        res = RES_OK;
    }

    g_rule_engine_current_rule = 0;

    return res;
}

RuleID rule_engine_add_rule(RuleItem* _PAGED rule)
{
    RuleID id = 0;

    if (rule == NULL) return 0;

    if (rule->id != 0 && rule->id < RULE_ENGINE_MAX_RULE_NUM)
    {
        // modify the rule
        g_rule_engine_rules[rule->id - 1] = *rule;
        id = rule->id;
        if (id > g_rule_engine_rule_num) g_rule_engine_rule_num = id;
    }
    else
    {
        // add a new rule
        if (g_rule_engine_rule_num >= RULE_ENGINE_MAX_RULE_NUM) return 0;

        id = g_rule_engine_rule_num + 1;

        g_rule_engine_rules[g_rule_engine_rule_num] = *rule;
        g_rule_engine_rules[g_rule_engine_rule_num].id = id;

        ++g_rule_engine_rule_num;
    }

    return id;
}

RuleItem* _PAGED rule_engine_get_rule(RuleID id)
{
    if (id<= 0 || id > g_rule_engine_rule_num) return NULL;

    return &g_rule_engine_rules[id - 1];
}

INT16U rule_engine_get_rule_num()
{
    return g_rule_engine_rule_num;
}

Result rule_engine_load_config(void)
{
    Result res;
    RuleConfigType type;

    //rule_config_writer_open();
    //return RES_OK;

    res = rule_config_reader_open();
    if (res != RES_OK) return res;

    res = rule_config_reader_next_type(&type);
    while (res == RES_OK)
    {
        switch(type)
        {
        case kRuleConfigTypeRuleStatus:
            res = rule_engine_load_config_status();
            break;
        case kRuleConfigTypeRule:
            res = rule_engine_load_config_rule();
            break;
        case kRuleConfigTypeEof:
            res = RES_ERROR;
            break;
        default:
            /** skip the unknown block */
            res = RES_ERROR;
            break;
        }

        if (res != RES_OK) break;

        res = rule_config_reader_next_type(&type);
    }

    rule_config_reader_close();

    return RES_OK;
}

Result rule_engine_load_config_rule(void)
{
    RuleItem rule;
    Result res;

    res = rule_config_reader_read_rule(&rule);
    if (res != RES_OK) return res;

    rule_engine_add_rule(&rule);

    return RES_OK;
}

Result rule_engine_load_config_status(void)
{
    INT16U rule_id;
    INT8U status;
    Result res;
    RuleItem* _PAGED rule;

    res = rule_config_reader_rule_read_status(&rule_id, &status);
    if (res != RES_OK) return res;

    rule = rule_engine_get_rule(rule_id);
    if (rule == NULL) return RES_OK;

    rule->status = status;

    return RES_OK;
}

RuleID rule_engine_get_current_rule_id(void)
{
    return g_rule_engine_current_rule;
}
