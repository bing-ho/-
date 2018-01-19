/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_input_signal_control.h
 * @brief
 * @note
 * @author
 * @date 2014-3-1
 *
 */

#ifndef BMS_INPUT_SIGNAL_CONTROL_H_
#define BMS_INPUT_SIGNAL_CONTROL_H_
#include "includes.h"

void input_signal_control_init(void);

typedef enum _InputControlType
{
    kInputTypeUnknown = 0,
    kInputTypeDischarging,
    kInputTypeCharging,
    kInputTypePreCharging,
    kInputTypeHeating,
    kInputTypeCoolint,
    kInputTypeChargerCC,
    kInputTypeChargerConnect,
    kInputTypePowerOn,
    kInputTypeSelfDefine,
    kInputTypeSelfDefineStart = kInputTypeSelfDefine,
    kInputTypeSigOne = kInputTypeSelfDefineStart,
    kInputTypeSigTwo,
    kInputTypeSwitchOne,
    kInputTypeSwitchTwo,
    kInputTypeSwitchThree,
    kInputTypeSwitchFour,
    kInputTypeST1,
    kInputTypeST2,
    kInputTypeST3,
    kInputTypeST4,
    kInputTypeSelfDefineStop = kInputTypeST4,
    kInputTypeReserved,
    kInputTypeMaxCount
}InputControlType;

void input_signal_control_load_config(void);

INT8U input_signal_control_is_high(InputControlType type);
INT8U input_signal_control_is_low(InputControlType type);
INT8U input_signal_control_get_id(InputControlType type);
InputControlType input_signal_control_get_type(INT8U id);
void input_signal_control_set_type(INT8U index, InputControlType type);
/* 对于输入开关信号类型：输入信号1~输入信号2及开关信号1~开关信号4,均转换成自定义类型上传给上位机 */
InputControlType input_signal_control_type_to_computer_display(InputControlType type);
InputControlType input_signal_control_type_from_computer_display(InputControlType type, INT8U input_num);

#endif /* BMS_INPUT_SIGNAL_CONTROL_H_ */
