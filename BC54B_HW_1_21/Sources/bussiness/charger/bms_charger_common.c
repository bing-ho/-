/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_charger_common.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-6-12
*
*/
#include "bms_charger.h"
#include "bms_charger_common.h"
#include "includes.h"
#include "bms_charger_tiecheng.h"
#include "bms_charger_gb.h"
#include "bms_charger_gw.h"
#include "bms_charger_none.h"
#include "j1939_cfg.h"
#include "bms_stat_chg_time.h"
#include "ect_intermediate.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C4001 // Condition always FALSE


OS_STK g_charger_tx_task_stack[CHARGER_TX_STK_SIZE];
OS_STK g_charger_rx_task_stack[CHARGER_RX_STK_SIZE];


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER
CanMessage g_charger_can_buffers[CHARGER_CAN_BUFFER_COUNT];
INT8U g_chargerProtocolIndex;
CHARGER_PARA_INDEX ChargerParaIndex;

static INT32U g_charger_control_bits = 0;

CHARGER_FUCTION ChargerFuction[PROTOCOL_MAX];

J1939CanContext* _PAGED g_chargerCanContextPtr;

J1939SendMessageBuff g_chargerSendMessage;
unsigned char g_chargerSendMsgDataBuff[CHARGER_SENT_MESSAGE_LENGTH_MAX];
J1939RecMessage g_chargerRecMessage;
unsigned char g_chargerRecMsgDataBuff[CHARGER_REC_MESSAGE_LENGTH_MAX];

J1939CanFrame g_chargerCanTempFrame; //回调函数使用
J1939CanContext g_j1939ChgCanContext;
J1939Event g_j1939ChgSendEventSem;
J1939Event g_j1939ChgRecEventSem;
J1939CanInfo g_j1939ChgCanInfo;
J1939CanTranPro g_j1939ChgSendTpIndex;
J1939CanTPBuff g_j1939ChgRecTpBuffIndex;
J1939TPChain g_j1939ChgRecTpBuffs[CHARGER_TP_REC_MAX];
J1939CanFrameBuff g_j1939ChgSendFrameBuffIndex;
J1939CanFrame g_j1939ChgSendFrameBuffs[CHARGER_SENT_FRAME_BUFF];
J1939CanFrameBuff g_j1939ChgRecPduBuffIndex;
J1939CanFrame g_j1939ChgRecPduBuffs[CHARGER_REC_PDU_BUFF];
J1939CanFrameBuff g_j1939ChgRecFrameBuffIndex;
J1939CanFrame g_j1939ChgRecFrameBuffs[CHARGER_REC_FRAME_BUFF];
J1939SendMessageBuff g_j1939ChgSendMsgBuff;
unsigned char g_j1939ChgSendMsgDataBuff[CHARGER_SENT_MESSAGE_LENGTH_MAX];
J1939RecTempMessageBuff g_j1939ChgRecTempMsgBuffIndex;
J1939RecMessage g_j1939ChgRecTempMsgBuffs[CHARGER_TP_REC_MAX];
unsigned char g_j1939ChgRecTempMsgDataBuff[CHARGER_TP_REC_MAX][CHARGER_REC_MESSAGE_LENGTH_MAX];
J1939RecMessageBuff g_j1939ChgRecMsgDataBuffIndex;
J1939RecMessage g_j1939ChgRecMsgDataBuffs[CHARGER_REC_MESSAGE_BUFF_MAX];
unsigned char g_j1939ChgRecMsgDataBuff[CHARGER_REC_MESSAGE_BUFF_MAX][CHARGER_REC_MESSAGE_LENGTH_MAX];

#pragma DATA_SEG DEFAULT

void self_check_charger_init(void);
extern void adc_onchip_init(void);
void charger_init(void)
{
    if(config_get(kBCUCommModeIndex) == BMS_SELF_CHECK_MODE)
    {
        g_chargerProtocolIndex = NO_PROTOCOL; //自检模式为无协议模式
        self_check_charger_init();
    }
    else
    {
        g_chargerProtocolIndex = (INT8U)config_get(kChargerProtocolIndex);

        if(g_chargerProtocolIndex == LIGOO_PROTOCOL)
            ChargerFuction[g_chargerProtocolIndex].ChargerInit = &tiecheng_charger_init;
        else if(g_chargerProtocolIndex == GUO_BIAO_PROTOCOL)
            ChargerFuction[g_chargerProtocolIndex].ChargerInit = &guobiao_charger_init;
        else if(g_chargerProtocolIndex == GUO_WANG_PROTOCOL)
            ChargerFuction[g_chargerProtocolIndex].ChargerInit = &guowang_charger_init;
        else
            ChargerFuction[g_chargerProtocolIndex].ChargerInit = &none_charger_init;

        g_chargerSendMessage.byte_max = CHARGER_SENT_MESSAGE_LENGTH_MAX;
          g_chargerSendMessage.data = g_chargerSendMsgDataBuff;
          g_chargerRecMessage.byte_max = CHARGER_REC_MESSAGE_LENGTH_MAX;
          g_chargerRecMessage.data = g_chargerRecMsgDataBuff;

        charger_clear_all_except_status();
        ChargerFuction[g_chargerProtocolIndex].ChargerInit();
    }
    adc_onchip_init();
}

void charger_uninit(void)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerUninit != NULL)
        ChargerFuction[g_chargerProtocolIndex].ChargerUninit();
}

Result charger_on(void)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargeOn != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargeOn();
    else
        return RES_ERROR;
}

Result charger_off(void)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargeOff != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargeOff();
    else
        return RES_ERROR;
}

BOOLEAN charger_is_charging(void)
{
#if SLOW_CHARGER_WORK_WITH_GUOBIAO_CHARGER_EN
    if(bms_slow_charger_is_charging() == TRUE) return TRUE;
#endif    

    if(ChargerFuction[g_chargerProtocolIndex].ChargeIsOn != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargeIsOn();
    else
        return FALSE;
}

/** 充电机是否连接工作 */
Result charger_is_connected(void)
{
#if SLOW_CHARGER_WORK_WITH_GUOBIAO_CHARGER_EN
    if(bms_slow_charger_is_connected() == TRUE) return TRUE;
#endif
    
    if(ChargerFuction[g_chargerProtocolIndex].ChargerIsConnected != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerIsConnected();
    else
        return FALSE;
}

Result charger_is_communication(void)
{
#if SLOW_CHARGER_WORK_WITH_GUOBIAO_CHARGER_EN
    if(bms_slow_charger_is_communication()) return TRUE;
#endif

    if(ChargerFuction[g_chargerProtocolIndex].ChargerIsCommunication != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerIsCommunication();
    else
        return FALSE;
}

Result charger_get_status(INT8U* status)
{
    if(ChargerFuction[g_chargerProtocolIndex].GetChargerStatus != NULL)
        return ChargerFuction[g_chargerProtocolIndex].GetChargerStatus(status);
    else
        return RES_ERROR;
}

Result charger_get_device_info(ChargerDeviceInfo* info)
{
    if(ChargerFuction[g_chargerProtocolIndex].GetChargerType != NULL)
        return ChargerFuction[g_chargerProtocolIndex].GetChargerType(info);
    else
        return RES_ERROR;
}

Result charger_set_current(INT16U current)
{
    /*if (config_get(kChargerCurIndex) != 0 && current >= config_get(kChargerCurIndex))
    {
        current = config_get(kChargerCurIndex);
    }
    */
    if(ChargerFuction[g_chargerProtocolIndex].SetChargeCur != NULL)
        return ChargerFuction[g_chargerProtocolIndex].SetChargeCur(current);
    else
        return RES_ERROR;
}

Result charger_get_current(INT16U* current)
{
    if(ChargerFuction[g_chargerProtocolIndex].GetChargeCur != NULL)
        return ChargerFuction[g_chargerProtocolIndex].GetChargeCur(current);
    else
        return RES_ERROR;
}

Result charger_set_voltage(INT16U voltage)
{
    if(ChargerFuction[g_chargerProtocolIndex].SetChargeVolt != NULL)
        return ChargerFuction[g_chargerProtocolIndex].SetChargeVolt(voltage);
    else
        return RES_ERROR;
}

Result charger_get_voltage(INT16U* voltage)
{
    if(ChargerFuction[g_chargerProtocolIndex].GetChargeVolt != NULL)
        return ChargerFuction[g_chargerProtocolIndex].GetChargeVolt(voltage);
    else
        return RES_ERROR;
}

Result charger_get_output_voltage(INT16U* voltage)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetOutputVolt != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetOutputVolt(voltage);
    else
        return RES_ERROR;
}

Result charger_get_output_current(INT16U* current)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetOutputCur != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetOutputCur(current);
    else
        return RES_ERROR;
}

Result charger_set_pulse_current(INT16U current)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerSetPulseCur != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerSetPulseCur(current);
    else
        return RES_ERROR;
}

Result charger_get_pulse_current(INT16U* current)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetPulseCur != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetPulseCur(current);
    else
        return RES_ERROR;
}

Result charger_set_pulse_charge_time(INT16U time)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerSetPulseChgTime != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerSetPulseChgTime(time);
    else
        return RES_ERROR;
}

Result charger_get_pulse_charge_time(INT16U* time)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetPulseChgTime != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetPulseChgTime(time);
    else
        return RES_ERROR;
}

Result charger_constant_current_voltage_ctl(void)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerConstantVoltCurCtl != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerConstantVoltCurCtl();
    else
        return RES_ERROR;
}

Result charger_get_charge_voltage_max(INT16U* voltage)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeVoltMax != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeVoltMax(voltage);
    else
        return RES_ERROR;
}

Result charger_get_charge_voltage_min(INT16U* voltage)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeVoltMin != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeVoltMin(voltage);
    else
        return RES_ERROR;
}

Result charger_get_charge_current_max(INT16U* current)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeCurMax != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeCurMax(current);
    else
        return RES_ERROR;
}

Result charger_get_charge_current_min(INT16U* current)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeCurMin != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeCurMin(current);
    else
        return RES_ERROR;
}

Result charger_get_charge_pulse_current_max(INT16U* current)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetPulseCurMax != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetPulseCurMax(current);
    else
        return RES_ERROR;
}

Result charger_get_charged_energy(INT16U* energy)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetChargedEnergy != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetChargedEnergy(energy);
    else
        return RES_ERROR;
}

Result charger_get_charge_time_eclipse(INT16U* time)
{
    INT32U temp;
    
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetChgTimeEclipse != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetChgTimeEclipse(time);
    else
    {
        temp = (INT16U)(get_bms_charge_eclipse_time_s()/60);
        if(temp > 0xFFFF)*time = 0xFFFF;
        else *time = (INT16U)temp;
        return RES_OK;
    }
}

Result charger_get_charg_time_require(INT16U* time)
{
    INT32U temp;
    
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetChgTimeRequire != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetChgTimeRequire(time);
    else
    {
        temp = get_bms_charge_require_time_s()/60;
        if(temp > 0xFFFF)*time = 0xFFFF;
        else *time = (INT16U)temp;
        return RES_OK;
    }
}

Result charger_get_charge_cycle(INT16U* cycle)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeCycle != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeCycle(cycle);
    else
        return RES_ERROR;
}

Result charger_get_charge_ready_status(void)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeReadyStatus != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerGetChargeReadyStatus();
    else
        return RES_ERROR;
}

INT16U charger_callback(can_t context, CanMessage* msg, void* userdata)
{
    INT8U i;
    
    UNUSED(userdata);
    UNUSED(context);

    g_chargerCanTempFrame.id.value = can_id_to_extend_id(msg->id.value);
    g_chargerCanTempFrame.byte_cnt = msg->len;
    for(i=0; i<msg->len; i++)
        g_chargerCanTempFrame.data[i] = msg->data[i];
    J1939CanRecFrame(g_chargerCanContextPtr,&g_chargerCanTempFrame);
    return TRUE;
}


Result charger_control_enable_charger(INT32U flag)
{
    if (flag == 0) return charger_on();

    g_charger_control_bits &= (~flag);

    if (g_charger_control_bits == 0)
    {
        return charger_on();
    }

    return RES_OK;
}

Result charger_control_disable_charger(INT32U flag)
{
    if (flag == 0) return charger_off();

    g_charger_control_bits |= flag;

    if (g_charger_control_bits != 0)
    {
        return charger_off();
    }

    return RES_OK;
}

INT32U charger_get_control_bits(void)
{
    INT32U ctl_bits;
    OS_CPU_SR cpu_sr = 0;
    
    OS_ENTER_CRITICAL();
    ctl_bits = g_charger_control_bits;
    OS_EXIT_CRITICAL();
    
    return ctl_bits;
}

Result charger_set_charge_except_status_with_num(INT8U num, INT8U flag)
{
    INT8U index, shift_num;
    OS_INIT_CRITICAL();
    
    if(num >= CHARGER_EXCEPT_NUM_MAX)
        return RES_ERR;
    
    index = num / CHARGER_EXCEPT_NUM_IN_BYTE;
    
    shift_num = (num % CHARGER_EXCEPT_NUM_IN_BYTE) * CHARGER_EXCEPT_USE_BIT_NUM;
    
    OS_ENTER_CRITICAL();
    charger_clear_charge_except_status_with_num(num);
    ChargerParaIndex.ChargerSts.ChgrExceptStatus[index] |= (flag << shift_num);
    OS_EXIT_CRITICAL();
    
    return RES_OK;   
}

Result charger_clear_charge_except_status_with_num(INT8U num)
{
    INT8U index, shift_num;
    OS_INIT_CRITICAL();
    
    if(num >= CHARGER_EXCEPT_NUM_MAX)
        return RES_ERR;
    
    index = num / CHARGER_EXCEPT_NUM_IN_BYTE;
    
    shift_num = (num % CHARGER_EXCEPT_NUM_IN_BYTE) * CHARGER_EXCEPT_USE_BIT_NUM;
    
    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChargerSts.ChgrExceptStatus[index] &= ~(CHARGER_EXCEPT_MASK_NUM << shift_num);
    OS_EXIT_CRITICAL();
    
    return RES_OK;   
}

INT8U charger_get_charge_except_status_with_num(INT8U num)
{
    INT8U index, flag, shift_num;
    OS_INIT_CRITICAL();
    
    if(num >= CHARGER_EXCEPT_NUM_MAX)
        return RES_ERR;
    
    shift_num = (num % CHARGER_EXCEPT_NUM_IN_BYTE) * CHARGER_EXCEPT_USE_BIT_NUM;
    index = num / CHARGER_EXCEPT_NUM_IN_BYTE;
    
    OS_ENTER_CRITICAL();
    flag = (ChargerParaIndex.ChargerSts.ChgrExceptStatus[index] >> shift_num) & CHARGER_EXCEPT_MASK_NUM;
    if(flag == CHARGER_EXCEPT_MASK_NUM)flag = 0xFF;
    OS_EXIT_CRITICAL();
    
    return flag;
}

void charger_clear_all_except_status(void)
{
    INT8U i;
    
    for(i=0; i<CHARGER_EXCEPT_STATUS_BUFF_NUM; i++)
        ChargerParaIndex.ChargerSts.ChgrExceptStatus[i] = CHARGER_EXCEPT_STATUS_DEFAULT;
}

INT8U charger_get_except_status_buff_num(void)
{
    return CHARGER_EXCEPT_STATUS_BUFF_NUM;   
}

INT8U charger_get_except_status_with_buff_num(INT8U num)
{
    if(num >= CHARGER_EXCEPT_STATUS_BUFF_NUM)
        return 0;
    return ChargerParaIndex.ChargerSts.ChgrExceptStatus[num];   
}

INT8U charger_is_defective(void)
{
    if ((charger_get_charge_except_status_with_num(CHARGER_HARDWARE_FAULT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_OVER_TEMPERATURE_EXCEPT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_INPUT_VOLT_EXCEPT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_RUN_STATUS_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_BMS_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_OUTPUT_VOLT_EXCEPT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_BATTERY_REVERSE_CONNECT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_INNER_OVER_TEMPERATURE_EXCEPT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_CHAGER_CONNECTOR_EXCEPT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_ENERGY_TRANSIMIT_EXCEPT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_CURRENT_NOT_MATCH_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_PHASE_LACK_EXCEPT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_HIGH_VOLT_ALARM_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_HIGH_CURRENT_ALARM_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_OVER_PROTECT_VOLT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_OVER_PROTECT_CURR_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_OVER_NOMINAL_VOLT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_OVER_NOMINAL_CURR_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_FAULT_EXCEPT_NUM) == 1) ||
       (charger_get_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_CHARGE_SPOT_NUM) == 1))
        return TRUE;
    return FALSE;
}

INT8U charger_charging_is_enable(void)
{
    if(ChargerFuction[g_chargerProtocolIndex].ChargerChargingIsEnable != NULL)
        return ChargerFuction[g_chargerProtocolIndex].ChargerChargingIsEnable();
    else
        return FALSE;
}

void j1939_comm_para_init(void)
{
    INT8U i;

    //J1939 driver init
    g_chargerCanContextPtr = &g_j1939ChgCanContext;

    g_j1939ChgCanContext.send_event_sem = &g_j1939ChgSendEventSem;
    g_j1939ChgCanContext.send_event_sem->event = J1939CreateSem(1);

    g_j1939ChgCanContext.receive_event_sem = &g_j1939ChgRecEventSem;
    g_j1939ChgCanContext.receive_event_sem->event = J1939CreateSem(0);

    g_j1939ChgCanContext.can_info = &g_j1939ChgCanInfo;

    g_j1939ChgCanContext.send_tp_index = &g_j1939ChgSendTpIndex;

    g_j1939ChgCanContext.rec_tp_buff = &g_j1939ChgRecTpBuffIndex;
    g_j1939ChgCanContext.rec_tp_buff->tp_max = CHARGER_TP_REC_MAX;
    g_j1939ChgCanContext.rec_tp_buff->buffer = g_j1939ChgRecTpBuffs;

    g_j1939ChgCanContext.send_frame_buff = &g_j1939ChgSendFrameBuffIndex;
    g_j1939ChgCanContext.send_frame_buff->buffer_cnt = CHARGER_SENT_FRAME_BUFF;
    g_j1939ChgCanContext.send_frame_buff->buffer = g_j1939ChgSendFrameBuffs;

    g_j1939ChgCanContext.rec_pdu_buff = &g_j1939ChgRecPduBuffIndex;
    g_j1939ChgCanContext.rec_pdu_buff->buffer_cnt = CHARGER_REC_PDU_BUFF;
    g_j1939ChgCanContext.rec_pdu_buff->buffer = g_j1939ChgRecPduBuffs;

    g_j1939ChgCanContext.rec_frame_buff = &g_j1939ChgRecFrameBuffIndex;
    g_j1939ChgCanContext.rec_frame_buff->buffer_cnt = CHARGER_REC_FRAME_BUFF;
    g_j1939ChgCanContext.rec_frame_buff->buffer = g_j1939ChgRecFrameBuffs;

    g_j1939ChgCanContext.send_message_buff = &g_j1939ChgSendMsgBuff;
    g_j1939ChgCanContext.send_message_buff->byte_max = CHARGER_SENT_MESSAGE_LENGTH_MAX;
    g_j1939ChgCanContext.send_message_buff->data = g_j1939ChgSendMsgDataBuff;

    g_j1939ChgCanContext.rec_temp_mess_buff = &g_j1939ChgRecTempMsgBuffIndex;
    g_j1939ChgCanContext.rec_temp_mess_buff->buffer = g_j1939ChgRecTempMsgBuffs;
    for(i=0; i<CHARGER_TP_REC_MAX; i++)
    {
        g_j1939ChgCanContext.rec_temp_mess_buff->buffer[i].data = g_j1939ChgRecTempMsgDataBuff[i];
        g_j1939ChgCanContext.rec_temp_mess_buff->buffer[i].byte_max = CHARGER_REC_MESSAGE_LENGTH_MAX;
    }
    g_j1939ChgCanContext.rec_temp_mess_buff->buffer_cnt = CHARGER_TP_REC_MAX;

    g_j1939ChgCanContext.rec_message_buff = &g_j1939ChgRecMsgDataBuffIndex;
    g_j1939ChgCanContext.rec_message_buff->buffer = g_j1939ChgRecMsgDataBuffs;
    for(i=0; i<CHARGER_REC_MESSAGE_BUFF_MAX; i++)
    {
        g_j1939ChgCanContext.rec_message_buff->buffer[i].data = g_j1939ChgRecMsgDataBuff[i];
        g_j1939ChgCanContext.rec_message_buff->buffer[i].byte_max = CHARGER_REC_MESSAGE_LENGTH_MAX;
    }
    g_j1939ChgCanContext.rec_message_buff->buffer_cnt = CHARGER_TP_REC_MAX;
    g_j1939ChgCanInfo.receive_callback = charger_callback;
    //end of J1939 driver init
}

INT16U charger_get_send_message_timeout(J1939SendMessageBuff* _PAGED msg)
{
    INT16U byte_cnt;
    INT32U timeout;

    if(msg->byte_cnt > msg->byte_max)
        byte_cnt = msg->byte_max;
    else
        byte_cnt = msg->byte_cnt;

    if(byte_cnt <= 8)
        return CHARGER_FRAME_TIMEOUT_DEFINE;
    else
    {
        timeout = (INT32U)CHARGER_FRAME_TIMEOUT_DEFINE*((byte_cnt + 6) / 7);
        if(timeout > 0xFFFF)
            return 0xFFFF;
        else
            return (INT16U)timeout;
    }
}

// charger self check mode function
INT8U g_charger_self_check_comm_status = 0;

INT8U self_check_charger_get_communication_status(void)
{
    return g_charger_self_check_comm_status;
}

void self_check_charger_task_tx_run(void* pdata)
{
    OS_CPU_SR cpu_sr = 0;
    J1939Result err = 0;
    
    UNUSED(pdata);
    for (;;)
    {
        err = J1939CanRecMessageWithWait(g_chargerCanContextPtr,&g_chargerRecMessage, 500);
        if(err != J1939_OK)
        {
            OS_ENTER_CRITICAL();
            g_charger_self_check_comm_status = 0;
            OS_EXIT_CRITICAL();
            continue;
        }
        if(g_chargerRecMessage.id.value == SELF_CHECK_CHARGER_RECEIVE_ID)
        {
            if(g_chargerRecMessage.data[0] == 0) g_charger_self_check_comm_status |= 0x03;
            else if(g_chargerRecMessage.data[0] == 0x10) g_charger_self_check_comm_status |= 0x05;
        }
    }
}

void self_check_charger_task_create(void)
{
    OSTaskCreate(self_check_charger_task_tx_run, (void *) NULL,
            (OS_STK *) &g_charger_rx_task_stack[CHARGER_RX_STK_SIZE - 1], CHARGER_RX_TASK_PRIO);
}

void self_check_charger_init(void)
{
    g_j1939ChgCanInfo.addr = 0x50;
    g_j1939ChgCanInfo.dev = (INT8U)config_get(kChargerCanChannelIndex);
    if(g_j1939ChgCanInfo.dev >= kCanDev3) g_j1939ChgCanInfo.dev = kCanDev4;
      
    if(g_j1939ChgCanInfo.dev == kCanDev4) g_j1939ChgCanInfo.bps = can_get_bps((INT8U)config_get(kCAN5BpsIndex));
    else g_j1939ChgCanInfo.bps = can_get_bps((INT8U)config_get(kCAN1BpsIndex + g_j1939ChgCanInfo.dev));
    
    g_j1939ChgCanInfo.id_style = SELF_CHECK_CHARGER_CAN_MODE;
    g_j1939ChgCanInfo.mask_id.value = SELF_CHECK_CHARGER_RECEIVE_MASK_ID;
    g_j1939ChgCanInfo.receive_id.value = SELF_CHECK_CHARGER_RECEIVE_ID;
    g_chargerRecMessage.data = g_chargerRecMsgDataBuff;
    g_chargerRecMessage.byte_max = CHARGER_REC_MESSAGE_LENGTH_MAX;

    j1939_comm_para_init();
    J1939InitWithBuff(g_chargerCanContextPtr,(J1939CanInfo* _PAGED)(&g_j1939ChgCanInfo));
    
    guobiao_cc_detection_init();
    job_schedule(MAIN_JOB_GROUP, GUOBIAO_CC_DETECT_JOB_PERIODIC, guobiao_cc_cc2_detect, NULL);
    
    cp_pwm_start();
    job_schedule(MAIN_JOB_GROUP, GUOBIAO_PWM_DETECT_JOB_PERIODIC, guobiao_pwm_detect, NULL);
    self_check_charger_task_create(); 
}