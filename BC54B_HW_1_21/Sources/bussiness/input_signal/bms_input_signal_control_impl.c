/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_input_signal_control_impl.c
 * @brief
 * @note
 * @author
 * @date 2014-3-1
 *
 */
#include "bms_input_signal_control_impl.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C4001 // Condition always FALSE
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

void input_signal_control_init(void)
{
    input_signal_init();
    input_signal_control_load_config();
}


static INT8U g_input_signal_control_map[kInputTypeMaxCount] = {0};

INT8U input_signal_control_is_high(InputControlType type)
{
    if (type <= kInputTypeUnknown || type >= kInputTypeMaxCount) return 0xFF;
	if (g_input_signal_control_map[type] == 0) return 0xFF;

    return input_signal_is_high(g_input_signal_control_map[type]);
}

INT8U input_signal_control_is_low(InputControlType type)
{
    if (type <= kInputTypeUnknown || type >= kInputTypeMaxCount) return 0xFF;
	if (g_input_signal_control_map[type] == 0) return 0xFF;

    return input_signal_is_low(g_input_signal_control_map[type]);
}

INT8U input_signal_control_get_id(InputControlType type)
{
    if (type <= kInputTypeUnknown || type >= kInputTypeMaxCount) return 0;
    return g_input_signal_control_map[type];
}

InputControlType input_signal_control_get_type(INT8U id)
{
    INT8U index;
    for (index = 0; index < kInputTypeMaxCount; ++index)
    {
        if (id == g_input_signal_control_map[index])
            return (InputControlType)index;
    }

    return kInputTypeUnknown;
}

void input_signal_control_load_config(void)
{
    INT16U index;
    INT8U type;
    for (index = kCfgInputTypeStart; index <= kCfgInputTypeEnd; ++index)
    {
        type = (INT8U)config_get(index);
        if (type <= 0 || type >= kInputTypeMaxCount) continue;

        g_input_signal_control_map[type] = (INT8U)(index - kCfgInputTypeStart + 1);
    }
}

void input_signal_control_set_type(INT8U index, InputControlType type)
{
    InputControlType old_type;
    if (type < kInputTypeUnknown || type >= kInputTypeMaxCount) return;
    if (index <= 0 || index > input_signal_get_count()) return;

    old_type = input_signal_control_get_type(index);//find the index input signal's type
    if (old_type != kInputTypeUnknown) g_input_signal_control_map[old_type] = 0;//cancle the old type input signal fuction

    g_input_signal_control_map[type] = index;

    if (index - 1 <= kCfgInputTypeEnd - kCfgInputTypeStart)
        config_save(kCfgInputTypeStart + index - 1, type);
}

InputControlType input_signal_control_type_to_computer_display(InputControlType type)
{
    if(type >= kInputTypeSelfDefineStart && type <= kInputTypeSelfDefineStop)
    {
        return kInputTypeSelfDefine;
    }
    return type;
}

InputControlType input_signal_control_type_from_computer_display(InputControlType type, INT8U input_num)
{
    if(input_num <=0 || input_num > input_signal_get_count()) return kInputTypeUnknown;

    if(type >= kInputTypeSelfDefineStart && type <= kInputTypeSelfDefineStop)
    {
        if(input_num <= 1) type = kInputTypePowerOn;
        else if(input_num <= 2) type = kInputTypeChargerConnect;
        else type = kInputTypeSelfDefineStart + input_num - 3;
    }
    return type;
}
