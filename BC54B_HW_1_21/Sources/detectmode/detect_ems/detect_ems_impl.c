/**
 *
 * Copyright (c) 2017 Ligoo Inc.
 *
 * @file detect_ems_impl.c
 * @brief
 * @note
 * @author
 * @date 2017-11-1
 *
 */
#include "detect_ems_impl.h"
#include "bms_ems_impl.h"
#include "detect_ems_message.h" 
#include "bms_power_control.h"
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#include "CanIf.h"      // for bootloader
#endif
#if BMS_SUPPORT_EMS == BMS_EMS_DEFAULT

#pragma MESSAGE DISABLE C1860  // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C5703  // Parameter is not used
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

/**********************************************
 * Definitions
 ***********************************************/
#define DBD_PREIOD            1000  //仪表通信时间
#define DBD_MIN_INTERVAL      5     //仪表通信最小间隔



/**********************************************
 * Functions
 ***********************************************/
void detect_ems_init(void)
{
    /** init the information */
    safe_memset(&g_ems_context, 0, sizeof(g_ems_context));   //g_ems_context清零
    safe_memset(&g_ems_can_info, 0, sizeof(g_ems_can_info)); //g_ems_can_info清零
    
    g_ems_can_info.dev = (INT8U)config_get(kEmsCanChannelIndex); 
     
    if(can_channel_is_valid(g_ems_can_info.dev) == FALSE)
    {
        g_ems_can_info.dev = EMS_CAN_DEV;
        bcu_reset_all_can_channel();
    }
    if(g_ems_can_info.dev == kCanDev3) 
        g_ems_can_info.bps = can_get_bps((INT8U)config_get(kCAN5BpsIndex));
    else 
        g_ems_can_info.bps = can_get_bps((INT8U)config_get(kCAN1BpsIndex + g_ems_can_info.dev));
 
    g_ems_can_info.receive_id = EMS_CAN_RECEIVE_ID;
    g_ems_can_info.mask_id = EMS_CAN_RECEIVE_MASK_ID;    
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
    g_ems_can_info.receive_1_id = CanIfRxPduConfig[0].CanId;
    g_ems_can_info.mask_1_id = 0x7ff;
#else
    g_ems_can_info.receive_1_id = EMS_CAN_RECEIVE_1_ID;
    g_ems_can_info.mask_1_id = EMS_CAN_RECEIVE_MASK_1_ID;
#endif    
    g_ems_can_info.mode = EMS_CAN_MODE;
    g_ems_can_info.mode_1 = EMS_CAN_MODE;
    g_ems_can_info.buffers = g_ems_can_buffers;
    g_ems_can_info.buffer_count = EMS_CAN_BUFFER_COUNT;
    g_ems_can_info.filtemode=CanFilte32;
    g_ems_context.can_context   = can_init(&g_ems_can_info);

    if(detect_ems_power_control_command_is_enable() == 1) 
        detect_ems_power_off();
    
    detect_ems_task_create();    //创建任务
}

void detect_ems_uninit(void)
{
    can_uninit(g_ems_context.can_context);
    g_ems_context.can_context = NULL;
}

void detect_ems_task_create(void)   //任务创建
{
    if(config_get(kBCUCommModeIndex) == BMS_SELF_CHECK_MODE)  //自检模式
    {
        OSTaskCreate(detect_ems_task_self_check_run, (void *) NULL, (OS_STK *) &g_ems_ecu_tx_task_stack[ECU_TX_STK_SIZE - 1], //
                ECUCAN_TX_TASK_PRIO);
    }
    else
    {           
        OSTaskCreate(detect_ems_task_ecu_tx_run, (void *) NULL, (OS_STK *) &g_ems_ecu_tx_task_stack[ECU_TX_STK_SIZE - 1], //
                ECUCAN_TX_TASK_PRIO);    //整车通信发送任务，发送各种BMS采集的信息。
                
        OSTaskCreate(detect_ems_task_ecu_rx_run, (void *) NULL, (OS_STK *) &g_ems_ecu_rx_task_stack[ECU_RX_STK_SIZE - 1], //
                ECUCAN_RX_TASK_PRIO);    //整车通信接收任务，接收各种需BMS处理的信息(包含UDS收发任务)。

        OSTaskCreate(detect_ems_task_dbd_tx_run, (void *) NULL, (OS_STK *) &g_ems_dbd_tx_task_stack[DBD_TX_STK_SIZE - 1], //
                DBDCAN_TX_TASK_PRIO);    //仪表通信发送任务，发送bms采集的单体信息。
    }
}

/*****************************************************************************
 *函数名称:detect_ems_get_chg_power_to_0_percent_flag
 *函数功能:获取充电降流至0的故障位
 *参    数:                                      
 *返 回 值:
 *修订信息:
 ******************************************************************************/
Result detect_ems_get_chg_power_to_0_percent_flag(void)
{
    if((bcu_get_high_temperature_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_low_temperature_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_charge_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_high_total_volt_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_chg_delta_volt_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_chg_delta_temp_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_chg_oc_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_battery_insulation_state() >= CURRENT_THIRD_ALARM)
    || ((bcu_get_chgr_outlet_high_temperature_state() >= kAlarmSecondLevel) && (charger_is_connected() == TRUE))
    )
    {
        return TRUE;
    }   
    return FALSE;
}

/*****************************************************************************
 *函数名称:detect_ems_get_chg_power_to_20_percent_flag
 *函数功能:获取充电降流至20%的故障位
 *参    数:                                      
 *返 回 值:
 *修订信息:
 ******************************************************************************/
Result detect_ems_get_chg_power_to_20_percent_flag(void)
{
    if((bcu_get_high_temperature_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_low_temperature_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_charge_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_high_total_volt_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_chg_delta_volt_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_chg_delta_temp_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_chg_oc_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_battery_insulation_state() >= CURRENT_SECOND_ALARM)
    || ((bcu_get_chgr_outlet_high_temperature_state() >= kAlarmFirstLevel) && (charger_is_connected() == TRUE))
    )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
 *函数名称:detect_ems_get_chg_power_to_50_percent_flag
 *函数功能:获取充电降流至50%的故障位
 *参    数:                                      
 *返 回 值:
 *修订信息:
 ******************************************************************************/
Result detect_ems_get_chg_power_to_50_percent_flag(void)
{
    if((bcu_get_high_temperature_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_low_temperature_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_charge_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_high_total_volt_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_chg_delta_volt_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_chg_delta_temp_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_chg_oc_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_battery_insulation_state() >= CURRENT_FIRST_ALARM)
    )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
 *函数名称:detect_ems_get_dchg_power_to_0_percent_flag
 *函数功能:获取放电降流至0的故障位
 *参    数:                                      
 *返 回 值:
 *修订信息:
 ******************************************************************************/
Result detect_ems_get_dchg_power_to_0_percent_flag(void)
{
    if((bcu_get_high_temperature_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_low_temperature_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_discharge_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_low_total_volt_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_dchg_delta_volt_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_dchg_delta_temp_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_low_soc_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_dchg_oc_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_battery_insulation_state() >= CURRENT_THIRD_ALARM)
    )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
 *函数名称:detect_ems_get_dchg_power_to_20_percent_flag
 *函数功能:获取放电降流至20的故障位
 *参    数:                                      
 *返 回 值:
 *修订信息:
 ******************************************************************************/
Result detect_ems_get_dchg_power_to_20_percent_flag(void)
{
    if((bcu_get_high_temperature_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_low_temperature_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_discharge_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_low_total_volt_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_dchg_delta_volt_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_dchg_delta_temp_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_low_soc_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_dchg_oc_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_battery_insulation_state() >= CURRENT_SECOND_ALARM)
    )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
 *函数名称:detect_ems_get_dchg_power_to_50_percent_flag
 *函数功能:获取放电降流至50的故障位
 *参    数:                                      
 *返 回 值:
 *修订信息:
 ******************************************************************************/
Result detect_ems_get_dchg_power_to_50_percent_flag(void)
{
    if((bcu_get_high_temperature_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_low_temperature_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_discharge_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_low_total_volt_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_dchg_delta_volt_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_dchg_delta_temp_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_low_soc_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_dchg_oc_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_battery_insulation_state() >= CURRENT_FIRST_ALARM)
    )
    {
        return TRUE;
    }
    return FALSE;
}   

/*****************************************************************************
 *函数名称:detect_ems_get_charger_continue_current
 *函数功能:获取最大可持续充电电流
 *参    数:                                      
 *返 回 值:最大可持续充电电流
 *修订信息:
 ******************************************************************************/
INT16U detect_ems_charger_continue_current_update(void)
{
    INT16U current = bms_get_charge_continue_current_max();
    
    if(detect_ems_get_chg_power_to_0_percent_flag() == TRUE)
    {
        current = 0;
    }
    else if(detect_ems_get_chg_power_to_20_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 5);
    }
    else if(detect_ems_get_chg_power_to_50_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 2);
    }
    
    return current;
}

/*****************************************************************************
 *函数名称:detect_ems_discharge_continue_current_update
 *函数功能:获取最大可持续放电电流
 *参    数:                                      
 *返 回 值:最大可持续放电电流
 *修订信息:
 ******************************************************************************/       
INT16U detect_ems_discharge_continue_current_update(void)
{
    INT16U current = bms_get_discharge_continue_current_max();
    
    if(detect_ems_get_dchg_power_to_0_percent_flag() == TRUE)
    {
        current = 0;
    } 
    else if(detect_ems_get_dchg_power_to_20_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 5);
    }
    else if(detect_ems_get_dchg_power_to_50_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 2);
    }
    
    return current;
}

/*****************************************************************************
 *函数名称:detect_ems_charge_current_update
 *函数功能:获取最大短时充电电流
 *参    数:                                      
 *返 回 值:最大短时充电电电流
 *修订信息:
 ******************************************************************************/  
INT16U detect_ems_charge_current_update(void)
{
    INT16U current = bms_get_charge_current_max();
    
    if(detect_ems_get_chg_power_to_0_percent_flag() == TRUE)
    {
        current = 0;
    }
    else if(detect_ems_get_chg_power_to_20_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 5);
    }
    else if(detect_ems_get_chg_power_to_50_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 2);
    }
    
    return current;
}

/*****************************************************************************
 *函数名称:detect_ems_discharge_current_update
 *函数功能:获取最大短时放电电流
 *参    数:                                      
 *返 回 值:最大短时放电电流
 *修订信息:
 ******************************************************************************/  
INT16U detect_ems_discharge_current_update(void)
{
    INT16U current = bms_get_discharge_current_max();
    
    if(detect_ems_get_dchg_power_to_0_percent_flag() == TRUE)
    {
        current = 0;
    } 
    else if(detect_ems_get_dchg_power_to_20_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 5);
    }
    else if(detect_ems_get_dchg_power_to_50_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 2);
    }
    
    return current;
}

/*****************************************************************************
 *函数名称:detect_ems_chg_fault_level_update
 *函数功能:获取充电故障等级
 *参    数:                                      
 *返 回 值:充电故障等级
 *修订信息:
 ******************************************************************************/      
AlarmLevel detect_ems_chg_fault_level_update(void)
{
    AlarmLevel alarm_level = kAlarmNone;
#if BMS_FORTH_ALARM_SUPPORT   
    if((bcu_get_chg_hv_state() >= kAlarmForthLevel)
    || (bcu_get_chg_htv_state() >= kAlarmForthLevel)
    || (bcu_get_chg_ht_state() >= kAlarmForthLevel)
    || (bcu_get_chg_lt_state() >= kAlarmForthLevel)
    || (bcu_get_chg_delta_volt_state() >= kAlarmForthLevel)
    || (bcu_get_chg_delta_temp_state() >= kAlarmForthLevel)
    || (bcu_get_chg_oc_state() >= kAlarmForthLevel)
    || (bcu_get_high_soc_state() >= kAlarmForthLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmForthLevel)
    || (bcu_get_chgr_dc_outlet_ht_state() == kAlarmSecondLevel)
    || (bcu_get_slave_communication_state() != kAlarmNone)
    || (bcu_get_voltage_exception_state() != kAlarmNone)
    || (bcu_get_temp_exception_state() != kAlarmNone)
    )
    {
        alarm_level = kAlarmForthLevel;
    }
    else if((bcu_get_chg_hv_state() >= kAlarmThirdLevel)
#else
    if((bcu_get_chg_hv_state() >= kAlarmThirdLevel)
#endif
    || (bcu_get_chg_htv_state() >= kAlarmThirdLevel)
    || (bcu_get_chg_ht_state() >= kAlarmThirdLevel)
    || (bcu_get_chg_lt_state() >= kAlarmThirdLevel)
    || (bcu_get_chg_delta_volt_state() >= kAlarmThirdLevel)
    || (bcu_get_chg_delta_temp_state() >= kAlarmThirdLevel)
    || (bcu_get_chg_oc_state() >= kAlarmThirdLevel)
    || (bcu_get_high_soc_state() >= kAlarmThirdLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmThirdLevel)
#if BMS_FORTH_ALARM_SUPPORT 
    || (bcu_get_chgr_dc_outlet_ht_state() == kAlarmFirstLevel)
#else
    || (bcu_get_chgr_dc_outlet_ht_state() == kAlarmSecondLevel)
    || (bcu_get_slave_communication_state() != kAlarmNone)
    || (bcu_get_voltage_exception_state() != kAlarmNone)
    || (bcu_get_temp_exception_state() != kAlarmNone)
#endif
    )
    {
        alarm_level = kAlarmThirdLevel;
    }
    else if((bcu_get_chg_hv_state() >= kAlarmSecondLevel)
    || (bcu_get_chg_htv_state() >= kAlarmSecondLevel)
    || (bcu_get_chg_ht_state() >= kAlarmSecondLevel)
    || (bcu_get_chg_lt_state() >= kAlarmSecondLevel)
    || (bcu_get_chg_delta_volt_state() >= kAlarmSecondLevel)
    || (bcu_get_chg_delta_temp_state() >= kAlarmSecondLevel)
    || (bcu_get_chg_oc_state() >= kAlarmSecondLevel)
    || (bcu_get_high_soc_state() >= kAlarmSecondLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmSecondLevel)
#if BMS_FORTH_ALARM_SUPPORT 
    
#else
    || (bcu_get_chgr_dc_outlet_ht_state() == kAlarmFirstLevel)
#endif
    )
    {
        alarm_level = kAlarmSecondLevel;
    }
    else if((bcu_get_chg_hv_state() >= kAlarmFirstLevel)
    || (bcu_get_chg_htv_state() >= kAlarmFirstLevel)
    || (bcu_get_chg_ht_state() >= kAlarmFirstLevel)
    || (bcu_get_chg_lt_state() >= kAlarmFirstLevel)
    || (bcu_get_chg_delta_volt_state() >= kAlarmFirstLevel)
    || (bcu_get_chg_delta_temp_state() >= kAlarmFirstLevel)
    || (bcu_get_chg_oc_state() >= kAlarmFirstLevel)
    || (bcu_get_high_soc_state() >= kAlarmFirstLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmFirstLevel)
    )
    {
        alarm_level = kAlarmFirstLevel;
    }
    else
    {
        alarm_level = kAlarmNone;
    }
    
    return alarm_level;
}

/*****************************************************************************
 *函数名称:detect_ems_dchg_fault_level_update
 *函数功能:获取放电故障等级
 *参    数:                                      
 *返 回 值:放电故障等级
 *修订信息:
 ******************************************************************************/ 
AlarmLevel detect_ems_dchg_fault_level_update(void)
{
    AlarmLevel alarm_level = kAlarmNone;
#if BMS_FORTH_ALARM_SUPPORT 
    if((bcu_get_dchg_lv_state() >= kAlarmForthLevel)
    || (bcu_get_dchg_ltv_state() >= kAlarmForthLevel)
    || (bcu_get_dchg_ht_state() >= kAlarmForthLevel)
    || (bcu_get_dchg_lt_state() >= kAlarmForthLevel)
    || (bcu_get_dchg_delta_volt_state() >= kAlarmForthLevel)
    || (bcu_get_dchg_delta_temp_state() >= kAlarmForthLevel)
    || (bcu_get_dchg_oc_state() >= kAlarmForthLevel)
    || (bcu_get_feedback_oc_state() >= kAlarmForthLevel)
    || (bcu_get_low_soc_state() >= kAlarmForthLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmForthLevel)
    || (bcu_get_slave_communication_state() != kAlarmNone)
    || (bcu_get_voltage_exception_state() != kAlarmNone)
    || (bcu_get_temp_exception_state() != kAlarmNone)
    )
    {
        alarm_level = kAlarmForthLevel;
    }
    else if((bcu_get_dchg_lv_state() >= kAlarmThirdLevel)
#else
    if((bcu_get_dchg_lv_state() >= kAlarmThirdLevel)
#endif
    || (bcu_get_dchg_ltv_state() >= kAlarmThirdLevel)
    || (bcu_get_dchg_ht_state() >= kAlarmThirdLevel)
    || (bcu_get_dchg_lt_state() >= kAlarmThirdLevel)
    || (bcu_get_dchg_delta_volt_state() >= kAlarmThirdLevel)
    || (bcu_get_dchg_delta_temp_state() >= kAlarmThirdLevel)
    || (bcu_get_dchg_oc_state() >= kAlarmThirdLevel)
    || (bcu_get_feedback_oc_state() >= kAlarmThirdLevel)
    || (bcu_get_low_soc_state() >= kAlarmThirdLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmThirdLevel)
#if BMS_FORTH_ALARM_SUPPORT
#else
    || (bcu_get_slave_communication_state() != kAlarmNone)
    || (bcu_get_voltage_exception_state() != kAlarmNone)
    || (bcu_get_temp_exception_state() != kAlarmNone)
#endif
    )
    {
        alarm_level = kAlarmThirdLevel;
    }
    else if((bcu_get_dchg_lv_state() >= kAlarmSecondLevel)
    || (bcu_get_dchg_ltv_state() >= kAlarmSecondLevel)
    || (bcu_get_dchg_ht_state() >= kAlarmSecondLevel)
    || (bcu_get_dchg_lt_state() >= kAlarmSecondLevel)
    || (bcu_get_dchg_delta_volt_state() >= kAlarmSecondLevel)
    || (bcu_get_dchg_delta_temp_state() >= kAlarmSecondLevel)
    || (bcu_get_dchg_oc_state() >= kAlarmSecondLevel)
    || (bcu_get_feedback_oc_state() >= kAlarmSecondLevel)
    || (bcu_get_low_soc_state() >= kAlarmSecondLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmSecondLevel)
    )
    {
        alarm_level = kAlarmSecondLevel;
    }
    else if((bcu_get_dchg_lv_state() >= kAlarmFirstLevel)
    || (bcu_get_dchg_ltv_state() >= kAlarmFirstLevel)
    || (bcu_get_dchg_ht_state() >= kAlarmFirstLevel)
    || (bcu_get_dchg_lt_state() >= kAlarmFirstLevel)
    || (bcu_get_dchg_delta_volt_state() >= kAlarmFirstLevel)
    || (bcu_get_dchg_delta_temp_state() >= kAlarmFirstLevel)
    || (bcu_get_dchg_oc_state() >= kAlarmFirstLevel)
    || (bcu_get_feedback_oc_state() >= kAlarmFirstLevel) 
    || (bcu_get_low_soc_state() >= kAlarmFirstLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmFirstLevel)
    )
    {
        alarm_level = kAlarmFirstLevel;
    }
    else
    {
        alarm_level = kAlarmNone;
    }
    
    return alarm_level;
}
    
/*****************************************************************************
 *函数名称:detect_ems_bms_alarm_level_update
 *函数功能:获取BMS故障等级
 *参    数:                                      
 *返 回 值:BMS故障等级
 *修订信息:
 ******************************************************************************/ 
AlarmLevel detect_ems_bms_alarm_level_update(void)
{
    AlarmLevel alarm_level = kAlarmNone;
        
    if(charger_is_connected() == FALSE)
    {
        alarm_level = ems_dchg_fault_level_update();
    }
    else
    {
        alarm_level = ems_chg_fault_level_update(); 
    }
    return alarm_level;
}

/*****************************************************************************
 *函数名称:detect_ems_get_Flt_from_alarm
 *函数功能:根据故障等级获取错误码(0:kAlarmNone,1:kAlarmFirstLevel,2:kAlarmSecondLevel,3:kAlarmThirdLevel,4:kAlarmForthLevel)
 *参    数:                                      
 *返 回 值:
 *修订信息:
 ******************************************************************************/ 
INT8U detect_ems_get_Lv4_from_alarm(AlarmLevel alarm_level)
{
    INT8U temp_value = 0;
    switch(alarm_level)
    {
#if BMS_FORTH_ALARM_SUPPORT
        case kAlarmForthLevel:
            temp_value = 4;
            break;
#endif
        case kAlarmThirdLevel:
            temp_value = 3;
            break;
        case kAlarmSecondLevel:
            temp_value = 2;
            break;
        case kAlarmFirstLevel:
            temp_value = 1;
            break;
        case kAlarmNone:
            temp_value = 0;
            break;
        default:
            temp_value = 0;
            break;
    }
    return temp_value;
}

/*****************************************************************************
 *函数名称:detect_ems_get_Flt_from_alarm
 *函数功能:根据故障等级获取错误码(0:kAlarmNone,kAlarmFirstLevel,1:kAlarmSecondLevel,2:kAlarmThirdLevel,3:kAlarmForthLevel)
 *参    数:                                      
 *返 回 值:
 *修订信息:
 ******************************************************************************/ 
INT8U detect_ems_get_Lv3_from_alarm(AlarmLevel alarm_level)
{
    INT8U temp_value = 0;
    switch(alarm_level)
    {
#if BMS_FORTH_ALARM_SUPPORT
        case kAlarmForthLevel:
            temp_value = 3;
            break;
#endif
        case kAlarmThirdLevel:
            temp_value = 2;
            break;
        case kAlarmSecondLevel:
            temp_value = 1;
            break;
        case kAlarmFirstLevel:
        case kAlarmNone:
            temp_value = 0;
            break;
        default:
            temp_value = 0;
            break;
    }
    return temp_value;
}

/*****************************************************************************
 *函数名称:detect_ems_total_fault_num_update
 *函数功能:获取系统总故障数
 *参    数:                                      
 *返 回 值:系统总故障数
 *修订信息:
 ******************************************************************************/ 
INT8U detect_ems_total_fault_num_update(void)
{
    INT8U temp_value = 0;
   
    if(bcu_get_delta_temperature_state() >= kAlarmSecondLevel)
    {
        temp_value++;
    }
    if(bcu_get_delta_voltage_state() >= kAlarmSecondLevel)
    {
        temp_value++;
    }
    if(bcu_get_high_temperature_state() >= kAlarmSecondLevel)
    {
        temp_value++;
    }
    if(bcu_get_low_temperature_state() >= kAlarmSecondLevel)
    {
        temp_value++;
    }
    if(bcu_get_chg_htv_state() >= kAlarmSecondLevel && charger_is_connected() == TRUE)
    {
        temp_value++;
    }
    if(bcu_get_dchg_ltv_state() >= kAlarmSecondLevel && charger_is_connected() == FALSE)
    {
        temp_value++;
    }
    if(bcu_get_high_soc_state() >= kAlarmSecondLevel && charger_is_connected() == TRUE)
    {
        temp_value++;
    }
    if(bcu_get_low_soc_state() >= kAlarmSecondLevel && charger_is_connected() == FALSE)
    {   
        temp_value++;
    }
    if(bcu_get_charge_state() >= kAlarmSecondLevel && charger_is_connected() == TRUE)
    {
        temp_value++;
    }
    if(bcu_get_discharge_state() >= kAlarmSecondLevel && charger_is_connected() == FALSE)
    {
        temp_value++;
    }
    if(bcu_get_chg_oc_state() >= kAlarmSecondLevel)
    {
        temp_value++;
    }
    if(bcu_get_dchg_oc_state() >= kAlarmSecondLevel)
    {
        temp_value++;
    }
    if(bcu_get_battery_insulation_state() >= kAlarmSecondLevel)
    {
        temp_value++;
    }

    return temp_value;
}

/*****************************************************************************
 *函数名称:detect_ems_max_fault_level_update
 *函数功能:获取系统最高故障等级
 *参    数:                                      
 *返 回 值:系统最高故障等级
 *修订信息:
 ******************************************************************************/ 
AlarmLevel detect_ems_max_fault_level_update(void)
{
    AlarmLevel temp_value = kAlarmNone;
#if BMS_FORTH_ALARM_SUPPORT
    if((bcu_get_delta_temperature_state() >= kAlarmForthLevel)
    || (bcu_get_delta_voltage_state() >= kAlarmForthLevel)
    || (bcu_get_high_temperature_state() >= kAlarmForthLevel)
    || (bcu_get_low_temperature_state() >= kAlarmForthLevel)
    || ((bcu_get_chg_htv_state() >= kAlarmForthLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_dchg_ltv_state() >= kAlarmForthLevel) && (charger_is_connected() == FALSE))
    || ((bcu_get_high_soc_state() >= kAlarmForthLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_low_soc_state() >= kAlarmForthLevel) && (charger_is_connected() == FALSE))
    || ((bcu_get_charge_state() >= kAlarmForthLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_discharge_state() >= kAlarmForthLevel) && (charger_is_connected() == FALSE))
    || (bcu_get_chg_oc_state() >= kAlarmForthLevel)
    || (bcu_get_dchg_oc_state() >= kAlarmForthLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmForthLevel)
    || ((bcu_get_chgr_dc_outlet_ht_state() >= kAlarmSecondLevel) && (charger_is_connected()== TRUE))   //充电口高温二级故障
    || bcu_get_slave_communication_state() != kAlarmNone 
    || bcu_get_voltage_exception_state() != kAlarmNone
    || bcu_get_temp_exception_state() != kAlarmNone
    )
    {
        temp_value = kAlarmForthLevel;
    }
    else if((bcu_get_delta_temperature_state() >= kAlarmThirdLevel)
#else
    if((bcu_get_delta_temperature_state() >= kAlarmThirdLevel)
#endif
    || (bcu_get_delta_voltage_state() >= kAlarmThirdLevel)  
    || (bcu_get_high_temperature_state() >= kAlarmThirdLevel)
    || (bcu_get_low_temperature_state() >= kAlarmThirdLevel)
    || ((bcu_get_chg_htv_state() >= kAlarmThirdLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_dchg_ltv_state() >= kAlarmThirdLevel) && (charger_is_connected() == FALSE))
    || ((bcu_get_high_soc_state() >= kAlarmThirdLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_low_soc_state() >= kAlarmThirdLevel) && (charger_is_connected() == FALSE))
    || ((bcu_get_charge_state() >= kAlarmThirdLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_discharge_state() >= kAlarmThirdLevel) && (charger_is_connected() == FALSE))
    || (bcu_get_chg_oc_state() >= kAlarmThirdLevel)
    || (bcu_get_dchg_oc_state() >= kAlarmThirdLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmThirdLevel)
#if BMS_FORTH_ALARM_SUPPORT
    || ((bcu_get_chgr_dc_outlet_ht_state() >= kAlarmSecondLevel) && (charger_is_connected()== TRUE))//充电口高温一级故障
#else
    || ((bcu_get_chgr_dc_outlet_ht_state() >= kAlarmFirstLevel) && (charger_is_connected()== TRUE))   //充电口高温二级故障
    || bcu_get_slave_communication_state() != kAlarmNone 
    || bcu_get_voltage_exception_state() != kAlarmNone
    || bcu_get_temp_exception_state() != kAlarmNone
#endif
    )
    {
        temp_value = kAlarmThirdLevel;
    }
    else if((bcu_get_delta_temperature_state() >= kAlarmSecondLevel)
    || (bcu_get_delta_voltage_state() >= kAlarmSecondLevel)
    || (bcu_get_high_temperature_state() >= kAlarmSecondLevel)
    || (bcu_get_low_temperature_state() >= kAlarmSecondLevel)
    || ((bcu_get_chg_htv_state() >= kAlarmSecondLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_dchg_ltv_state() >= kAlarmSecondLevel) && (charger_is_connected() == FALSE))
    || ((bcu_get_high_soc_state() >= kAlarmSecondLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_low_soc_state() >= kAlarmSecondLevel) && (charger_is_connected() == FALSE))
    || ((bcu_get_charge_state() >= kAlarmSecondLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_discharge_state() >= kAlarmSecondLevel) && (charger_is_connected() == FALSE))
    || (bcu_get_chg_oc_state() >= kAlarmSecondLevel)
    || (bcu_get_dchg_oc_state() >= kAlarmSecondLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmSecondLevel)
#if BMS_FORTH_ALARM_SUPPORT
#else
    || ((bcu_get_chgr_dc_outlet_ht_state() >= kAlarmFirstLevel) && (charger_is_connected()== TRUE))//充电口高温一级故障
#endif  
    )
    {
        temp_value = kAlarmSecondLevel;
    }
    else if((bcu_get_delta_temperature_state() >= kAlarmFirstLevel)
    || (bcu_get_delta_voltage_state() >= kAlarmFirstLevel)
    || (bcu_get_high_temperature_state() >= kAlarmFirstLevel)
    || (bcu_get_low_temperature_state() >= kAlarmSecondLevel)
    || ((bcu_get_chg_htv_state() >= kAlarmFirstLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_dchg_ltv_state() >= kAlarmFirstLevel) && (charger_is_connected() == FALSE))
    || ((bcu_get_high_soc_state() >= kAlarmFirstLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_low_soc_state() >= kAlarmFirstLevel) && (charger_is_connected() == FALSE))
    || ((bcu_get_charge_state() >= kAlarmFirstLevel) && (charger_is_connected() == TRUE))
    || ((bcu_get_discharge_state() >= kAlarmFirstLevel) && (charger_is_connected() == FALSE))
    || (bcu_get_chg_oc_state() >= kAlarmFirstLevel)
    || (bcu_get_dchg_oc_state() >= kAlarmFirstLevel)
    || (bcu_get_battery_insulation_state() >= kAlarmFirstLevel)
    )
    {
        temp_value = kAlarmFirstLevel;
    }
    else
    {
        temp_value = kAlarmNone;
    }
    return temp_value;
}
 
/***********************OTHER FUNCTION START*******************************/

/***********************OTHER FUNCTION END*********************************/

 
//整车控制上下电命令使能检查函数
INT8U detect_ems_power_control_command_is_enable(void)
{
    return 0; //默认不使能
}

void detect_ems_power_on(void)
{
    relay_control_set_enable_flag(kRelayTypeDischarging, TRUE);
}

void detect_ems_power_off(void)
{
    relay_control_set_enable_flag(kRelayTypeDischarging, FALSE);
}

// self check function
void detect_ems_task_self_check_message(void)
{
    g_ems_ecu_message.data[0] = 0x00;
    g_ems_ecu_message.data[1] = 0x01;
    g_ems_ecu_message.data[2] = 0x02;
    g_ems_ecu_message.data[3] = 0x03;
    g_ems_ecu_message.data[4] = 0x04;   
    g_ems_ecu_message.data[5] = 0x05;
    g_ems_ecu_message.data[6] = 0x06;
    g_ems_ecu_message.data[7] = 0x07;

    g_ems_ecu_message.id.value = can_id_from_extend_id(SELF_CHECK_CHARGER_RECEIVE_ID);
    g_ems_ecu_message.len = CAN_DATA_MAX_LEN;
    can_send(g_ems_context.can_context, &g_ems_ecu_message);
}

void detect_ems_task_self_check_run(void *pdata)
{
   for (;;)
   {
      detect_ems_task_self_check_message();
      sleep(100);
   }
}

#endif
