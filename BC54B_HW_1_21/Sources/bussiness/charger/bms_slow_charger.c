/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_slow_charger.c
 * @brief
 * @note
 * @author
 * @date 2015-2-2
 *
 */
#ifndef BMS_SLOW_CHARGER_C
#define BMS_SLOW_CHARGER_C

#include "bms_slow_charger.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER

INT8U g_slow_charger_communication_flag = 0; //通信状态标志
INT8U g_slow_charger_charging_flag = 0;
INT32U g_slow_charger_send_tick = 0;
INT32U g_slow_charger_rec_tick = 0;

CanMessage g_slow_charger_send_message = {0};
CanMessage g_slow_charger_rec_message = {0};

#pragma DATA_SEG DEFAULT

Result bms_slow_charger_add_receive_id(J1939CanInfo* _PAGED can_info);


void bms_slow_charger_init(J1939CanInfo* _PAGED can_info)
{
    g_slow_charger_send_tick = get_tick_count();
    g_slow_charger_rec_tick = g_slow_charger_send_tick;
    bms_slow_charger_add_receive_id(can_info);
}

void bms_slow_charger_uninit(void)
{
    
}

Result bms_slow_charger_add_receive_id(J1939CanInfo* _PAGED can_info)
{
    if(can_info == NULL) return RES_ERR;
    
    return J1939AddReceiveID(can_info, BMS_SLOW_CHARGER_RECEIVE_ID);
}

void bms_slow_charger_communication_check(void)
{
    if(g_slow_charger_communication_flag == FALSE) return;
    
    if(get_interval_by_tick(g_slow_charger_rec_tick, get_tick_count()) >= 5000)
        bms_slow_charger_set_communication_flag(FALSE);
}

INT8U bms_slow_charger_is_communication(void)
{
    if(g_slow_charger_communication_flag == TRUE) return TRUE;
    
    return FALSE;
}

void bms_slow_charger_set_communication_flag(INT8U comm_flag)
{
    if(comm_flag) g_slow_charger_communication_flag = TRUE;
    else g_slow_charger_communication_flag = FALSE;
}

INT8U bms_slow_charger_is_connected(void)
{
    if(guobiao_charger_cc_is_connected()) return TRUE;
    if(guobiao_charger_pwm_is_connected()) return TRUE;

    return FALSE;
}

void bms_slow_charger_set_charging_flag(INT8U flag)
{
    if(flag) g_slow_charger_charging_flag = TRUE;
    else g_slow_charger_charging_flag = FALSE;
}

INT8U bms_slow_charger_is_charging(void)
{
    if(g_slow_charger_communication_flag == TRUE && g_slow_charger_charging_flag == TRUE) return TRUE;
    
    return FALSE;
}

void bms_slow_charger_send_message(CanDev dev)
{
    INT8U index = 0;
    INT16U current = 0, current_max = 0;
    CanMessage* _PAGED message;

    message = &g_slow_charger_send_message;
    // 充电继电器使能控制
    relay_control_set_enable_flag(kRelayTypeCharging, TRUE);
    // 通信周期
    if(get_interval_by_tick(g_slow_charger_send_tick, get_tick_count()) < 1000) return;
    // 通信超时计时
    g_slow_charger_send_tick = get_tick_count();
    
	message->id.value = can_id_from_extend_id(BMS_SLOW_CHARGER_SEND_ID);
	// 充电需求总压
	WRITE_BT_INT16U(message->data, index, ChargerParaIndex.ChgrSetCfg.ChargeVolt);
	// 充电需求电流
	if(bcu_get_soh_calib_cur_limit_flag())
	{
	    current = SOH_CALIB_LIMIT_CUR;
	}
	else
	{
	    current = ChargerParaIndex.ChgrSetCfg.ChargeCur;
    	current_max = guobiao_get_allow_charge_current_max();
        if(current > current_max) current = current_max;
	}
	WRITE_BT_INT16U(message->data, index, current);
	// 充电允许
	WRITE_BT_INT8U(message->data, index,  ChargerParaIndex.ChgrSetCfg.ChargeOnOff);
	// 保留
	WRITE_BT_INT16U(message->data, index, 0xFFFF);
	WRITE_BT_INT8U(message->data, index,  0xFF);	

	message->len = index;
	dev_can_send(dev, message);
}

INT8U bms_is_slow_charger_can_id(INT32U id)
{
    if(id == BMS_SLOW_CHARGER_RECEIVE_ID) return TRUE;
    
    return FALSE;
}

Result bms_slow_charger_receive(CanMessage* _PAGED rec_msg)
{
    if(rec_msg == NULL) return RES_ERR;
    
    g_slow_charger_rec_tick = get_tick_count();
    // 充电机输出电压
    ChargerParaIndex.ChargerSts.OutputChgVolt = ((INT16U)rec_msg->data[0]<<8) + rec_msg->data[1];
    // 充电机输出电流
    ChargerParaIndex.ChargerSts.OutputChgCur = ((INT16U)rec_msg->data[2]<<8) + rec_msg->data[3];
    // 充电机通信状态
    bms_slow_charger_set_communication_flag(TRUE);
    // 充电机充电状态
    if(ChargerParaIndex.ChargerSts.OutputChgCur > 0)
        bms_slow_charger_set_charging_flag(TRUE);
    else
        bms_slow_charger_set_charging_flag(FALSE);
    // 充电机硬件故障
    if (rec_msg->data[4] & 0x01)
        charger_set_charge_except_status_with_num(CHARGER_HARDWARE_FAULT_NUM, 0x01);
    else
        charger_clear_charge_except_status_with_num(CHARGER_HARDWARE_FAULT_NUM);
    // 充电机温度异常
    if (rec_msg->data[4] & 0x02)
        charger_set_charge_except_status_with_num(CHARGER_OVER_TEMPERATURE_EXCEPT_NUM, 0x01);
    else
        charger_clear_charge_except_status_with_num(CHARGER_OVER_TEMPERATURE_EXCEPT_NUM);
    // 充电机输入电压异常
    if (rec_msg->data[4] & 0x04)
        charger_set_charge_except_status_with_num(CHARGER_INPUT_VOLT_EXCEPT_NUM, 0x01);
    else
        charger_clear_charge_except_status_with_num(CHARGER_INPUT_VOLT_EXCEPT_NUM);
    // 充电机运行（启动）状态
    if ((rec_msg->data[4] & 0x08))
        charger_set_charge_except_status_with_num(CHARGER_RUN_STATUS_NUM, 0x01);
    else
        charger_clear_charge_except_status_with_num(CHARGER_RUN_STATUS_NUM);
    // BMS通信故障
    if ((rec_msg->data[4] & 0x10))
        charger_set_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_BMS_NUM, 0x01);
    else
        charger_clear_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_BMS_NUM);
    return RES_OK;
}

Result bms_slow_charger_receive_message(J1939CanContext* _PAGED context, J1939RecMessage* _PAGED rec_msg)
{
    if(context == NULL || rec_msg == NULL) return RES_ERR;
    
    J1939RecMessage2CanMessage(rec_msg, &g_slow_charger_rec_message);
    return bms_slow_charger_receive(&g_slow_charger_rec_message);
}

#endif