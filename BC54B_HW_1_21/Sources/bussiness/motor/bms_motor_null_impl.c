/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file bms_motor_null_impl.c
* @brief 
* @note  
* @author 
* @date 2012-5-26  
*  
*/
#include "bms_motor.h"

#if BMS_SUPPORT_MOTOR == 0

#pragma MESSAGE DISABLE C4001 // Condition always FALSE

void motor_init(void)
{
}

void motor_uninit(void)
{
}

BOOLEAN motor_is_connected(void)
{
    return FALSE;
}

Result motor_enable_event(INT16U event)
{
    UNUSED(event);
    return RES_ERROR;
}

Result motor_disable_event(INT16U event)
{
    UNUSED(event);
    return RES_ERROR;
}

Result motor_get_event(INT16U* event)
{
    UNUSED(event);
    return RES_ERROR;
}
#endif



