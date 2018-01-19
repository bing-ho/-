/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_relay_impl.c
 * @brief
 * @note
 * @author
 * @date 2012-5-24
 *
 */
#include "bms_relay_impl.h"


#if BMS_HARDWARE_PLATFORM

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

#define SET_REG(ID, VALUE) \
    BMS_RELAY_##ID##_SEL = 1; \
    BMS_RELAY_##ID##_DAT = VALUE;\
    g_relay_last_tick[ID] = get_tick_count();

#define VALUE_MEANS_RELAY_ON(VALUE) \
    ((VALUE == kRelayOn || VALUE == kRelayForceOn) ? 1 : 0)    

#define LOAD_RELAY_START_ID      1
#define LOAD_RELAY_END_ID        6

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

static INT8U g_relay_status[BMS_RELAY_MAX_COUNT + 1] = {0};
static INT32U g_last_load_relay_tick = 0;
static INT8U g_relay_last_normal_ctl_flag[BMS_RELAY_MAX_COUNT + 1] = {0};
static INT32U g_relay_last_tick[BMS_RELAY_MAX_COUNT + 1] = {0};
static RelayTroubleStatus g_relay_trouble_status[BMS_RELAY_MAX_COUNT + 1] = {0};

#pragma DATA_SEG DEFAULT

LIST_ITEM* g_relay_pending_list = NULL;
RelayPendingItem g_relay_pending_item[BMS_RELAY_MAX_COUNT + 1];

void relay_init(void)
{
    INT8U i = 0;
    
    g_relay_pending_list = list_init();
    g_relay_pending_list->next = NULL;
    
    for (i = 0; i <= BMS_RELAY_MAX_COUNT; i++)
    {
        g_relay_pending_item[i].next = NULL;
        g_relay_pending_item[i].id = i;
        g_relay_pending_item[i].value = 0;
    }
    job_schedule(MAIN_JOB_GROUP, RELAY_PENDING_CHECK_CYCLE, relay_pending_check, NULL);
}

Result relay_io_set(INT8U id, INT8U value)
{
    INT8U flag = 0;
    
    if (id > BMS_RELAY_MAX_COUNT) return RES_ERROR;
    
    if (VALUE_MEANS_RELAY_ON(value)) flag = 1;
    
    switch (id)
    {
    case 1:
        SET_REG(1, flag);
        break;
    case 2:
        SET_REG(2, flag);
        break;
    case 3:
        SET_REG(3, flag);
        break;
    case 4:
        SET_REG(4, flag);
        break;
    case 5:
        SET_REG(5, flag);
        break;
    case 6:
        SET_REG(6, flag);
        break;
    case 7:
        SET_REG(7, flag);
        break;
    case 8:
        SET_REG(8, flag);
        break;
    default:
        break;
    }

    g_relay_status[id] = value;

    return RES_OK;
}

Result relay_set(INT8U id, INT8U value)
{
    INT8U flag = 0;
    OS_CPU_SR cpu_sr = 0;
    
    if (id > BMS_RELAY_MAX_COUNT) return RES_ERROR;
    if (VALUE_MEANS_RELAY_ON(value)) flag = 1;
    
    if (id >= LOAD_RELAY_START_ID && id <= LOAD_RELAY_END_ID) // 负载继电器
    {
        if(flag)
        {
            if(!relay_is_on(id))
            {
                OS_ENTER_CRITICAL();
                g_relay_pending_item[id].value = value;
                relay_pending_list_add(id);
                OS_EXIT_CRITICAL();
                return RES_OK;
            }
        }
        else
        {
            relay_pending_list_remove(id);
        }
    }
    return relay_io_set(id, value);
}

Result relay_update_last_normal_clt_flag(INT8U id, INT8U value)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    g_relay_last_normal_ctl_flag[id] = value;
    
    return RES_OK;
}

INT8U relay_is_force_status(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if(g_relay_status[id] >= kRelayForceStart) return 1;
    
    return 0;
}

INT8U relay_get_force_status(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    
    if(!relay_is_force_status(id)) return 0;
    
    return g_relay_status[id];
}

Result relay_on(INT8U id)
{
    relay_update_last_normal_clt_flag(id, kRelayOn);
    if(relay_is_force_status(id)) return RES_EXCEED_LIMITED;
    return relay_set(id, kRelayOn);
}

Result relay_off(INT8U id)
{
    relay_update_last_normal_clt_flag(id, kRelayOff);
    if(relay_is_force_status(id)) return RES_EXCEED_LIMITED;
    return relay_set(id, kRelayOff);
}

Result relay_force_on(INT8U id)
{
    return relay_set(id, kRelayForceOn);   
}

Result relay_force_off(INT8U id)
{
    return relay_set(id, kRelayForceOff);
}

Result relay_force_cancle(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if (!relay_is_force_status(id)) return 0;
    
    if(g_relay_last_normal_ctl_flag[id] != kRelayOn && g_relay_last_normal_ctl_flag[id] != kRelayOff) 
        g_relay_last_normal_ctl_flag[id] = kRelayOff;
    return relay_set(id, g_relay_last_normal_ctl_flag[id]);
}

INT8U relay_is_on(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if(VALUE_MEANS_RELAY_ON(g_relay_status[id])) return 1;
    
    return 0;
}

INT8U relay_count(void)
{
    return BMS_RELAY_MAX_COUNT;
}

RelayTroubleStatus relay_get_instant_trouble_status(INT8U id, INT8U input_signal_id)
{
    RelayTroubleStatus status = kRelayNormal;
    
    if(id > BMS_RELAY_MAX_COUNT) return kRelayNormal;
    if(input_signal_id > input_signal_get_count()) return kRelayNormal;
    
    if(relay_is_on(id) == 1)
    {
        if(input_signal_is_high(input_signal_id) == 1)
        {
            status = kRelayOpenCircuitTrouble;
        }
    }
    else
    {
        if(input_signal_is_high(input_signal_id) == 0)
        {
            status = kRelayAdhesionTrouble;
        }
    }
    
    return status;
}

RelayTroubleStatus relay_get_trouble(INT8U id)
{
    OS_CPU_SR cpu_sr = 0;
    RelayTroubleStatus status = kRelayTroubleNoCheck;
    
    if (id > BMS_RELAY_MAX_COUNT) return kRelayNormal;
    OS_ENTER_CRITICAL();
    status = g_relay_trouble_status[id];
    OS_EXIT_CRITICAL();
    
    return status;
}

void relay_set_trouble(INT8U id, RelayTroubleStatus status)
{
    OS_CPU_SR cpu_sr = 0;
    
    if (id > BMS_RELAY_MAX_COUNT) return;
    if (status >= kRelayTroubleMax) return;
    
    OS_ENTER_CRITICAL();
    g_relay_trouble_status[id] = status;
    OS_EXIT_CRITICAL();
}

RelayTroubleStatus relay_get_trouble_status(INT8U id, INT8U input_signal_id)
{
    if(id > BMS_RELAY_MAX_COUNT) return kRelayNormal;
    if(input_signal_id > input_signal_get_count()) return kRelayNormal;
    
    if(g_relay_last_tick[id] == 0) return kRelayNormal;
    
    if(get_interval_by_tick(g_relay_last_tick[id], get_tick_count()) < BMS_LOAD_RELAY_ON_TIME) return g_relay_trouble_status[id];
    
    g_relay_trouble_status[id] = relay_get_instant_trouble_status(id, input_signal_id);
    
    return g_relay_trouble_status[id];
}

void relay_pending_check(void* pdata)
{
    INT32U now_tick = 0;
    RelayPendingItem* item = NULL;
    OS_CPU_SR cpu_sr = 0;
    
    pdata = pdata;
    
    if (g_relay_pending_list->next == NULL) return;
    now_tick = get_tick_count();
    if (get_interval_by_tick(g_last_load_relay_tick, now_tick) >= BMS_RELAY_LOAD_RELAY_ON_MIN_INTERVAL)
    {
        g_last_load_relay_tick = now_tick;
        OS_ENTER_CRITICAL();
        item = (RelayPendingItem*)(g_relay_pending_list->next);
        relay_io_set(item->id, item->value); //控制继电器IO
        relay_pending_list_remove(item->id);
        OS_EXIT_CRITICAL();
    }
}

Result relay_pending_list_add(INT8U id)
{
    OS_CPU_SR cpu_sr = 0;
    
    if(id > BMS_RELAY_MAX_COUNT) return 0;
    
    OS_ENTER_CRITICAL();
    if (list_find(g_relay_pending_list, (LIST_ITEM*)(&g_relay_pending_item[id])) == 0)
    {
        list_add(g_relay_pending_list, (LIST_ITEM*)(&g_relay_pending_item[id])); //加入等待列表
    }
    OS_EXIT_CRITICAL();
    
    return 1;
}

Result relay_pending_list_remove(INT8U id)
{
    OS_CPU_SR cpu_sr = 0;
    RelayPendingItem* item = NULL;
    
    if(id > BMS_RELAY_MAX_COUNT) return 0;
    OS_ENTER_CRITICAL();
    item = (RelayPendingItem*)(&g_relay_pending_item[id]);
    list_remove(g_relay_pending_list, (LIST_ITEM*)item); //从等待列表中移除
    item->next = NULL;
    item->value = 0;
    OS_EXIT_CRITICAL();
    
    return 1;
}

#endif

