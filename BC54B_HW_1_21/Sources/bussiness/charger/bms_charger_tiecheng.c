/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_charger.c
 * @brief
 * @note
 * @author
 * @date 2012-5-8
 *
 */
#ifndef BMS_CHARGER_TIECHENG_C
#define BMS_CHARGER_TIECHENG_C

#include "bms_charger_tiecheng.h"
#include "app_cfg.h"
#include "bms_eeprom.h"
#include "bms_util.h"
#include "j1939_cfg.h"
#include "bms_charger_common.h"
#include "bms_charger_none.h"
#include "bms_buffer.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C4001 // Condition always FALSE
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

//int g_charger_status = 0;

/********************************************
 * Data
 ********************************************/

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER
const ChargerDeviceInfo g_tiecheng_charger_info = { 0, 0, "Charger", "", "Tiecheng", "tiecheng" };
#pragma DATA_SEG DEFAULT
INT8U tiecheng_charger_is_connected(void);

/********************************************
 * Function Area
 ********************************************/

void tiecheng_charger_uninit(void)
{
    J1939UnInit(g_chargerCanContextPtr);
    can_uninit(g_can_info[g_j1939ChgCanInfo.dev]);
    g_can_info[g_j1939ChgCanInfo.dev] = NULL;
}

Result tiecheng_charger_on(void)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeOnOff = TIECHENG_CHARGER_ENABLE_CONTROL_BYTE;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result tiecheng_charger_off(void)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeOnOff = TIECHENG_CHARGER_DISABLE_CONTROL_BYTE;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

INT8U tiecheng_charger_is_connected(void)
{
    int cpu_sr = 0;
    INT8U res;

    OS_ENTER_CRITICAL();
    if(ChargerParaIndex.ChargerSts.IsCommunication)
        res = TRUE;
    else
        res = FALSE;
    OS_EXIT_CRITICAL();

    return res;
}

Result tiecheng_charger_is_communication()
{
    OS_CPU_SR cpu_sr = 0;
    BOOLEAN res;

    OS_ENTER_CRITICAL();
    if(ChargerParaIndex.ChargerSts.IsCommunication)
        res = TRUE;
    else
        res = FALSE;
    OS_EXIT_CRITICAL();
    return res;
}

BOOLEAN tiecheng_charger_is_charging(void)
{
    int cpu_sr = 0;
    BOOLEAN res;

    OS_ENTER_CRITICAL();
    if(ChargerParaIndex.ChargerSts.IsCharging && ChargerParaIndex.ChargerSts.IsCommunication)
        res = TRUE;
    else
        res = FALSE;
    OS_EXIT_CRITICAL();

    return res;
}

Result tiecheng_charger_charge_is_ready_status(void)
{
    return tiecheng_charger_is_connected();
}

Result tiecheng_charger_get_status(INT8U* status)
{
    int cpu_sr = 0;
    if (status == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *status = ChargerParaIndex.ChargerSts.ChgrExceptStatus[0];
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result tiecheng_charger_get_device_info(ChargerDeviceInfo* info)
{
    return RES_ERROR;
}

INT16U tiecheng_charger_get_type(void)
{
    return g_tiecheng_charger_info.type;
}

Result tiecheng_charger_set_current(INT16U current)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeCur = current;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result tiecheng_charger_get_current(INT16U* current)
{
    int cpu_sr = 0;
    if (current == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *current = ChargerParaIndex.ChgrSetCfg.ChargeCur;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result tiecheng_charger_set_max_voltage(INT16U voltage)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeVolt = voltage;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result tiecheng_charger_get_max_voltage(INT16U* voltage)
{
    int cpu_sr = 0;
    if (voltage == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *voltage = ChargerParaIndex.ChgrSetCfg.ChargeVolt;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result tiecheng_charger_get_output_voltage(INT16U* voltage)
{
    int cpu_sr = 0;
    if (voltage == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *voltage = ChargerParaIndex.ChargerSts.OutputChgVolt;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result tiecheng_charger_get_output_current(INT16U* current)
{
    int cpu_sr = 0;
    if (current == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *current = ChargerParaIndex.ChargerSts.OutputChgCur;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

INT8U tiecheng_charger_charging_is_enable(void)
{
    if(ChargerParaIndex.ChgrSetCfg.ChargeOnOff == TIECHENG_CHARGER_ENABLE_CONTROL_BYTE) return TRUE;
    
    return FALSE;
}

void tiecheng_charger_task_create(void)
{
    OSTaskCreate(tiecheng_charger_task_tx_run, (void *) NULL,
            (OS_STK *) &g_charger_tx_task_stack[CHARGER_TX_STK_SIZE - 1], CHARGER_TX_TASK_PRIO);
    OSTaskCreate(tiecheng_charger_task_rx_run, (void *) NULL,
            (OS_STK *) &g_charger_rx_task_stack[CHARGER_RX_STK_SIZE - 1], CHARGER_RX_TASK_PRIO);
}

void tiecheng_charger_task_tx_run(void* pdata)
{
    for (;;)
    {
        OSTimeDly(TIECHENG_CHARGER_SEND_FRAME_INTERVAL);
        
        /** fill the frame */
        tiecheng_charger_fill_control_frame(&g_chargerSendMessage);

        /*发送充电器控制报文*/
        J1939CanSendMessageWithFinish(g_chargerCanContextPtr, &g_chargerSendMessage, 50);
    }
}

void tiecheng_charger_task_rx_run(void* pdata)
{
    OS_CPU_SR cpu_sr = 0;
    J1939Result err = 0;

    for (;;)
    {
        err = J1939CanRecMessageWithWait(g_chargerCanContextPtr,&g_chargerRecMessage, 5000);
        if(err != J1939_OK)
        {
            OS_ENTER_CRITICAL();
            ChargerParaIndex.ChargerSts.OutputChgVolt = 0;
            ChargerParaIndex.ChargerSts.OutputChgCur = 0;
            ChargerParaIndex.ChargerSts.IsCommunication = 0;
            ChargerParaIndex.ChargerSts.IsCharging = FALSE;
            //charger_clear_all_except_status();
            OS_EXIT_CRITICAL();
            continue;
        }
        if(g_chargerRecMessage.id.value != TIECHENG_CHARGER_BMS_RECEIVE_ID)
        {
            continue;
        }
        ChargerParaIndex.ChargerSts.OutputChgVolt = ((INT16U)g_chargerRecMessage.data[0]<<8) + g_chargerRecMessage.data[1];
        ChargerParaIndex.ChargerSts.OutputChgCur = ((INT16U)g_chargerRecMessage.data[2]<<8) + g_chargerRecMessage.data[3];
        ChargerParaIndex.ChargerSts.IsCommunication = TRUE;
        if(ChargerParaIndex.ChargerSts.OutputChgCur > 0)
            ChargerParaIndex.ChargerSts.IsCharging = TRUE;
        else
            ChargerParaIndex.ChargerSts.IsCharging = FALSE;
        if (g_chargerRecMessage.data[4] & TIECHENG_CHARGER_FLAG_HARDWARE_EXCEPTION)
            charger_set_charge_except_status_with_num(CHARGER_HARDWARE_FAULT_NUM, 0x01);
        else
            charger_clear_charge_except_status_with_num(CHARGER_HARDWARE_FAULT_NUM);
        
        if (g_chargerRecMessage.data[4] & TIECHENG_CHARGER_FLAG_TEMPERATURE_EXCEPTION)
            charger_set_charge_except_status_with_num(CHARGER_OVER_TEMPERATURE_EXCEPT_NUM, 0x01);
        else
            charger_clear_charge_except_status_with_num(CHARGER_OVER_TEMPERATURE_EXCEPT_NUM);
        
        if (g_chargerRecMessage.data[4] & TIECHENG_CHARGER_FLAG_INPUT_VOLTAGE_EXCEPTION)
            charger_set_charge_except_status_with_num(CHARGER_INPUT_VOLT_EXCEPT_NUM, 0x01);
        else
            charger_clear_charge_except_status_with_num(CHARGER_INPUT_VOLT_EXCEPT_NUM);
        
        if ((g_chargerRecMessage.data[4] & TIECHENG_CHARGER_FLAG_BATTERY_VOLTAGE_CLOSE))
            charger_set_charge_except_status_with_num(CHARGER_RUN_STATUS_NUM, 0x01);
        else
            charger_clear_charge_except_status_with_num(CHARGER_RUN_STATUS_NUM);
        
        if ((g_chargerRecMessage.data[4] & TIECHENG_CHARGER_FLAG_COMM_TIMEOUT))
            charger_set_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_BMS_NUM, 0x01);
        else
            charger_clear_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_BMS_NUM);
    }
}

void tiecheng_charger_fill_control_frame(J1939SendMessageBuff* _PAGED message)
{
    INT8U index = 0;

	message->pgn = 0x06;
	message->id.value = 0x1806E5F4;
	WRITE_BT_INT16U(message->data, index, ChargerParaIndex.ChgrSetCfg.ChargeVolt);
	WRITE_BT_INT16U(message->data, index, ChargerParaIndex.ChgrSetCfg.ChargeCur);
	WRITE_BT_INT8U(message->data, index,  ChargerParaIndex.ChgrSetCfg.ChargeOnOff);
	WRITE_BT_INT16U(message->data, index, 0xFFFF);
	WRITE_BT_INT8U(message->data, index,  0xFF);	

	message->byte_cnt = index;
}

Result tiecheng_charger_on_config_changing(ConfigIndex index, INT16U new_value)
{
    switch(index)
    {
    case kChargerCurIndex:
        return charger_set_current(new_value);
        break;
    case kChargerVoltIndex:
        return charger_set_voltage(new_value);
        break;
    }
    return RES_OK;
}

void tiecheng_charger_init(void)
{
    ChargerFuction[LIGOO_PROTOCOL].ChargerInit = &tiecheng_charger_init;
    ChargerFuction[LIGOO_PROTOCOL].ChargerUninit = &tiecheng_charger_uninit;
    ChargerFuction[LIGOO_PROTOCOL].ChargeOn = &tiecheng_charger_on;
    ChargerFuction[LIGOO_PROTOCOL].ChargeOff = &tiecheng_charger_off;
    ChargerFuction[LIGOO_PROTOCOL].ChargeIsOn = &tiecheng_charger_is_charging;
    ChargerFuction[LIGOO_PROTOCOL].ChargerIsConnected = &tiecheng_charger_is_connected;
    ChargerFuction[LIGOO_PROTOCOL].GetChargerStatus = &tiecheng_charger_get_status;
    ChargerFuction[LIGOO_PROTOCOL].GetChargerType = &tiecheng_charger_get_device_info;
    ChargerFuction[LIGOO_PROTOCOL].SetChargeVolt = &tiecheng_charger_set_max_voltage;
    ChargerFuction[LIGOO_PROTOCOL].SetChargeCur = &tiecheng_charger_set_current;
    ChargerFuction[LIGOO_PROTOCOL].GetChargeVolt = &tiecheng_charger_get_max_voltage;
    ChargerFuction[LIGOO_PROTOCOL].GetChargeCur = &tiecheng_charger_get_current;
    ChargerFuction[LIGOO_PROTOCOL].ChargerGetOutputVolt = &tiecheng_charger_get_output_voltage;
    ChargerFuction[LIGOO_PROTOCOL].ChargerGetOutputCur = &tiecheng_charger_get_output_current;
    ChargerFuction[LIGOO_PROTOCOL].ChargerGetChargeReadyStatus = &tiecheng_charger_charge_is_ready_status;
    ChargerFuction[LIGOO_PROTOCOL].ChargerChargingIsEnable = &tiecheng_charger_charging_is_enable;
    ChargerFuction[LIGOO_PROTOCOL].ChargerIsCommunication = &tiecheng_charger_is_communication;
    
    g_j1939ChgCanInfo.addr = 0x50;
  	g_j1939ChgCanInfo.dev = (INT8U)config_get(kChargerCanChannelIndex);
  	if(can_channel_is_valid(g_j1939ChgCanInfo.dev) == FALSE)
	{
	    g_j1939ChgCanInfo.dev = BMS_CHARGER_CAN_DEV;
	    bcu_reset_all_can_channel();
	}
    if(g_j1939ChgCanInfo.dev == kCanDev3) g_j1939ChgCanInfo.bps = can_get_bps((INT8U)config_get(kCAN5BpsIndex));
    else g_j1939ChgCanInfo.bps = can_get_bps((INT8U)config_get(kCAN1BpsIndex + g_j1939ChgCanInfo.dev));
	
  	g_j1939ChgCanInfo.id_style = TIECHENG_CHARGER_CAN_MODE;
  	g_j1939ChgCanInfo.mask_id.value = 0;
  	g_j1939ChgCanInfo.receive_id.value = TIECHENG_CHARGER_BMS_RECEIVE_ID;
  	
  	j1939_comm_para_init();
  	
  	J1939InitWithBuff(g_chargerCanContextPtr,(J1939CanInfo* _PAGED)(&g_j1939ChgCanInfo));

    tiecheng_charger_set_max_voltage(config_get(kChargerVoltIndex));
    tiecheng_charger_set_current(config_get(kChargerCurIndex));
    BMS_CHARGER_ENABLE_DEFAULT ? tiecheng_charger_on() : tiecheng_charger_off();

    config_register_observer(kChargerCurIndex, kChargerVoltIndex,tiecheng_charger_on_config_changing);
    relay_control_set_enable_flag(kRelayTypeCharging, TRUE);
    tiecheng_charger_task_create(); 
}

#endif

