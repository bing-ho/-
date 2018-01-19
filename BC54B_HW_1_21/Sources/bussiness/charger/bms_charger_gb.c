/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_charger_gb.c
 * @brief
 * @note
 * @author
 * @date 2012-5-14
 *
 */

#ifndef BMS_CHARGER_GB_C
#define BMS_CHARGER_GB_C


#include "bms_charger_gb.h"
#include "bms_business_defs.h"
#include "bms_can.h"
#include "bms_charger_common.h"
#include "bms_charger_none.h"
#include "bms_relay_control.h"
#include "bms_buffer.h"
#include "bms_stat_chg_time.h"
#include "ads1013.h"
//#include "cp_pwm.h"
#include "adc_xgate.h"
#include "adc0_intermediate.h"

#include "ect_intermediate.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C4001 // Condition always FALSE
#pragma MESSAGE DISABLE C4301 // Inline expansion done for function call
#pragma MESSAGE DISABLE C5919 // Conversion of floating to unsigned integral
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler
#pragma MESSAGE DISABLE C2705 // Possible loss of data

#define SET_SEM(x) (XGSEM = 0x0101 << (x))
#define TST_SEM(x) (XGSEM & 0x0001 << (x))
#define REL_SEM(x) (XGSEM = 0x0100 << (x))     

#define GUOBIAO_PWM_DUTY_OFFSET     17

#define GUOBIAO_CC_VALUE_BUFF_MAX   3
#define GUOBIAO_CC_MAGNIFICATION    6
#define GUOBIAO_CC_FULL_VALUE       12000 //factor:0.001V
#define GUOBIAO_CC2_FULL_VALUE      12000 //factor:0.001V

#define GUOBIAO_CC_AD_PORT          6
#define GUOBIAO_CC2_AD_PORT_V114    5
#define GUOBIAO_CC2_AD_PORT_V116    13

#define GUOBIAO_PWM_BUFF_MAX        50 //必须大于1
#define GUOBIAO_GET_INTERVAL(LAST, NOW) (NOW >= LAST ? NOW-LAST : 0xFFFF-LAST+NOW+1)

#define GUOBIAO_CHARGER_S2_ON()     DDRP_DDRP5 = 1;PTP_PTP5 = 1
#define GUOBIAO_CHARGER_S2_OFF()    DDRP_DDRP5 = 1;PTP_PTP5 = 0
#define GUOBIAO_CHARGER_S2_IS_ON()  (PTP_PTP5 == 1)

//#define GUOBIAO_CP_DUTY_IS_VALID(pwm)   (!((pwm < 30) || (pwm > 900 && pwm <= 970) || (pwm > 970)))
#define GUOBIAO_CP_DUTY_IS_VALID(pwm)   (!((pwm < 10) ||(pwm > 999)))

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER
GuoBiaoCharger g_guobiao_charger = {NULL};

const ChargerDeviceInfo g_guobiao_charger_info = { 0, 0, "Charger", GUOBIAO_CHARGER_PROTOCOL_VERSION_DEFAULT, "Guobiao", "guobiao" };
GuoBiaoChargerCommStage g_gbChargerCommStage = kPhysicalConnectStage;
static INT8U g_guobiao_handshake_rec_state = 0; //0-等待接收到辨识00, 1-等待接收到辨识AA 2-接收辨识完成
static INT8U g_guobiao_charge_ready_flag = 0;
static INT8U g_guobiao_bms_charge_ready_send_flag = FALSE;
static GuoBiaoStopChgDev g_guobiao_stop_charge_dev = kGuoBiaoNoneStop;
static INT8U g_guobiao_bms_stop_reason=0;
static INT16U g_guobiao_bms_wrong_reason = 0;
static INT8U g_guobiao_bms_error_reason = 0;
static INT8U g_guobiao_charger_stop_reason=0;
static INT16U g_guobiao_charger_wrong_reason = 0;
static INT8U g_guobiao_charger_error_reason = 0;
static INT8U g_guobiao_large_current_flag = 0;
static INT8U g_guobiao_pgn_6400_send_flag = 0;
static INT32U g_guobiao_pgn_4096_send_flag = 0;
static INT8U g_guobiao_pgn_4608_rec_flag = 0;
static INT32U g_guobiao_pgn_4352_4864_send_tick = 0;
static INT8U g_guobiao_pgn_4352_4864_send_flag = 0;
static INT8U g_guobiao_pgn_4096_zero_current_send_flag = 0;
static INT32U g_guobiao_pgn_5376_5632_send_tick = 0;
static INT8U g_guobiao_pgn_5376_5632_send_flag = 0;
static INT32U g_guobiao_check_charge_stop_time = 0;
static INT8U g_guobiao_bms_comm_timeout_reason[GUOBIAO_BMS_TIMEOUT_BYTE_NUM];
static INT8U g_guobiao_charger_comm_timeout_reason[GUOBIAO_CHG_TIMEOUT_BYTE_NUM];

static ADS1013_HANDLE g_guoBiaoAds1013Handle = 0;
INT16U g_guoBiaoCCValue = 0;
INT16U g_guoBiaoCC2Value = 0;
static INT16U g_guoBiaoCCValueBuff[GUOBIAO_CC_VALUE_BUFF_MAX] = {0};

//static INT8U guobiao_pwm_buff_write_index = 0;
//static INT32U g_guobiao_pwm_tick = 0;
//static INT8U guobiao_pwm_edge_flag = 0;
//static INT16U guobiao_pwm_rising_buff[GUOBIAO_PWM_BUFF_MAX] = {0};
//static INT16U guobiao_pwm_falling_buff[GUOBIAO_PWM_BUFF_MAX] = {0};

INT16U g_guobiao_pwm_duty_cycle = 0; // 占空比, 单位千分之一
INT16U g_guobiao_pwm_frequence = 0;  // CP_PWM频率, 单位HZ
//static INT8U guobiao_ads1013_require_flag = 0;
GuoBiaoChgSelfCheckFault g_guobiao_self_check_fault_num = kGBChgFaultNone;
static GuoBiaoShakeHandStage g_guobiao_shake_hand_stage = kShakeHandStageNone;
static INT32U g_guobiao_charger_oc_last_tick = 0;
static INT16U g_guobiao_current_last = 0;
static INT32U g_guobiao_current_last_tick = 0;
static INT8U g_guobiao_comm_timeout_cnt = 0;
static INT8U g_guobiao_comm_timeout_cnt_switch = 0;
static INT32U g_relay_adhesion_check_flag = TRUE;
static INT32U g_relay_adhesion_check_tick = 0;
static INT32U g_bms_charge_ready_tick = 0;
static INT32U g_pwm_connect_last_tick = 0;
static INT32U g_guobiao_crm_00_receive_tick = 0;
static INT32U g_guobiao_crm_aa_receive_tick = 0;
static INT32U g_guobiao_cro_aa_receive_tick = 0;
static INT32U g_guobiao_csd_receive_tick = 0;


static const struct adc_onchip_channel *g_gb_charge_cc2_adc;

#pragma DATA_SEG DEFAULT

#define GUOBIAO_SET_SEND_CYCLE(CYCLE_PARA, CYCLE)   CYCLE_PARA = CYCLE
void guobiao_bhm_pgn_9984_send(void);
void guobiao_bhm_pgn_9984_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
void guobiao_brm_pgn_512_send(void);
void guobiao_brm_pgn_512_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
void guobiao_bcp_pgn_1536_send(void);
void guobiao_bcp_pgn_1536_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
void guobiao_bro_pgn_2304_send(void);
void guobiao_bro_pgn_2304_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
void guobiao_bcl_pgn_4096_send(void);
void guobiao_bcl_pgn_4096_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
void guobiao_pgn_4352_4864_send(void);
INT8U guobiao_bcs_pgn_4352_send(void);
void guobiao_bcs_pgn_4352_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
INT8U guobiao_bsm_pgn_4864_send(void);
void guobiao_bsm_pgn_4864_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
void guobiao_pgn_5376_5632_send(void);
INT8U guobiao_bmv_pgn_5376_send(void);
void guobiao_bmv_pgn_5376_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
INT8U guobiao_bmt_pgn_5632_send(void);
void guobiao_bmt_pgn_5632_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
INT8U guobiao_bsp_pgn_5888_send(void);
void guobiao_bsp_pgn_5888_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
void guobiao_bst_pgn_6400_send(void);
void guobiao_bst_pgn_6400_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
void guobiao_bsd_pgn_7168_send(void);
void guobiao_bsd_pgn_7168_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
void guobiao_csd_receive_timeout_check(void);
void guobiao_bem_pgn_7680_send(void);
void guobiao_bem_pgn_7680_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
void guobiao_chm_pgn_9728_receive(J1939RecMessage* _PAGED msg);
void guobiao_crm_00_receive_timeout_check(void);
void guobiao_crm_aa_receive_timeout_check(void);
void guobiao_crm_pgn_256_receive(J1939RecMessage* _PAGED msg);
void guobiao_cts_pgn_1792_receive(J1939RecMessage* _PAGED msg);
void guobiao_cml_pgn_2048_receive(J1939RecMessage* _PAGED msg);
void guobiao_cro_aa_receive_timeout_check(void);
void guobiao_cro_pgn_2560_receive(J1939RecMessage* _PAGED msg);
void guobiao_ccs_pgn_4608_receive(J1939RecMessage* _PAGED msg);
void guobiao_cst_pgn_6656_receive(J1939RecMessage* _PAGED msg);
void guobiao_csd_pgn_7424_receive(J1939RecMessage* _PAGED msg);
void guobiao_cem_pgn_7936_receive(J1939RecMessage* _PAGED msg);
INT8U guobiao_check_charge_req(INT16U delay);
INT8U guobiao_charge_is_stop(void);
INT16S guobiao_current_to_bms_current(INT16S cur);
INT16S guobiao_current_from_bms_current(INT16S cur);
INT8U guobiao_cc_16A_detect_is_enable(void);
INT8U guobiao_cc_32A_detect_is_enable(void);
INT8U guobiao_cc_detect_is_enable(void);
INT8U guobiao_cc2_detect_is_enable(void);
INT8U guobiao_pwm_detect_is_enable(void);
INT8U guobiao_charger_cc_is_connected(void);
INT8U guobiao_charger_cc2_is_connected(void);
INT8U guobiao_charger_pwm_is_connected(void);
Result guobiao_charger_is_communication(void);

void guobiao_charger_set_selfcheck_fault_num(GuoBiaoChgSelfCheckFault fault_num);
void guobiao_charger_update_selfcheck_fault_num(GuoBiaoChgSelfCheckFault fault_num);
void guobiao_charger_clear_selfcheck_fault_num(void);
void guobiao_charger_update_selfcheck_fault_num_with_control_bits(void);
INT8U guobiao_charger_is_charging_ready(void);
void guobiao_charger_charging_ready_check(void);
void guobiao_s2_update(void);
void guobiao_comm_timeout_cnt_update(void);
INT8U guobiao_charge_is_finish_stop(void); //国标充电结束为正常充电完成结束

/********************************************
 * Function Area
 ********************************************/

void guobiao_charger_uninit(void)
{
    can_uninit(g_guobiao_charger.can_context);
    g_guobiao_charger.can_context = NULL;
}

Result guobiao_charger_on(void)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeOnOff = GUOBIAO_CHARGER_ENABLE_CONTROL_BYTE;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guobiao_charger_off(void)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeOnOff = GUOBIAO_CHARGER_DISABLE_CONTROL_BYTE;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

INT8U guobiao_charger_is_connected(void)
{
    OS_CPU_SR cpu_sr = 0;
    INT16U cc_value, cc2_value;
    INT8U mode, res = FALSE;
    float pwm;

    mode = (INT8U)config_get(kGBChargerConnectModeIndex);
    OS_ENTER_CRITICAL();
    cc_value = g_guoBiaoCCValue;
    cc2_value = g_guoBiaoCC2Value;
    pwm = g_guobiao_pwm_duty_cycle;            
    OS_EXIT_CRITICAL();
    
    if(mode == kGBChargerConnectNone) 
    {
        if(guobiao_charger_is_communication()) res = TRUE;
    }
    else if(mode == kGBChargerConnectInputSignal)
    {
        res = input_signal_control_is_high(kInputTypeChargerConnect);
        if(res == 0xFF) res = TRUE;
    }
    else if(mode == kGBChargerConnectAuto)
    {
        OS_ENTER_CRITICAL();
        if(pwm >= 10)
        {
            if(guobiao_charger_pwm_is_connected() == FALSE) 
            {
                OS_EXIT_CRITICAL();
                return FALSE;
            }
        }
        OS_EXIT_CRITICAL();
        if(guobiao_charger_cc_is_connected()) res = TRUE;
        if(guobiao_charger_cc2_is_connected()) res = TRUE;
        if(guobiao_charger_pwm_is_connected()) res = TRUE;
    }
    else
    {
        if(guobiao_cc_detect_is_enable()) 
            res = guobiao_charger_cc_is_connected();
        else if(guobiao_cc2_detect_is_enable()) 
            res = guobiao_charger_cc2_is_connected();
        else
            res = TRUE;
        
        if(guobiao_pwm_detect_is_enable())
        {
            if(res == TRUE && guobiao_charger_pwm_is_connected())
            {
                res = TRUE;
            }
            else
            {
                res = FALSE;
            }
        }
    }
    return res;
}

Result guobiao_charger_is_communication(void)
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

BOOLEAN guobiao_charger_is_charging(void)
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

Result guobiao_charger_charge_is_ready_status()
{
    int cpu_sr = 0;
    BOOLEAN res;

    OS_ENTER_CRITICAL();
    if(g_guobiao_charge_ready_flag == BMS_CHARGE_READY && g_gbChargerCommStage >= kChargeReadyStage && g_gbChargerCommStage < kChargeEndStage)
        res = TRUE;
    else
        res = FALSE;
    OS_EXIT_CRITICAL();
    return res;
}

Result guobiao_charger_get_status(INT8U* status)
{
    int cpu_sr = 0;
    if (status == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *status = ChargerParaIndex.ChargerSts.ChgrExceptStatus[0];
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guobiao_charger_get_type(INT16U *status)
{
    if (status) *status = g_guobiao_charger_info.type;
    return RES_OK;
}

Result guobiao_charger_set_current(INT16U current)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeCur = current;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guobiao_charger_get_current(INT16U* current)
{
    int cpu_sr = 0;
    if (current == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *current = ChargerParaIndex.ChgrSetCfg.ChargeCur;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guobiao_charger_set_max_voltage(INT16U voltage)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeVolt = voltage;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guobiao_charger_get_max_voltage(INT16U* voltage)
{
    int cpu_sr = 0;
    if (voltage == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *voltage = ChargerParaIndex.ChgrSetCfg.ChargeVolt;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guobiao_charger_get_output_voltage(INT16U* voltage)
{
    int cpu_sr = 0;
    if (voltage == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *voltage = ChargerParaIndex.ChargerSts.OutputChgVolt;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guobiao_charger_get_output_current(INT16U* current)
{
    int cpu_sr = 0;
    if (current == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *current = ChargerParaIndex.ChargerSts.OutputChgCur;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

INT8U guobiao_charger_charging_is_enable(void)
{
    if(ChargerParaIndex.ChgrSetCfg.ChargeOnOff == GUOBIAO_CHARGER_ENABLE_CONTROL_BYTE) return TRUE;
    
    return FALSE;
}
//返回国标充通信状态
GuoBiaoChargerCommStage guobiao_charger_get_CommStage(void) 
{
  return  g_gbChargerCommStage;
}
void guobiao_charger_task_tx_run(void* pdata)
{
    INT8U send_cnt = 0, flag = 0;
    INT32U last_tick, diff;
    INT16U comm_cycle;
    void* p_arg=pdata; /*prevent 'pdata' complier has no use*/
    
    sleep(1000);
    for(;;)
    {
        guobiao_s2_update();
        
#if SLOW_CHARGER_WORK_WITH_GUOBIAO_CHARGER_EN
        if(bms_slow_charger_is_connected() && bms_slow_charger_is_communication()) 
        {
            bms_slow_charger_communication_check();
            bms_slow_charger_send_message(g_chargerCanContextPtr->can_info->dev);
            sleep(5);
            continue;
        }
#endif
        last_tick = get_tick_count();
        switch(g_gbChargerCommStage)
        {
            case kPhysicalConnectStage://等待硬件连接
                GUOBIAO_SET_SEND_CYCLE(comm_cycle, 50);//更新最大通信周期
                ChargerParaIndex.ChargerSts.IsCommunication  = 0;
                break;
            case kShakeHandsStage://握手阶段
                charger_clear_all_except_status();
                GUOBIAO_SET_SEND_CYCLE(comm_cycle, GUOBIAO_SHAKE_HAND_TIME);//更新最大通信周期
                if(g_guobiao_shake_hand_stage == kShakeHandStartStage)
                {
                    guobiao_bhm_pgn_9984_send();
                }
                else
                {
                    guobiao_brm_pgn_512_send();
                    guobiao_crm_aa_receive_timeout_check();
                }
                guobiao_crm_00_receive_timeout_check();
                break;
            case kParaConfigStage://充电配置阶段
                GUOBIAO_SET_SEND_CYCLE(comm_cycle, GUOBIAO_PARA_CONFIG_TIME);//更新最大通信周期
                guobiao_bcp_pgn_1536_send();
                break;
            case kChargeReadyStage://充电配置完成阶段
                GUOBIAO_SET_SEND_CYCLE(comm_cycle, GUOBIAO_CONFIG_READY_TIME);//更新最大通信周期
                guobiao_bro_pgn_2304_send();
                guobiao_cro_aa_receive_timeout_check();
                guobiao_charger_charging_ready_check();
                break;
            case kChargeStage://充电阶段
                GUOBIAO_SET_SEND_CYCLE(comm_cycle, GUOBIAO_CHARGING_TIME);//更新最大通信周期
                send_cnt = 0;
                if(guobiao_charge_is_stop())
                {
                    if(g_guobiao_stop_charge_dev == kGuoBiaoNoneStop)
                        g_guobiao_stop_charge_dev = kGuoBiaoBms;
                    g_guobiao_pgn_4352_4864_send_flag = 0;//保证PGN=4352结束前发送一次
                    if(g_guobiao_pgn_4096_zero_current_send_flag)
                        g_gbChargerCommStage = kChargeStopStage;   
                }
                guobiao_bcl_pgn_4096_send();
                guobiao_pgn_4352_4864_send();
                guobiao_pgn_5376_5632_send();//发送单体电压、温度
                break;
            case kChargeStopStage://中止充电阶段
                GUOBIAO_SET_SEND_CYCLE(comm_cycle, GUOBIAO_CHARGE_STOP_TIME);//更新最大通信周期
                guobiao_bst_pgn_6400_send();
                if(++send_cnt >= GUOBIAO_MESSAGE_RE_SEND_TIME)//多次发送，防止总线数据丢失
                {
                    send_cnt = 0;
                    //if(guobiao_charge_is_finish_stop() == 0) g_gbChargerCommStage = kChargeEndStage;
                    if(g_guobiao_stop_charge_dev == kGuoBiaoCharger)
                    {
                        g_guobiao_pgn_6400_send_flag = 1;
                    }
                }
                break;
            case kChargeStatisticsStage://充电统计阶段
                //GUOBIAO_CHARGER_S2_OFF();
                GUOBIAO_SET_SEND_CYCLE(comm_cycle, GUOBIAO_STATISTICS_TIME);//更新最大通信周期
                send_cnt = 0;
                guobiao_bsd_pgn_7168_send();
                guobiao_csd_receive_timeout_check();
                break;
            case kChargeErrorStage:
                GUOBIAO_SET_SEND_CYCLE(comm_cycle, GUOBIAO_STATISTICS_TIME);//更新最大通信周期
                guobiao_bem_pgn_7680_send();
                if(++send_cnt >= GUOBIAO_MESSAGE_RE_SEND_TIME)//多次发送，防止总线数据丢失
                {
                    send_cnt = 0;
                    g_gbChargerCommStage = kChargeEndStage;   
                }
                break;
            case kChargeEndStage://充电结束阶段
                GUOBIAO_SET_SEND_CYCLE(comm_cycle, 50);//更新最大通信周期
                //GUOBIAO_CHARGER_S2_OFF();
                send_cnt = 0;   //所有参数复位
                g_guobiao_charge_ready_flag = 0;
                g_guobiao_crm_00_receive_tick = 0;
                g_guobiao_crm_aa_receive_tick = 0;
                g_guobiao_cro_aa_receive_tick = 0;
                g_guobiao_csd_receive_tick = 0;
                g_guobiao_bms_charge_ready_send_flag = FALSE;
                g_guobiao_stop_charge_dev = kGuoBiaoNoneStop;
                g_guobiao_large_current_flag = 0;
                g_guobiao_pgn_6400_send_flag = 0;
                g_guobiao_pgn_4608_rec_flag = 0;
                g_guobiao_bms_stop_reason=0;
                g_guobiao_bms_wrong_reason = 0;
                g_guobiao_bms_error_reason = 0;
                g_guobiao_charger_stop_reason=0;
                g_guobiao_charger_wrong_reason = 0;
                g_guobiao_charger_error_reason = 0;
                g_guobiao_pgn_4096_zero_current_send_flag = 0;
                g_guobiao_pgn_4352_4864_send_tick = 0;
                g_guobiao_pgn_4352_4864_send_flag = 0;
                g_guobiao_pgn_5376_5632_send_tick = 0;
                g_guobiao_pgn_5376_5632_send_flag = 0;
                g_guobiao_check_charge_stop_time = 0;
                ChargerParaIndex.ChargerSts.IsCharging = FALSE;
                ChargerParaIndex.ChargerSts.OutputChgVolt = 0x00;
                ChargerParaIndex.ChargerSts.OutputChgCur = 0x00;
                ChargerParaIndex.ChargerSts.ChgrOutVoltMax = 0x00;
                ChargerParaIndex.ChargerSts.ChgrOutVoltMin = 0x00;
                ChargerParaIndex.ChargerSts.ChgrOutCurMin = 0x00;
                if(bcu_get_charging_current() <= config_get(kChgCutOffCurIndex))   //状态机复位
                {
                    g_guobiao_handshake_rec_state = 0;
                    g_relay_adhesion_check_flag = TRUE;
                    g_guobiao_comm_timeout_cnt_switch = FALSE;
                    relay_control_set_enable_flag(kRelayTypeCharging, FALSE);
                    g_guobiao_shake_hand_stage = kShakeHandStageNone;
                    //removed to keep last hand-shake version state
                    //guobiao_charger_protocol_version_set((PINT8U)GUOBIAO_CHARGER_PROTOCOL_VERSION_DEFAULT);
                    g_gbChargerCommStage = kPhysicalConnectStage;
                }
                break;
            default:
                g_gbChargerCommStage = kPhysicalConnectStage;//复位
                break;
        }
        diff = get_interval_by_tick(last_tick, get_tick_count());
        if(diff < comm_cycle)
            sleep(comm_cycle - (INT16U)diff);
        else
            sleep(5);
    }
}

void guobiao_charger_task_rx_run(void* pdata)
{
    INT16U rec_time=0;
    void* p_arg=pdata; /*prevent 'pdata' complier has no use*/

    for(;;)
    {
        switch(g_gbChargerCommStage)//更新接收超时时间
        {
            case kPhysicalConnectStage://等待硬件连接
                if(g_guobiao_comm_timeout_cnt == 0)
                    rec_time = GUOBIAO_SHAKE_HAND_TIMEOUT;//通信超时时间
                else
                    rec_time = GUOBIAO_SHAKE_HAND_CHS_TIMEOUT;
                break;
            case kShakeHandsStage://握手阶段
                if(guobiao_charger_protocol_version_is_v11())
                    rec_time = GUOBIAO_SHAKE_HAND_CHS_TIMEOUT;
                else
                    rec_time = GUOBIAO_SHAKE_HAND_TIMEOUT;
                break;
            case kParaConfigStage://充电配置阶段
                rec_time = GUOBIAO_PARA_CONFIG_TIMEOUT;
                break;
            case kChargeReadyStage://充电准备阶段
                rec_time = GUOBIAO_CONFIG_READY_TIMEOUT;
                break;
            case kChargeStage://充电阶段
                if(g_guobiao_pgn_4608_rec_flag == 1)
                    rec_time = GUOBIAO_CHARGING_TIMEOUT;
                else
                    rec_time = GUOBIAO_TIMEOUT_DEFAULT;
                break;
            case kChargeStopStage://中止充电阶段
                rec_time = GUOBIAO_CHARGE_STOP_TIMEOUT;    
                break;    
            case kChargeStatisticsStage://充电统计阶段
                rec_time = GUOBIAO_STATISTICS_TIMEOUT;
                break;
            case kChargeErrorStage://充电错误阶段
                rec_time = GUOBIAO_CHARGE_ERROR_TIMEOUT;
                break;
            case kChargeEndStage://充电结束阶段
                rec_time = GUOBIAO_SHAKE_HAND_TIMEOUT;
                break;
            default:
                rec_time = GUOBIAO_TIMEOUT_DEFAULT;
                break;
        }
        if (J1939_OK == J1939CanRecMessageWithWait(g_chargerCanContextPtr, &g_chargerRecMessage, rec_time))
        {
            ChargerParaIndex.ChargerSts.IsCommunication = TRUE;
#if SLOW_CHARGER_WORK_WITH_GUOBIAO_CHARGER_EN
            if(bms_slow_charger_is_connected())
            {
                if(bms_is_slow_charger_can_id(g_chargerRecMessage.id.value))
                {
                    bms_slow_charger_set_communication_flag(TRUE);
                    bms_slow_charger_receive_message(g_chargerCanContextPtr, &g_chargerRecMessage);
                    continue;
                }
            }
#endif
            switch(g_chargerRecMessage.pgn)
            {
                case GUOBIAO_MESSAGE_CHM_PGN_9728: //充电机握手启动
                    guobiao_chm_pgn_9728_receive(&g_chargerRecMessage);
                    guobiao_charger_protocol_version_set((PINT8U)GUOBIAO_CHARGER_PROTOCOL_VERSION_DEFAULT);
                    break;
                case GUOBIAO_MESSAGE_CRM_PGN_256://充电机握手
                    guobiao_crm_pgn_256_receive(&g_chargerRecMessage);
                    if(g_gbChargerCommStage == kChargeErrorStage)break;
                    g_guobiao_bms_comm_timeout_reason[0] &= ~GUOBIAO_REC_SPN2560_00_TIMEOUT_VALUE;
                    if(g_chargerRecMessage.data[0] == CHG_IDENTIFICATION_NUM)
                    {
                        g_guobiao_bms_comm_timeout_reason[0] &= ~GUOBIAO_REC_SPN2560_AA_TIMEOUT_VALUE;
                    }
                    break;
                case GUOBIAO_MESSAGE_CTS_PGN_1792://充电机时间同步
                    guobiao_cts_pgn_1792_receive(&g_chargerRecMessage);
                    break;
                case GUOBIAO_MESSAGE_CML_PGN_2048://充电机最大输出能力
                    guobiao_cml_pgn_2048_receive(&g_chargerRecMessage);
                    if(g_gbChargerCommStage == kChargeErrorStage)break;
                    g_guobiao_bms_comm_timeout_reason[1] &= ~GUOBIAO_REC_CAPABILITY_TIMEOUT_VALUE;
                    break;
                case GUOBIAO_MESSAGE_CRO_PGN_2560://充电机输出就绪
                    guobiao_cro_pgn_2560_receive(&g_chargerRecMessage);
                    if(g_gbChargerCommStage == kChargeErrorStage)break;
                    g_guobiao_bms_comm_timeout_reason[1] &= ~GUOBIAO_REC_CHARGER_READY_TIMEOUT_VALUE;
                    break;
                case GUOBIAO_MESSAGE_CCS_PGN_4608://充电机充电状态
                    guobiao_ccs_pgn_4608_receive(&g_chargerRecMessage);
                    if(g_gbChargerCommStage == kChargeErrorStage)break;
                    g_guobiao_bms_comm_timeout_reason[2] &= ~GUOBIAO_REC_CHARGER_STATUS_TIMEOUT_VALUE;
                    break;
                case GUOBIAO_MESSAGE_CST_PGN_6656://充电机中止充电
                    if(g_gbChargerCommStage >= kChargeStatisticsStage) break;
                    guobiao_cst_pgn_6656_receive(&g_chargerRecMessage);
                    if(g_guobiao_stop_charge_dev == kGuoBiaoBms)
                    {
                        /*if(guobiao_charge_is_finish_stop())*/ g_gbChargerCommStage = kChargeStatisticsStage;
                    }
                    else if(g_guobiao_stop_charge_dev == kGuoBiaoNoneStop)
                    {
                        /*if(guobiao_charge_is_finish_stop() == 0) g_gbChargerCommStage = kChargeEndStage; //故障停止，直接结束
                        else */g_guobiao_stop_charge_dev = kGuoBiaoCharger; //正常充电完成
                    }
                    else if(g_guobiao_stop_charge_dev == kGuoBiaoCharger)
                    {
                        if(g_guobiao_pgn_6400_send_flag == 1)
                        {
                            g_guobiao_pgn_6400_send_flag = 0;
                            g_gbChargerCommStage = kChargeStatisticsStage;
                        }
                    }
                    if(g_gbChargerCommStage == kChargeErrorStage)break;
                    g_guobiao_bms_comm_timeout_reason[2] &= ~GUOBIAO_REC_CHARGER_STOP_TIMEOUT_VALUE; 
                    break;
                case GUOBIAO_MESSAGE_CSD_PGN_7424://充电统计信息
                    guobiao_csd_pgn_7424_receive(&g_chargerRecMessage);
                    g_gbChargerCommStage = kChargeEndStage;
                    if(g_gbChargerCommStage == kChargeErrorStage)break;
                    g_guobiao_bms_comm_timeout_reason[3] &= ~GUOBIAO_REC_CHARGER_STATIS_TIMEOUT_VALUE;
                    break;
                case GUOBIAO_MESSAGE_CEM_PGN_7936://充电机错误信息
                    guobiao_cem_pgn_7936_receive(&g_chargerRecMessage);
                    g_gbChargerCommStage = kChargeEndStage;
                    break;
            }
        }
        else
        {
            switch(g_gbChargerCommStage)//更新错误原因
            {
                case kPhysicalConnectStage://等待硬件连接
                    g_guobiao_bms_comm_timeout_reason[0] |= GUOBIAO_REC_SPN2560_00_TIMEOUT_VALUE;
                    guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3901);
                    break;
                case kShakeHandsStage://握手阶段
                    if(g_guobiao_handshake_rec_state == 0) //未接收到辨识00报文
                    {
                        g_guobiao_bms_comm_timeout_reason[0] |= GUOBIAO_REC_SPN2560_00_TIMEOUT_VALUE;
                        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3901);
                    }
                    else if(g_guobiao_handshake_rec_state == 1) //未接收到辨识AA报文
                    {
                        g_guobiao_bms_comm_timeout_reason[0] |= GUOBIAO_REC_SPN2560_AA_TIMEOUT_VALUE;
                        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3902);
                    }
                    break;
                case kParaConfigStage://充电配置阶段
                    g_guobiao_bms_comm_timeout_reason[1] |= GUOBIAO_REC_CAPABILITY_TIMEOUT_VALUE;
                    guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3903);
                    break;
                case kChargeReadyStage://充电准备阶段
                    g_guobiao_bms_comm_timeout_reason[1] |= GUOBIAO_REC_CHARGER_READY_TIMEOUT_VALUE;
                    guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3904);
                    break;
                case kChargeStage://充电阶段
                    g_guobiao_bms_comm_timeout_reason[2] |= GUOBIAO_REC_CHARGER_STATUS_TIMEOUT_VALUE;   
                    guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3905);
                    break;
                case kChargeStopStage://中止充电阶段
                    g_guobiao_bms_comm_timeout_reason[2] |= GUOBIAO_REC_CHARGER_STOP_TIMEOUT_VALUE; 
                    guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3906);
                    break;    
                case kChargeStatisticsStage://充电统计阶段
                    g_guobiao_bms_comm_timeout_reason[3] |= GUOBIAO_REC_CHARGER_STATIS_TIMEOUT_VALUE;
                    guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3907);
                    break;
            }
            if(
#if SLOW_CHARGER_WORK_WITH_GUOBIAO_CHARGER_EN
                bms_slow_charger_is_connected() == 0 &&
#endif
                guobiao_charger_is_connected() &&
                kChargeEndStage != g_gbChargerCommStage && 
                kPhysicalConnectStage <= g_gbChargerCommStage)
                g_gbChargerCommStage = kChargeErrorStage;
            bms_slow_charger_set_communication_flag(FALSE);
            bms_slow_charger_set_charging_flag(FALSE);
            ChargerParaIndex.ChargerSts.IsCommunication = FALSE;
            //charger_clear_all_except_status();
        }
    }
}

void guobiao_charger_task_create(void)
{
    OSTaskCreate(guobiao_charger_task_tx_run, (void *) NULL,
            (OS_STK *) &g_charger_tx_task_stack[CHARGER_TX_STK_SIZE - 1], CHARGER_TX_TASK_PRIO);
    OSTaskCreate(guobiao_charger_task_rx_run, (void *) NULL,
            (OS_STK *) &g_charger_rx_task_stack[CHARGER_RX_STK_SIZE - 1], CHARGER_RX_TASK_PRIO);
}

Result guobiao_charger_on_config_changing(ConfigIndex index, INT16U new_value)
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

void guobiao_charger_init(void)
{
#if BMS_SUPPORT_BY5248D == 0      //一体机模式不支持114
#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    if(hardware_io_revision_get() >= HW_VER_116){//V1.16 & V1.20
        g_gb_charge_cc2_adc = &adc_onchip_cc2_adc_v116;
    }
    else{
        g_gb_charge_cc2_adc = &adc_onchip_cc2_adc_v114;
    }
#else
    g_gb_charge_cc2_adc = &adc_onchip_cc2_adc_v116;
#endif
#else
    g_gb_charge_cc2_adc = &adc_onchip_cc2_adc_v116;   
#endif

    ChargerFuction[GUO_BIAO_PROTOCOL].ChargerInit = &guobiao_charger_init;
    ChargerFuction[GUO_BIAO_PROTOCOL].ChargerUninit = &guobiao_charger_uninit;
    ChargerFuction[GUO_BIAO_PROTOCOL].ChargeOn = &guobiao_charger_on;
    ChargerFuction[GUO_BIAO_PROTOCOL].ChargeOff = &guobiao_charger_off;
    ChargerFuction[GUO_BIAO_PROTOCOL].ChargeIsOn = &guobiao_charger_is_charging;
    ChargerFuction[GUO_BIAO_PROTOCOL].ChargerIsConnected = &guobiao_charger_is_connected;
    ChargerFuction[GUO_BIAO_PROTOCOL].GetChargerStatus = &guobiao_charger_get_status;
    ChargerFuction[GUO_BIAO_PROTOCOL].GetChargerType = &guobiao_charger_get_type;
    ChargerFuction[GUO_BIAO_PROTOCOL].SetChargeVolt = &guobiao_charger_set_max_voltage;
    ChargerFuction[GUO_BIAO_PROTOCOL].SetChargeCur = &guobiao_charger_set_current;
    ChargerFuction[GUO_BIAO_PROTOCOL].GetChargeVolt = &guobiao_charger_get_max_voltage;
    ChargerFuction[GUO_BIAO_PROTOCOL].GetChargeCur = &guobiao_charger_get_current;
    ChargerFuction[GUO_BIAO_PROTOCOL].ChargerGetOutputVolt = &guobiao_charger_get_output_voltage;
    ChargerFuction[GUO_BIAO_PROTOCOL].ChargerGetOutputCur = &guobiao_charger_get_output_current;
    ChargerFuction[GUO_BIAO_PROTOCOL].ChargerGetChargeReadyStatus = &guobiao_charger_charge_is_ready_status;
    ChargerFuction[GUO_BIAO_PROTOCOL].ChargerChargingIsEnable = &guobiao_charger_charging_is_enable;
    ChargerFuction[GUO_BIAO_PROTOCOL].ChargerIsCommunication = &guobiao_charger_is_communication;
    
    g_j1939ChgCanInfo.addr = (INT8U)BCU_CAN_ADDR_DEF;//config_get(kBcuCanAddrIndex);
      g_j1939ChgCanInfo.dev = (INT8U)config_get(kChargerCanChannelIndex);
      if(can_channel_is_valid(g_j1939ChgCanInfo.dev) == FALSE)
    {
        g_j1939ChgCanInfo.dev = BMS_CHARGER_CAN_DEV;
        bcu_reset_all_can_channel();
    }
    if(g_j1939ChgCanInfo.dev == kCanDev3) g_j1939ChgCanInfo.bps = can_get_bps((INT8U)config_get(kCAN5BpsIndex));
    else g_j1939ChgCanInfo.bps = can_get_bps((INT8U)config_get(kCAN1BpsIndex + g_j1939ChgCanInfo.dev));
    
      g_j1939ChgCanInfo.id_style = GUOBIAO_CHARGER_CAN_MODE;
      g_j1939ChgCanInfo.mask_id.value = GUOBIAO_CHARGER_BMS_RECEIVE_MASK_ID;
      g_j1939ChgCanInfo.receive_id.value = GUOBIAO_CHARGER_BMS_RECEIVE_ID;
    
    j1939_comm_para_init();
#if SLOW_CHARGER_WORK_WITH_GUOBIAO_CHARGER_EN
    bms_slow_charger_init(&g_j1939ChgCanInfo);
#endif
    J1939InitWithBuff(g_chargerCanContextPtr,(J1939CanInfo* _PAGED)(&g_j1939ChgCanInfo));

    guobiao_charger_set_max_voltage(config_get(kChargerVoltIndex));
    guobiao_charger_set_current(config_get(kChargerCurIndex));

    BMS_CHARGER_ENABLE_DEFAULT ? guobiao_charger_on() : guobiao_charger_off();

    config_register_observer(kChargerCurIndex, kChargerVoltIndex,guobiao_charger_on_config_changing);
    if(guobiao_cc_detect_is_enable() || guobiao_cc2_detect_is_enable()) 
    {
        guobiao_cc_detection_init();
        job_schedule(MAIN_JOB_GROUP, GUOBIAO_CC_DETECT_JOB_PERIODIC, guobiao_cc_cc2_detect, NULL);
    }
    if(guobiao_pwm_detect_is_enable() == TRUE) 
    {
        //pwm_ect_init();
        cp_pwm_start();
        //g_guobiao_pwm_tick = get_tick_count();
        job_schedule(MAIN_JOB_GROUP, GUOBIAO_PWM_DETECT_JOB_PERIODIC, guobiao_pwm_detect, NULL);
    }
    g_relay_adhesion_check_tick = get_tick_count();
    GUOBIAO_CHARGER_S2_OFF();
    
    guobiao_charger_task_create();
}

void guobiao_bhm_pgn_9984_send(void)
{
    guobiao_bhm_pgn_9984_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    J1939CanSendMessageWithFinish(g_chargerCanContextPtr, &g_chargerSendMessage, charger_get_send_message_timeout(&g_chargerSendMessage));
}

void guobiao_bhm_pgn_9984_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0;
    INT16U macro_temp;

    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BHM_PGN_9984;
    pri = GUOBIAO_MESSAGE_PRI_DEFAULT;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChargerVoltIndex)+100, msg->byte_max);//最高允许充电电压 当前值+10V
    
    msg->byte_cnt = index;
}

void guobiao_brm_pgn_512_send(void)
{
    guobiao_brm_pgn_512_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    J1939CanSendMessageWithFinish(g_chargerCanContextPtr, &g_chargerSendMessage, charger_get_send_message_timeout(&g_chargerSendMessage));
}

void guobiao_brm_pgn_512_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0;
    INT16U macro_temp;
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BRM_PGN_512;
    if(guobiao_charger_protocol_version_is_v11())
        pri = GUOBIAO_2015_BRM_PGN_PRI;
    else
        pri = GUOBIAO_MESSAGE_PRI_DEFAULT;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;
    if(guobiao_charger_protocol_version_is_v11())
    {
        WRITE_LT_INT24U_WITH_BUFF_SIZE(msg->data, index, GUOBIAO_PROTOCOL_VERSION_2015, msg->byte_max);//协议版本号
    }
    else
    {
        WRITE_LT_INT24U_WITH_BUFF_SIZE(msg->data, index, GUOBIAO_PROTOCOL_VERSION_2011, msg->byte_max);//协议版本号
    }
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, GUOBIAO_BATTERY_TYPE, msg->byte_max);//电池类型
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kNominalCapIndex), msg->byte_max);//额定容量    config_get(kNominalCapIndex)*10
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kNominalTotalVolt), msg->byte_max);//额定总压
    WRITE_LT_INT32U_WITH_BUFF_SIZE(msg->data, index, GUOBIAO_LONG_VALUE_DEFAULT, msg->byte_max);//厂商名称
    WRITE_LT_INT32U_WITH_BUFF_SIZE(msg->data, index, GUOBIAO_LONG_VALUE_DEFAULT, msg->byte_max);//电池组序号
    WRITE_LT_INT24U_WITH_BUFF_SIZE(msg->data, index, GUOBIAO_3_BYTES_VALUE_DEFAULT, msg->byte_max);//电池组生产日期
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCycleCntIndex), msg->byte_max);//电池组充放电次数
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0x00, msg->byte_max);//电池组充放电次数
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, GUOBIAO_BYTE_VALUE_DEFAULT, msg->byte_max);//电池组产权标识
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, GUOBIAO_BYTE_VALUE_DEFAULT, msg->byte_max);//保留
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kVIN1Index), msg->byte_max);//车辆识别码VIN
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kVIN2Index), msg->byte_max);//车辆识别码VIN
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kVIN3Index), msg->byte_max);//车辆识别码VIN
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kVIN4Index), msg->byte_max);//车辆识别码VIN
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kVIN5Index), msg->byte_max);//车辆识别码VIN
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kVIN6Index), msg->byte_max);//车辆识别码VIN
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kVIN7Index), msg->byte_max);//车辆识别码VIN
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kVIN8Index), msg->byte_max);//车辆识别码VIN
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kVIN9Index), msg->byte_max);//车辆识别码VIN
    if(guobiao_charger_protocol_version_is_v11())
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DEVICE_SW_BUILD_VER, msg->byte_max); //软件版本流水号
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BMS_BUILD_DAY, msg->byte_max); //BMS软件编译时间
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BMS_BUILD_MONTH, msg->byte_max);
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, BMS_BUILD_YEAR, msg->byte_max);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DEVICE_SW_REV_VER, msg->byte_max); //BMS软件版本号
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DEVICE_SW_MINOR_VER, msg->byte_max);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DEVICE_SW_MAJOR_VER, msg->byte_max);
    }
    
    msg->byte_cnt = index;
}

void guobiao_bcp_pgn_1536_send(void)
{
    guobiao_bcp_pgn_1536_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    J1939CanSendMessageWithFinish(g_chargerCanContextPtr, &g_chargerSendMessage, charger_get_send_message_timeout(&g_chargerSendMessage));
}

void guobiao_bcp_pgn_1536_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0;
    INT16U macro_temp;
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BCP_PGN_1536;
    if(guobiao_charger_protocol_version_is_v11())
        pri = GUOBIAO_2015_BCP_PGN_PRI;
    else
        pri = GUOBIAO_MESSAGE_PRI_DEFAULT;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHVTrdAlarmIndex)/10+2, msg->byte_max);//最高允许充电单体电压
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, guobiao_current_from_bms_current(config_get(kChgOCTrdAlarmIndex)), msg->byte_max);//最高允许充电电流
    macro_temp = (INT16U)((INT32U)config_get(kNominalCapIndex)*config_get(kNominalTotalVolt)/1000);
    macro_temp = DIVISION(macro_temp,10);/////////////////////////////////////////////////////////////////
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, macro_temp, msg->byte_max);//额定能量
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChargerVoltIndex)+100, msg->byte_max);//最高允许充电电压  当前值+10V
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgHTTrdAlarmIndex), msg->byte_max);//最高允许充电温度
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_SOC()/10, msg->byte_max);//当前SOC
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_100MV(bcu_get_total_voltage()), msg->byte_max);//当前总压
        
    msg->byte_cnt = index;
}

void guobiao_cro_aa_receive_timeout_check(void)
{
    INT32U now_tick = get_tick_count();
    
    if(g_guobiao_bms_charge_ready_send_flag == FALSE)
    {
        return;
    }
    if(g_guobiao_cro_aa_receive_tick == 0) g_guobiao_cro_aa_receive_tick = now_tick;
    if(get_interval_by_tick(g_guobiao_cro_aa_receive_tick, now_tick) >= 60000)
    {
        g_gbChargerCommStage = kChargeErrorStage;
        g_guobiao_bms_comm_timeout_reason[1] |= GUOBIAO_REC_CHARGER_READY_TIMEOUT_VALUE;
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3904);
    }
}

void guobiao_bro_pgn_2304_send(void)
{
    guobiao_bro_pgn_2304_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    J1939CanSendMessageWithFinish(g_chargerCanContextPtr, &g_chargerSendMessage, charger_get_send_message_timeout(&g_chargerSendMessage));
}

void guobiao_bro_pgn_2304_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr, flag = 0;
    INT16U index = 0;
    
    flag = g_guobiao_charge_ready_flag;
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BRO_PGN_2304;
    pri = GUOBIAO_BRO_PGN_2304_PRI;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, flag, msg->byte_max);//充电准备就绪状态
    if(flag == BMS_CHARGE_READY) g_guobiao_bms_charge_ready_send_flag = TRUE;
            
    msg->byte_cnt = index;
}

void guobiao_bcl_pgn_4096_send(void)
{
    INT32U now_tick = get_tick_count();
    
    if(get_interval_by_tick(g_guobiao_pgn_4096_send_flag, now_tick) < 50) return;
    g_guobiao_pgn_4096_send_flag = now_tick;
    
    guobiao_bcl_pgn_4096_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    J1939CanSendMessageWithFinish(g_chargerCanContextPtr, &g_chargerSendMessage, charger_get_send_message_timeout(&g_chargerSendMessage));
}

void guobiao_get_decrease_current(INT16U new_cur, INT16U* _PAGED current)
{
    INT16U value;
    INT32U now_tick;
    
    if (current == NULL) return;
    
    value = *current;
    now_tick = get_tick_count();
    if (new_cur >= value) 
    {
        *current = new_cur;
    }
    else
    {
        if (get_interval_by_tick(g_guobiao_current_last_tick, now_tick) >= 1000)
        {
            INT16U diff = value - new_cur;
            g_guobiao_current_last_tick = now_tick;
            if(diff > GUOBIAO_CURRENT_DECREASE_MAX_PER_S) value -= GUOBIAO_CURRENT_DECREASE_MAX_PER_S;
            else value = new_cur;
            
            *current = value;
        }
    }
}

void guobiao_bcl_pgn_4096_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0;
    INT16U temp, current_max;
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BCL_PGN_4096;
    pri = GUOBIAO_MESSAGE_PRI_DEFAULT;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;

    temp = ChargerParaIndex.ChgrSetCfg.ChargeVolt;
    if(ChargerParaIndex.ChargerSts.ChgrOutVoltMax && temp>ChargerParaIndex.ChargerSts.ChgrOutVoltMax)
        temp = ChargerParaIndex.ChargerSts.ChgrOutVoltMax;
    else if(ChargerParaIndex.ChargerSts.ChgrOutVoltMin && temp<ChargerParaIndex.ChargerSts.ChgrOutVoltMin)
        temp = ChargerParaIndex.ChargerSts.ChgrOutVoltMin;
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);//充电电压需求
    
    if(!(g_guobiao_bms_stop_reason || g_guobiao_bms_wrong_reason || g_guobiao_bms_error_reason))
    {
        temp = ChargerParaIndex.ChgrSetCfg.ChargeCur;
        if(ChargerParaIndex.ChargerSts.ChgrOutCurMax && temp > ChargerParaIndex.ChargerSts.ChgrOutCurMax)
            temp = ChargerParaIndex.ChargerSts.ChgrOutCurMax;
        else if(ChargerParaIndex.ChargerSts.ChgrOutCurMin && temp < ChargerParaIndex.ChargerSts.ChgrOutCurMin)
            temp = ChargerParaIndex.ChargerSts.ChgrOutCurMin;
        current_max = guobiao_get_allow_charge_current_max();
        if(temp > current_max) 
        {
            temp = current_max;
            g_guobiao_current_last = temp;
        }
        else
        {
           if(guobiao_charger_protocol_version_is_v11())
              guobiao_get_decrease_current(temp, &g_guobiao_current_last);
           else
              g_guobiao_current_last = temp;
        }
    }
    else
    {
        g_guobiao_current_last = 0;
        g_guobiao_pgn_4096_zero_current_send_flag = 1;
    }
    temp = guobiao_current_from_bms_current(g_guobiao_current_last);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);//充电电流需求
    
    temp = MV_TO_100MV(bcu_get_total_voltage());
    if(temp > config_get(kChargerVoltIndex) || abs(temp - config_get(kChargerVoltIndex) < 20))
        temp = 0x01;
    else
        temp = 0x02;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (INT8U)temp, msg->byte_max);
            
    msg->byte_cnt = index;
}

void guobiao_pgn_4352_4864_send(void)
{
    INT32U now_tick = get_tick_count();
    
    if(g_guobiao_pgn_4352_4864_send_tick == 0) g_guobiao_pgn_4352_4864_send_tick = now_tick;
    if(g_guobiao_pgn_4352_4864_send_flag == 2)
    {
        if(get_interval_by_tick(g_guobiao_pgn_4352_4864_send_tick, now_tick) >= GUOBIAO_CHARGING_TIME_4352-2)
        {
            g_guobiao_pgn_4352_4864_send_tick = now_tick;
            g_guobiao_pgn_4352_4864_send_flag = 0;
        }
        else
        {
            return;
        }
    }
    if(g_guobiao_pgn_4352_4864_send_flag == 0)
    {
        if(guobiao_bcs_pgn_4352_send() == J1939_OK)
            g_guobiao_pgn_4352_4864_send_flag = 1;
    }
    else if(g_guobiao_pgn_4352_4864_send_flag == 1)
    {
        if(guobiao_bsm_pgn_4864_send() == J1939_OK)
            g_guobiao_pgn_4352_4864_send_flag = 2;   
    }
    else
        g_guobiao_pgn_4352_4864_send_flag = 0;
}

INT8U guobiao_bcs_pgn_4352_send(void)
{
    guobiao_bcs_pgn_4352_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    return J1939CanSendMessage(g_chargerCanContextPtr, &g_chargerSendMessage);
}

void guobiao_bcs_pgn_4352_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0;
    INT16U temp;
    INT32U data;
    INIT_WRITE_BUFFFER();
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BCS_PGN_4352;
    if(guobiao_charger_protocol_version_is_v11())
        pri = GUOBIAO_2015_BCS_PGN_PRI;
    else
        pri = GUOBIAO_MESSAGE_PRI_DEFAULT;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_100MV(bcu_get_total_voltage()), msg->byte_max);//检测总压
    temp = guobiao_current_from_bms_current((INT16S)bcu_get_current());
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);//检测电流
    temp = MV_TO_10MV(bcu_get_high_voltage())+((bcu_get_high_voltage_id()<<4)&0xf000);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);//最高单体电压及位置
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, SOC_TO_PERCENT(bcu_get_SOC()), msg->byte_max);//SOC
    data = get_bms_charge_require_time_s();
    if(data == 0xFFFFFFFF)
    {
        temp = 600;
    }
    else
    {
        data = data/60;
        if(data > 600)
            temp = 600;
        else
            temp = (INT16U)data;
    }
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);//剩余充电时间
                
    msg->byte_cnt = index;
}

INT8U guobiao_bsm_pgn_4864_send(void)
{
    guobiao_bsm_pgn_4864_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    return J1939CanSendMessage(g_chargerCanContextPtr, &g_chargerSendMessage);
}

void guobiao_bsm_pgn_4864_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0,bat_num=0;
    INT16U temp;
    //INIT_WRITE_BUFFFER();
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BSM_PGN_4864;
    pri = GUOBIAO_MESSAGE_PRI_DEFAULT;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;

    bat_num = (INT8U)bcu_get_bat_num_in_pack(bcu_get_high_voltage_id());
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bat_num, msg->byte_max);//最高单体电压编号
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bcu_get_high_temperature(), msg->byte_max);//最高温度
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (INT8U)bcu_get_temperature_num_in_pack(bcu_get_high_temperature_id()), msg->byte_max);//最高温度编号
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, TEMP_4_DISPLAY((INT8U)bcu_get_low_temperature()), msg->byte_max);//最低温度
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (INT8U)bcu_get_temperature_num_in_pack(bcu_get_low_temperature_id()), msg->byte_max);//最低温度编号
    temp = 0;
    if(bcu_get_charge_state() >= kAlarmThirdLevel) //过充
        temp |= 0x01;
    else if(bcu_get_discharge_state() >= kAlarmThirdLevel)
        temp |= 0x02;
    
    if(bcu_get_high_soc_state() >= kAlarmThirdLevel)
        temp |= 0x04;
    else if(bcu_get_low_soc_state() == kAlarmThirdLevel)
        temp |= 0x08;
    if(bcu_get_chg_oc_state() >= kAlarmThirdLevel)
        temp |= 0x10;
    if(bcu_get_chg_ht_state() >= kAlarmThirdLevel)
        temp |= 0x40;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);
    
    temp = 0;
    if(bcu_get_battery_insulation_state() >= kAlarmThirdLevel)  //漏电
        temp |= 0x01;
    if(guobiao_charge_is_stop() == FALSE)
        temp |= 0x10;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (temp|0xc0), msg->byte_max);
                
    msg->byte_cnt = index;
}

void guobiao_pgn_5376_5632_send(void)
{
    INT32U cycle = GUOBIAO_2015_CHARGING_TIME_5376;
    INT32U now_tick = get_tick_count();
    
    if(g_guobiao_pgn_5376_5632_send_tick == 0) g_guobiao_pgn_5376_5632_send_tick = now_tick;
    if(g_guobiao_pgn_5376_5632_send_flag == 2)//延时
    {
        if(guobiao_charger_protocol_version_is_v11() == 0) cycle = GUOBIAO_CHARGING_TIME_5376;
        if(get_interval_by_tick(g_guobiao_pgn_5376_5632_send_tick, now_tick) >= cycle)
        {
            g_guobiao_pgn_5376_5632_send_tick = now_tick;
            g_guobiao_pgn_5376_5632_send_flag = 0;
        }
        else
        {
            return;
        }
    }
    if(g_guobiao_pgn_5376_5632_send_flag == 0)
    {
        if(guobiao_bmv_pgn_5376_send() == 0)//发送单体电压成功
            g_guobiao_pgn_5376_5632_send_flag = 1; 
    }
    else if(g_guobiao_pgn_5376_5632_send_flag == 1)
    {
        if(guobiao_bmt_pgn_5632_send() == 0)//发送单体温度成功   
            g_guobiao_pgn_5376_5632_send_flag = 2;     
    }
    else
        g_guobiao_pgn_5376_5632_send_flag = 0;
}

INT8U guobiao_bmv_pgn_5376_send(void)
{
    guobiao_bmv_pgn_5376_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    return J1939CanSendMessage(g_chargerCanContextPtr, &g_chargerSendMessage);
}

void guobiao_bmv_pgn_5376_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0, i, j,slave_num, bat_num;
    INIT_WRITE_BUFFFER();
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BMV_PGN_5376;
    if(guobiao_charger_protocol_version_is_v11())
        pri = GUOBIAO_2015_BMV_PGN_PRI;
    else
        pri = GUOBIAO_MESSAGE_PRI_DEFAULT;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;

    slave_num = (INT8U)config_get(kSlaveNumIndex);
    
    for(i=0; i<slave_num; i++)
    {
        bat_num = bmu_get_voltage_num(i);
        for(j=0; j<bat_num; j++)
        {
            WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bmu_get_voltage_item(i, j)/10, msg->byte_max);//单体电压
        }
    }
    
    msg->byte_cnt = index;
}

INT8U guobiao_bmt_pgn_5632_send(void)
{
    guobiao_bmt_pgn_5632_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    return J1939CanSendMessage(g_chargerCanContextPtr, &g_chargerSendMessage);
}

void guobiao_bmt_pgn_5632_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT8U i, j,slave_num, temp_num;
    INT16U index = 0;
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BMT_PGN_5632;
    if(guobiao_charger_protocol_version_is_v11())
        pri = GUOBIAO_2015_BMT_PGN_PRI;
    else
        pri = GUOBIAO_MESSAGE_PRI_DEFAULT;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;

    slave_num = (INT8U)config_get(kSlaveNumIndex);
    
    for(i=0; i<slave_num; i++)
    {
        temp_num = bmu_get_temperature_num(i);
        for(j=0; j<temp_num; j++)
        {
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_temperature_item(i, j), msg->byte_max);//单体温度
        }
    }
    
    msg->byte_cnt = index;
}

INT8U guobiao_bsp_pgn_5888_send(void)
{
    guobiao_bsp_pgn_5888_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    return J1939CanSendMessage(g_chargerCanContextPtr, &g_chargerSendMessage);
}

void guobiao_bsp_pgn_5888_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0;
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BSP_PGN_5888;
    pri = GUOBIAO_MESSAGE_PRI_DEFAULT;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;

    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);//预留
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);//预留
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);//预留
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);//预留
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);//预留
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);//预留
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);//预留
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);//预留
    
    msg->byte_cnt = index;
}

void guobiao_csd_receive_timeout_check(void)
{
    INT32U now_tick = get_tick_count();
    
    if(g_guobiao_csd_receive_tick == 0) g_guobiao_csd_receive_tick = now_tick;
    if(get_interval_by_tick(g_guobiao_csd_receive_tick, now_tick) >= 10000)
    {
        g_gbChargerCommStage = kChargeErrorStage;
        g_guobiao_csd_receive_tick = now_tick;
        g_guobiao_bms_comm_timeout_reason[3] |= GUOBIAO_REC_CHARGER_STATIS_TIMEOUT_VALUE;
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3907);
    }
}

void guobiao_bst_pgn_6400_send(void)
{
    guobiao_bst_pgn_6400_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    J1939CanSendMessageWithFinish(g_chargerCanContextPtr, &g_chargerSendMessage, charger_get_send_message_timeout(&g_chargerSendMessage));
}

void guobiao_bst_pgn_6400_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0, temp = 0;
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BST_PGN_6400;
    pri = GUOBIAO_BST_PGN_6400_PRI;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;

    temp = g_guobiao_bms_stop_reason;
    if(guobiao_charger_protocol_version_is_v11() && g_guobiao_stop_charge_dev == kGuoBiaoCharger) temp |= 0x40;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);//BMS中止充电原因
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, g_guobiao_bms_wrong_reason, msg->byte_max);//BMS中止充电故障原因
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (g_guobiao_bms_error_reason|0xf0), msg->byte_max);//BMS中止充电错误原因
                
    msg->byte_cnt = index;
}

void guobiao_bsd_pgn_7168_send(void)
{
    guobiao_bsd_pgn_7168_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    J1939CanSendMessageWithFinish(g_chargerCanContextPtr, &g_chargerSendMessage, charger_get_send_message_timeout(&g_chargerSendMessage));
}

void guobiao_bsd_pgn_7168_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0;
    INIT_WRITE_BUFFFER();
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BSD_PGN_7168;
    pri = GUOBIAO_MESSAGE_PRI_DEFAULT;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, SOC_TO_PERCENT(bcu_get_SOC()), msg->byte_max);//中止SOC
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, VOLT_4_DISPLAY(bcu_get_low_voltage())/10, msg->byte_max);//最低电压
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_high_voltage()/10, msg->byte_max);//最高电压
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, TEMP_4_DISPLAY((INT8U)bcu_get_low_temperature()), msg->byte_max);//最低温度
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bcu_get_high_temperature(), msg->byte_max);//最高温度
                
    msg->byte_cnt = index;
}

void guobiao_bem_pgn_7680_send(void)
{
    guobiao_bem_pgn_7680_fill(g_chargerCanContextPtr, &g_chargerSendMessage);
    J1939CanSendMessageWithFinish(g_chargerCanContextPtr, &g_chargerSendMessage, charger_get_send_message_timeout(&g_chargerSendMessage));
}

void guobiao_bem_pgn_7680_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    INT8U pri, dest_addr;
    INT16U index = 0;
    
    dest_addr = GUOBIAO_CHARGER_ID;
    msg->pgn = GUOBIAO_MESSAGE_BEM_PGN_7680;
    pri = GUOBIAO_BEM_PGN_7680_PRI;
    msg->id.value = ((INT32U)pri << 26)+((INT32U)msg->pgn << 8)+((INT16U)dest_addr<<8)+context->can_info->addr;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (g_guobiao_bms_comm_timeout_reason[0]|0xf0), msg->byte_max);//错误原因1
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (g_guobiao_bms_comm_timeout_reason[1]|0xf0), msg->byte_max);//错误原因2
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (g_guobiao_bms_comm_timeout_reason[2]|0xf0), msg->byte_max);//错误原因3
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (g_guobiao_bms_comm_timeout_reason[3]|0xfc), msg->byte_max);//错误原因4
                
    msg->byte_cnt = index;
}

void guobiao_chm_pgn_9728_receive(J1939RecMessage* _PAGED msg)
{
    INT8U index = 0;
    INT32U version = 0;
    
    if(msg == NULL)
        return;
    if(msg->byte_cnt <= 0)
        return;

    //if(g_guobiao_comm_timeout_cnt >= GUOBIAO_COMM_ABORT_TIMEOUT_TIMES) return; //三次超时后不再进行充电
    if(!guobiao_charger_is_connected())
    {
        guobiao_charger_set_selfcheck_fault_num(kGBChgFaultLineOff);
        return;
    }

    if(g_gbChargerCommStage >= kParaConfigStage) return;
    if(g_gbChargerCommStage == kPhysicalConnectStage) guobiao_charger_clear_selfcheck_fault_num();
    if(index+2 < msg->byte_cnt)
    {
        version = READ_LT_INT24U(msg->data, index);
    }
    if(version == GUOBIAO_PROTOCOL_VERSION_2015)
    {
        INT8U i=0;
        for(i=0; i<GUOBIAO_BMS_TIMEOUT_BYTE_NUM; i++)
        g_guobiao_bms_comm_timeout_reason[i] = 0;
    
        if(g_guobiao_crm_00_receive_tick == 0) g_guobiao_crm_00_receive_tick = get_tick_count();
        g_guobiao_shake_hand_stage = kShakeHandStartStage;
        g_gbChargerCommStage = kShakeHandsStage;
    }
}

void guobiao_crm_00_receive_timeout_check(void)
{
    INT32U now_tick = get_tick_count();
    
    if(g_guobiao_crm_00_receive_tick == 0) g_guobiao_crm_00_receive_tick = now_tick;
    if(get_interval_by_tick(g_guobiao_crm_00_receive_tick, now_tick) >= 30000)
    {
        g_gbChargerCommStage = kChargeErrorStage;
        g_guobiao_crm_00_receive_tick = now_tick;
        g_guobiao_bms_comm_timeout_reason[0] |= GUOBIAO_REC_SPN2560_00_TIMEOUT_VALUE;
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3901);
    }
}

void guobiao_crm_aa_receive_timeout_check(void)
{
    INT32U now_tick = get_tick_count();
    
    if(g_guobiao_crm_aa_receive_tick == 0) g_guobiao_crm_aa_receive_tick = now_tick;
    if(get_interval_by_tick(g_guobiao_crm_aa_receive_tick, now_tick) >= 5000)
    {
        g_gbChargerCommStage = kChargeErrorStage;
        g_guobiao_bms_comm_timeout_reason[0] |= GUOBIAO_REC_SPN2560_AA_TIMEOUT_VALUE;
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3902);
    }
}

void guobiao_crm_pgn_256_receive(J1939RecMessage* _PAGED msg)
{
    INT8U i = 0;
    
    if(msg == NULL)
        return;
    if(msg->byte_cnt <= 0)
        return;
    //if(g_guobiao_comm_timeout_cnt >= GUOBIAO_COMM_ABORT_TIMEOUT_TIMES) return; //三次超时后不再进行充电
    if(!guobiao_charger_is_connected())
    {
        guobiao_charger_set_selfcheck_fault_num(kGBChgFaultLineOff);
        return;
    }
    if(g_gbChargerCommStage >= kParaConfigStage) return;
    if(g_gbChargerCommStage == kPhysicalConnectStage) guobiao_charger_clear_selfcheck_fault_num();
    if(g_guobiao_shake_hand_stage == kShakeHandStageNone && g_guobiao_comm_timeout_cnt == 0) //通信超时重试时不进行握手启动
    {
        guobiao_charger_protocol_version_set((PINT8U)GUOBIAO_CHARGER_PROTOCOL_VERSION_V10);
    }
    g_guobiao_shake_hand_stage = kShakeHandIdentifyStage;
    
    for(i=0; i<GUOBIAO_BMS_TIMEOUT_BYTE_NUM; i++)
        g_guobiao_bms_comm_timeout_reason[i] = 0;
    
    if(msg->data[0] == CHG_UN_IDENTIFICATION_NUM)
    {
        g_guobiao_handshake_rec_state = 1;
        g_gbChargerCommStage = kShakeHandsStage;
    }
    else if(msg->data[0] == CHG_IDENTIFICATION_NUM)
    {
        g_guobiao_handshake_rec_state = 2;
        g_bms_charge_ready_tick = get_tick_count();
        g_guobiao_crm_aa_receive_tick = g_bms_charge_ready_tick;
        g_gbChargerCommStage = kParaConfigStage;
    }
}

void guobiao_cts_pgn_1792_receive(J1939RecMessage* _PAGED msg)
{
    if(msg == NULL)
        return;
    if(msg->byte_cnt <= 0)
        return;
}

void guobiao_cml_pgn_2048_receive(J1939RecMessage* _PAGED msg)
{
    INT16U index = 0;
    INT16U max_value,min_value;
    
    if(msg == NULL)
        return;
    if(msg->byte_cnt <= 0)
        return;
    
    if(g_gbChargerCommStage != kParaConfigStage && g_gbChargerCommStage != kChargeReadyStage) return;
    
    if(index+1 < msg->byte_cnt) max_value = READ_LT_INT16U(msg->data, index);//ChargerParaIndex.ChargerSts.ChgrOutVoltMax = READ_LT_INT16U(msg->data, index);
    if(index+1 < msg->byte_cnt) min_value = READ_LT_INT16U(msg->data, index);//ChargerParaIndex.ChargerSts.ChgrOutVoltMin = READ_LT_INT16U(msg->data, index);
    if(min_value >= max_value)  min_value = 0;
    ChargerParaIndex.ChargerSts.ChgrOutVoltMax = max_value;
    ChargerParaIndex.ChargerSts.ChgrOutVoltMin = min_value;
    
    if(index+1 < msg->byte_cnt) max_value = guobiao_current_to_bms_current(READ_LT_INT16U(msg->data, index));//ChargerParaIndex.ChargerSts.ChgrOutCurMax = guobiao_current_to_bms_current(READ_LT_INT16U(msg->data, index));
    if(guobiao_charger_protocol_version_is_v11())
    {
        if(index+1 < msg->byte_cnt) min_value = guobiao_current_to_bms_current(READ_LT_INT16U(msg->data, index));//ChargerParaIndex.ChargerSts.ChgrOutCurMin = guobiao_current_to_bms_current(READ_LT_INT16U(msg->data, index));
        if(min_value >= max_value)  min_value = 0;
        ChargerParaIndex.ChargerSts.ChgrOutCurMin = min_value;
    } 
    ChargerParaIndex.ChargerSts.ChgrOutCurMax = max_value;
   
    g_gbChargerCommStage = kChargeReadyStage;
}

void guobiao_cro_pgn_2560_receive(J1939RecMessage* _PAGED msg)
{
    INT16U index = 0;
    
    if(msg == NULL)
        return;
    if(msg->byte_cnt <= 0)
        return;
    
    if((msg->data[0] == BMS_CHARGE_READY)&&
        (g_gbChargerCommStage == kChargeReadyStage)&&
        (g_guobiao_bms_charge_ready_send_flag == TRUE))
    {
        g_gbChargerCommStage = kChargeStage;
    }
}

void guobiao_ccs_pgn_4608_receive(J1939RecMessage* _PAGED msg)
{
    INT8U charge_state = 0;
    INT16U index = 0, current = 0;
    
    if(msg == NULL)
        return;
    if(msg->byte_cnt < 6)
        return;
    
    if(index+1 < msg->byte_cnt) ChargerParaIndex.ChargerSts.OutputChgVolt = READ_LT_INT16U(msg->data, index);
    if(index+1 < msg->byte_cnt) ChargerParaIndex.ChargerSts.OutputChgCur = guobiao_current_to_bms_current(READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt) ChargerParaIndex.ChargerSts.ChgTimeEclip = READ_LT_INT16U(msg->data, index);
    if(index < msg->byte_cnt) charge_state = (READ_LT_INT8U(msg->data, index)) & 0x03;
    if(ChargerParaIndex.ChargerSts.OutputChgVolt && 
        ChargerParaIndex.ChargerSts.OutputChgCur && 
        (guobiao_charger_protocol_version_is_v11()==0 || charge_state)) //0:暂停 1:允许
    {
        ChargerParaIndex.ChargerSts.IsCharging = TRUE;
    }
    else
    {
        ChargerParaIndex.ChargerSts.IsCharging = FALSE;
    }
    
    g_guobiao_pgn_4608_rec_flag = 1;
}

void guobiao_cst_pgn_6656_receive(J1939RecMessage* _PAGED msg)
{
    INT8U flag;
    INT16U index = 0;
    
    if(msg == NULL)
        return;
    if(msg->byte_cnt < 4)
        return;
    
    if(index < msg->byte_cnt) g_guobiao_charger_stop_reason = READ_LT_INT8U(msg->data, index);
    if(index+1 < msg->byte_cnt) g_guobiao_charger_wrong_reason = READ_LT_INT16U(msg->data, index);
    if(index < msg->byte_cnt) g_guobiao_charger_error_reason = READ_LT_INT8U(msg->data, index);

    flag = g_guobiao_charger_stop_reason & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_STOP_CHG_WITH_FINISH, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_STOP_CHG_WITH_FINISH);
    
    flag = (g_guobiao_charger_stop_reason >> 2) & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_MANUAL_STOP_NUM, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_MANUAL_STOP_NUM);
    
    flag = (g_guobiao_charger_stop_reason >> 4) & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_FAULT_EXCEPT_NUM, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_FAULT_EXCEPT_NUM);
    
    flag = (g_guobiao_charger_stop_reason >> 6) & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_BMS_STOP_CHARGE_EXCEPT_NUM, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_BMS_STOP_CHARGE_EXCEPT_NUM);

    flag = g_guobiao_charger_wrong_reason & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_OVER_TEMPERATURE_EXCEPT_NUM, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_OVER_TEMPERATURE_EXCEPT_NUM);
    
    flag = (g_guobiao_charger_wrong_reason >> 2) & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_CHAGER_CONNECTOR_EXCEPT_NUM, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_CHAGER_CONNECTOR_EXCEPT_NUM);
    
    flag = (g_guobiao_charger_wrong_reason >> 4) & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_INNER_OVER_TEMPERATURE_EXCEPT_NUM, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_INNER_OVER_TEMPERATURE_EXCEPT_NUM);
    
    flag = (g_guobiao_charger_wrong_reason >> 6) & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_ENERGY_TRANSIMIT_EXCEPT_NUM, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_ENERGY_TRANSIMIT_EXCEPT_NUM);
    
    flag = (g_guobiao_charger_wrong_reason >> 8) & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_EMERGENCY_STOP_NUM, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_EMERGENCY_STOP_NUM);
    
    flag = (g_guobiao_charger_wrong_reason >> 10) & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_FAULT_EXCEPT_NUM, flag);
    
    flag = g_guobiao_charger_error_reason & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_CURRENT_NOT_MATCH_NUM, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_CURRENT_NOT_MATCH_NUM);
    
    flag = (g_guobiao_charger_error_reason >> 2) & 0x03;
    if(flag == 1)
        charger_set_charge_except_status_with_num(CHARGER_OUTPUT_VOLT_EXCEPT_NUM, flag);
    else if(flag == 0)
        charger_clear_charge_except_status_with_num(CHARGER_OUTPUT_VOLT_EXCEPT_NUM);
    
    g_gbChargerCommStage = kChargeStopStage;
}

void guobiao_cem_pgn_7936_receive(J1939RecMessage* _PAGED msg)
{
    INT16U index = 0;
    
    if(msg == NULL)
        return;
    if(msg->byte_cnt < 4)
        return;
    
    if(index < msg->byte_cnt) g_guobiao_charger_comm_timeout_reason[0] = READ_LT_INT8U(msg->data, index);
    if(index < msg->byte_cnt) g_guobiao_charger_comm_timeout_reason[1] = READ_LT_INT8U(msg->data, index);
    if(index < msg->byte_cnt) g_guobiao_charger_comm_timeout_reason[2] = READ_LT_INT8U(msg->data, index);
    if(index < msg->byte_cnt) g_guobiao_charger_comm_timeout_reason[3] = READ_LT_INT8U(msg->data, index);
    
    if(0x01 == (g_guobiao_charger_comm_timeout_reason[0] & 3)) 
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3921);
    if(0x01 == (g_guobiao_charger_comm_timeout_reason[1] & 3)) 
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3922);
    if(0x01 == ((g_guobiao_charger_comm_timeout_reason[1] >> 2) & 3)) 
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3923);
    if(0x01 == ((g_guobiao_charger_comm_timeout_reason[2]) & 3)) 
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3924);
    if(0x01 == ((g_guobiao_charger_comm_timeout_reason[2] >> 2) & 3)) 
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3925);
    if(0x01 == ((g_guobiao_charger_comm_timeout_reason[2] >> 4) & 3)) 
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3926);
    if(0x01 == ((g_guobiao_charger_comm_timeout_reason[3]) & 3)) 
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultSPN3927);
    
    g_gbChargerCommStage = kChargeStopStage;
}

void guobiao_csd_pgn_7424_receive(J1939RecMessage* _PAGED msg)
{
    INT16U index = 0;
    
    if(msg == NULL)
        return;
    if(msg->byte_cnt < 4)
        return;
    
    g_guobiao_csd_receive_tick = get_tick_count();
    if(index < msg->byte_cnt) ChargerParaIndex.ChargerSts.ChgTimeEclip = READ_LT_INT8U(msg->data, index);//总充电时间
    if(index+1 < msg->byte_cnt) ChargerParaIndex.ChargerSts.ChgEnergyEclip = READ_LT_INT16U(msg->data, index);//总充电能量
}


INT8U guobiao_check_charge_req(INT16U delay)
{
    INT16U reason = 0;
    INT16U data = 0;
    INT32U chg_ctl_bits = charger_get_control_bits();
    
    //充电停止原因
    //if(SOC_TO_PERCENT(bcu_get_SOC()) >= 100)//SOC过大
    //    temp |= 0x01;
    
    if(chg_ctl_bits & SELFCHECK_FULL_CHARGE)//达到总压设定值
    {
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultFullCharge);
        reason |= 0x04;
    }
    if(chg_ctl_bits & SELFCHECK_CHG_TV_OHV)
    {
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultHTV);
        reason |= 0x04;
    }
    if(chg_ctl_bits & SELFCHECK_CHG_OHV) //达到单体电压设定值
    {
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultHV);
        reason |= 0x10;
    }
    g_guobiao_bms_stop_reason = (INT8U)reason;
    //充电故障原因
    reason = 0;
    if(chg_ctl_bits & SELFCHECK_CC) //充电过流
    {
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultOCC);
        reason |= 0x01;
    }
    data = MV_TO_100MV(bcu_get_total_voltage());
    if((ChargerParaIndex.ChargerSts.ChgrOutVoltMax && data > ChargerParaIndex.ChargerSts.ChgrOutVoltMax)|| //电压不匹配
        (data < ChargerParaIndex.ChargerSts.ChgrOutVoltMin))
    {
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultTVException);
        reason |= 0x04;
    }
    g_guobiao_bms_error_reason = (INT8U)reason;
    //充电错误原因
    reason = 0;
    if(chg_ctl_bits & SELFCHECK_INSU)  //漏电
    {
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultLeak);
        reason |= 0x01;
    }
    // 输出连接器过温故障
    if(chg_ctl_bits & SELFCHECK_CHR_OUTLET_HT)
    {
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultOthers);
        reason |= 0x04;
    }
    if(guobiao_charger_is_connected() == FALSE) // 充电连接器故障
    {
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultLineOff);
        reason |= 0x40;
    }
    if(chg_ctl_bits & SELFCHECK_CHG_OHT) //过温
    {
        guobiao_charger_update_selfcheck_fault_num(kGBChgFaultBatteryHT);
        reason |= 0x100;
    }
    if(guobiao_charger_protocol_version_is_v11())
    {
#if GUOBIAO_RELAY_ADHESION_CHECK_EN
        if(relay_control_get_adhesion_status(kRelayTypeCharging) == kRelayAdhesionTrouble) //继电器粘连
        {
            guobiao_charger_update_selfcheck_fault_num(kGBChgFaultAdhesion);
            reason |= 0x400;
        }
#endif
        //检测点2电压检测故障
    }
    if(//g_guobiao_comm_timeout_cnt >= GUOBIAO_COMM_ABORT_TIMEOUT_TIMES ||
        (reason==0 && g_guobiao_bms_stop_reason==0 && g_guobiao_bms_error_reason==0 && ChargerParaIndex.ChgrSetCfg.ChargeOnOff!=GUOBIAO_CHARGER_ENABLE_CONTROL_BYTE)) //其他故障
    {
        //if(g_guobiao_comm_timeout_cnt >= GUOBIAO_COMM_ABORT_TIMEOUT_TIMES)
        //    guobiao_charger_update_selfcheck_fault_num(kGBChgFaultOverTimeoutTimes);
        //else
            guobiao_charger_update_selfcheck_fault_num_with_control_bits();
        if(guobiao_charger_protocol_version_is_v11()) reason |= 0x4000;
        else reason |= 0x400;
    }
    g_guobiao_bms_wrong_reason = reason;
    
    if(g_guobiao_bms_stop_reason || g_guobiao_bms_wrong_reason || g_guobiao_bms_error_reason)
    {
        if(g_guobiao_check_charge_stop_time == 0)
            g_guobiao_check_charge_stop_time = get_tick_count();
        if(get_interval_by_tick(g_guobiao_check_charge_stop_time, get_tick_count()) >= delay)
        {
            g_guobiao_check_charge_stop_time = 0;
            return FALSE;
        }
    }
    else
    {
        g_guobiao_check_charge_stop_time = 0;
    }
    return TRUE;   
}

INT8U guobiao_charge_is_stop(void)
{
    if(guobiao_check_charge_req(1000) == FALSE)
        return TRUE;
    
    return FALSE;
}

INT16S guobiao_current_to_bms_current(INT16S cur)
{
    if(cur > GUOBIAO_CURRENT_OFFSET)
        return 0;
    if(cur < 0)
        return GUOBIAO_CURRENT_OFFSET;
    
    return -1*(cur - GUOBIAO_CURRENT_OFFSET);
}

INT16S guobiao_current_from_bms_current(INT16S cur)
{
    if(cur < 0)
        return 0;
    if(cur > GUOBIAO_CURRENT_OFFSET)
        cur = GUOBIAO_CURRENT_OFFSET;
    
    return -1*cur + GUOBIAO_CURRENT_OFFSET;
}

extern void adc_onchip_init(void);
void guobiao_cc_detection_init(void)
{
    //g_gb_charge_cc_ad.interval = 2000;
    //g_gb_charge_cc_ad.times = GUOBIAO_CC_VALUE_BUFF_MAX;
    //g_gb_charge_cc_ad.buf = (INT16U* _PAGED)g_guoBiaoCCValueBuff;
    //g_gb_charge_cc_ad.notify_finished = gb_charge_cc_notify_finished;
    adc_onchip_init();
}

void guobiao_cc_update(void)
{
    INT8U i = 0;
    INT16U value;
    ADS1013_HANDLE handle = 0;
    INT8U flag = 0;
    
    if(guobiao_cc_detect_is_enable() == FALSE) return;
    //adc0_channel_start(GUOBIAO_CC_AD_PORT,&g_gb_charge_cc_ad);
    
    //while (flag == 0) sleep(10);
    
    //value = average_check(g_guoBiaoCCValueBuff, GUOBIAO_CC_VALUE_BUFF_MAX);
    
    //if(!xadc0_start(&g_gb_charge_cc_ad_param)) return;
    //if (!xadc0_wait(&value, g_gb_charge_cc_ad_param.interval * g_gb_charge_cc_ad_param.times * 2)) return;
    //value = (INT16U)((INT32U)value * ADC_REFERENCE / 4096);

    if (!adc_onchip_read(&adc_onchip_cc_adc, &value)) return;
    //if(value >= GUOBIAO_CC_CC2_OFFSET) value = value - GUOBIAO_CC_CC2_OFFSET; //二极管压降校正
    //else value = 0;
    g_guoBiaoCCValue = value;
}

void guobiao_cc2_update(void)
{
    INT8U i = 0;
    INT16U value;
    ADS1013_HANDLE handle = 0;
    INT8U flag = 0;
    
    //选择启动CC/CC2
    if(guobiao_cc2_detect_is_enable() == FALSE) return;

    //adc0_channel_start(GUOBIAO_CC2_AD_PORT,&g_gb_charge_cc_ad);
    
    //while (flag == 0) sleep(10);
        
    //value = average_check(g_guoBiaoCCValueBuff, GUOBIAO_CC_VALUE_BUFF_MAX);
    
    //if(!xadc0_start(g_gb_charge_cc2_ad_param)) return;
    //if (!xadc0_wait(&value, g_gb_charge_cc2_ad_param->interval * g_gb_charge_cc2_ad_param->times * 2)) return;
    //value = (INT16U)((INT32U)value * ADC_REFERENCE / 4096);
    if (!adc_onchip_read(g_gb_charge_cc2_adc, &value)) return;    
    //if(value >= GUOBIAO_CC_CC2_OFFSET) value = value - GUOBIAO_CC_CC2_OFFSET; //二极管压降校正
    //else value = 0;
    g_guoBiaoCC2Value = value;
}

void guobiao_cc_cc2_detect(void* data)
{
    UNUSED(data);

    guobiao_cc_update();
    guobiao_cc2_update();
}

INT8U pwm_frequence_is_valid(INT16U freq)
{
    if(freq < CP_PWM_FREQUENCE_MIN || freq > CP_PWM_FREQUENCE_MAX) return FALSE;
    
    return TRUE;
}

void guobiao_pwm_detect(void* data)
{

#if 1
    int cpu_sr = 0;
    unsigned short duty = cp_pwm_get_duty();
    unsigned short freq = cp_pwm_get_frequence();
    static INT8U dly1=0,dly2=0,dly3=0;
    (void)data;
    //if (duty >= GUOBIAO_PWM_DUTY_OFFSET) duty -= GUOBIAO_PWM_DUTY_OFFSET;

    OS_ENTER_CRITICAL();
    g_guobiao_pwm_frequence = freq;
    if(pwm_frequence_is_valid(freq) == TRUE) 
    {
        dly1=0;dly2=0;     
        g_guobiao_pwm_duty_cycle = duty; 
    }
    else
    { 
        if((freq == 0)&&(cp_input_is_high()))
         { 
           dly2=0;  
           if(++dly1 > 4)
           {
             dly1 = 4;
             g_guobiao_pwm_duty_cycle = 1000; 
           }
         }
        else
         {
           dly1=0;  
           if(++dly2 > 4)
           {
             dly2 = 4;
             g_guobiao_pwm_duty_cycle = 0; 
           }
         }
    }
    OS_EXIT_CRITICAL();
    if(GUOBIAO_CP_DUTY_IS_VALID(g_guobiao_pwm_duty_cycle) == 0)
    {
        if(++dly3 > 4)
        {
            dly3 = 4;
            g_pwm_connect_last_tick = get_tick_count();
        }
    }
    else dly3 = 0;
    
#else
    float low_time = 0, high_time = 0;
    
    UNUSED(data);
    
    do /* 试图锁定信号量2 */
    {
        SET_SEM(2);  
    } while (!TST_SEM(2)); /* 检测是否成功锁定信号量2 */
 
    /* CPU12X访问共享资源 */
    if(pwm_finish_flag == 1)//是否为采样完成状态
    {
        g_guobiao_pwm_tick = get_tick_count();
        
        high_time = pwm_high_acc;
        low_time = pwm_low_acc;
        
        low_time = low_time / (low_time + high_time);
        g_guobiao_pwm_duty_cycle = low_time;
        //if(low_time >= GUOBIAO_PWM_DUTY_OFFSET) g_guobiao_pwm_duty_cycle = low_time - GUOBIAO_PWM_DUTY_OFFSET;
        //else g_guobiao_pwm_duty_cycle = 0;
        
        pwm_finish_flag = 2; //更新状态，进入计算完成状态
    }
    else
    {
        if(get_interval_by_tick(g_guobiao_pwm_tick, get_tick_count()) >= GUOBIAO_PWM_COMM_TIMEOUT)
            g_guobiao_pwm_duty_cycle = 0;
    }
    REL_SEM(2);  /* CPU12X释放信号量2 */
#endif
}


void guobiao_pwm_isr(void)
{
    
}

INT8U guobiao_cc_16A_detect_is_enable(void)
{
    INT8U mode;
    
    mode = (INT8U)config_get(kGBChargerConnectModeIndex);
    
    switch(mode)
    {
        case kGBChargerConnectAC16A3B: //配置不进行上位机16A 32A区分，都开启CC
        case kGBChargerConnectAC16A3C:
        case kGBChargerConnectAC16A2B:
        case kGBChargerConnectAC16A1B:
        case kGBChargerConnectAC32A3B:
        case kGBChargerConnectAC32A3C:
        case kGBChargerConnectAC32A2B:
        case kGBChargerConnectAC32A1B:
        case kGBChargerConnectAuto:
            return TRUE;break;
    }
    return FALSE;
}

INT8U guobiao_cc_32A_detect_is_enable(void)
{
    INT8U mode;
    
    mode = (INT8U)config_get(kGBChargerConnectModeIndex);
    
    switch(mode)
    {
        case kGBChargerConnectAC16A3B: //配置不进行上位机16A 32A区分，都开启CC
        case kGBChargerConnectAC16A3C:
        case kGBChargerConnectAC16A2B:
        case kGBChargerConnectAC16A1B:
        case kGBChargerConnectAC32A3B:
        case kGBChargerConnectAC32A3C:
        case kGBChargerConnectAC32A2B:
        case kGBChargerConnectAC32A1B:
        case kGBChargerConnectAuto:
            return TRUE;break;
    }
    return FALSE;
}

INT8U guobiao_cc_detect_is_enable(void)
{
    if(guobiao_cc_16A_detect_is_enable() == TRUE) return TRUE;
    if(guobiao_cc_32A_detect_is_enable() == TRUE) return TRUE;
    
    return FALSE;
}

INT8U guobiao_cc2_detect_is_enable(void)
{
    INT8U mode;
    
    mode = (INT8U)config_get(kGBChargerConnectModeIndex);
    
    switch(mode)
    {
        case kGBChargerConnectDC:
        case kGBChargerConnectAuto:
            return TRUE;break;
    }
    return FALSE;
}

INT8U guobiao_pwm_detect_is_enable(void)
{
    INT8U mode;
    
    mode = (INT8U)config_get(kGBChargerConnectModeIndex);
    
    switch(mode)
    {
        case kGBChargerConnectAC16A3A:
        case kGBChargerConnectAC16A3B:
        case kGBChargerConnectAC16A3C:
        case kGBChargerConnectAC16A2B:
        case kGBChargerConnectAC32A3A:
        case kGBChargerConnectAC32A3B:
        case kGBChargerConnectAC32A3C:
        case kGBChargerConnectAC32A2B:
        case kGBChargerConnectAuto:
            return TRUE;break;
    }
    return FALSE;
}

INT8U guobiao_charger_cc_1500OHM_is_connected(void)
{
    OS_CPU_SR cpu_sr = 0;
    INT16U cc_value;

    OS_ENTER_CRITICAL();
    cc_value = g_guoBiaoCCValue;
    OS_EXIT_CRITICAL();

    if(guobiao_cc_detect_is_enable())
    {
        if(cc_value >= GUOBIAO_CC_1500OHM_VALUE_MIN && cc_value <= GUOBIAO_CC_1500OHM_VALUE_MAX) return TRUE;
    }

    return FALSE;
}

INT8U guobiao_charger_cc_680OHM_is_connected(void)
{
    OS_CPU_SR cpu_sr = 0;
    INT16U cc_value;
    
    OS_ENTER_CRITICAL();
    cc_value = g_guoBiaoCCValue;
    OS_EXIT_CRITICAL();
    
    if(guobiao_cc_detect_is_enable())
    {
        if(cc_value >= GUOBIAO_CC_680OHM_VALUE_MIN && cc_value <= GUOBIAO_CC_680OHM_VALUE_MAX) return TRUE;
    }
    
    return FALSE;
}

INT8U guobiao_charger_cc_220OHM_is_connected(void)
{
    OS_CPU_SR cpu_sr = 0;
    INT16U cc_value;
    
    OS_ENTER_CRITICAL();
    cc_value = g_guoBiaoCCValue;
    OS_EXIT_CRITICAL();
    
    if(guobiao_cc_detect_is_enable())
    {
        if(cc_value >= GUOBIAO_CC_220OHM_VALUE_MIN && cc_value <= GUOBIAO_CC_220OHM_VALUE_MAX) return TRUE;
    }

    return FALSE;
}

INT8U guobiao_charger_cc_100OHM_is_connected(void)
{
    OS_CPU_SR cpu_sr = 0;
    INT16U cc_value;

    OS_ENTER_CRITICAL();
    cc_value = g_guoBiaoCCValue;
    OS_EXIT_CRITICAL();

    if(guobiao_cc_detect_is_enable())
    {
        if(cc_value >= GUOBIAO_CC_100OHM_VALUE_MIN && cc_value <= GUOBIAO_CC_100OHM_VALUE_MAX) return TRUE;
    }

    return FALSE;
}

INT8U guobiao_charger_cc_is_connected(void)
{
    if(guobiao_charger_cc_680OHM_is_connected() == TRUE ||
        guobiao_charger_cc_220OHM_is_connected() == TRUE)
        return TRUE;
    if(guobiao_charger_protocol_version_is_v11()) //V1.1
    {
        if(guobiao_charger_cc_100OHM_is_connected() == TRUE ||
            guobiao_charger_cc_1500OHM_is_connected() == TRUE)
        return TRUE;
    }
    return FALSE;
}

INT8U guobiao_charger_cc2_is_connected(void)
{
    OS_CPU_SR cpu_sr = 0;
    INT16U cc2_value;
    
    OS_ENTER_CRITICAL();
    cc2_value = g_guoBiaoCC2Value;
    OS_EXIT_CRITICAL();
    
    if(guobiao_cc2_detect_is_enable())
    {
        if(cc2_value >= GUOBIAO_CC2_VALUE_MIN && cc2_value <= GUOBIAO_CC2_VALUE_MAX) return TRUE;
    }
    
    return FALSE;
}

INT8U guobiao_charger_pwm_is_connected(void)
{
    INT8U ret = FALSE;
    INT32U now_tick = get_tick_count();
    
    if(guobiao_pwm_detect_is_enable()) 
    {
        INT16U pwm;
        OS_CPU_SR cpu_sr = 0;
        OS_ENTER_CRITICAL();
        pwm = g_guobiao_pwm_duty_cycle;//0.1%
        OS_EXIT_CRITICAL();
        
        if(GUOBIAO_CP_DUTY_IS_VALID(pwm)) 
        {
            if(get_interval_by_tick(g_pwm_connect_last_tick, now_tick) >= 3000)
            {
                //g_pwm_connect_last_tick = now_tick;
                return TRUE;
            }
        }
        else
        {
            g_pwm_connect_last_tick = now_tick;
        }
    }
    else
    {
        g_pwm_connect_last_tick = now_tick;
    }
    return ret;
}

void guobiao_charger_set_selfcheck_fault_num(GuoBiaoChgSelfCheckFault fault_num)
{
    OS_CPU_SR cpu_sr = 0;
    
    OS_ENTER_CRITICAL();
    if(g_guobiao_self_check_fault_num != fault_num)
        g_guobiao_self_check_fault_num = fault_num;
    OS_EXIT_CRITICAL();
}

GuoBiaoChgSelfCheckFault guobiao_charger_get_selfcheck_fault_num(void)
{
    GuoBiaoChgSelfCheckFault current_num;
    OS_CPU_SR cpu_sr = 0;
    
    OS_ENTER_CRITICAL();
    current_num = g_guobiao_self_check_fault_num;
    OS_EXIT_CRITICAL();
    
    return current_num;
}

void guobiao_charger_update_selfcheck_fault_num(GuoBiaoChgSelfCheckFault fault_num)
{
    if(fault_num >= kGBChgFaultSPN3902 && fault_num <= kGBChgFaultSPN3927)
        guobiao_comm_timeout_cnt_update();
    if(guobiao_charger_get_selfcheck_fault_num() == kGBChgFaultNone)
        guobiao_charger_set_selfcheck_fault_num(fault_num);
}

void guobiao_charger_clear_selfcheck_fault_num(void)
{
    guobiao_charger_set_selfcheck_fault_num(kGBChgFaultNone);
}

void guobiao_charger_update_selfcheck_fault_num_with_control_bits(void)
{
    INT32U ctl_bits;
    
    ctl_bits = charger_get_control_bits();
    if(ctl_bits == 0) return;
    
    if((ctl_bits & SELFCHECK_OHV) || (ctl_bits & SELFCHECK_CHG_OHV)) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultHV); //单体高压
    else if((ctl_bits & SELFCHECK_OLV) || (ctl_bits & SELFCHECK_CHG_OLV)) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultLV); //单体低压
    else if((ctl_bits & SELFCHECK_OHT) || (ctl_bits & SELFCHECK_CHG_OHT)) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultBatteryHT); //单体高温
    else if((ctl_bits & SELFCHECK_OLT) || (ctl_bits & SELFCHECK_CHG_OLT)) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultBatteryLT); //单体低温
    else if((ctl_bits & SELFCHECK_DT) || (ctl_bits & SELFCHECK_CHG_DT)) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultDT); //单体温差
    else if((ctl_bits & SELFCHECK_DV) || (ctl_bits & SELFCHECK_CHG_DV)) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultDV); //单体压差
    else if(ctl_bits & SELFCHECK_COMM) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultCommAbort); //内网通信
    else if(ctl_bits & SELFCHECK_LOW_SOC) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultLSOC); //SOC低
    else if(ctl_bits & SELFCHECK_HIGH_SOC) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultHSOC); //SOC高
    else if(ctl_bits & SELFCHECK_INSU) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultLeak); //系统漏电
    else if((ctl_bits & SELFCHECK_TV_OHV) || (ctl_bits & SELFCHECK_CHG_TV_OHV)) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultHTV); //总压高
    else if((ctl_bits & SELFCHECK_TV_OLV) || (ctl_bits & SELFCHECK_CHG_TV_OLV)) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultLTV); //总压低
    else if(ctl_bits & SELFCHECK_VOLT_EXCEPTION) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultVoltLine); //电压排线脱落
    else if(ctl_bits & SELFCHECK_TEMP_EXCEPTION) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultTempLine); //温感排线脱落
    else if(ctl_bits & SELFCHECK_CC) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultOCC); //充电过流
    else if(ctl_bits & SELFCHECK_DC) guobiao_charger_update_selfcheck_fault_num(kGBChgFaultODC); //放电过流
    else guobiao_charger_update_selfcheck_fault_num(kGBChgFaultOthers); //其他故障
}

#pragma INLINE
static INT16U ac_current_to_dc_current(INT16U ac_volt, INT16U ac_cur, INT16U dc_volt)
{
    return DIVISION(GAIN(ac_volt, ac_cur), dc_volt);
}

INT16U guobiao_get_pwm_allow_charge_current_max(void)
{
    OS_CPU_SR cpu_sr = 0;
    INT16U pwm = 0;
    INT16U current = 0x7FFF;
    
    if(guobiao_charger_protocol_version_is_v11() == 0) return current; //v1.0版本
    
    OS_ENTER_CRITICAL();
    pwm = g_guobiao_pwm_duty_cycle; //0.1%
    OS_EXIT_CRITICAL();
    
    if(guobiao_charger_pwm_is_connected())
    {
        INT16U volt = 0, ac_volt = GUOBIAO_AC_VOLT_DEFAULT;
        
        if(bcu_slave_self_check()) volt = MV_TO_V(bcu_get_total_voltage());
        if(volt == 0)
            volt = TV_100_MV_TO_V(config_get(kChargerVoltIndex));
        
        if(pwm >= 80 && pwm < 100) current = CURRENT_100_MA_FROM_A(6);
        else if(pwm >= 100 && pwm <= 850) current = (INT16U)DIVISION((pwm * 6), 10);
        else if(pwm > 850 && pwm <= 900)
        {
            current = (INT16U)DIVISION(((pwm - 640) * 25), 10);
            if(current > CURRENT_100_MA_FROM_A(63)) current = CURRENT_100_MA_FROM_A(63); 
        }
        else current = 0;//根据国标在8%以下，和90%以上都不允许充电
        if(guobiao_charger_cc_100OHM_is_connected() == TRUE)
            ac_volt = GUOBIAO_THREE_PHASE_VOLT;
        else if(guobiao_charger_cc_1500OHM_is_connected() == TRUE)
            ac_volt = GUOBIAO_SINGLE_PHASE_VOLT;
        
        if(volt) current = ac_current_to_dc_current(ac_volt, current, volt);
        else current = 0;
    }
    return current; //0.1A/bit
}

INT16U guobiao_get_cc_allow_charge_current_max(void)
{
    INT8U flag = 0;
    INT16U current = 0x7FFF;
    INT16U ac_volt=GUOBIAO_AC_VOLT_DEFAULT, dc_volt=0;
    
    if(guobiao_charger_cc_680OHM_is_connected() == TRUE)
    {
        flag = 1;
        if(current > GUOBIAO_CC_680OHM_CURRENT_MAX) current = GUOBIAO_CC_680OHM_CURRENT_MAX;
    }
    else if(guobiao_charger_cc_220OHM_is_connected() == TRUE)
    {
        flag = 1;
        if(current > GUOBIAO_CC_220OHM_CURRENT_MAX) current = GUOBIAO_CC_220OHM_CURRENT_MAX;
    }
    else
    {
        if(guobiao_charger_protocol_version_is_v11())
        {
            if(guobiao_charger_cc_1500OHM_is_connected() == TRUE)
            {
                flag = 1;
                ac_volt = GUOBIAO_SINGLE_PHASE_VOLT;
                if(current > GUOBIAO_CC_1500OHM_CURRENT_MAX) current = GUOBIAO_CC_1500OHM_CURRENT_MAX;
            }
            else if(guobiao_charger_cc_100OHM_is_connected() == TRUE)
            {
                flag = 1;
                ac_volt = GUOBIAO_THREE_PHASE_VOLT;
                if(current > GUOBIAO_CC_100OHM_CURRENT_MAX) current = GUOBIAO_CC_100OHM_CURRENT_MAX;
            }
        }
    }
    if(flag)
    {
        if(bcu_slave_self_check()) dc_volt = MV_TO_V(bcu_get_total_voltage());
        if(dc_volt == 0)
            dc_volt = TV_100_MV_TO_V(config_get(kChargerVoltIndex));
        if(dc_volt) current = ac_current_to_dc_current(ac_volt, current, dc_volt);
        else current = 0;
    }
    return current; //0.1A/bit
}

INT16U guobiao_get_allow_charge_current_max(void)
{
    INT16U current = 0x7FFF, temp;
    
    temp = guobiao_get_pwm_allow_charge_current_max();
    current = guobiao_get_cc_allow_charge_current_max();
    if(current > temp) current = temp;
    
    return current;
}

INT8U guobiao_charger_protocol_version_is_v11(void) //v1.1
{
    if(safe_strcmp(g_guobiao_charger_info.version, GUOBIAO_CHARGER_PROTOCOL_VERSION_V11) == 0) return TRUE;
    
    return FALSE;
}

void guobiao_charger_protocol_version_set(PINT8U str)
{
    safe_memcpy((PINT8U)g_guobiao_charger_info.version, str, safe_strlen((PCSTR)str));
}

INT8U guobiao_charger_is_charging_ready(void) // return 0-waitting 1-ready 2-fault
{
#if GUOBIAO_RELAY_ADHESION_CHECK_EN
    if (insu_type_is_none() == 0 && relay_control_is_on(kRelayTypeDischarging)!=1 && relay_control_is_on(kRelayTypePreCharging)!=1)
    {
        if(g_relay_adhesion_check_flag == TRUE &&
            bcu_get_insulation_work_state()!=kInsulationWorkStateNotConnected &&
            bcu_get_insulation_work_state()!=kInsulationWorkStateVoltExcept)
        {
            g_relay_adhesion_check_flag = FALSE;
            relay_control_update_adhesion_status(kRelayTypeCharging, GUOBIAO_RELAY_ADHESION_CHECK_TV);
        }
    }
#endif
    if (guobiao_check_charge_req(0)) 
    {
        relay_control_set_enable_flag(kRelayTypeCharging, TRUE);
        if (relay_control_get_id(kRelayTypeCharging)==0 || relay_control_is_on(kRelayTypeCharging) == 1)
            return 1;
    }
    else
    {
        return 2;
    }
    return 0;
}

void guobiao_charger_charging_ready_check(void)
{
    INT8U flag = guobiao_charger_is_charging_ready();
    
    if(flag == 1)
    {
        g_guobiao_charge_ready_flag = BMS_CHARGE_READY;
    }
    else
    {
        if(flag == 2 || //故障
            (get_elapsed_tick_count(g_bms_charge_ready_tick) >= config_get(kChgRelayOnDlyIndex) + 5000)) //准备就绪超时进入中止充电阶段
        {
            g_gbChargerCommStage = kChargeStopStage;
            return;
        }
        else
            g_guobiao_charge_ready_flag = BMS_CHARGE_NOT_READY;
    }
}

INT8U guobiao_s2_is_needing_cc(void)
{
    if(guobiao_cc_detect_is_enable() && config_get(kGBChargerConnectModeIndex) != kGBChargerConnectAuto)
        return TRUE;
    
    return FALSE;
}

void guobiao_s2_update(void)
{
    INT8U enable_flag = 0;
    
    if ((guobiao_s2_is_needing_cc()==FALSE || guobiao_charger_cc_is_connected()) &&
        guobiao_charger_pwm_is_connected() && //TODO
        bcu_get_chg_start_failure_state() == kStartFailureRecovery &&
        guobiao_check_charge_req(0) == TRUE)
        enable_flag = 1;
        
    if (GUOBIAO_CHARGER_S2_IS_ON()) //已闭合
    {
        if (enable_flag == 0) //充电电流小于截止电流
        {
            if (g_guobiao_bms_error_reason ||
                g_guobiao_bms_wrong_reason ||
                bcu_get_charging_current() < config_get(kChgCutOffCurIndex))
            {
                GUOBIAO_CHARGER_S2_OFF();
            }
        }
    }
    else
    {
        if (enable_flag)
        {
            GUOBIAO_CHARGER_S2_ON();
        }
    }
}

void guobiao_comm_timeout_cnt_update(void)
{
    if(g_guobiao_comm_timeout_cnt_switch) return;
    
    g_guobiao_comm_timeout_cnt_switch = TRUE;
    g_guobiao_comm_timeout_cnt++;
}

INT8U guobiao_charge_is_finish_stop(void) //国标充电结束为正常充电完成结束
{
    if(g_guobiao_bms_wrong_reason == 0 &&
        g_guobiao_bms_error_reason == 0 &&
        g_guobiao_charger_wrong_reason == 0 &&
        g_guobiao_charger_error_reason == 0)
    {
        if((g_guobiao_charger_stop_reason&0x0F) ||
            (g_guobiao_bms_stop_reason&0x3F))
            return TRUE;
    }
    return FALSE;    
}

/*
INT8U guobiao_charger_is_pwm_over_current(void)
{
    INT8U flag = FALSE;
    INT16U current = 0, current_max = 0;
    INT32U now_tick;
    
    now_tick = get_tick_count();
    current = bcu_get_charging_current();
    current_max = guobiao_get_pwm_allow_charge_current_max();
    
    if(current_max <= CURRENT_100_MA_FROM_A(20))
    {
        if(current > CURRENT_100_MA_FROM_A(22))
        {
            if(get_interval_by_tick(g_guobiao_charger_oc_last_tick, now_tick) >= 5000) flag = TRUE;
        }
        else
        {
            g_guobiao_charger_oc_last_tick = now_tick;
        }
    }
    else
    {
        if(current > DIVISION(current_max * 11, 10))
        {
            if(get_interval_by_tick(g_guobiao_charger_oc_last_tick, now_tick) >= 5000) flag = TRUE;
        }
        else
        {
            g_guobiao_charger_oc_last_tick = now_tick;
        }
    }
    
    return flag;
}
*/

#endif

