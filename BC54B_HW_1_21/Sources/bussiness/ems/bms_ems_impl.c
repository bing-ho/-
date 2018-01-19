/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_ems_impl.c
 * @brief
 * @note
 * @author
 * @date 2012-5-24
 *
 */
#include "bms_ems_impl.h"
#include "ems_message.h" 
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
#define DBD_PREIOD            1000  //�Ǳ�ͨ��ʱ��
#define DBD_MIN_INTERVAL      5     //�Ǳ�ͨ����С���

OS_STK g_ems_ecu_tx_task_stack[ECU_TX_STK_SIZE] = { 0 };    //����ͨ�ŷ��������ջ
OS_STK g_ems_ecu_rx_task_stack[ECU_RX_STK_SIZE] = { 0 };    //����ͨ�Ž��������ջ   
OS_STK g_ems_dbd_tx_task_stack[DBD_TX_STK_SIZE] = { 0 };    //�Ǳ�ͨ�ŷ��������ջ

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_EMS

EmsContext g_ems_context;   //������������

CanInfo g_ems_can_info;     //��ems_buffer����Ϣ���浽g_ems_can_info
CanMessage g_ems_can_buffers[EMS_CAN_BUFFER_COUNT];   //��ems�б��ĵ�10��������

CanMessage g_ems_ecu_message;   //�洢����������Ϣ��
CanMessage g_ems_ecu_rx_message;   //�洢����������Ϣ�� 
CanMessage g_ems_dbd_message;   //�洢�Ǳ�������Ϣ��
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
udsbuffrerinfo    uds_buffrer_info;
CanMessage g_uds_can_buffers[UDS_BUFFER_COUNTER]; 
 #endif
#pragma DATA_SEG DEFAULT

/**********************************************
 * Functions
 ***********************************************/
void ems_init(void)
{
    /** init the information */
    safe_memset(&g_ems_context, 0, sizeof(g_ems_context));   //g_ems_context����
    safe_memset(&g_ems_can_info, 0, sizeof(g_ems_can_info)); //g_ems_can_info����
    
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
    #if (STANDARD_ID == UDS_CAN_ID_TYPE) 
    g_ems_can_info.mask_1_id    = 0x7ff;
   #else
     g_ems_can_info.mask_1_id   = 0x1fffffff;
   #endif
#else
    g_ems_can_info.receive_1_id = EMS_CAN_RECEIVE_1_ID;
    g_ems_can_info.mask_1_id = EMS_CAN_RECEIVE_MASK_1_ID;
#endif    
    g_ems_can_info.mode = EMS_CAN_MODE;
    g_ems_can_info.mode_1 = EMS_CAN_MODE;
    g_ems_can_info.buffers = g_ems_can_buffers;
    g_ems_can_info.buffer_count = EMS_CAN_BUFFER_COUNT;
    g_ems_can_info.filtemode=CanFilte32;
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
    g_ems_can_info.receive_callback = ems_can_callback;
    
    uds_buffrer_info.buffers    =  g_uds_can_buffers;
    uds_buffrer_info.buffer_count = UDS_BUFFER_COUNTER;
#endif 
    g_ems_context.can_context   = can_init(&g_ems_can_info);

    if(ems_power_control_command_is_enable() == 1) 
        ems_power_off();
    
    ems_task_create();    //��������
}

void ems_uninit(void)
{
    can_uninit(g_ems_context.can_context);
    g_ems_context.can_context = NULL;
}

void ems_task_create(void)   //���񴴽�
{
    if(config_get(kBCUCommModeIndex) == BMS_SELF_CHECK_MODE)  //�Լ�ģʽ
    {
        OSTaskCreate(ems_task_self_check_run, (void *) NULL, (OS_STK *) &g_ems_ecu_tx_task_stack[ECU_TX_STK_SIZE - 1], //
                ECUCAN_TX_TASK_PRIO);
    }
    else
    {           
        OSTaskCreate(ems_task_ecu_tx_run, (void *) NULL, (OS_STK *) &g_ems_ecu_tx_task_stack[ECU_TX_STK_SIZE - 1], //
                ECUCAN_TX_TASK_PRIO);    //����ͨ�ŷ������񣬷��͸���BMS�ɼ�����Ϣ��
                
        OSTaskCreate(ems_task_ecu_rx_run, (void *) NULL, (OS_STK *) &g_ems_ecu_rx_task_stack[ECU_RX_STK_SIZE - 1], //
                ECUCAN_RX_TASK_PRIO);    //����ͨ�Ž������񣬽��ո�����BMS��������Ϣ(����UDS�շ�����)��

        OSTaskCreate(ems_task_dbd_tx_run, (void *) NULL, (OS_STK *) &g_ems_dbd_tx_task_stack[DBD_TX_STK_SIZE - 1], //
                DBDCAN_TX_TASK_PRIO);    //�Ǳ�ͨ�ŷ������񣬷���bms�ɼ��ĵ�����Ϣ��
    }
}

/*****************************************************************************
 *��������:ems_get_chg_power_to_0_percent_flag
 *��������:��ȡ��罵����0�Ĺ���λ
 *��    ��:                                      
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
Result ems_get_chg_power_to_0_percent_flag(void)
{
    if((bcu_get_charge_state() >= CURRENT_THIRD_ALARM)
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
 *��������:ems_get_chg_power_to_20_percent_flag
 *��������:��ȡ��罵����20%�Ĺ���λ
 *��    ��:                                      
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
Result ems_get_chg_power_to_20_percent_flag(void)
{
    if((bcu_get_charge_state() >= CURRENT_SECOND_ALARM)
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
 *��������:ems_get_chg_power_to_50_percent_flag
 *��������:��ȡ��罵����50%�Ĺ���λ
 *��    ��:                                      
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
Result ems_get_chg_power_to_50_percent_flag(void)
{
    if((bcu_get_charge_state() >= CURRENT_FIRST_ALARM)
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
 *��������:ems_get_dchg_power_to_0_percent_flag
 *��������:��ȡ�ŵ罵����0�Ĺ���λ
 *��    ��:                                      
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
Result ems_get_dchg_power_to_0_percent_flag(void)
{
    if((bcu_get_discharge_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_low_total_volt_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_dchg_delta_volt_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_dchg_delta_temp_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_dchg_oc_state() >= CURRENT_THIRD_ALARM)
    || (bcu_get_battery_insulation_state() >= CURRENT_THIRD_ALARM)
    )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
 *��������:ems_get_dchg_power_to_20_percent_flag
 *��������:��ȡ�ŵ罵����20�Ĺ���λ
 *��    ��:                                      
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
Result ems_get_dchg_power_to_20_percent_flag(void)
{
    if((bcu_get_discharge_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_low_total_volt_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_dchg_delta_volt_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_dchg_delta_temp_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_dchg_oc_state() >= CURRENT_SECOND_ALARM)
    || (bcu_get_battery_insulation_state() >= CURRENT_SECOND_ALARM)
    )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
 *��������:ems_get_dchg_power_to_50_percent_flag
 *��������:��ȡ�ŵ罵����50�Ĺ���λ
 *��    ��:                                      
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
Result ems_get_dchg_power_to_50_percent_flag(void)
{
    if((bcu_get_discharge_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_low_total_volt_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_dchg_delta_volt_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_dchg_delta_temp_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_dchg_oc_state() >= CURRENT_FIRST_ALARM)
    || (bcu_get_battery_insulation_state() >= CURRENT_FIRST_ALARM)
    )
    {
        return TRUE;
    }
    return FALSE;
}   

/*****************************************************************************
 *��������:ems_get_charger_continue_current
 *��������:��ȡ���ɳ���������
 *��    ��:                                      
 *�� �� ֵ:���ɳ���������
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_charger_continue_current_update(void)
{
    INT16U current = bms_get_charge_continue_current_max();
    
    if(ems_get_chg_power_to_0_percent_flag() == TRUE)
    {
        current = 0;
    }
    else if(ems_get_chg_power_to_20_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 5);
    }
    else if(ems_get_chg_power_to_50_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 2);
    }
    
    return current;
}

/*****************************************************************************
 *��������:ems_discharge_continue_current_update
 *��������:��ȡ���ɳ����ŵ����
 *��    ��:                                      
 *�� �� ֵ:���ɳ����ŵ����
 *�޶���Ϣ:
 ******************************************************************************/       
INT16U ems_discharge_continue_current_update(void)
{
    INT16U current = bms_get_discharge_continue_current_max();
    
    if(ems_get_dchg_power_to_0_percent_flag() == TRUE)
    {
        current = 0;
    } 
    else if(ems_get_dchg_power_to_20_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 5);
    }
    else if(ems_get_dchg_power_to_50_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 2);
    }
    
    return current;
}

/*****************************************************************************
 *��������:ems_charge_current_update
 *��������:��ȡ����ʱ������
 *��    ��:                                      
 *�� �� ֵ:����ʱ�������
 *�޶���Ϣ:
 ******************************************************************************/  
INT16U ems_charge_current_update(void)
{
    INT16U current = bms_get_charge_current_max();
    
    if(ems_get_chg_power_to_0_percent_flag() == TRUE)
    {
        current = 0;
    }
    else if(ems_get_chg_power_to_20_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 5);
    }
    else if(ems_get_chg_power_to_50_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 2);
    }
    
    return current;
}

/*****************************************************************************
 *��������:ems_discharge_current_update
 *��������:��ȡ����ʱ�ŵ����
 *��    ��:                                      
 *�� �� ֵ:����ʱ�ŵ����
 *�޶���Ϣ:
 ******************************************************************************/  
INT16U ems_discharge_current_update(void)
{
    INT16U current = bms_get_discharge_current_max();
    
    if(ems_get_dchg_power_to_0_percent_flag() == TRUE)
    {
        current = 0;
    } 
    else if(ems_get_dchg_power_to_20_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 5);
    }
    else if(ems_get_dchg_power_to_50_percent_flag() == TRUE)
    {
        current = (INT16U)DIVISION(current, 2);
    }
    
    return current;
}

/*****************************************************************************
 *��������:ems_chg_fault_level_update
 *��������:��ȡ�����ϵȼ�
 *��    ��:                                      
 *�� �� ֵ:�����ϵȼ�
 *�޶���Ϣ:
 ******************************************************************************/      
AlarmLevel ems_chg_fault_level_update(void)
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
 *��������:ems_dchg_fault_level_update
 *��������:��ȡ�ŵ���ϵȼ�
 *��    ��:                                      
 *�� �� ֵ:�ŵ���ϵȼ�
 *�޶���Ϣ:
 ******************************************************************************/ 
AlarmLevel ems_dchg_fault_level_update(void)
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
 *��������:ems_bms_alarm_level_update
 *��������:��ȡBMS���ϵȼ�
 *��    ��:                                      
 *�� �� ֵ:BMS���ϵȼ�
 *�޶���Ϣ:
 ******************************************************************************/ 
AlarmLevel ems_bms_alarm_level_update(void)
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
 *��������:ems_get_Flt_from_alarm
 *��������:���ݹ��ϵȼ���ȡ������(0:kAlarmNone,1:kAlarmFirstLevel,2:kAlarmSecondLevel,3:kAlarmThirdLevel,4:kAlarmForthLevel)
 *��    ��:                                      
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/ 
INT8U ems_get_Lv4_from_alarm(AlarmLevel alarm_level)
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
 *��������:ems_get_Flt_from_alarm
 *��������:���ݹ��ϵȼ���ȡ������(0:kAlarmNone,kAlarmFirstLevel,1:kAlarmSecondLevel,2:kAlarmThirdLevel,3:kAlarmForthLevel)
 *��    ��:                                      
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/ 
INT8U ems_get_Lv3_from_alarm(AlarmLevel alarm_level)
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
 *��������:ems_total_fault_num_update
 *��������:��ȡϵͳ�ܹ�����
 *��    ��:                                      
 *�� �� ֵ:ϵͳ�ܹ�����
 *�޶���Ϣ:
 ******************************************************************************/ 
INT8U ems_total_fault_num_update(void)
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
 *��������:ems_max_fault_level_update
 *��������:��ȡϵͳ��߹��ϵȼ�
 *��    ��:                                      
 *�� �� ֵ:ϵͳ��߹��ϵȼ�
 *�޶���Ϣ:
 ******************************************************************************/ 
AlarmLevel ems_max_fault_level_update(void)
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
    || ((bcu_get_chgr_dc_outlet_ht_state() >= kAlarmSecondLevel) && (charger_is_connected()== TRUE))   //���ڸ��¶�������
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
    || ((bcu_get_chgr_dc_outlet_ht_state() >= kAlarmSecondLevel) && (charger_is_connected()== TRUE))//���ڸ���һ������
#else
    || ((bcu_get_chgr_dc_outlet_ht_state() >= kAlarmFirstLevel) && (charger_is_connected()== TRUE))   //���ڸ��¶�������
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
    || ((bcu_get_chgr_dc_outlet_ht_state() >= kAlarmFirstLevel) && (charger_is_connected()== TRUE))//���ڸ���һ������
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

 
//�����������µ�����ʹ�ܼ�麯��
INT8U ems_power_control_command_is_enable(void)
{
    return 0; //Ĭ�ϲ�ʹ��
}

void ems_power_on(void)
{
    relay_control_set_enable_flag(kRelayTypeDischarging, TRUE);
}

void ems_power_off(void)
{
    relay_control_set_enable_flag(kRelayTypeDischarging, FALSE);
}

// self check function
void ems_task_self_check_message(void)
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

void ems_task_self_check_run(void *pdata)
{
   for (;;)
   {
      ems_task_self_check_message();
      sleep(100);
   }
}

#if( SWITCH_ON == UDS_SWITCH_CONFIG)


INT16U ems_can_callback(can_t context, CanMessage* msg, void* userdata)
{

    OS_CPU_SR cpu_sr = 0;
    INT32U temp_id = 0;
    UNUSED(userdata);
       OS_ENTER_CRITICAL();
        #if (STANDARD_ID == UDS_CAN_ID_TYPE)  
          temp_id = (INT32U)can_id_to_std_id(msg->id.value);
        #else
          temp_id   = (INT32U)can_id_to_extend_id(msg->id.value); 
        #endif 
        if( (temp_id == (INT32U)(CanIfRxPduConfig[0].CanId)) || (temp_id == (INT32U)(CanIfRxPduConfig[1].CanId))) 
        {
            
            safe_memcpy((PINT8U)&(uds_buffrer_info.buffers[uds_buffrer_info.read_buffer_write_pos]), (PINT8U)msg, sizeof(*msg));
            uds_buffrer_info.read_buffer_write_pos = (INT8U)((1 + uds_buffrer_info.read_buffer_write_pos) % (uds_buffrer_info.buffer_count));
            if (uds_buffrer_info.read_buffer_count < uds_buffrer_info.buffer_count)
            {
                ++uds_buffrer_info.read_buffer_count;
               OS_EXIT_CRITICAL();
          
            }
            else
            {
                uds_buffrer_info.read_buffer_read_pos = (INT8U)((uds_buffrer_info.read_buffer_read_pos + 1) % (uds_buffrer_info.buffer_count));
                OS_EXIT_CRITICAL();
                DEBUG("can", "drop a frame because the buffers are full.");
            }
        }
        else
        {

             safe_memcpy((PINT8U)&(context->info.buffers[context->read_buffer_write_pos]), (PINT8U)msg, sizeof(*msg));
            context->read_buffer_write_pos = (INT8U)((1 + context->read_buffer_write_pos) % (context->info.buffer_count));
            if (context->read_buffer_count < context->info.buffer_count)
            {
                ++context->read_buffer_count;
                OS_EXIT_CRITICAL();
                OSSemPost(context->read_buffer_event);
            }
            else
            {
                context->read_buffer_read_pos = (INT8U)((context->read_buffer_read_pos + 1) % (context->info.buffer_count));
                OS_EXIT_CRITICAL();
                DEBUG("can", "drop a frame because the buffers are full.");
            }

         }   
    
}
#endif

#endif