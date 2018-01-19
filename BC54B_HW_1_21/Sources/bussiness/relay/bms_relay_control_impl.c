/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_relay_control_impl.c
 * @brief
 * @note
 * @author
 * @date 2012-5-22
 *
 */
#include "bms_relay_control_impl.h"
#include "run_mode.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C4001 // Condition always FALSE
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
static INT8U g_relay_control_flag = 0;
static INT8U g_relay_control_enable_flag = RELAY_CONTROL_ENABLE_FLAG_DEF & (~RELAY_CHG_BIT);
static RelayCtlStatus g_relay_force_command[BMS_RELAY_MAX_COUNT + 1] = {0};
#pragma DATA_SEG DEFAULT

void relay_control_init(void)
{
    relay_init();
    
    relay_control_load_config();
    relay_control_off_all();
}


INT8U g_relay_control_map[kRelayTypeMaxCount] = {0};

void relay_control_on(RelayControlType type)
{
    INT8U id = 0xFF;
    
    if (RUN_MODE_WAKEUP == runmode_get()) return;
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return;
    
    id = g_relay_control_map[type];
    if(relay_control_enable_is_on(type) == 0) return;
    if(relay_control_is_on(type) == 1 || relay_is_pending_on(id) == 1) return;
    
    if(relay_on(g_relay_control_map[type]) == RES_OK)
        relay_control_set_flag(type, 1);
}

INT8U relay_control_is_on(RelayControlType type)
{
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return 0xFF;
	if (g_relay_control_map[type] == 0) return 0xFF;

    return relay_is_on(g_relay_control_map[type]);
}

void relay_control_off(RelayControlType type)
{
    INT8U id = 0xFF;
    
    if (type < kRelayTypeUnknown || type >= kRelayTypeMaxCount) return;
    
    id = g_relay_control_map[type];
    
    if(relay_control_is_on(type) == 0 && relay_get_pending_status(id) != kRelayOn) return;
    
    if(relay_off(id) == RES_OK)
        relay_control_set_flag(type, 0);
}

void relay_force_control_on(RelayControlType type)
{
    INT8U id = 0xFF;
    
    if (RUN_MODE_WAKEUP == runmode_get()) return;
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return;
    
    id = g_relay_control_map[type];
    if(relay_is_force_control_on(type) == 1 || relay_get_pending_status(id) == kRelayForceOn) return;
    
    if(relay_force_on(id) == RES_OK)
    {
        g_relay_force_command[id] = kRelayForceOn;
        relay_control_set_flag(type, 1);
    }
}

void relay_force_control_off(RelayControlType type)
{
    INT8U id = 0xFF;
    
    if (type < kRelayTypeUnknown || type >= kRelayTypeMaxCount) return;
    
    id = g_relay_control_map[type];
    if(relay_is_force_control_off(type) == 1 || relay_get_pending_status(id) == kRelayForceOff) return;
    
    if(relay_force_off(id) == RES_OK)
    {
        g_relay_force_command[id] = kRelayForceOff;
        relay_control_set_flag(type, 0);
    }
}

void relay_force_control_cancle(RelayControlType type)
{
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return;
    
    //if(relay_control_enable_is_on(type) == 0) return;
    
    relay_force_cancle(g_relay_control_map[type]);
    g_relay_force_command[g_relay_control_map[type]] = 0;
    relay_control_set_flag(type, relay_is_on(g_relay_control_map[type]));    
}

INT8U relay_is_force_control_on(RelayControlType type)
{
    INT8U status;
    
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return 0;
    
    status = relay_get_force_status(g_relay_control_map[type]);
    if(status == kRelayForceOn) return 1;
    
    return 0;
}

INT8U relay_is_force_control_off(RelayControlType type)
{
    INT8U status;
    
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return 0;
    
    status = relay_get_force_status(g_relay_control_map[type]);
    if(status == kRelayForceOff) return 1;
    
    return 0;
}

INT8U relay_control_get_id(RelayControlType type)
{
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return 0;
    return g_relay_control_map[type];
}

RelayControlType relay_control_get_type(INT8U id)
{
    INT8U index;
    for (index = 1; index < kRelayTypeMaxCount; ++index)
    {
        if (id == g_relay_control_map[index])
            return (RelayControlType)index;
    }

    return kRelayTypeUnknown;
}

void relay_control_load_config(void)
{
    INT16U index;
    INT8U type;
    for (index = kCfgRelayTypeStart; index <= kCfgRelayTypeEnd; ++index)
    {
        type = (INT8U)config_get(index);
        if (type <= 0 || type >= kRelayTypeMaxCount) continue;

        g_relay_control_map[type] = (INT8U)(index - kCfgRelayTypeStart + 1);
    }
}

void relay_control_off_all(void)
{
    INT8U index, num;
    if (config_get(kRelayPreCharge)) relay_control_off(kRelayTypePreCharging);
    num = relay_count();
    for (index = 1; index <= num; ++index)
    {
        relay_force_cancle(index);
        relay_off(index);
    }
}

void relay_control_set_type(INT8U index, RelayControlType type)
{
    RelayControlType old_type;
    if (type < kRelayTypeUnknown || type >= kRelayTypeMaxCount) return;
    if (index <= 0 || index > relay_count()) return;

    old_type = relay_control_get_type(index);//find the index relay's type
    if (old_type != kRelayTypeUnknown) g_relay_control_map[old_type] = 0;//cancle the old type relay fuction

    g_relay_control_map[type] = index;

    if (index - 1 <= kCfgRelayTypeEnd - kCfgRelayTypeStart)
        config_save(kCfgRelayTypeStart + index - 1, type);
}

void relay_control_set_flag(RelayControlType type, INT8U is_on)
{
    INT8U flag = 0;
    
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return;
    switch (type)
    {
    case kRelayTypeCharging:
        flag = RELAY_CHG_BIT;
        break;
    case kRelayTypeDischarging:
        flag = RELAY_DIS_CHG_BIT;
        break;
    case kRelayTypePreCharging:
        flag = RELAY_PRE_CHG_BIT;
        break;
    default:
        return;
    }

    if (is_on)
    {
        g_relay_control_flag |= flag;
    }
    else
    {
        g_relay_control_flag &= ~flag;
    }
}

INT8U relay_control_get_flag(void)
{
    return g_relay_control_flag;
}

void relay_control_set_enable_flag(RelayControlType type, INT8U is_on)
{
    INT8U flag = 0;
    
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return;
    switch (type)
    {
    case kRelayTypeCharging:
        flag = RELAY_CHG_BIT;
        break;
    case kRelayTypeDischarging:
        flag = RELAY_DIS_CHG_BIT;
        break;
    case kRelayTypePreCharging:
        flag = RELAY_PRE_CHG_BIT;
        break;
    default:
        return;
    }

    if (is_on)
    {
        g_relay_control_enable_flag |= flag;
    }
    else
    {
        //relay_control_off(type);
        g_relay_control_enable_flag &= ~flag;
    }
}

INT8U relay_control_get_enable_flag(void)
{
    return g_relay_control_enable_flag;
}

INT8U relay_control_enable_is_on(RelayControlType type)
{
    INT8U flag = relay_control_get_enable_flag();
    
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return 0;
    
    switch (type)
    {
    case kRelayTypeCharging:
        flag = flag & RELAY_CHG_BIT;
        break;
    case kRelayTypeDischarging:
        flag = flag & RELAY_DIS_CHG_BIT;
        break;
    case kRelayTypePreCharging:
        flag = flag & RELAY_PRE_CHG_BIT;
        break;
    default:
        return RELAY_DEFAULT_ENABLE_FLAG;
    }
    
    if(flag) return 1;
    
    return 0;
}

RelayTroubleStatus relay_control_get_instant_trouble_status(RelayControlType type, InputControlType input_signal_type)
{
    INT8U relay_id, input_signal_id = 0;
    
    if (type <= kRelayTypeUnknown && type >= kRelayTypeMaxCount) return kRelayNormal;
    
    input_signal_id = input_signal_control_get_id(input_signal_type);
    relay_id = relay_control_get_id(type);
    if (input_signal_id == 0 || relay_id == 0) return kRelayNormal;
    
    return relay_get_instant_trouble_status(relay_id, input_signal_id);
}

RelayTroubleStatus relay_control_get_trouble_status(RelayControlType type, InputControlType input_signal_type)
{
    INT8U relay_id, input_signal_id = 0;
    
    if (type <= kRelayTypeUnknown && type >= kRelayTypeMaxCount) return kRelayNormal;
    
    input_signal_id = input_signal_control_get_id(input_signal_type);
    relay_id = relay_control_get_id(type);
    if (input_signal_id == 0 || relay_id == 0) return kRelayNormal;
    
    return relay_get_trouble_status(relay_id, input_signal_id);
}

//ÒÀÀµ¾øÔµÄ£¿éÍ¨ÐÅ×´Ì¬
void relay_control_update_adhesion_status(RelayControlType type, INT16U tv)
{
    INT8U relay_id = relay_control_get_id(type);
    RelayTroubleStatus status = relay_get_trouble(relay_id);
    
    if ((type <= kRelayTypeUnknown && type >= kRelayTypeMaxCount) ||
        bcu_get_insulation_work_state() == kInsulationWorkStateNotConnected || 
        relay_control_is_on(type) == 1) return;
    
    if (tv >= BMS_RELAY_ADHESION_VOLT_MIN) 
    {
        status = kRelayAdhesionTrouble;
    }
    else
    {
        status = kRelayNormal;
    }
    relay_set_trouble(relay_id, status);
}

RelayTroubleStatus relay_control_get_adhesion_status(RelayControlType type)
{
    INT8U relay_id = relay_control_get_id(type);
    
    return relay_get_trouble(relay_id);
}

RelayControlType relay_control_relay_type_to_computer_display(RelayControlType type)
{
    if(type >= kRelayTypeSelfDefineStart && type <= kRelayTypeSelfDefineStop)
    {
        return kRelayTypeSelfDefine;
    }
    return type;
}

RelayControlType relay_control_relay_type_from_computer_display(RelayControlType type, INT8U relay_num)
{
    if(relay_num <0 || relay_num > relay_count()) return kRelayTypeUnknown;

    if(type >= kRelayTypeSelfDefineStart && type <= kRelayTypeSelfDefineStop)
    {
        return kRelayTypeSelfDefineStart + relay_num - 1;
    }
    return type;
}

RelayCtlStatus relay_control_get_force_command(RelayControlType type)
{
    RelayCtlStatus status = 0;
    OS_CPU_SR cpu_sr = 0;
    
    if (type <= kRelayTypeUnknown && type >= kRelayTypeMaxCount) return 0;
    
    OS_ENTER_CRITICAL();
    status = g_relay_force_command[g_relay_control_map[type]];
    OS_EXIT_CRITICAL();
    
    return status;
}

Result relay_control_delay_on(RelayControlType type, INT16U delay)
{
    INT8U id = 0xFF;
    
    if (type <= kRelayTypeUnknown || type >= kRelayTypeMaxCount) return RES_ERR;
    
    id = g_relay_control_map[type];
    if(relay_control_enable_is_on(type) == 0) return RES_ERR;
    if(relay_control_is_on(type) == 1 || relay_is_pending_on(id) == 1) return RES_OK;;
    
    if(relay_delay_on(g_relay_control_map[type], delay) != RES_OK)  return RES_ERR;
    
    relay_control_set_flag(type, 1);
    return RES_OK;
}

Result relay_control_delay_off(RelayControlType type, INT16U delay)
{
    INT8U id = 0xFF;
    
    if (type < kRelayTypeUnknown || type >= kRelayTypeMaxCount) return RES_ERROR;
    
    id = g_relay_control_map[type];
    
    if(relay_control_is_on(type) == 0 && relay_get_pending_status(id) != kRelayOn) return RES_OK;
    
    if(relay_delay_off(id, delay) != RES_OK) return RES_ERROR;

    relay_control_set_flag(type, 0);
    return RES_OK;
}
