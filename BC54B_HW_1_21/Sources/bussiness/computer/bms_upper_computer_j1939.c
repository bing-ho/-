/*
 * bms_upper_computer_j1939.c
 *
 *  Created on: 2012-9-19
 *      Author: Administrator
 */

#ifndef BMS_UPPER_COMPUTER_J1939_C_
#define BMS_UPPER_COMPUTER_J1939_C_

#include "bms_upper_computer_j1939.h"
#include "dtu_m35.h"
#include "test_mode.h"
#include "bms_input_signal.h"
#include "bms_charger.h"
#include "run_mode.h"
#include "bms_mlock.h"
#include "bms_charger_temperature.h"
#include "ltc68041_impl.h"
#include "main_bsu_relay_ctrl.h"
#include "bsu_sample.h"
#include "bsu_active_balance.h"
#include "bms_data_read_impl.h"

#include "bms_ocv_soc_bm.h"

#if BMS_SUPPORT_UPPER_COMPUTER == BMS_UPPER_COMPUTER_J1939

#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C5702 // Local variable '' declared in function '' but not referenced
#pragma MESSAGE DISABLE C5919 // Conversion of floating to unsigned integral
#pragma MESSAGE DISABLE C12056

#define FILL_MSG_HEADER(PGN, PRI, DST, SRC)     msg->pgn = PGN;                             \
                                                msg->id.data[0] = PRI << 2;                 \
                                                msg->id.data[1] = (INT8U)(msg->pgn >> 8);   \
                                                msg->id.data[2] = (INT8U)DST;               \
                                                msg->id.data[3] = (INT8U)SRC;


union upper_computer_task_stack g_slave_task_stack;

//OS_STK g_slave_tx_task_stack[SLAVE_TX_STK_SIZE] =
//{ 0 };
//OS_STK g_slave_rx_task_stack[SLAVE_RX_STK_SIZE] =
//{ 0 };

#define FRAME_INTERVAL_TIME 2

OS_EVENT* g_slaveSendEventSem;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_UPPER
J1939SendMessageBuff g_slaveSendMessage;
unsigned char g_slaveSendMessageDataBuff[UPPER_SENT_MESSAGE_LENGTH_MAX];
J1939RecMessage g_slaveRecMessage;
unsigned char g_slaveRecMessageDataBuff[UPPER_REC_MESSAGE_LENGTH_MAX];
INT8U g_slaveNumReq;
INT16U g_slaveMessagePgnReq;
J1939CanContext* _PAGED g_slaveCanContextPtr;

INT16U g_rule_writer_last_pack_index = 0;

static RuleItem g_upper_computer_rule_item;
J1939CanFrame g_upperCanTempFrame; //回调函数使用
J1939CanContext g_upperCanContext;
J1939Event g_upperSendEventSem;
J1939Event g_upperRecEventSem;
J1939CanInfo g_upperCanInfo;
J1939CanTranPro g_upperSendTpIndex;
J1939CanTPBuff g_upperRecTpBuffIndex;
J1939TPChain g_upperRecTpBuffs[UPPER_TP_REC_MAX];
J1939CanFrameBuff g_upperSendFrameBuffIndex;
J1939CanFrame g_upperSendFrameBuffs[UPPER_SENT_FRAME_BUFF];
J1939CanFrameBuff g_upperRecPduBuffIndex;
J1939CanFrame g_upperRecPduBuffs[UPPER_REC_PDU_BUFF];
J1939CanFrameBuff g_upperRecFrameBuffIndex;
J1939CanFrame g_upperRecFrameBuffs[UPPER_REC_FRAME_BUFF];
J1939SendMessageBuff g_upperSendMessageBuff;
unsigned char g_upperSendMessageDataBuff[UPPER_SENT_MESSAGE_LENGTH_MAX];
J1939RecTempMessageBuff g_upperRecTempMessageBuffIndex;
J1939RecMessage g_upperRecTempMessageBuffs[UPPER_TP_REC_MAX];
unsigned char g_upperRecTempMessageDataBuff[UPPER_TP_REC_MAX][UPPER_REC_MESSAGE_LENGTH_MAX];
J1939RecMessageBuff g_upperRecMessageBuffIndex;
J1939RecMessage g_upperRecMessageBuffs[UPPER_REC_MESSAGE_BUFF_MAX];
unsigned char g_upperRecMessageDataBuffs[UPPER_REC_MESSAGE_BUFF_MAX][UPPER_REC_MESSAGE_LENGTH_MAX];

INT32U g_bcu_req_insu_with_precharge_tick = 0; //在预充过程中，绝缘信息请求计时，防止从机信息无法更新
INT32U g_bcu_req_slave_finish_tick = 0; //所有从机数据上传正常的计时，定义最大超时时间，防止一直不间断请求从机信息引起上位机无法正常通信
#pragma DATA_SEG DEFAULT

#define READ_DATA(ID, TYPE, OFFSET) READ_DATA_EX(ID, TYPE, OFFSET, 0)

#define READ_DATA_EX(ID, TYPE, OFFSET, DEFAULT) \
        if (OFFSET + sizeof(TYPE) <= rec_msg->byte_cnt) \
        {\
          ID = READ_LT_##TYPE(rec_msg->data, OFFSET); \
        }\
        else     \
        {\
          ID = DEFAULT;\
          OFFSET += sizeof(TYPE);\
        }

#define READ_STRING(ID, OFFSET) \
        READ_DATA(len, INT16U, OFFSET); \
        if (OFFSET + len + 2 <= rec_msg->byte_cnt && len > 0 && *(rec_msg->data + OFFSET + len - 1) == '\0') \
            ID = (PCSTR)rec_msg->data + OFFSET; \
        OFFSET += len;

#define WRITE_STRING(DATA, OFFSET, TEXT)\
        len = safe_strlen(TEXT) + 1; \
        WRITE_LT_INT16U_WITH_BUFF_SIZE(DATA, OFFSET, len, msg->byte_max); \
        safe_memcpy(DATA + OFFSET, (PINT8U)TEXT, len); \
        OFFSET += len;

#define SLAVE_ID_TO_BUFF_NUM(ID)     (ID-SLAVE_ADDR_OFFSET-1)    //将ID(181,...)转换成缓存号(BUFF=0,1,...)
#define SLAVE_BUFF_NUM_TO_ID(NUM)    (NUM+SLAVE_ADDR_OFFSET+1)   //将缓存号(BUFF=0,1,...)转换成ID(181,...)

#define SLAVE_ID_TO_SLAVE_NUM(ID)    (ID-SLAVE_ADDR_OFFSET)      //将ID(181,...)转换成从机号(1,2...)
#define SLAVE_NUM_TO_SLAVE_ID(NUM)   (NUM+SLAVE_ADDR_OFFSET)     //将从机号(1,2,...)转换成ID(181,...)

#define SLAVE_NUM_TO_SLAVE_BUFF_NUM(NUM)    (NUM - 1)            //将从机号(1,2,...)转换成缓存号(0,1,...)
#define SLAVE_BUFF_NUM_TO_SLAVE_NUM(NUM)    (NUM + 1)            //将缓存号(0,1,...)转换成从机号(1,2,...)

void upper_computer_task_create(char is_test_mode);
INT32U bcu_get_slave_req_cycle(void);
Result bcu_ping_device(J1939SendMessageBuff* _PAGED msg, INT8U id);
void bcu_req_insu(J1939SendMessageBuff* _PAGED msg);
void bcu_req_insu_with_precharge(J1939SendMessageBuff* _PAGED msg);
Result bcu_req_insu_message(J1939SendMessageBuff* _PAGED msg);
Result bcu_send_message_to_insu(J1939SendMessageBuff* _PAGED msg);
void bcu_send_message_to_insu_fill(J1939SendMessageBuff* _PAGED msg);
void bcu_req_pcm(J1939SendMessageBuff* _PAGED msg);
Result bcu_req_pcm_message(J1939SendMessageBuff* _PAGED msg);
void bcu_req_para_reset(void);
void bcu_req_all_slaves(J1939SendMessageBuff* _PAGED msg);
INT16U bcu_get_send_message_timeout(J1939SendMessageBuff* _PAGED msg);
void bcu_req_slave(J1939SendMessageBuff* _PAGED msg, INT8U id, INT16U* _PAGED pgn);
void bcu_req_slave_message_fill(J1939SendMessageBuff*  _PAGED msg, INT8U slave, INT16U pgn);
void bcu_req_slave_and_pgn_update(INT8U*  _PAGED slave, INT16U*  _PAGED pgn);
void bcu_req_slave_pgn_next(INT16U* _PAGED pgn);
void bcu_req_slave_next(INT8U* _PAGED slave);
void bcu_send_slave_balance_ctl_message_fill(J1939SendMessageBuff* _PAGED msg, INT8U id);

INT8U bcu_base_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_battery_coding_update(J1939RecMessage* _PAGED msg);
INT8U bcu_common_alarm_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_volt_alarm_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_current_alarm_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_temperature_alarm_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_soc_alarm_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_insu_alarm_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_insu2_alarm_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_relay_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_heat_cool_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_charger_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_chg_volt_alarm_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_chg_temp_alarm_config_update(J1939RecMessage* _PAGED msg);
INT8U bcu_mlock_config_update(J1939RecMessage* _PAGED msg);
INT8U bmu_base_config_update(J1939RecMessage* _PAGED msg);
INT8U bmu_balance_config_update(J1939RecMessage* _PAGED msg);
INT8U bmu_5160_config_update(J1939RecMessage* _PAGED msg);
INT8U dtu_config_update(J1939RecMessage *_PAGED msg);
INT8U bcu_comm_ctrl_update(J1939RecMessage* _PAGED msg);   
INT8U bcu_chgr_outlet_config_update(J1939RecMessage* _PAGED msg);

INT8U require_pgn_is_bcu_pgn(INT16U pgn);
INT8U send_bcu_pgn_message_required(INT16U pgn);
void bms_query_device_comm_info(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bms_query_ping_info(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bms_query_relay_info(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_base_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_battery_coding_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_common_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_volt_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_current_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_temperature_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_soc_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_insu_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_insu2_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_relay_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_heat_cool_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_charger_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_chg_volt_alarm_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_chg_temp_alarm_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_base_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_statistic_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_chgr_outlet_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);

INT8U bmu_base_status_update(J1939RecMessage* _PAGED msg);
INT8U bmu_volt_status_update(J1939RecMessage* _PAGED msg);
INT8U bmu_temp_status_update(J1939RecMessage* _PAGED msg);
INT8U bmu_relay_status_update(J1939RecMessage* _PAGED msg);
INT8U bmu_alarm_status_update(J1939RecMessage* _PAGED msg);
void insu_base_message_update(J1939RecMessage* _PAGED msg);
void pcm_base_message_update(J1939RecMessage* _PAGED msg);
void bms_alarm_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bms_relay_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bms_insu_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bms_charger_info_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bms_mlock_info_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bms_rule_read_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bms_expression_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bms_rule_write_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bcu_debug_info_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bmu_base_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bmu_relay_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bmu_alarm_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bmu_balance_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bmu_5160_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bmu_base_status_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bmu_volt_status_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bmu_temperature_status_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bmu_relay_status_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bmu_alarm_status_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
void bms_confirm_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, INT8U ack, INT8U group, INT32U pgn, INT8U dest);
INT8U bms_config_device_comm_info(J1939RecMessage* _PAGED rec_msg);
INT8U bms_config_device_relay_info(J1939RecMessage* _PAGED rec_msg);
void bms_handle_test_info(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
//void bms_handle_debug_req(J1939CanContext* _PAGED context, J1939RecMessage* _PAGED receive_msg, J1939SendMessageBuff* _PAGED send_msg);
void bms_date_time_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg);
INT8U bms_config_date_time_info(J1939RecMessage* _PAGED msg);
Result bcu_send_message_to_slave(J1939SendMessageBuff* _PAGED msg, INT16U response_timeout);
static INT8U bcu_nandflash_data_fill(J1939CanContext* _PAGED context,const struct StorageObject *__FAR obj, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg) ;
static void bcu_nandflash_scan(J1939RecMessage* _PAGED rec_msg); 
/**********************************************
 *
 * Implementation
 *
 ***********************************************/

INT16U upper_computer_callback(can_t context, CanMessage* msg, void* userdata)
{
    INT8U i;
    UNUSED(userdata);

    (void)context;
    g_upperCanTempFrame.id.value = can_id_to_extend_id(msg->id.value);
    g_upperCanTempFrame.byte_cnt = msg->len;
    for(i=0; i<msg->len; i++)
        g_upperCanTempFrame.data[i] = msg->data[i];
    J1939CanRecFrame(g_slaveCanContextPtr,&g_upperCanTempFrame);
    return 1;
}

void upper_computer_init_ext(char is_test_mode)
{
    INT16U i;
    
    if (g_slaveCanContextPtr != NULL ) return;

    g_upperCanInfo.addr = BCU_CAN_ADDR_DEF;//BCU_CAN_ADDR_DEF;//(unsigned char)config_get(kBcuCanAddrIndex);
    g_upperCanInfo.dev = (INT8U)config_get(kSlaveCanChannelIndex);
    if(can_channel_is_valid(g_upperCanInfo.dev) == FALSE || g_upperCanInfo.dev != UPPER_COMPUTER_CAN_DEV)
    {
        g_upperCanInfo.dev = UPPER_COMPUTER_CAN_DEV;
        bcu_reset_all_can_channel();
    }
    if(g_upperCanInfo.dev == kCanDev3) g_upperCanInfo.bps = can_get_bps((INT8U)config_get(kCAN5BpsIndex));
    else g_upperCanInfo.bps = can_get_bps((INT8U)config_get(kCAN1BpsIndex + g_upperCanInfo.dev));
    
    g_upperCanInfo.id_style = UPPER_COMPUTER_CAN_MODE;
    g_upperCanInfo.mask_id.value = UPPER_COMPUTER_CAN_RECEIVE_MASK_ID;
    g_upperCanInfo.receive_id.value = UPPER_COMPUTER_CAN_RECEIVE_ID|((INT16U)g_upperCanInfo.addr<<8);

    g_slaveSendMessage.byte_max = UPPER_SENT_MESSAGE_LENGTH_MAX;
    g_slaveSendMessage.data = g_slaveSendMessageDataBuff;

    g_slaveRecMessage.byte_max = UPPER_REC_MESSAGE_LENGTH_MAX;
    g_slaveRecMessage.data = g_slaveRecMessageDataBuff;
    if(g_slaveSendEventSem == NULL)
        g_slaveSendEventSem = OSSemCreate(0);
    //J1939 driver init
    g_slaveCanContextPtr = &g_upperCanContext;

    g_upperCanContext.send_event_sem = &g_upperSendEventSem;
    if(g_upperCanContext.send_event_sem->event == NULL)
        g_upperCanContext.send_event_sem->event = J1939CreateSem(0);

    g_upperCanContext.receive_event_sem = &g_upperRecEventSem;
    if(g_upperCanContext.receive_event_sem->event == NULL)
        g_upperCanContext.receive_event_sem->event = J1939CreateSem(0);

    g_upperCanContext.can_info = &g_upperCanInfo;

    g_upperCanContext.send_tp_index = &g_upperSendTpIndex;

    g_upperCanContext.rec_tp_buff = &g_upperRecTpBuffIndex;
    g_upperCanContext.rec_tp_buff->tp_max = UPPER_TP_REC_MAX;
    g_upperCanContext.rec_tp_buff->buffer = g_upperRecTpBuffs;

    g_upperCanContext.send_frame_buff = &g_upperSendFrameBuffIndex;
    g_upperCanContext.send_frame_buff->buffer_cnt = UPPER_SENT_FRAME_BUFF;
    g_upperCanContext.send_frame_buff->buffer = g_upperSendFrameBuffs;

    g_upperCanContext.rec_pdu_buff = &g_upperRecPduBuffIndex;
    g_upperCanContext.rec_pdu_buff->buffer_cnt = UPPER_REC_PDU_BUFF;
    g_upperCanContext.rec_pdu_buff->buffer = g_upperRecPduBuffs;

    g_upperCanContext.rec_frame_buff = &g_upperRecFrameBuffIndex;
    g_upperCanContext.rec_frame_buff->buffer_cnt = UPPER_REC_FRAME_BUFF;
    g_upperCanContext.rec_frame_buff->buffer = g_upperRecFrameBuffs;

    g_upperCanContext.send_message_buff = &g_upperSendMessageBuff;
    g_upperCanContext.send_message_buff->byte_max = UPPER_SENT_MESSAGE_LENGTH_MAX;
    g_upperCanContext.send_message_buff->data = g_upperSendMessageDataBuff;

    g_upperCanContext.rec_temp_mess_buff = &g_upperRecTempMessageBuffIndex;
    g_upperCanContext.rec_temp_mess_buff->buffer = g_upperRecTempMessageBuffs;
    for(i=0; i<UPPER_TP_REC_MAX; i++)
    {
        g_upperCanContext.rec_temp_mess_buff->buffer[i].data = g_upperRecTempMessageDataBuff[i];
        g_upperCanContext.rec_temp_mess_buff->buffer[i].byte_max = UPPER_REC_MESSAGE_LENGTH_MAX;
    }
    g_upperCanContext.rec_temp_mess_buff->buffer_cnt = UPPER_TP_REC_MAX;

    g_upperCanContext.rec_message_buff = &g_upperRecMessageBuffIndex;
    g_upperCanContext.rec_message_buff->buffer = g_upperRecMessageBuffs;
    for(i=0; i<UPPER_REC_MESSAGE_BUFF_MAX; i++)
    {
        g_upperCanContext.rec_message_buff->buffer[i].data = g_upperRecMessageDataBuffs[i];
        g_upperCanContext.rec_message_buff->buffer[i].byte_max = UPPER_REC_MESSAGE_LENGTH_MAX;
    }

    g_upperCanContext.rec_message_buff->buffer_cnt = UPPER_TP_REC_MAX;
    g_upperCanInfo.receive_callback = upper_computer_callback;
    //end of J1939 driver init
    J1939InitWithBuff(g_slaveCanContextPtr,(J1939CanInfo* _PAGED)(&g_upperCanInfo));

    upper_computer_task_create(is_test_mode);
}

void upper_computer_init(void) {
    upper_computer_init_ext(0);
}

INT8U bcu_req_slave_device_is_stop(void)
{
    if(rule_config_writer_is_open()) return TRUE;
    
    return FALSE;
}

void slave_task_run_tx(void* pdata)
{
    J1939SendMessageBuff SendMessage;
    INT8U SendMessageBuff[CAN_DATA_MAX_LEN];
    INT32U last_tick;
    void* userData = pdata;

    sleep(BMS_POWER_ON_VALID_SLAVE_DELAY);
    bcu_req_para_reset();
    SendMessage.byte_max = CAN_DATA_MAX_LEN;
    SendMessage.data = SendMessageBuff;
    for (;;)
    {
        sleep(5);
        watch_dog_feed(WDT_SLAVECAN_ID);
        if(get_elapsed_tick_count(last_tick) >= bcu_get_slave_req_cycle())
        {
            last_tick = get_tick_count();
        
            bcu_req_all_slaves(&SendMessage);
            
            bcu_req_insu_with_precharge(&SendMessage);
            
#if BMS_SUPPORT_PCM_MODE > 0        
            bcu_req_pcm(&SendMessage);
#endif
        }
    }
}

INT32U bcu_get_slave_req_cycle(void)
{
    if(relay_control_is_on(kRelayTypePreCharging) == 1) return config_get(kReqBmuMessageIntervalIndex);
    
    if(bcu_slave_self_check() == 0)
    {
        if(g_bcu_req_slave_finish_tick == 0) g_bcu_req_slave_finish_tick = get_tick_count();
        if(get_elapsed_tick_count(g_bcu_req_slave_finish_tick) < BCU_REQ_SLAVE_ALL_VALID_INFO_TIMEOUT)
        {
            return config_get(kReqBmuMessageIntervalIndex);
        }
    }
    return BMS_SLAVE_CAN_QUERY_PEROID;
}

void bcu_req_para_reset(void)
{
    g_slaveNumReq=SLAVE_START_NUM;
    g_slaveMessagePgnReq=BMU_STATUS_START_PGN;
}

void bcu_check_slave_addr_conflict_with_byu(INT8U* _PAGED slave)
{
    
 #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1    
    if(config_get(kBCUCommModeIndex) != BMS_COMM_SLAVE_AND_BYU_MODE) return;
    
    
    if(*slave == BYU_USE_SLAVE_INDEX + 1)
    {
        bcu_req_slave_next(slave);
    }
#endif
#if  BMS_SUPPORT_HARDWARE_LTC6804 == 1  
     (void)slave;
    if(config_get(kBCUCommModeIndex) != BMS_COMM_BYU_MODE) return;
  #endif  
}

void bcu_req_all_slaves(J1939SendMessageBuff* _PAGED msg)
{
    INT8U flag = 1;
    INT32U last_tick,diff;

    if(!(config_get(kBCUCommModeIndex)==BMS_COMM_SLAVE_MODE ||
        ((config_get(kBCUCommModeIndex)==BMS_COMM_SLAVE_AND_BYU_MODE) && config_get(kSlaveNumIndex)>1))) return;

    g_slaveNumReq = SLAVE_START_NUM;
    g_slaveMessagePgnReq = BMU_STATUS_START_PGN;
    while(flag)
    {
        if(bcu_req_slave_device_is_stop()) return;
            
        last_tick = get_tick_count();
        bcu_check_slave_addr_conflict_with_byu(&g_slaveNumReq);
        bcu_req_slave(msg, g_slaveNumReq, &g_slaveMessagePgnReq);
        diff = get_interval_by_tick(last_tick, get_tick_count());
        if(diff < config_get(kReqBmuMessageIntervalIndex))
            sleep(config_get(kReqBmuMessageIntervalIndex) - (INT16U)diff);
        else
            sleep(config_get(kReqBmuMessageIntervalIndex));
        bcu_req_slave_next(&g_slaveNumReq);
        g_slaveMessagePgnReq = BMU_STATUS_START_PGN;
        if((g_slaveNumReq == SLAVE_START_NUM)&&(g_slaveMessagePgnReq == BMU_STATUS_START_PGN))
        {
            flag = 0;
        }
    }
}

INT16U bcu_get_send_message_timeout(J1939SendMessageBuff* _PAGED msg)
{
    INT16U byte_cnt;
    INT32U timeout;

    if(msg->byte_cnt > msg->byte_max)
        byte_cnt = msg->byte_max;
    else
        byte_cnt = msg->byte_cnt;

    if(byte_cnt <= 8)
        return CAN_FRAME_TIMEOUT_DEFINE;
    else
    {
        timeout = (INT32U)CAN_FRAME_TIMEOUT_DEFINE*((byte_cnt + 6) / 7);
        if(timeout > 0xFFFF)
            return 0xFFFF;
        else
            return (INT16U)timeout;
    }
}

Result bcu_send_message_to_slave(J1939SendMessageBuff* _PAGED msg, INT16U response_timeout)
{
    INT8U err;
    INT16U send_timeout;
    
    if(msg == NULL) return RES_ERROR;
    
    send_timeout = bcu_get_send_message_timeout(msg);
    
    if (J1939CanSendMessageWithFinish(g_slaveCanContextPtr, msg, send_timeout) == J1939_OK)
    {
        OSSemPend(g_slaveSendEventSem, response_timeout, &err);
        if(err == OS_NO_ERR)
        {
            return RES_OK;
        }
    }
    
    return ERR_TIMEOUT;
}


void bcu_req_insu_with_precharge(J1939SendMessageBuff* _PAGED msg)
{
    INT8U pre_flag = 0;
    
    if(msg == NULL) return;
    
    if(bcu_req_slave_device_is_stop()) return;
    g_bcu_req_insu_with_precharge_tick = get_tick_count();
    
    do
    {
        bcu_req_insu(msg);
        if(get_elapsed_tick_count(g_bcu_req_insu_with_precharge_tick) >= BCU_REQ_INSU_WITH_PRECHARGE_TIMEOUT)
        {
            break;
        }
        pre_flag = relay_control_is_on(kRelayTypePreCharging);
        if(pre_flag == 1) sleep(config_get(kReqBmuMessageIntervalIndex)); //在预充状态下启动请求周期延时
    }while(pre_flag == 1);
}

void bcu_req_insu(J1939SendMessageBuff* _PAGED msg)
{
    if(msg == NULL) return;
    if(!(insu_type_is_can()||insu_type_is_both())) return;
    
    g_slaveNumReq = INSU_ADDRESS;
    g_slaveMessagePgnReq = BMS_DEVICE_PING_PGN;
    if(bcu_ping_device(msg, g_slaveNumReq) != RES_OK) return;
    
    if(bcu_send_message_to_insu(msg)) return;
    if(bcu_req_insu_message(msg) != RES_OK) return;
    
}

Result bcu_ping_device(J1939SendMessageBuff* _PAGED msg, INT8U id)
{
    Result err;
    
    if(msg == NULL) return RES_ERR;
    
    bcu_req_slave_message_fill(msg, id, BMS_DEVICE_PING_PGN);
    start_bcu_insu_heart_beat_tick();
    err = bcu_send_message_to_slave(msg, BCU_REQ_BMU_FIRST_FRAME_TIMEOUT);
    
    if(err != RES_OK) return RES_ERR;
    
    sleep(FRAME_INTERVAL_TIME);
    return RES_OK;
}

Result bcu_req_insu_message(J1939SendMessageBuff* _PAGED msg)
{
    Result err;
    
    if(msg == NULL) return RES_ERR;
    
    g_slaveMessagePgnReq = INSU_BASE_STATUS_PGN;
    bcu_req_slave_message_fill(msg, g_slaveNumReq, INSU_BASE_STATUS_PGN);
    start_bcu_insu_heart_beat_tick();
    err = bcu_send_message_to_slave(msg, config_get(kBmuResponseTimeoutIndex));
    
    if(err != RES_OK) return RES_ERR;
    
    sleep(FRAME_INTERVAL_TIME);
    return RES_OK;
}

Result bcu_send_message_to_insu(J1939SendMessageBuff* _PAGED msg)
{
    Result err;
    
    if(msg == NULL) return RES_ERR;
    
    bcu_send_message_to_insu_fill(msg);
    start_bcu_insu_heart_beat_tick();
    err = bcu_send_message_to_slave(msg, config_get(kBmuResponseTimeoutIndex));
    
    if(err != RES_OK) return RES_ERR;
    
    sleep(FRAME_INTERVAL_TIME);
    return RES_OK;
}

void bcu_send_message_to_insu_fill(J1939SendMessageBuff* _PAGED msg)
{
    INT8U index=0, data = 0;
    
    if(msg == NULL) return;
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0x00, msg->byte_max);
    if(charger_is_connected())
        data |= 0x01;
    if(relay_control_is_on(kRelayTypePreCharging) == 1)
        data |= 0x02;
    WRITE_BT_INT8U_WITH_BUFF_SIZE(msg->data, index, data, msg->byte_max);
    
    msg->byte_cnt = index;
    FILL_MSG_HEADER(BCU_TO_LDM_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, g_slaveNumReq, BCU_CAN_ADDR_DEF);//config_get(kBcuCanAddrIndex)
}

void bcu_req_pcm(J1939SendMessageBuff* _PAGED msg)
{
    if(msg == NULL) return;
    
    if(bcu_req_slave_device_is_stop()) return;
    g_slaveNumReq = PCM_ADDRESS;
    g_slaveMessagePgnReq = BMS_DEVICE_PING_PGN;
    if(bcu_ping_device(msg, g_slaveNumReq) != RES_OK) return;
    
    if(bcu_req_pcm_message(msg) != RES_OK) return;    
}

Result bcu_req_pcm_message(J1939SendMessageBuff* _PAGED msg)
{
    Result err;
    
    if(msg == NULL) return RES_ERR;
    
    g_slaveMessagePgnReq = PCM_BASE_STATUS_PGN;
    bcu_req_slave_message_fill(msg, g_slaveNumReq, PCM_BASE_STATUS_PGN);
    start_bcu_pcm_heart_beat_tick();
    err = bcu_send_message_to_slave(msg, config_get(kBmuResponseTimeoutIndex));
    
    if(err != RES_OK) return RES_ERR;
    
    sleep(FRAME_INTERVAL_TIME);
    return RES_OK;
}

void bcu_req_slave(J1939SendMessageBuff* _PAGED msg, INT8U id, INT16U* _PAGED pgn)
{
    Result err;
    INT8U flag = 1;

    bcu_send_slave_balance_ctl_message_fill(msg, id);
    start_bmu_heart_beat_tick(SLAVE_NUM_TO_SLAVE_BUFF_NUM(id));
    err = bcu_send_message_to_slave(msg, BCU_REQ_BMU_FIRST_FRAME_TIMEOUT);
    
    if(err != RES_OK) return;
    
    sleep(FRAME_INTERVAL_TIME);

    while(flag)
    {
        if(bcu_req_slave_device_is_stop()) return;
        
        bcu_req_slave_message_fill(msg, SLAVE_NUM_TO_SLAVE_ID(id) ,*pgn);
        err = bcu_send_message_to_slave(msg, config_get(kBmuResponseTimeoutIndex));
        
        if(err == RES_OK)
        {
            bcu_req_slave_pgn_next(pgn);
            if(*pgn == BMU_STATUS_START_PGN) flag = 0;
            else sleep(FRAME_INTERVAL_TIME);
        }
        else flag = 0;
    }
}

void bcu_send_slave_balance_ctl_message_fill(J1939SendMessageBuff* _PAGED msg, INT8U id)
{
    INT8U index=0, data = 0;
    INIT_WRITE_BUFFFER();

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0x00, msg->byte_max);
    if(charger_is_connected() && bcu_is_charging())
        data |= 0x01;
    if(config_get(kBmuBalanceEnableIndex))
        data |= 0x02;
    data |= 0x04;//M5160电源控制位
#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN
    {
        // 唤醒模式不允许从机控制继电器,防止亏空铅酸电池
        int is_wakeup_mode = (runmode_get() == RUN_MODE_WAKEUP);
        if(is_wakeup_mode || (bmu_heat_control_is_on(SLAVE_NUM_TO_SLAVE_BUFF_NUM(id)) == 0)) {
            data |= 0x08;
        }
        if(is_wakeup_mode || (bmu_cool_control_is_on(SLAVE_NUM_TO_SLAVE_BUFF_NUM(id)) == 0)) {
            data |= 0x10;        
        }
    }
#endif
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, data, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_average_voltage(), msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0xFF, msg->byte_max);
    msg->byte_cnt = index;
    FILL_MSG_HEADER(BCU_TO_BMU_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, SLAVE_NUM_TO_SLAVE_ID(id), BCU_CAN_ADDR_DEF);//config_get(kBcuCanAddrIndex)
}

void bcu_req_slave_message_fill(J1939SendMessageBuff* _PAGED msg, INT8U slave, INT16U pgn)
{
    INT8U index=0;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0x00, msg->byte_max);
    WRITE_BT_INT16U_WITH_BUFF_SIZE(msg->data, index, pgn, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0xFFFF, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0xFF, msg->byte_max);

    msg->byte_cnt = index;
    FILL_MSG_HEADER(MESSAGE_REQUIRE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, slave, BCU_CAN_ADDR_DEF);//config_get(kBcuCanAddrIndex)
    msg->pgn = pgn;
}

void bcu_req_slave_next(INT8U* _PAGED slave)
{
    INT8U req_slave;

    req_slave = *slave;
    req_slave++;
    if(req_slave > config_get(kSlaveNumIndex))
        req_slave = SLAVE_START_NUM;

    *slave = req_slave;
}

void bcu_req_slave_pgn_next(INT16U* _PAGED pgn)
{
    INT16U req_pgn;

    req_pgn = *pgn;
    req_pgn += 0x100;
    if(req_pgn > BMU_STATUS_STOP_PGN)
    {
        req_pgn = BMU_STATUS_START_PGN;
    }
    *pgn = req_pgn;
}

void bcu_req_slave_and_pgn_update(INT8U*  _PAGED slave, INT16U*  _PAGED pgn)
{
    INT8U req_slave;
    INT16U req_pgn;

    req_slave = *slave;
    req_pgn = *pgn;

    bcu_req_slave_pgn_next(&req_pgn);

    if(req_pgn == BMU_STATUS_START_PGN)
    {
        bcu_req_slave_next(&req_slave);
    }

    *slave = req_slave;
    *pgn = req_pgn;
}

void slave_task_run_rx(void* pdata)
{
    INT8U rec;
    INT16U pgn = 0;
    void *data = pdata;

    for (;;)
    {
        if (J1939_OK == J1939CanRecMessageWithWait(g_slaveCanContextPtr, &g_slaveRecMessage, 0))
        {
            switch(g_slaveRecMessage.pgn)
            {
            case BMS_DEVICE_COMM_CTL_PGN:
                rec = bcu_comm_ctrl_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BMS_DEVICE_COMM_CTL_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BMS_DEVICE_COMM_INFO_PGN:
                rec = bms_config_device_comm_info(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BMS_DEVICE_COMM_INFO_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case DATE_TIME_PGN:
                rec = bms_config_date_time_info(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, DATE_TIME_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BMS_DEVICE_RELAY_PGN:
                rec = bms_config_device_relay_info(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BMS_DEVICE_RELAY_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BMS_PACK_TEST_PGN:
                bms_handle_test_info(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            //case BMS_DEBUG_REQ_PGN:
            //    bms_handle_debug_req(g_slaveCanContextPtr, &g_slaveRecMessage, &g_slaveSendMessage);
            //    J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
            //    break;
            case BCU_RULE_READ_REQ_PGN:
                bms_rule_read_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_RULE_WRITE_REQ_PGN:
                bms_rule_write_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_EXPRESS_REQ_PGN:
                bms_expression_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_BASE_CONFIG_PGN:
                rec = bcu_base_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_BASE_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_BATTERY_CODING_PGN:
                rec = bcu_battery_coding_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_BATTERY_CODING_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_COMMON_ALARM_CONFIG_PGN:
                rec = bcu_common_alarm_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_COMMON_ALARM_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_VOLT_ALARM_CONFIG_PGN:
                rec = bcu_volt_alarm_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_VOLT_ALARM_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_CURRENT_ALARM_CONFIG_PGN:
                rec = bcu_current_alarm_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_CURRENT_ALARM_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_TEMPERATURE_ALARM_CONFIG_PGN:
                rec = bcu_temperature_alarm_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_TEMPERATURE_ALARM_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_SOC_ALARM_CONFIG_PGN:
                rec = bcu_soc_alarm_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_SOC_ALARM_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_INSU_ALARM_CONFIG_PGN:
                rec = bcu_insu_alarm_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_INSU_ALARM_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_INSU2_ALARM_CONFIG_PGN:
                rec = bcu_insu2_alarm_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_INSU2_ALARM_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;    
            case BCU_RELAY_CONFIG_PGN:
                rec = bcu_relay_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_RELAY_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_HEAT_COOL_CONFIG_PGN:
                rec = bcu_heat_cool_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_HEAT_COOL_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_CHARGER_CONFIG_PGN:
                rec = bcu_charger_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_CHARGER_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_CHG_VOLT_ALARM_CONFIG_PGN:
                rec = bcu_chg_volt_alarm_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_CHG_VOLT_ALARM_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_CHG_TEMP_ALARM_CONFIG_PGN:
                rec = bcu_chg_temp_alarm_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_CHG_TEMP_ALARM_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_MLOCK_CONTROL_PGN:
                rec = bcu_mlock_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_MLOCK_CONTROL_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BMU_BASE_CONFIG_PGN:
                rec = bmu_base_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BMU_BASE_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BMU_RELAY_CONFIG_PGN:
                rec = RESPONSE_ACK;
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BMU_RELAY_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BMU_ALARM_CONFIG_PGN:
                rec = RESPONSE_ACK;
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BMU_ALARM_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BMU_BALANCE_CONFIG_PGN:
                rec = bmu_balance_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BMU_BALANCE_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BMU_5160_CONFIG_PGN:
                rec = bmu_5160_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BMU_5160_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BMU_BASE_STATUS_PGN:
                bmu_base_status_update(&g_slaveRecMessage);
                bmu_heart_beat_tick_update(SLAVE_ID_TO_BUFF_NUM(g_slaveRecMessage.id.data[3]));
                if(((SLAVE_ID_TO_SLAVE_NUM(g_slaveRecMessage.id.data[3]))==g_slaveNumReq)&&(g_slaveMessagePgnReq==g_slaveRecMessage.pgn))
                    OSSemPost(g_slaveSendEventSem);
                break;
            case BMU_VOLT_STATUS_PGN:
                bmu_volt_status_update(&g_slaveRecMessage);
                bmu_heart_beat_tick_update(SLAVE_ID_TO_BUFF_NUM(g_slaveRecMessage.id.data[3]));
                if(((SLAVE_ID_TO_SLAVE_NUM(g_slaveRecMessage.id.data[3]))==g_slaveNumReq)&&(g_slaveMessagePgnReq==g_slaveRecMessage.pgn))
                    OSSemPost(g_slaveSendEventSem);
                break;
            case BMU_TEMP_STATUS_PGN:
                bmu_temp_status_update(&g_slaveRecMessage);
                bmu_heart_beat_tick_update(SLAVE_ID_TO_BUFF_NUM(g_slaveRecMessage.id.data[3]));
                if(((SLAVE_ID_TO_SLAVE_NUM(g_slaveRecMessage.id.data[3]))==g_slaveNumReq)&&(g_slaveMessagePgnReq==g_slaveRecMessage.pgn))
                    OSSemPost(g_slaveSendEventSem);
                break;
            case BMU_RELAY_STATUS_PGN:
                bmu_relay_status_update(&g_slaveRecMessage);
                bmu_heart_beat_tick_update(SLAVE_ID_TO_BUFF_NUM(g_slaveRecMessage.id.data[3]));
                if(((SLAVE_ID_TO_SLAVE_NUM(g_slaveRecMessage.id.data[3]))==g_slaveNumReq)&&(g_slaveMessagePgnReq==g_slaveRecMessage.pgn))
                    OSSemPost(g_slaveSendEventSem);
                break;
            case BMU_ALARM_STATUS_PGN:
                bmu_alarm_status_update(&g_slaveRecMessage);
                bmu_heart_beat_tick_update(SLAVE_ID_TO_BUFF_NUM(g_slaveRecMessage.id.data[3]));
                if(((SLAVE_ID_TO_SLAVE_NUM(g_slaveRecMessage.id.data[3]))==g_slaveNumReq)&&(g_slaveMessagePgnReq==g_slaveRecMessage.pgn))
                    OSSemPost(g_slaveSendEventSem);
                break;
            case BMS_DEVICE_PING_PGN:
                if(g_slaveRecMessage.id.data[3] == g_slaveNumReq && g_slaveMessagePgnReq == BMS_DEVICE_PING_PGN)
                {
                    if(g_slaveRecMessage.id.data[3] == INSU_ADDRESS) bcu_insu_heart_beat_tick_update();
                    if(g_slaveRecMessage.id.data[3] == PCM_ADDRESS) bcu_pcm_heart_beat_tick_update();
                    
                    OSSemPost(g_slaveSendEventSem);
                }
                break;
            case INSU_BASE_STATUS_PGN:
                if(g_slaveRecMessage.id.data[3] == INSU_ADDRESS && g_slaveMessagePgnReq == INSU_BASE_STATUS_PGN)
                {
                    bcu_insu_heart_beat_tick_update();
                    insu_base_message_update(&g_slaveRecMessage);
                    OSSemPost(g_slaveSendEventSem);
                }
                break;
            case PCM_BASE_STATUS_PGN:
                if(g_slaveRecMessage.id.data[3] == PCM_ADDRESS && g_slaveMessagePgnReq == PCM_BASE_STATUS_PGN)
                {
                    bcu_pcm_heart_beat_tick_update();
                    pcm_base_message_update(&g_slaveRecMessage);
                    OSSemPost(g_slaveSendEventSem);
                }
                break;
            case DTU_CONFIG_MESSAGE_PGN:
                rec = dtu_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, DTU_CONFIG_MESSAGE_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case BCU_CHGR_OUTLET_TEMP_CONFIG_PGN:
                rec = bcu_chgr_outlet_config_update(&g_slaveRecMessage);
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, rec, 0, BCU_CHGR_OUTLET_TEMP_CONFIG_PGN, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            case MESSAGE_REQUIRE_PGN:
                if (g_slaveRecMessage.id.data[2] == g_slaveCanContextPtr->can_info->addr) //对本机请求数据
                {
                    pgn = ((INT16U) g_slaveRecMessage.data[1] << 8) + g_slaveRecMessage.data[0];
                    if(require_pgn_is_bcu_pgn(pgn))
                        send_bcu_pgn_message_required(pgn);
                }
                break;
            case MESSAGE_CONFIRM_PGN:
                if((SLAVE_ID_TO_SLAVE_NUM(g_slaveRecMessage.id.data[3]))==g_slaveNumReq ||
                    g_slaveRecMessage.id.data[3] == INSU_ADDRESS ||
                    g_slaveRecMessage.id.data[3] == PCM_ADDRESS)
                    OSSemPost(g_slaveSendEventSem);
                break;
            case BCU_NAND_SCAN_PGN:      //接收上位机请求Flash数据
                bcu_nandflash_scan(&g_slaveRecMessage);                        
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, RESPONSE_ACK, 0, g_slaveRecMessage.pgn, g_slaveRecMessage.id.data[3]);                   
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));                
                break;
            default:
                bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, RESPONSE_NACK, 0, g_slaveRecMessage.pgn, g_slaveRecMessage.id.data[3]);
                J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
                break;
            }
        }
        else
        {

        }
    }
}

static INT8U bcu_nandflash_data_fill(J1939CanContext* _PAGED context,const struct StorageObject *__FAR obj, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg) 
{                
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    INT32U start_page_addr, end_page_addr, Page_Addr; 
 
    INT32U read_require_num = (((INT32U)rec_msg->data[7])<<24) + (((INT32U)rec_msg->data[6])<<16) + (((INT32U)rec_msg->data[5])<<8) + (INT32U)rec_msg->data[4];
    
    if(bms_get_nandflash_scan_require() == 1)   //nand 日期查询未结束
    {
        return 1;       	
    }
    
    start_page_addr = bms_get_nand_start_page_addr();
    end_page_addr   = bms_get_nand_stop_page_addr(); 
 
    if((start_page_addr == 0)||(end_page_addr == 0))   //未查询到有效日期
    {
        return 1;     
    }
    
    if(start_page_addr <= end_page_addr)        //数据段 位置 64 - start - end -1023
    {
        Page_Addr = read_require_num/8 + start_page_addr;  
        if(Page_Addr > end_page_addr)          //读取超出范围
        {
            return 1;
        } 
    } 
    else      //数据段 位置 start - 1023 - 64 - end
    { 
        if((read_require_num/8 + start_page_addr) <= (info->pages_per_block * (obj->last_block + 1) - 1)) 
        {
            Page_Addr = read_require_num/8 + start_page_addr;
        } 
        else
        {
           Page_Addr = (read_require_num/8 + start_page_addr) - (info->pages_per_block * (obj->last_block + 1) - 1) + (info->pages_per_block * obj->first_block) - 1;
           if(Page_Addr > end_page_addr) //读取超出范围
           {
                return 1;       
           } 
        }     
    } 
      
    FILL_MSG_HEADER(BCU_NAND_READ_PGN,BCU_CAN_MESSAGE_PRI_DEFAULT,rec_msg->id.data[3],context->can_info->addr);     
    
    if(obj->flash->ops->read_page(obj->flash->platform, Page_Addr))
    {
	    	return 1;       
    }
    
  	if(obj->flash->ops->read_page_data(obj->flash->platform, (INT16U)((read_require_num%8)*256),msg->data, 256) != 256)
  	{
  	  	return 1;
  	}
	
    msg->byte_cnt = 256;
     
   	return 0;
}

void bcu_nandflash_scan(J1939RecMessage* _PAGED rec_msg) 
{  
    INT32U start_date;
    INT32U end_date;

    start_date = (((INT32U)rec_msg->data[3])<<24) + (((INT32U)rec_msg->data[2])<<16) + (((INT32U)rec_msg->data[1])<<8) + (INT32U)rec_msg->data[0];
    end_date = (((INT32U)rec_msg->data[7])<<24) + (((INT32U)rec_msg->data[6])<<16) + (((INT32U)rec_msg->data[5])<<8) + (INT32U)rec_msg->data[4];    

    bms_set_nandflash_scan_require(start_date, end_date);
}
//static
void dtu_info_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg) {
    INT8U index = 0;
    union {
        unsigned short lacci[2];
        unsigned char signal_quality[2];
        unsigned char selfnum[12];
        unsigned char imei[16];
        unsigned char cimi[21];
        unsigned char runtime_status[5];
        INT32U packet_counter;
    } temp;
    
    FILL_MSG_HEADER(DTU_INFO_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0x00, msg->byte_max);
    DTUM35_GetLacCi(temp.lacci);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp.lacci[0], msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp.lacci[1], msg->byte_max);
    DTUM35_GetSingalQuality(&temp.signal_quality[0], &temp.signal_quality[1]);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, temp.signal_quality[0], msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, temp.signal_quality[1], msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DTUM35_GetOPS(), msg->byte_max);
    DTUM35_GetPhoneNum((char *)temp.selfnum, sizeof(temp.selfnum));
    WRITE_BUFFER(msg->data, index, (unsigned char *)temp.selfnum, sizeof(temp.selfnum));
    DTUM35_GetIMEI((char *)temp.imei, sizeof(temp.imei));
    WRITE_BUFFER(msg->data, index, temp.imei, sizeof(temp.imei));
    DTUM35_GetCIMI((char *)temp.cimi, sizeof(temp.cimi));
    WRITE_BUFFER(msg->data, index, temp.cimi, sizeof(temp.cimi));
    DTUM35_GetRuntimeStatusAll(temp.runtime_status, sizeof(temp.runtime_status));
    WRITE_BUFFER(msg->data, index, temp.runtime_status, sizeof(temp.runtime_status));
    temp.packet_counter = DTUM35_GetPacketCounter();
    WRITE_LT_INT32U_WITH_BUFF_SIZE(msg->data, index, temp.packet_counter, msg->byte_max);

    msg->byte_cnt = index;
}
//static
void dtu_config_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg) {
    INT8U index = 0;
    unsigned char addr[40];
    unsigned int port = 0;
    unsigned char type;

    FILL_MSG_HEADER(DTU_CONFIG_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);

    memset(addr, 0, sizeof(addr));

    if(input_signal_is_high_by_name("GSM_ID"))
    {
        type = 0; // 无DTU
    } 
    else 
    {
        type = 1; // 板载DTU   
    }
    config_get_dtu_type(&type);
    config_get_dtu_server((unsigned char *_PAGED)addr, sizeof(addr), &port);

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0x00, msg->byte_max);
    WRITE_BUFFER(msg->data, index, addr, strlen((char *)addr)+ 1);
    WRITE_BT_INT16U_WITH_BUFF_SIZE(msg->data, index, port, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, type, msg->byte_max);

    msg->byte_cnt = index;
}

INT8U require_pgn_is_bcu_pgn(INT16U pgn)
{
    if(pgn >= BMU_BASE_CONFIG_PGN && pgn <= BMU_5160_CONFIG_PGN) return TRUE;
    if(pgn >= BMU_BASE_STATUS_PGN && pgn <= BMU_ALARM_STATUS_PGN) return TRUE;
    if(pgn == BCU_INSU2_ALARM_CONFIG_PGN) return TRUE;
    if(pgn == DTU_CONFIG_MESSAGE_PGN) return TRUE;
    
    if(((pgn >= BMS_DEVICE_COMM_INFO_PGN)&&(pgn <= BCU_PGN_END)) || pgn == DATE_TIME_PGN)
        return TRUE;
    else
        return FALSE;
}

INT8U send_bcu_pgn_message_required(INT16U pgn)
{
    INT8U res = J1939_OK, nack_flag = 0;

    switch(pgn)
    {
    case BMS_DEVICE_COMM_INFO_PGN:
        bms_query_device_comm_info(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMS_DEVICE_PING_PGN:
        bms_query_ping_info(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMS_DEVICE_RELAY_PGN:
        bms_query_relay_info(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_BASE_CONFIG_PGN:
        bcu_base_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_BATTERY_CODING_PGN:
        bcu_battery_coding_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_COMMON_ALARM_CONFIG_PGN:
        bcu_common_alarm_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_VOLT_ALARM_CONFIG_PGN:
        bcu_volt_alarm_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_CURRENT_ALARM_CONFIG_PGN:
        bcu_current_alarm_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_TEMPERATURE_ALARM_CONFIG_PGN:
        bcu_temperature_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_SOC_ALARM_CONFIG_PGN:
        bcu_soc_alarm_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_INSU_ALARM_CONFIG_PGN:
        bcu_insu_alarm_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_INSU2_ALARM_CONFIG_PGN:
        bcu_insu2_alarm_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;    
    case BCU_RELAY_CONFIG_PGN:
        bcu_relay_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_HEAT_COOL_CONFIG_PGN:
        bcu_heat_cool_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_CHARGER_CONFIG_PGN:
        bcu_charger_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_CHG_VOLT_ALARM_CONFIG_PGN:
        bcu_chg_volt_alarm_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_CHG_TEMP_ALARM_CONFIG_PGN:
        bcu_chg_temp_alarm_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_BASE_MESSAGE_PGN: //BCU基本信息
        bcu_base_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_STATISTIC_MESSAGE_PGN://BCU统计信息
        bcu_statistic_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_ALARM_MESSAGE_PGN://BCU报警信息
        bms_alarm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_RELAY_MESSAGE_PGN://BCU继电器信息
        bms_relay_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_INSU_MESSAGE_PGN://BCU绝缘信息
        bms_insu_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_CHARGER_INFO_MESSAGE_PGN: //充电机信息
        bms_charger_info_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_MLOCK_CONTROL_PGN: //电子锁控制
        bms_mlock_info_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_DEBUG_MESSAGE_PGN:
        bcu_debug_info_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMU_BASE_CONFIG_PGN:
        bmu_base_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMU_RELAY_CONFIG_PGN:
        bmu_relay_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMU_ALARM_CONFIG_PGN:
        bmu_alarm_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMU_BALANCE_CONFIG_PGN:
        bmu_balance_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMU_5160_CONFIG_PGN:
        bmu_5160_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMU_BASE_STATUS_PGN:
        bmu_base_status_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMU_VOLT_STATUS_PGN:
        bmu_volt_status_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMU_TEMP_STATUS_PGN:
        bmu_temperature_status_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMU_RELAY_STATUS_PGN:
        bmu_relay_status_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BMU_ALARM_STATUS_PGN:
        bmu_alarm_status_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case DATE_TIME_PGN:
        bms_date_time_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case DTU_INFO_MESSAGE_PGN:
        dtu_info_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case DTU_CONFIG_MESSAGE_PGN:
        dtu_config_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;
    case BCU_CHGR_OUTLET_TEMP_CONFIG_PGN:
        bcu_chgr_outlet_config_fill(g_slaveCanContextPtr, &g_slaveSendMessage, &g_slaveRecMessage);
        break;

    case BCU_NAND_READ_PGN:      //上传Flash数据
        if(bcu_nandflash_data_fill(g_slaveCanContextPtr,&record_storage_obj,&g_slaveSendMessage,&g_slaveRecMessage)) 
        {
            nack_flag = 1;
            bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, RESPONSE_NACK, 0, pgn, g_slaveRecMessage.id.data[3]);
        } 
        break;

    default:
        nack_flag = 1;
        bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, RESPONSE_NACK, 0, pgn, g_slaveRecMessage.id.data[3]);
        break;
    }
    res = J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
    if(res == J1939_ERR_TP_BUSY)//传输协议传输缓存满
    {
        bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, RESPONSE_BUSY, 0, pgn, g_slaveRecMessage.id.data[3]);
        res = J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
    }
    else if(res == J1939_ERR_BUFF_FULL)//单PDU发送缓存满
    {
        if(nack_flag == 0)
            bms_confirm_message_fill(g_slaveCanContextPtr, &g_slaveSendMessage, RESPONSE_BUSY, 0, pgn, g_slaveRecMessage.id.data[3]);
        sleep(5);
        res = J1939CanSendMessageWithFinish(g_slaveCanContextPtr, &g_slaveSendMessage, bcu_get_send_message_timeout(&g_slaveSendMessage));
    }
    return res;
}

INT8U bcu_comm_ctrl_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 0;
    INT8U id, cmd, temp;
    if (msg == NULL || msg->byte_cnt < 2) return RESPONSE_NACK;

    id = READ_LT_INT8U(msg->data, index);
    if (id != 0) return RESPONSE_NACK;

    cmd = READ_LT_INT8U(msg->data, index);
    temp = READ_LT_INT8U(msg->data, index);
    if (cmd == BCU_COMM_CMD_ENTER_TEST)         //进入测试模式
    {
        runmode_reset_to(RUN_MODE_TEST);
        //bcu_start_reset();
        return RESPONSE_ACK;
    }
    else if(cmd == BCU_COMM_CMD_ENTER_READ && charger_is_connected() == 0 && bcu_get_discharging_current() < CURRENT_START_TO_READ_MODE)     //进入读取模式
    {
        runmode_reset_to(RUN_MODE_READ);
        return RESPONSE_ACK;     
    }
    else if(cmd == BCU_COMM_CMD_ENTER_DETECT)   //进入强检模式
    {
        runmode_reset_to(RUN_MODE_DETECT);
        return RESPONSE_ACK;     
    }
    else if(cmd == BCU_COMM_CMD_ENTER_NORMAL)   //恢复正常模式
    {
        runmode_reset_to(RUN_MODE_NORMAL);
        return RESPONSE_ACK;
    }

    config_save(kBootloaderUpdateModeIndex, temp);
    if (cmd == BCU_COMM_CMD_RESTART)
    {
        bcu_start_reset();
        return RESPONSE_ACK;
    }

    return RESPONSE_NACK;
}


INT8U bcu_base_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;
    INT16U left_cap;

    config_set_save_err_flag(0);
    if(index < msg->byte_cnt)config_save(kSlaveNumIndex, READ_LT_INT8U(msg->data, index));
    index++;
    index+=1;//if(index < msg->byte_cnt)config_save(kTemperatureNumIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)bcu_set_current_auto_check_flag(READ_LT_INT16U(msg->data, index) !=0 ? 1 : 0);
    if(index+1 < msg->byte_cnt)config_save(kNominalCapIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kTotalCapIndex, READ_LT_INT16U(msg->data, index));
    // Left Cap
    if(index+1 < msg->byte_cnt)
    {
        left_cap = READ_LT_INT16U(msg->data, index);
        if (left_cap != 0xFFFF && left_cap != config_get(kLeftCapIndex))
        {
            soc_update_left_cap_by_tenfold_ah(left_cap);
            
            set_delta_battery_cap_ams(0);
            set_left_cap_ams_display((INT32U)left_cap*3600*100);
            set_soc_bm_high((float)(left_cap) / config_get(kTotalCapIndex));
            set_soc_bm_low((float)(left_cap) / config_get(kTotalCapIndex));
            
            save_delta_battery_cap_ams_to_config();
            save_left_cap_ams_display_to_config();
            
            config_save(kSOCBMAvailableFlag, 0);
        }
    }

    if(index+1 < msg->byte_cnt)config_save(kSystemVoltageCalibration, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kAlarmBeepInterval, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kAlarmBeepMaskLow, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kAlarmBeepMaskHigh, READ_LT_INT16U(msg->data, index));
    //if(index+1 < msg->byte_cnt)config_save(kChargerProtocolIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kBmuBalanceEnableIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kCurSenTypeIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kCapCalibHighAverageVoltage, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kCapCalibLowAverageVoltage, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kCapCalibLowSoc, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kCapCalibLowSocDly, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kReqBmuMessageIntervalIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kBmuResponseTimeoutIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kCurZeroFilterIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kCurSampleintervalIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kCurSampleCntIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kInsulationType, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kBatteryTotalNum, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kNominalTotalVolt, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChargeEfficiencyFactor, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDisChargeEfficiencyFactor, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kBmuMaxVoltageNumIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kBmuMaxTempNumIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCAN1BpsIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCAN2BpsIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCAN3BpsIndex, READ_LT_INT8U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kRS4851BpsIndex, READ_LT_INT8U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kRS4852BpsIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kBCUCommModeIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kGBChargerConnectModeIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kSlaveCanChannelIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChargerCanChannelIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kEmsCanChannelIndex, READ_LT_INT8U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kHmiRS485ChannelIndex, READ_LT_INT8U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDtuRS485ChannelIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCurSenRangeTypeIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCAN5BpsIndex, READ_LT_INT8U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDtuCanChannelIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kSysStatusSaveIntervalIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgTotalTimeMaxIndex, READ_LT_INT8U(msg->data, index));    
    left_cap = READ_LT_INT8U(msg->data, index);
    if(left_cap == BMS_VIN_LEN)
    {
        if(index+1 < msg->byte_cnt)config_save(kVIN1Index, READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)config_save(kVIN2Index, READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)config_save(kVIN3Index, READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)config_save(kVIN4Index, READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)config_save(kVIN5Index, READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)config_save(kVIN6Index, READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)config_save(kVIN7Index, READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)config_save(kVIN8Index, READ_LT_INT16U(msg->data, index));
        if(index < msg->byte_cnt)config_save(kVIN9Index, READ_LT_INT8U(msg->data, index));
    }
    else
    {
        index += left_cap;
    }
    if(index+1 < msg->byte_cnt)config_save(kWakeupIntervalMinutes, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kWorkMaxTimeAfterWakeupSeconds, READ_LT_INT16U(msg->data, index));
    
    if(index+1 < msg->byte_cnt)config_save(kCurSenType1Index, READ_LT_INT16U(msg->data, index));////////////////////
    
    if(index+1 < msg->byte_cnt)config_save(kChgTotalCapLowIndex, READ_LT_INT16U(msg->data, index)); //累计充电电量
    if(index+1 < msg->byte_cnt)config_save(kChgTotalCapHighIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgTotalCapLowIndex, READ_LT_INT16U(msg->data, index));//累计放电电量
    if(index+1 < msg->byte_cnt)config_save(kDChgTotalCapHighIndex, READ_LT_INT16U(msg->data, index));
    
    if(index+1 < msg->byte_cnt)config_save(kSOC0DischgFactorIndex, READ_LT_INT16U(msg->data, index));//放电因子
    
    index+=2;//UD_SOC              预留BC52A
    index+=1;//S文件记录开关       预留BC52A
    index+=1;//S文件UD信息记录开关 预留BC52A
    index+=1;//启动UDSOC矫正开关   预留BC52A 
      
    if(index+1 < msg->byte_cnt)config_save(kCurrentDivideResIndex, READ_LT_INT16U(msg->data, index)); //分流器电阻值
    
    if(index+1 <= msg->byte_cnt)config_save(kCanWkupEnableIndex, READ_LT_INT8U(msg->data, index));  //网络唤醒使能
    
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

INT8U bcu_battery_coding_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 0, res;

    config_set_save_err_flag(0);
    
    if(index+1 < msg->byte_cnt)config_save(kcompany_1, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kcompany_2_with_producttype, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(k_batterytype_with_standard1, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kstandard2_with_retrace_info1, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kretrace_info2, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kretrace_info3, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kretrace_info4, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kproduct_date1, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kproduct_date2_with_battery_SN1, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kbattery_SN2, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kbattery_SN3, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kbattery_SN4, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kladder_utilization, READ_LT_INT16U(msg->data, index));
    
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}


INT8U bcu_chgr_outlet_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;
    
    config_set_save_err_flag(0);
    if(index+1 < msg->byte_cnt)config_save(kChgrAcTempNumIndex, READ_LT_INT8U(msg->data, index));
    
    if(index+1 < msg->byte_cnt)config_save(kChgrAcOutletHTFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+2 < msg->byte_cnt)config_save(kChgrAcOutletHTFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index+1 < msg->byte_cnt)config_save(kChgrAcOutletHTFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrAcOutletHTFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));

    if(index+1 < msg->byte_cnt)config_save(kChgrAcOutletHTSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+2 < msg->byte_cnt)config_save(kChgrAcOutletHTSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index+1 < msg->byte_cnt)config_save(kChgrAcOutletHTSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrAcOutletHTSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    
    index+=1;//if(index+1 < msg->byte_cnt)config_save(kChgrAcOutletHTTrdAlarmIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrAcOutletHTTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index+1 < msg->byte_cnt)config_save(kChgrAcOutletHTTrdAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrAcOutletHTTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    
    if(index+1 < msg->byte_cnt)config_save(kChgrDcTempNumIndex, READ_LT_INT8U(msg->data, index));
    
    if(index+1 < msg->byte_cnt)config_save(kChgrDcOutletHTFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+2 < msg->byte_cnt)config_save(kChgrDcOutletHTFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index+1 < msg->byte_cnt)config_save(kChgrDcOutletHTFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrDcOutletHTFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));

    if(index+1 < msg->byte_cnt)config_save(kChgrDcOutletHTSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+2 < msg->byte_cnt)config_save(kChgrDcOutletHTSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index+1 < msg->byte_cnt)config_save(kChgrDcOutletHTSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrDcOutletHTSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    
    index+=1;//if(index+1 < msg->byte_cnt)config_save(kChgrDcOutletHTTrdAlarmIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrDcOutletHTTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index+1 < msg->byte_cnt)config_save(kChgrDcOutletHTTrdAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrDcOutletHTTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    
    if(index+2 < msg->byte_cnt)config_save(kChgrOutletTempLineFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));    // 充电插座温感排线报警一级延时
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrOutletTempLineFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index)); // 充电插座温感排线报警一级释放延时
    if(index+2 < msg->byte_cnt)config_save(kChgrOutletTempLineSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));    // 充电插座温感排线报警二级延时
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrOutletTempLineSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index)); // 充电插座温感排线报警二级释放延时
    index+=2;//if(index+2 < msg->byte_cnt)config_save(kChgrOutletTempLineTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));    // 充电插座温感排线报警三级延时
    index+=2;//if(index+2 <= msg->byte_cnt)config_save(kChgrOutletTempLineTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index)); // 充电插座温感排线报警三级释放延时
    
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;

}

void bms_query_ping_info(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT16U index = 0;
    
    FILL_MSG_HEADER(BMS_DEVICE_PING_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;
    msg->byte_cnt = index;
}

void bms_query_device_comm_info(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT16U index = 0, read_pos;
    INT16U len = 0,macro_temp;
    INT16U data;
    
    FILL_MSG_HEADER(BMS_DEVICE_COMM_INFO_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_J1939_PROTOCOL_MAJOR_VERSION, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_J1939_PROTOCOL_MINOR_VERSION, msg->byte_max);
    // 大端字节流
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kUUIDPart1), msg->byte_max);
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kUUIDPart2), msg->byte_max);
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kUUIDPart3), msg->byte_max);
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kUUIDPart4), msg->byte_max);
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kUUIDPart5), msg->byte_max);
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kUUIDPart6), msg->byte_max);
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kUUIDPart7), msg->byte_max);
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kUUIDPart8), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kProductDatePart1), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kProductDatePart2), msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, DEVICE_TYPE, msg->byte_max);
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kHardwareRevisionIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DEVICE_SW_MAJOR_VER, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DEVICE_SW_MINOR_VER, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DEVICE_SW_REV_VER, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, DEVICE_SW_BUILD_VER, msg->byte_max);
    WRITE_STRING(msg->data, index, bcu_get_device_name_4_display());
    WRITE_STRING(msg->data, index, DEVICE_VENDOR_NAME);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, ORDER_SW_MAJOR_VER, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, ORDER_SW_MINOR_VER, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, ORDER_SW_REV_VER, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, ORDER_SW_BUILD_VER, msg->byte_max);
    // 小端字节流
    len = 0;
    data = 0;
    read_pos = 0;
    while (read_pos < 7)
    {
        if (config_get(kSNNumber1Index+read_pos) != '\0')
        {
            len++;
            if((config_get(kSNNumber1Index+read_pos) >> 8) == '\0')
            {
                data = 1;
                break;
            }
            read_pos++;
        }
        else
        {
            break;
        }
    }
    if(data == 1 && len > 0) data = len * 2 - 1;//更新实际长度
    else data = len * 2;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, data, msg->byte_max);
    read_pos = 0;
    while (len--)
    {
        data = config_get(kSNNumber1Index+read_pos);
        if ((data >> 8) == '\0')
        {
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, data, msg->byte_max);
        }
        else
        {
            WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, data, msg->byte_max);
        }
        read_pos++;
    }
    msg->byte_cnt = index;
}

void bms_query_relay_info(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    RelayCtlStatus status;
    INT16U offset = 0;
    INT16U len = 0;
    INT8U index, relay_num = relay_count();

    FILL_MSG_HEADER(BMS_DEVICE_RELAY_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, relay_num, msg->byte_max);
    for (index = 1; index <= relay_num; ++index)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, relay_is_on(index), msg->byte_max);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(
                                    msg->data,
                                    offset,
                                    relay_control_relay_type_to_computer_display(relay_control_get_type(index)),
                                    msg->byte_max);
    }
    for (index = 1; index <= relay_num; ++index)
    {
        status = relay_control_get_force_command(relay_control_get_type(index));
        if(status == kRelayForceOn)
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0x01, msg->byte_max)
        else if(status == kRelayForceOff)
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0x02, msg->byte_max)
        else
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0x00, msg->byte_max)
    }
    relay_num = input_signal_get_count(); 
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, relay_num, msg->byte_max);
    for (index = 1; index <=relay_num; ++index)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, input_signal_is_low(index), msg->byte_max);
        
        WRITE_LT_INT8U_WITH_BUFF_SIZE(
                                    msg->data,
                                    offset,
                                    input_signal_control_type_to_computer_display(input_signal_control_get_type(index)),
                                    msg->byte_max);
    }

    msg->byte_cnt = offset;
}

INT8U bms_config_device_relay_info(J1939RecMessage* _PAGED rec_msg)
{
    int read_pos = 1;
    INT16U offset = 0;
    INT8U  index, relay_num = 0, status, type;

    READ_DATA(relay_num, INT8U, read_pos);
    for (index = 0; index < relay_num; ++index)
    {
        READ_DATA_EX(status, INT8U, read_pos, 0xFF);
        READ_DATA_EX(type, INT8U, read_pos, 0xFF);
        if (type != 0xFF)
        {
            relay_control_set_type(index + 1,
                                relay_control_relay_type_from_computer_display(type, index + 1));
        }
        // TODO: control
        //if (status != 0xFF) relay_set(index + 1, status);
    }
    for(index = 0; index < relay_num; ++index)
    {
        READ_DATA_EX(status, INT8U, read_pos, 0xFF);
        if(status == 0x00)
            relay_force_control_cancle(relay_control_get_type(index+1));
        else if(status == 0x01)
            relay_force_control_on(relay_control_get_type(index+1));
        else if(status == 0x02)
            relay_force_control_off(relay_control_get_type(index+1));
    }
    READ_DATA(relay_num, INT8U, read_pos);
    for(index = 0; index < relay_num; ++index)
    {
        READ_DATA_EX(status, INT8U, read_pos, 0xFF);
        READ_DATA_EX(type, INT8U, read_pos, 0xFF);
        if (type != 0xFF)
        {
            input_signal_control_set_type(index + 1,
                                input_signal_control_type_from_computer_display(type, index + 1));
        }
    }

    return RESPONSE_ACK;
}

INT8U bms_config_device_comm_info(J1939RecMessage* _PAGED rec_msg)
{
    int read_pos = 3;
    INT16U len;
    INT16U data;
    INT32U data32;
    INT16U index = 0;
    PCSTR str;

    if (read_pos+16 <= rec_msg->byte_cnt)
    {
        // 大端字节流
        data = READ_BT_INT16U(rec_msg->data, read_pos);
        config_save(kUUIDPart1, data);
        data = READ_BT_INT16U(rec_msg->data, read_pos);
        config_save(kUUIDPart2, data);
        data = READ_BT_INT16U(rec_msg->data, read_pos);
        config_save(kUUIDPart3, data);
        data = READ_BT_INT16U(rec_msg->data, read_pos);
        config_save(kUUIDPart4, data);
        data = READ_BT_INT16U(rec_msg->data, read_pos);
        config_save(kUUIDPart5, data);
        data = READ_BT_INT16U(rec_msg->data, read_pos);
        config_save(kUUIDPart6, data);
        data = READ_BT_INT16U(rec_msg->data, read_pos);
        config_save(kUUIDPart7, data);
        data = READ_BT_INT16U(rec_msg->data, read_pos);
        config_save(kUUIDPart8, data);
    }
    else
    {
        return RESPONSE_NACK;
    }

    if (read_pos + sizeof(INT32U) <= rec_msg->byte_cnt)
    {
        data32 = READ_LT_INT32U(rec_msg->data, read_pos);
        if (data32 != 0)
        {
            config_save(kProductDatePart1, (INT16U)data32);
            config_save(kProductDatePart2, (INT16U)(data32 >> 16));
        }
    }
    read_pos += 2; //设备类型
    if (read_pos + 1 < rec_msg->byte_cnt) //硬件版本
    {
        data = READ_BT_INT16U(rec_msg->data, read_pos);
        config_save(kHardwareRevisionIndex, data);
    }
    read_pos += 5; //软件版本
    if (read_pos <= rec_msg->byte_cnt)
    {
        READ_STRING(str, read_pos); //设备名称
    }
    if (read_pos <= rec_msg->byte_cnt)
    {
        READ_STRING(str, read_pos); //厂商名称
    }
    read_pos += 5; //订单版本
    if (read_pos < rec_msg->byte_cnt) //SN码
    {
        INT8U offset = 0;
        index = 0;
        len = READ_LT_INT8U(rec_msg->data, read_pos);
        // 小端字节流
        while(index <len)
        {
            if (index++ < len)
            {
                if (index+1 <len)
                {
                    data = READ_LT_INT16U(rec_msg->data, read_pos); index++;
                }
                else
                {
                    data = READ_LT_INT8U(rec_msg->data, read_pos);
                }
                if (offset < 7) config_save(kSNNumber1Index+offset, data);
                offset++;
            }
        }
        while(offset < 7)
        {
            config_save(kSNNumber1Index+offset, '\0'); offset++;
        }
    }
    return RESPONSE_ACK;
}

INT8U bms_config_date_time_info(J1939RecMessage* _PAGED msg)
{
    int read_pos = 0;
    DateTime tm;
    INT16U index = 0;
    Result res;

    if(msg->byte_cnt >= 6)
    {
        memset(&tm, 0, sizeof(tm));

        tm.second = READ_LT_INT8U(msg->data, read_pos);
        tm.minute = READ_LT_INT8U(msg->data, read_pos);
        tm.hour = READ_LT_INT8U(msg->data, read_pos);
        tm.month = READ_LT_INT8U(msg->data, read_pos);
        tm.day = READ_LT_INT8U(msg->data, read_pos) / 4;
        tm.year = READ_LT_INT8U(msg->data, read_pos) - 15;
        res = clock_set(&tm);

        return res == RES_OK ? RESPONSE_ACK : RESPONSE_NACK;
    }
    else
        return RESPONSE_NACK;
}

void bcu_chgr_outlet_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_CHGR_OUTLET_TEMP_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgrAcTempNumIndex), msg->byte_max);
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgrAcOutletHTFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgrAcOutletHTFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHGR_AC_OUTLET_HT_FST_ALARM_REL, msg->byte_max);//config_get(kChgrAcOutletHTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHGR_AC_OUTLET_HT_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgrAcOutletHTFstAlarmRelDlyIndex)
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgrAcOutletHTSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgrAcOutletHTSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHGR_AC_OUTLET_HT_SND_ALARM_REL, msg->byte_max);//config_get(kChgrAcOutletHTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHGR_AC_OUTLET_HT_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgrAcOutletHTSndAlarmRelDlyIndex)
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_OUTLET_HT_TRD_ALARM_DEF, msg->byte_max);//config_get(kChgrAcOutletHTTrdAlarmIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_OUTLET_HT_TRD_ALARM_DLY_DEF, msg->byte_max);//config_get(kChgrAcOutletHTTrdAlarmDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_OUTLET_HT_TRD_ALARM_REL_DEF, msg->byte_max);//config_get(kChgrAcOutletHTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_OUTLET_HT_TRD_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kChgrAcOutletHTTrdAlarmRelDlyIndex)
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgrDcTempNumIndex), msg->byte_max);
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgrDcOutletHTFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgrDcOutletHTFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHGR_DC_OUTLET_HT_FST_ALARM_REL, msg->byte_max);//config_get(kChgrDcOutletHTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHGR_DC_OUTLET_HT_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgrDcOutletHTFstAlarmRelDlyIndex)
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgrDcOutletHTSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgrDcOutletHTSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHGR_DC_OUTLET_HT_SND_ALARM_REL, msg->byte_max);//config_get(kChgrDcOutletHTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHGR_DC_OUTLET_HT_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgrDcOutletHTSndAlarmRelDlyIndex)
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_OUTLET_HT_TRD_ALARM_DEF, msg->byte_max);//config_get(kChgrDcOutletHTTrdAlarmIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_OUTLET_HT_TRD_ALARM_DLY_DEF, msg->byte_max);//config_get(kChgrDcOutletHTTrdAlarmDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_OUTLET_HT_TRD_ALARM_REL_DEF, msg->byte_max);//config_get(kChgrDcOutletHTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_OUTLET_HT_TRD_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kChgrDcOutletHTTrdAlarmRelDlyIndex)
    
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgrOutletTempLineFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHGR_OUTLET_TEMP_LINE_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgrOutletTempLineFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgrOutletTempLineSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHGR_OUTLET_TEMP_LINE_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgrOutletTempLineSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, TEMP_LINE_TRD_ALARM_DLY_DEF, msg->byte_max);//config_get(kChgrOutletTempLineTrdAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, TEMP_LINE_TRD_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kChgrOutletTempLineTrdAlarmRelDlyIndex)
    
    msg->byte_cnt = index;
}

void bcu_base_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_BASE_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kSlaveNumIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);//TODO:BMU监测电池数目  bmu_get_voltage_num(0)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, SLAVE_TEMPERA_NUM_DEF, msg->byte_max);//config_get(kTemperatureNumIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 0, msg->byte_max);//bcu_get_current_auto_check_flag()
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kNominalCapIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kTotalCapIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kLeftCapIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kSystemVoltageCalibration), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, ALARM_BEEP_INTERVAL_DEF, msg->byte_max);//config_get(kAlarmBeepInterval)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, ALARM_BEEP_MASK_LOW_DEF, msg->byte_max);//config_get(kAlarmBeepMaskLow)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, ALARM_BEEP_MASK_HIGH_DEF, msg->byte_max);//config_get(kAlarmBeepMaskHigh)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kBmuBalanceEnableIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCurSenTypeIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCapCalibHighAverageVoltage), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCapCalibLowAverageVoltage), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCapCalibLowSoc), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCapCalibLowSocDly), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kReqBmuMessageIntervalIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kBmuResponseTimeoutIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCurZeroFilterIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_CURRENT_SAMPLE_PERIOD, msg->byte_max);//config_get(kCurSampleintervalIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_CURRENT_SAMPLE_COUNT, msg->byte_max);//config_get(kCurSampleCntIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kInsulationType), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, BATTERY_TOTAL_NUM_DEF, msg->byte_max);//config_get(kBatteryTotalNum)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kNominalTotalVolt), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHARGE_EFFICIENCY_FACTOR_DEF, msg->byte_max);//config_get(kChargeEfficiencyFactor)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DISCHARGE_EFFICIENCY_FACTOR_DEF, msg->byte_max);//config_get(kDisChargeEfficiencyFactor)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kBmuMaxVoltageNumIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kBmuMaxTempNumIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kCAN1BpsIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kCAN2BpsIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kCAN3BpsIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, RS485_DTU_BPS_DEF, msg->byte_max);//config_get(kRS4851BpsIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, RS485_SLAVE_BPS_DEF, msg->byte_max);//config_get(kRS4852BpsIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kBCUCommModeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kGBChargerConnectModeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kSlaveCanChannelIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChargerCanChannelIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kEmsCanChannelIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, HMI_DEV_NAME, msg->byte_max);//config_get(kHmiRS485ChannelIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DTU_DEV_NAME, msg->byte_max);//config_get(kDtuRS485ChannelIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kCurSenRangeTypeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kCAN5BpsIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DTU_CAN_DEV, msg->byte_max);//config_get(kDtuCanChannelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kSysStatusSaveIntervalIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgTotalTimeMaxIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BMS_VIN_LEN, msg->byte_max); //VIN码长度
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kVIN1Index), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kVIN2Index), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kVIN3Index), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kVIN4Index), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kVIN5Index), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kVIN6Index), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kVIN7Index), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kVIN8Index), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kVIN9Index), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kWakeupIntervalMinutes), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kWorkMaxTimeAfterWakeupSeconds), msg->byte_max);    
    
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCurSenType1Index), msg->byte_max);
    
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgTotalCapLowIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgTotalCapHighIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgTotalCapLowIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgTotalCapHighIndex), msg->byte_max);
    
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kSOC0DischgFactorIndex), msg->byte_max);
      
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);   //UD_SOC  预留BC52A
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);   //S文件记录开关  预留BC52A
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);   //S文件UD信息记录开关  预留BC52A
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);   //启动UDSOC矫正开关  预留BC52A
    
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kCurrentDivideResIndex), msg->byte_max); //分流器电阻值 0.1uΩ
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kCanWkupEnableIndex), msg->byte_max); //CAN唤醒使能
      
    msg->byte_cnt = index;
}

void bcu_battery_coding_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_BATTERY_CODING_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kcompany_1), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kcompany_2_with_producttype), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(k_batterytype_with_standard1), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kstandard2_with_retrace_info1), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kretrace_info2), msg->byte_max);
    
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kretrace_info3), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kretrace_info4), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kproduct_date1), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kproduct_date2_with_battery_SN1), msg->byte_max);
    
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kbattery_SN2), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kbattery_SN3), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kbattery_SN4), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kladder_utilization), msg->byte_max);
        
        
    msg->byte_cnt = index;
}

INT8U bcu_common_alarm_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSystemVoltHVIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSystemVoltHVDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSystemVoltHVReleaseIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSystemVoltHVReleaseDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSystemVoltLVIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSystemVoltLVDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSystemVoltLVReleaseIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSystemVoltLVReleaseDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kCommFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kCommFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kCommSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kCommSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kCommTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kCommTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));

    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

void bcu_common_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_COMMON_ALARM_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_SYS_VOLT_HIGH_DEF, msg->byte_max);//config_get(kSystemVoltHVIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_SYS_VOLT_HIGH_DLY_DEF, msg->byte_max);//config_get(kSystemVoltHVDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_SYS_VOLT_HIGH_DEF, msg->byte_max);//config_get(kSystemVoltHVReleaseIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_SYS_VOLT_HIGH_DLY_DEF, msg->byte_max);//config_get(kSystemVoltHVReleaseDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_SYS_VOLT_LOW_DEF, msg->byte_max);//config_get(kSystemVoltLVIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_SYS_VOLT_LOW_DLY_DEF, msg->byte_max);//config_get(kSystemVoltLVDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_SYS_VOLT_LOW_DEF, msg->byte_max);//config_get(kSystemVoltLVReleaseIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_SYS_VOLT_LOW_DLY_DEF, msg->byte_max);//config_get(kSystemVoltLVReleaseDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCommFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, COMM_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kCommFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, COMM_SND_ALARM_DLY_DEF, msg->byte_max);//config_get(kCommSndAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, COMM_SND_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kCommSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, COMM_TRD_ALARM_DLY_DEF, msg->byte_max);//config_get(kCommTrdAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, COMM_TRD_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kCommTrdAlarmRelDlyIndex)

    msg->byte_cnt = index;
}
INT8U bcu_volt_alarm_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index+1 < msg->byte_cnt)config_save(kDChgHTVFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHTVFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHTVFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHTVFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHTVSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHTVSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHTVSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHTVSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLTVFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLTVFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLTVFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLTVFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLTVSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLTVSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLTVSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLTVSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHVFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHVFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHVFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHVFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHVSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHVSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHVSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHVSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLVFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLVFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLVFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLVFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLVSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLVSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLVSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLVSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgDVFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgDVFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgDVFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgDVFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgDVSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgDVSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgDVSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgDVSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kVLineFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kVLineFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHTVTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHTVTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHTVTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHTVTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLTVTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLTVTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLTVTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLTVTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHVTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHVTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHVTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHVTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLVTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLVTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLVTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLVTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgDVTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgDVTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgDVTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgDVTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kVoltLineSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kVoltLineSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kVoltLineTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kVoltLineTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}
void bcu_volt_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_VOLT_ALARM_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHTVFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHTVFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HTV_FST_ALARM_REL, msg->byte_max);//config_get(kDChgHTVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HTV_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgHTVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHTVSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHTVSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HTV_SND_ALARM_REL, msg->byte_max);//config_get(kDChgHTVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HTV_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgHTVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLTVFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLTVFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LTV_FST_ALARM_REL, msg->byte_max);//config_get(kDChgLTVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LTV_FST_ALARN_REL_DLY, msg->byte_max);//config_get(kDChgLTVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLTVSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLTVSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LTV_SND_ALARM_REL, msg->byte_max);//config_get(kDChgLTVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LTV_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgLTVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHVFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHVFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HV_FST_ALARM_REL, msg->byte_max);//config_get(kDChgHVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HV_FST_AlARM_REL_DLY, msg->byte_max);//config_get(kDChgHVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHVSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHVSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HV_SND_AlARM_REL, msg->byte_max);//config_get(kDChgHVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HV_SND_AlARM_REL_DLY, msg->byte_max);//config_get(kDChgHVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLVFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLVFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LV_FST_ALARM_REL, msg->byte_max);//config_get(kDChgLVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LV_FST_AlARM_REL_DLY, msg->byte_max);//config_get(kDChgLVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLVSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLVSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LV_SND_ALARM_REL, msg->byte_max);//config_get(kDChgLVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LV_SND_AlARM_REL_DLY, msg->byte_max);//config_get(kDChgLVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgDVFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgDVFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_DV_FST_ALARM_REL, msg->byte_max);//config_get(kDChgDVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_DV_FST_AlARM_REL_DLY, msg->byte_max);//config_get(kDChgDVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgDVSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgDVSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_DV_SND_AlARM_REL, msg->byte_max);//config_get(kDChgDVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_DV_SND_AlARM_REL_DLY, msg->byte_max);//config_get(kDChgDVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kVLineFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, VLINE_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kVLineFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHTVTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHTVTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HTV_TRD_ALARM_REL, msg->byte_max);//config_get(kDChgHTVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HTV_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgHTVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLTVTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLTVTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LTV_TRD_ALARM_REL, msg->byte_max);//config_get(kDChgLTVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LTV_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgLTVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHVTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHVTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HV_TRD_ALARM_REL, msg->byte_max);//config_get(kDChgHVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HV_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgHVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLVTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLVTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LV_TRD_ALARM_REL, msg->byte_max);//config_get(kDChgLVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LV_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgLVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgDVTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgDVTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_DV_TRD_ALARM_REL, msg->byte_max);//config_get(kDChgDVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_DV_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgDVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, VOLT_LINE_SND_ALARM_DLY_DEF, msg->byte_max);//config_get(kVoltLineSndAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, VOLT_LINE_SND_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kVoltLineSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, VOLT_LINE_TRD_ALARM_DLY_DEF, msg->byte_max);//config_get(kVoltLineTrdAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, VOLT_LINE_TRD_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kVoltLineTrdAlarmRelDlyIndex)
    
    msg->byte_cnt = index;
}
INT8U bcu_current_alarm_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index+1 < msg->byte_cnt)config_save(kChgOCFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgOCFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgOCFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgOCFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgOCSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgOCSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgOCSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgOCSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgOCFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgOCFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgOCFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgOCFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgOCSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgOCSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgOCSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgOCSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgOCTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgOCTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgOCTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgOCTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgOCTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgOCTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgOCTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgOCTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCurSensorReverseIndex, READ_LT_INT8U(msg->data, index));

    if(index < msg->byte_cnt)config_save(kCurrentCalibrationChgkIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCurrentCalibrationChgbIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCurrentCalibrationDchgkIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCurrentCalibrationDchgbIndex, READ_LT_INT16U(msg->data, index));
    

    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}
void bcu_current_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_CURRENT_ALARM_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgOCFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgOCFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_OC_FST_ALARM_REL, msg->byte_max);//config_get(kChgOCFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_OC_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgOCFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgOCSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgOCSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_OC_SND_ALARM_REL, msg->byte_max);//config_get(kChgOCSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_OC_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgOCSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgOCFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgOCFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_OC_FST_ALARM_REL, msg->byte_max);//config_get(kDChgOCFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_OC_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgOCFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgOCSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgOCSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_OC_SND_ALARM_REL, msg->byte_max);//config_get(kDChgOCSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_OC_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgOCSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgOCTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgOCTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_OC_TRD_ALARM_REL, msg->byte_max);//config_get(kChgOCTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_OC_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kChgOCTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgOCTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgOCTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_OC_TRD_ALARM_REL, msg->byte_max);//config_get(kDChgOCTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_OC_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgOCTrdAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kCurSensorReverseIndex), msg->byte_max);

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCurrentCalibrationChgkIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCurrentCalibrationChgbIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCurrentCalibrationDchgkIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCurrentCalibrationDchgbIndex), msg->byte_max);

    msg->byte_cnt = index;
}
INT8U bcu_temperature_alarm_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;
    INT16U skip;

    config_set_save_err_flag(0);
    if(index < msg->byte_cnt)config_save(kDChgHTFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHTFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDChgHTFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHTFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kDChgHTSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHTSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDChgHTSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHTSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kDChgLTFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLTFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDChgLTFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLTFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kDChgLTSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLTSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDChgLTSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLTSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kDChgDTFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgDTFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDChgDTFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgDTFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kDChgDTSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgDTSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDChgDTSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgDTSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
#if !REMOVE_NO_USED_IDX_TO_SAVE_EEPROM
    if(index < msg->byte_cnt)config_save(kRiseTempFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kRiseTempFstAlarmTimeIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kRiseTempFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kRiseTempFstAlarmRelTimeIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kRiseTempSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kRiseTempSndAlarmTimeIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kRiseTempSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kRiseTempSndAlarmRelTimeIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kFallTempFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kFallTempFstAlarmTimeIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kFallTempFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kFallTempFstAlarmTimeRelIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kFallTempSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kFallTempSndAlarmTimeIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kFallTempSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kFallTempSndAlarmTimeRelIndex, READ_LT_INT16U(msg->data, index));
#else
    if(index < msg->byte_cnt) skip = READ_LT_INT8U(msg->data, index);
    if(index+1 < msg->byte_cnt)skip = READ_LT_INT16U(msg->data, index);
    if(index < msg->byte_cnt)skip = READ_LT_INT8U(msg->data, index) ;
    if(index+1 < msg->byte_cnt)skip = READ_LT_INT16U(msg->data, index);
    if(index < msg->byte_cnt)skip = READ_LT_INT8U(msg->data, index);
    if(index+1 < msg->byte_cnt)skip = READ_LT_INT16U(msg->data, index);
    if(index < msg->byte_cnt)skip = READ_LT_INT8U(msg->data, index);
    if(index+1 < msg->byte_cnt)skip = READ_LT_INT16U(msg->data, index);
    if(index < msg->byte_cnt)skip = READ_LT_INT8U(msg->data, index);
    if(index+1 < msg->byte_cnt)skip = READ_LT_INT16U(msg->data, index);
    if(index < msg->byte_cnt)skip = READ_LT_INT8U(msg->data, index);
    if(index+1 < msg->byte_cnt)skip = READ_LT_INT16U(msg->data, index);
    if(index < msg->byte_cnt)skip = READ_LT_INT8U(msg->data, index);
    if(index+1 < msg->byte_cnt)skip = READ_LT_INT16U(msg->data, index);
    if(index < msg->byte_cnt)skip = READ_LT_INT8U(msg->data, index);
    if(index+1 < msg->byte_cnt)skip = READ_LT_INT16U(msg->data, index);
#endif
    if(index+1 < msg->byte_cnt)config_save(kTLineFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kTLineFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgHTFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHTFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgHTFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgHTSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHTSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgHTSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kDChgHTTrdAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgHTTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDChgHTTrdAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgHTTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kDChgLTTrdAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgLTTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDChgLTTrdAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgLTTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kDChgDTTrdAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDChgDTTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kDChgDTTrdAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kDChgDTTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kTempLineSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kTempLineSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kTempLineTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kTempLineTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;    
}
void bcu_temperature_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_TEMPERATURE_ALARM_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgHTFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHTFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DCHG_HT_FST_ALARM_REL, msg->byte_max);//config_get(kDChgHTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HT_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgHTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgHTSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHTSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DCHG_HT_SND_ALARM_REL, msg->byte_max);//config_get(kDChgHTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HT_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgHTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgLTFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLTFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DCHG_LT_FST_ALARM_REL, msg->byte_max);//config_get(kDChgLTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LT_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgLTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgLTSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLTSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DCHG_LT_SND_ALARM_REL, msg->byte_max);//config_get(kDChgLTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LT_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgLTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgDTFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgDTFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DCHG_DT_FST_ALARM_REL, msg->byte_max);//config_get(kDChgDTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index,DCHG_DT_FST_ALARM_REL_DLY, msg->byte_max); //config_get(kDChgDTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgDTSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgDTSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DCHG_DT_SND_ALARM_REL, msg->byte_max);//config_get(kDChgDTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_DT_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgDTSndAlarmRelDlyIndex)
#if !REMOVE_NO_USED_IDX_TO_SAVE_EEPROM
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kRiseTempFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kRiseTempFstAlarmTimeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kRiseTempFstAlarmRelIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kRiseTempFstAlarmRelTimeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kRiseTempSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kRiseTempSndAlarmTimeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kRiseTempSndAlarmRelIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kRiseTempSndAlarmRelTimeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kFallTempFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kFallTempFstAlarmTimeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kFallTempFstAlarmRelIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kFallTempFstAlarmTimeRelIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kFallTempSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kFallTempSndAlarmTimeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kFallTempSndAlarmRelIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kFallTempSndAlarmTimeRelIndex), msg->byte_max);
#else
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
#endif
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kTLineFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, TLINE_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kTLineFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgHTFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_HT_FST_ALARM_REL, msg->byte_max);//config_get(kChgHTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HT_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgHTSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_HT_SND_ALARM_REL, msg->byte_max);//config_get(kChgHTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HT_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgHTTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgHTTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DCHG_HT_TRD_ALARM_REL, msg->byte_max);//config_get(kDChgHTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_HT_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgHTTrdAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgLTTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgLTTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DCHG_LT_TRD_ALARM_REL, msg->byte_max);//config_get(kDChgLTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_LT_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgLTTrdAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgDTTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDChgDTTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DCHG_DT_TRD_ALARM_REL, msg->byte_max);//config_get(kDChgDTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, DCHG_DT_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kDChgDTTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, TEMP_LINE_SND_ALARM_DLY_DEF, msg->byte_max);//config_get(kTempLineSndAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, TEMP_LINE_SND_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kTempLineSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, TEMP_LINE_TRD_ALARM_DLY_DEF, msg->byte_max);//config_get(kTempLineTrdAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, TEMP_LINE_TRD_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kTempLineTrdAlarmRelDlyIndex)
    
    msg->byte_cnt = index;
}
INT8U bcu_soc_alarm_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index+1 < msg->byte_cnt)config_save(kLSOCSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kLSOCSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kLSOCSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kLSOCSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kLSOCFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kLSOCFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kLSOCFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kLSOCFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kHSOCSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kHSOCSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kHSOCSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kHSOCSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kHSOCFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kHSOCFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kHSOCFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kHSOCFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kSOCLowTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kSOCLowTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSOCLowTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSOCLowTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSOCHighTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSOCHighTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSOCHighTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kSOCHighTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}
void bcu_soc_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_SOC_ALARM_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kLSOCSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kLSOCSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, LSOC_SND_ALARM_REL, msg->byte_max);//config_get(kLSOCSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, LSOC_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kLSOCSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kLSOCFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kLSOCFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, LSOC_FST_ALARM_REL, msg->byte_max);//config_get(kLSOCFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, LSOC_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kLSOCFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_HIGH_SND_ALARM_DEF, msg->byte_max);//config_get(kHSOCSndAlarmIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_HIGH_SND_ALARM_DLY_DEF, msg->byte_max);//config_get(kHSOCSndAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_HIGH_SND_ALARM_REL_DEF, msg->byte_max);//config_get(kHSOCSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_HIGH_SND_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kHSOCSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kHSOCFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kHSOCFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, HSOC_FST_ALARM_REL, msg->byte_max);//config_get(kHSOCFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, HSOC_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kHSOCFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kSOCLowTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kSOCLowTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_LOW_TRD_ALARM_REL, msg->byte_max);//config_get(kSOCLowTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_LOW_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kSOCLowTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_HIGH_TRD_ALARM_DEF, msg->byte_max);//config_get(kSOCHighTrdAlarmIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_HIGH_TRD_ALARM_DLY_DEF, msg->byte_max);//config_get(kSOCHighTrdAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_HIGH_TRD_ALARM_REL_DEF, msg->byte_max);//config_get(kSOCHighTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, SOC_HIGH_TRD_ALARM_REL_DLY_DEF, msg->byte_max);//config_get(kSOCHighTrdAlarmRelDlyIndex)
    
    msg->byte_cnt = index;
}
INT8U bcu_insu_alarm_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index+1 < msg->byte_cnt)config_save(kInsuFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kInsuFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kInsuFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kInsuFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));

    if(index+1 < msg->byte_cnt)config_save(kInsuSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kInsuSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kInsuSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kInsuSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));

    if(index+1 < msg->byte_cnt)config_save(kInsuTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kInsuTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kInsuTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kInsuTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));

    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

INT8U bcu_insu2_alarm_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index < msg->byte_cnt)config_save(kEepromInsuResCalibFlagIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kEepromTvRangeIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kEepromMosOnDelayIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kEepromNormalCheckCycleIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kEepromInsuResSwitchInChgIndex, READ_LT_INT8U(msg->data, index));

    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

INT8U bcu_relay_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index+1 < msg->byte_cnt)config_save(kChgRelayOnDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgRelayOffDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDisChgRelayOnDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kDisChgRelayOffDlyIndex, READ_LT_INT16U(msg->data, index));

    if(index+1 < msg->byte_cnt)config_save(kPreChgRelayOnDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kPreChgRelayOffDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kReserveRelayOnDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kReserveRelayOffDlyIndex, READ_LT_INT16U(msg->data, index));

    if(index < msg->byte_cnt)config_save(kRelayDischargeSamePort, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kRelayChargeMutex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kRelayPreCharge, READ_LT_INT8U(msg->data, index));

    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

INT8U bcu_heat_cool_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index < msg->byte_cnt)config_save(kHeatOnTemperature, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kHeatOffTemperature, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCoolOnTemperature, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kCoolOffTemperature, READ_LT_INT8U(msg->data, index));

    index+=2;//if(index+1 < msg->byte_cnt)config_save(kHeatOnTempDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kHeatOffTempDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kCoolOnTempDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kCoolOffTempDlyIndex, READ_LT_INT16U(msg->data, index));

    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

INT8U bcu_charger_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index < msg->byte_cnt)config_save(kChargerProtocolIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChargerVoltIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChargerCurIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)READ_LT_INT16U(msg->data, index);
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgFullChgTVReleaseIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgCutOffCurIndex, READ_LT_INT16U(msg->data, index));

    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

INT8U bcu_chg_volt_alarm_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index+1 < msg->byte_cnt)config_save(kChgHTVFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHTVFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTVFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTVFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHTVSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHTVSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTVSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTVSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHTVTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHTVTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTVTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTVTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLTVFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLTVFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLTVFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLTVFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLTVSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLTVSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLTVSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLTVSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLTVTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLTVTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLTVTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLTVTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHVFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHVFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHVFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHVFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHVSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHVSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHVSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHVSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHVTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHVTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHVTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHVTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLVFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLVFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLVFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLVFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLVSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLVSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLVSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLVSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLVTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLVTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLVTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLVTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgDVFstAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgDVFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgDVFstAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgDVFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgDVSndAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgDVSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgDVSndAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgDVSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgDVTrdAlarmIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgDVTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgDVTrdAlarmRelIndex, READ_LT_INT16U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgDVTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

INT8U bcu_chg_temp_alarm_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index < msg->byte_cnt)config_save(kChgHTFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHTFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgHTFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgHTSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHTSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgHTSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgHTTrdAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgHTTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgHTTrdAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgHTTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgLTFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLTFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgLTFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLTFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgLTSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLTSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgLTSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLTSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgLTTrdAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgLTTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgLTTrdAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgLTTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgDTFstAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgDTFstAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgDTFstAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgDTFstAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgDTSndAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgDTSndAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgDTSndAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgDTSndAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kChgDTTrdAlarmIndex, READ_LT_INT8U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kChgDTTrdAlarmDlyIndex, READ_LT_INT16U(msg->data, index));
    index+=1;//if(index < msg->byte_cnt)config_save(kChgDTTrdAlarmRelIndex, READ_LT_INT8U(msg->data, index));
    index+=2;//if(index+1 < msg->byte_cnt)config_save(kChgDTTrdAlarmRelDlyIndex, READ_LT_INT16U(msg->data, index));
    
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}


INT8U bcu_mlock_config_update(J1939RecMessage* _PAGED msg)
{//DevIdx + type + len + param[N] + status + ctrl
    INT8U index = 1, res = 0;
    INT8U mlock_type        = 0;
    INT8U mlock_param_len   = 0;
    INT8U mlock_status      = 0;
    INT8U mlock_ctrl        = 0;
    config_set_save_err_flag(0);
    
    if(index < msg->byte_cnt) mlock_type = READ_LT_INT8U(msg->data, index);
    if(mlock_type != 0xFF)//0xFF: no change, 00: None, 01: busbar, 02:Amphenol
    {
        if(mlock_type != config_get(kMlockType))
        {
            config_save(kMlockType, mlock_type);
            //mlock_init();
        }
        if(mlock_type == 0)
        {
            return RESPONSE_ACK;
        }
    }
    
    if(index < msg->byte_cnt) mlock_status = READ_LT_INT8U(msg->data, index);//skip, 0:unlock, 1:lock, FF: unknown
    
    if(index < msg->byte_cnt)  //g_ChgLock_SetState=READ_LT_INT8U(msg->data, index);
    {
        mlock_ctrl=READ_LT_INT8U(msg->data, index);
        chglock_set_setstate(mlock_ctrl);//g_ChgLock_SetState= mlock_ctrl; 
    } 
    /*{   
        g_ChgLock_SetState   = READ_LT_INT8U(msg->data, index);// 00: normal, 01:unlock, 02:lock
        if(mlock_ctrl == 0x01)
        {
            mlock_unlock();
        }
        else if(mlock_ctrl == 0x02)
        {
            mlock_lock();
        }
    }*/ 
    
    if(index < msg->byte_cnt) 
    {
        mlock_param_len = READ_LT_INT8U(msg->data, index);
        config_save(kMlockParamLength, mlock_param_len);
        if(index+1 < msg->byte_cnt)config_save(kMlockParam1, READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)config_save(kMlockParam2, READ_LT_INT16U(msg->data, index));
        if(index   < msg->byte_cnt)config_save(kMlockParam3, READ_LT_INT8U(msg->data, index));
        /*
        if(mlock_type == 0x02)// 02:Amphenol
        {
            if(index+1 < msg->byte_cnt)config_save(kMlockParam1, READ_LT_INT16U(msg->data, index));
            if(index+1 < msg->byte_cnt)config_save(kMlockParam2, READ_LT_INT16U(msg->data, index));
            if(index   < msg->byte_cnt)config_save(kMlockParam3, READ_LT_INT8U(msg->data, index));
        }
        else if(mlock_type == 0x01)// 01: busbar
        {
            if(index+1 < msg->byte_cnt)config_save(kMlockParam1, READ_LT_INT16U(msg->data, index));
            if(index+1 < msg->byte_cnt)config_save(kMlockParam2, READ_LT_INT16U(msg->data, index));
            if(index   < msg->byte_cnt)config_save(kMlockParam3, READ_LT_INT8U(msg->data, index));
        } */
    }
    mlock_init();   /////////////////////////////////////////////////////////////////////////////
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

void bms_mlock_info_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0, status;
    INT8U mlock_type = 0;
    INT8U mlock_param_len = 0;
    ChgLock_StateType mlock_status;
    
    FILL_MSG_HEADER(BCU_MLOCK_CONTROL_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    mlock_type      = (INT8U)config_get(kMlockType);
    if(mlock_type == 0)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, mlock_type,msg->byte_max);
        msg->byte_cnt = 2;
        return;
    }
    mlock_param_len = (INT8U)config_get(kMlockParamLength);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, mlock_type,     msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, chglock_get_logic_state(),  msg->byte_max);//mlock_status0 - 解锁, 1 - 锁止, FF-未知
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0x00,           msg->byte_max); //00: normal, 01:force unlock, 02:force lock
    
    if((mlock_type == 0x02)||(mlock_type == 0x01)||(mlock_type == 0x03))//01: 巴斯巴 02:Amphenol
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 5,    msg->byte_max);
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kMlockParam1), msg->byte_max);
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kMlockParam2), msg->byte_max);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kMlockParam3), msg->byte_max);
    }
    else
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0,    msg->byte_max);
    }

    msg->byte_cnt = index;
}

INT8U bmu_base_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index < msg->byte_cnt)READ_LT_INT8U(msg->data, index);//read byu slave num
    if(index < msg->byte_cnt)READ_LT_INT8U(msg->data, index);
    if(index+1 < msg->byte_cnt)config_save(kBYUVoltCorrectIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)READ_LT_INT8U(msg->data, index);
    if(index+1 < msg->byte_cnt)READ_LT_INT16U(msg->data, index);
    if(index < msg->byte_cnt)READ_LT_INT8U(msg->data, index);
    if(index < msg->byte_cnt)READ_LT_INT8U(msg->data, index);
        
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

INT8U bmu_balance_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, res;

    config_set_save_err_flag(0);
    if(index+1 < msg->byte_cnt)config_save(kBYUBalDiffVoltMaxIndex, READ_LT_INT16U(msg->data, index));//read byu slave num
    if(index+1 < msg->byte_cnt)config_save(kBYUBalStartVoltIndex, READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)config_save(kBYUBalDiffVoltMinIndex, READ_LT_INT16U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kBYUBalNumIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)READ_LT_INT8U(msg->data, index);
    if(index < msg->byte_cnt)config_save(kBYUTotalBalEnIndex, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)READ_LT_INT8U(msg->data, index);
    if(index < msg->byte_cnt)config_save(kBYUBalTypeIndex, READ_LT_INT8U(msg->data, index));
   
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

INT8U bmu_5160_config_update(J1939RecMessage* _PAGED msg)
{
    INT8U i, index = 1, res;
    INT8U max_num = 0;
    INT8U tmp_8u = 0;
    INT16U tmp_16u = 0;
    config_set_save_err_flag(0);
    max_num = READ_LT_INT8U(msg->data, index);
    if(max_num > PARAM_BSU_NUM_MAX) max_num = PARAM_BSU_NUM_MAX;
    for(i=0; i< max_num; i++)
    {
        if(index < msg->byte_cnt) 
        {
            tmp_8u = READ_LT_INT8U(msg->data, index);
            if(tmp_8u>LTC6804_MAX_CELL_NUM)
              tmp_8u=LTC6804_MAX_CELL_NUM;
            if((tmp_8u < LTC6804_HALF_CELL_NUM)&&(tmp_8u != 0))
              tmp_8u =  LTC6804_HALF_CELL_NUM;
            tmp_16u = config_get(kBYUVoltTempCnt1Index+ i);
            tmp_16u = (tmp_16u&0x00FF);
            tmp_16u +=(((INT16U)tmp_8u)<<8);
            config_save(kBYUVoltTempCnt1Index + i, tmp_16u);
        }
    }
    
    max_num = READ_LT_INT8U(msg->data, index);
    if(max_num > PARAM_BSU_NUM_MAX) max_num = PARAM_BSU_NUM_MAX;
    for(i=0; i< max_num; i++)
    {
        if(index < msg->byte_cnt) 
        {
            tmp_8u = READ_LT_INT8U(msg->data, index);
            if (tmp_8u > LTC6804_MAX_TEMP_NUM)
        	{
        		tmp_8u = LTC6804_MAX_TEMP_NUM;
        	}
            tmp_16u = config_get(kBYUVoltTempCnt1Index+ i);
            tmp_16u = (tmp_16u&0xFF00);
            tmp_16u +=tmp_8u;
            config_save(kBYUVoltTempCnt1Index + i, tmp_16u); 
        }
    }
    if(index < msg->byte_cnt)config_save(kBYUBalTypeIndex, READ_LT_INT8U(msg->data, index));
    max_num = READ_LT_INT8U(msg->data, index);
    if(max_num > PARAM_BSU_NUM_MAX) max_num = PARAM_BSU_NUM_MAX;
    for(i=0; i< max_num; i++)
    {
        if(index < msg->byte_cnt) 
        {
            tmp_16u = READ_LT_INT16U(msg->data, index);
            tmp_16u &= 0x0FFF;
            config_save(kBSU1VoltSampleNoUseBitIndex + i, tmp_16u); 
        }
    }
    if(index < msg->byte_cnt)config_save(kBsuRelay1CfgType, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)config_save(kBsuRelay2CfgType, READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt) 
    {
        tmp_8u = READ_LT_INT8U(msg->data, index);
        if(tmp_8u == 0)
            main_bsu_relay_force_control_cancle(1);
        else if(tmp_8u == 1)
            main_bsu_relay_force_control_off(1);
        else 
            main_bsu_relay_force_control_on(1);
    }
    if(index < msg->byte_cnt) 
    {
        tmp_8u = READ_LT_INT8U(msg->data, index);
        if(tmp_8u == 0)
            main_bsu_relay_force_control_cancle(2);
        else if(tmp_8u == 1)
            main_bsu_relay_force_control_off(2);
        else 
            main_bsu_relay_force_control_on(2);
    }
    res = config_get_save_err_flag();
    if(res != 0)
        return res;
    else
        return RESPONSE_ACK;
}

INT8U dtu_config_update(J1939RecMessage *_PAGED msg) {
    unsigned char type;
    unsigned char *_PAGED addr;
    unsigned short port;
    if (msg->byte_cnt < 5) { // data size too small
        return RESPONSE_NACK;
    }
    if (msg->data[msg->byte_cnt - 4] != 0) { // addr end char
        return RESPONSE_NACK;
    }

    type = msg->data[msg->byte_cnt - 1];
    if (type != 0 && type != 1 && type != 2) { // type error
        return RESPONSE_NACK;
    }

    port = ((unsigned short)(msg->data[msg->byte_cnt - 3])) << 8;
    port += msg->data[msg->byte_cnt - 2];
    addr = &msg->data[1];
    
    if (RES_OK != config_save_dtu_type(type)) {
        return RESPONSE_NACK;
    }

    if (RES_OK != config_save_dtu_server(addr, port)) {
        return RESPONSE_NACK;
    }

    return RESPONSE_ACK;
}

INT8U bmu_base_status_update(J1939RecMessage* _PAGED msg)
{
    INT8U addr,index;
    INT8U voltage_num, balance_group;

    addr = SLAVE_ID_TO_BUFF_NUM(msg->id.data[3]);
    if(addr>=config_get(kSlaveNumIndex))
        return RESPONSE_NACK;
    voltage_num = msg->data[1];
    balance_group = (INT8U)(voltage_num + 7) / 8;
    bmu_set_voltage_num(addr, voltage_num);
    bmu_set_heat_status(addr, msg->data[2]);
    bmu_set_cool_status(addr, msg->data[3]);
    for (index = 0; index < balance_group; index++)
    {
        if (msg->byte_cnt > (index + 4))
            bmu_set_balance_state(addr, index, msg->data[index + 4]);
        else
            bmu_set_balance_state(addr, index, 0);
    }

    return RESPONSE_ACK;
}

INT8U bmu_volt_status_update(J1939RecMessage* _PAGED msg)
{
    INT8U addr, i, longth, index;

    addr = SLAVE_ID_TO_BUFF_NUM(msg->id.data[3]);
    if(addr>=config_get(kSlaveNumIndex))
        return RESPONSE_NACK;
    longth = msg->data[1];
    bmu_set_voltage_num(addr, msg->data[1]);
    index = 2;
    for(i=0; i<longth; i++)
    {
        bmu_set_voltage_item(addr, i, READ_LT_INT16U(msg->data, index));
    }

    return RESPONSE_ACK;
}

INT8U bmu_temp_status_update(J1939RecMessage* _PAGED msg)
{
    INT8U addr, i, longth, index, value, offset;

    addr = SLAVE_ID_TO_BUFF_NUM(msg->id.data[3]);
    if(addr>=config_get(kSlaveNumIndex))
        return RESPONSE_NACK;
    longth = msg->data[1]+msg->data[2+msg->data[1]];
    bmu_set_total_temperature_num(addr, longth);

    index = 2;
    for(i=0; i<longth; i++)
    {
        value = READ_LT_INT8U(msg->data, index);
        if(i==msg->data[1]) 
        {
            continue;
        }
        
        if(bmu_temperature_is_heat_temperature(addr, i+1) == TRUE)
        {
            offset = bmu_get_temperature_num(addr) + bmu_heat_temp_pos_2_num(addr, i+1);
        }
#if BMU_CHR_OUTLET_TEMP_SUPPORT
        else if(bmu_temperature_is_chr_outlet_temperature(addr, i+1) == TRUE)
        {
            offset = bmu_get_temperature_num(addr) + bmu_get_heat_temperature_num(addr) + bmu_outlet_temp_pos_2_num(addr, i+1);
        }
#endif
        else
        {
            offset = bmu_temperature_pos_2_num(addr, i+1);
        }
        if(offset != 0xFF)
            bmu_set_temperature_item(addr, offset, value);
    }
    return RESPONSE_ACK;
}

INT8U bmu_relay_status_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, addr, relay_num = 0, flag;
    if(msg == NULL)
        RESPONSE_NACK;
    
#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN
    addr = SLAVE_ID_TO_BUFF_NUM(msg->id.data[3]);
    if(addr>=config_get(kSlaveNumIndex))
        return RESPONSE_NACK;
    relay_num = READ_LT_INT8U(msg->data, index);
    if(index < msg->byte_cnt) //1号继电器状态更新
    {
        if(relay_num)
        {
            flag = READ_LT_INT8U(msg->data, index);
            if(flag) bmu_flag_relay_state(addr, 1);
            else bmu_unflag_relay_state(addr, 1);
            
        #if BCU_CONTROL_SLAVE_COOL_RELAY_NUM == 1
            bmu_slave_cool_control_update(addr, flag);
        #else
            bmu_slave_heat_control_update(addr, flag);
        #endif
        }
    }
    if(index < msg->byte_cnt) //2号继电器状态更新
    {
        if(relay_num > 1)
        {
            flag = READ_LT_INT8U(msg->data, index);
            if(flag) bmu_flag_relay_state(addr, 2);
            else bmu_unflag_relay_state(addr, 2);
            
        #if BCU_CONTROL_SLAVE_COOL_RELAY_NUM == 2
            bmu_slave_cool_control_update(addr, flag);
        #else
            bmu_slave_heat_control_update(addr, flag);
        #endif
        }
    }
#endif
    return RESPONSE_ACK;
}

INT8U bmu_alarm_status_update(J1939RecMessage* _PAGED msg)
{
    INT8U addr, index, bal_num;

    addr = SLAVE_ID_TO_BUFF_NUM(msg->id.data[3]);
    if(addr>=config_get(kSlaveNumIndex))
        return RESPONSE_NACK;
    bmu_set_high_volt_state(addr, msg->data[1]);
    bmu_set_low_volt_state(addr, msg->data[2]);
    bmu_set_high_temp_state(addr, msg->data[3]);
    bmu_set_commabort_state(addr, msg->data[4]);
    bal_num = msg->data[5];
    bmu_set_balance_num(addr, bal_num);
    index = 6;
    if(bal_num == 0)
        bmu_set_balance_except_state(addr, 0);
    else if(bal_num == 1)
        bmu_set_balance_except_state(addr, READ_LT_INT8U(msg->data, index));
    else if(bal_num == 2)
        bmu_set_balance_except_state(addr, READ_LT_INT16U(msg->data, index));
    else if(bal_num == 3)
        bmu_set_balance_except_state(addr, READ_LT_INT24U(msg->data, index));
    else if(bal_num == 4)
        bmu_set_balance_except_state(addr, READ_LT_INT32U(msg->data, index));
    bmu_set_low_temp_state(addr, READ_LT_INT8U(msg->data, index));
    return RESPONSE_ACK;
}

void insu_base_message_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1;
    
    if(msg == NULL) return;           
    if(msg->id.data[3] != INSU_ADDRESS)return;
    if(insu_type_is_both()) 
    {
        if(index+1 < msg->byte_cnt)bcu_set_ext_positive_insulation_resistance(READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)bcu_set_ext_negative_insulation_resistance(READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)bcu_set_ext_insulation_total_voltage((INT32U)(READ_LT_INT16U(msg->data, index))*100);
        if(index < msg->byte_cnt)bcu_set_ext_insulation_work_state(READ_LT_INT8U(msg->data, index));
        if(index < msg->byte_cnt)bcu_set_ext_insulation_error(READ_LT_INT8U(msg->data, index));
        if(index < msg->byte_cnt){READ_LT_INT8U(msg->data, index);}
        if(index+1 < msg->byte_cnt)bcu_set_ext_insulation_pre_total_voltage((INT32U)(READ_LT_INT16U(msg->data, index))*100);
        if(index+1 < msg->byte_cnt)bcu_set_ext_insulation_pch_total_voltage((INT32U)(READ_LT_INT16U(msg->data, index))*100);
    }
    if(insu_type_is_can()) 
    {
    if(index+1 < msg->byte_cnt)bcu_set_positive_insulation_resistance(READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)bcu_set_negative_insulation_resistance(READ_LT_INT16U(msg->data, index));
    if(index+1 < msg->byte_cnt)bcu_set_insulation_total_voltage((INT32U)(READ_LT_INT16U(msg->data, index))*100);
    if(index < msg->byte_cnt)bcu_set_insulation_work_state(READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt)bcu_set_insulation_error(READ_LT_INT8U(msg->data, index));
    if(index < msg->byte_cnt){READ_LT_INT8U(msg->data, index);}
    if(index+1 < msg->byte_cnt)bcu_set_insulation_pre_total_voltage((INT32U)(READ_LT_INT16U(msg->data, index))*100);
    if(index+1 < msg->byte_cnt)bcu_set_insulation_pch_total_voltage((INT32U)(READ_LT_INT16U(msg->data, index))*100);
    }
    
    bcu_set_insulation_online(1);
}

void pcm_base_message_update(J1939RecMessage* _PAGED msg)
{
    INT8U index = 1, tv_num;
    
    if(msg == NULL) return;           
    if(msg->id.data[3] != PCM_ADDRESS)return;
    
    if(index+1 < msg->byte_cnt)tv_num = READ_LT_INT8U(msg->data, index);
    if(tv_num > 0)
    {
        if(index+1 < msg->byte_cnt)bcu_set_pcm_total_voltage(READ_LT_INT16U(msg->data, index));
        if(index+1 < msg->byte_cnt)bcu_set_pcm_tv_rate_of_change(READ_LT_INT16U(msg->data, index) - PCM_TV_RATE_OF_CHANGE_OFFSET_DEF);
    }
}

void bcu_insu_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_INSU_ALARM_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kInsuFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kInsuFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, INSU_FST_ALARM_REL, msg->byte_max);//config_get(kInsuFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, INSU_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kInsuFstAlarmRelDlyIndex)

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kInsuSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kInsuSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, INSU_SND_ALARM_REL, msg->byte_max);//config_get(kInsuSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, INSU_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kInsuSndAlarmRelDlyIndex)

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kInsuTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kInsuTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, INSU_TRD_ALARM_REL, msg->byte_max);//config_get(kInsuTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, INSU_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kInsuTrdAlarmRelDlyIndex)

    msg->byte_cnt = index;
}

void bcu_insu2_alarm_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_INSU2_ALARM_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = context->can_info->addr;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kEepromInsuResCalibFlagIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kEepromTvRangeIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kEepromMosOnDelayIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kEepromNormalCheckCycleIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kEepromInsuResSwitchInChgIndex), msg->byte_max);

    msg->byte_cnt = index;
}

void bcu_relay_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_RELAY_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgRelayOnDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgRelayOffDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDisChgRelayOnDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDisChgRelayOffDlyIndex), msg->byte_max);

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kPreChgRelayOnDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kPreChgRelayOffDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, RESERVE_RELAY_ON_DLY_DEF, msg->byte_max);//config_get(kReserveRelayOnDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, RESERVE_RELAY_OFF_DLY_DEF, msg->byte_max);//config_get(kReserveRelayOffDlyIndex)

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kRelayDischargeSamePort), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kRelayChargeMutex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kRelayPreCharge), msg->byte_max);

    msg->byte_cnt = index;
}

void bcu_heat_cool_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_HEAT_COOL_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kHeatOnTemperature), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kHeatOffTemperature), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kCoolOnTemperature), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kCoolOffTemperature), msg->byte_max);

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_HEAT_ON_DLY_DEF, msg->byte_max);//config_get(kHeatOnTempDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_HEAT_OFF_DLY_DEF, msg->byte_max);//config_get(kHeatOffTempDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_COOL_ON_DLY_DEF, msg->byte_max);//config_get(kCoolOnTempDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CFG_COOL_OFF_DLY_DEF, msg->byte_max);//config_get(kCoolOffTempDlyIndex)

    msg->byte_cnt = index;
}

void bcu_charger_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_CHARGER_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChargerProtocolIndex), msg->byte_max);

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChargerVoltIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChargerCurIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, BMS_FULL_CHG_RELEASE_VOLT, msg->byte_max);//config_get(kChgFullChgTVReleaseIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgCutOffCurIndex), msg->byte_max);

    msg->byte_cnt = index;
}

void bcu_chg_volt_alarm_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;           
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_CHG_VOLT_ALARM_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTVFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTVFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HTV_FST_ALARM_REL, msg->byte_max);//config_get(kChgHTVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HTV_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHTVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTVSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTVSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HTV_SND_ALARM_REL, msg->byte_max);//config_get(kChgHTVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HTV_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHTVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTVTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTVTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HTV_TRD_ALARM_REL, msg->byte_max);//config_get(kChgHTVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HTV_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHTVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLTVFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLTVFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LTV_FST_ALARM_REL, msg->byte_max);//config_get(kChgLTVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LTV_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgLTVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLTVSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLTVSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LTV_SND_ALARM_REL, msg->byte_max);//config_get(kChgLTVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LTV_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgLTVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLTVTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLTVTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LTV_TRD_ALARM_REL, msg->byte_max);//config_get(kChgLTVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LTV_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kChgLTVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHVFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHVFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HV_FST_ALARM_REL, msg->byte_max);//config_get(kChgHVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HV_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHVSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHVSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HV_SND_ALARM_REL, msg->byte_max);//config_get(kChgHVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HV_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHVTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHVTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HV_TRD_ALARM_REL, msg->byte_max);//config_get(kChgHVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HV_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLVFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLVFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LV_FST_ALARM_REL, msg->byte_max);//config_get(kChgLVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LV_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgLVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLVSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLVSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LV_SND_ALARM_REL, msg->byte_max);//config_get(kChgLVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LV_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgLVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLVTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLVTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LV_TRD_ALARM_REL, msg->byte_max);//config_get(kChgLVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LV_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kChgLVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgDVFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgDVFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_DV_FST_ALARM_REL, msg->byte_max);//config_get(kChgDVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_DV_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgDVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgDVSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgDVSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_DV_SND_ALARM_REL, msg->byte_max);//config_get(kChgDVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_DV_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgDVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgDVTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgDVTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_DV_TRD_ALARM_REL, msg->byte_max);//config_get(kChgDVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_DV_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kChgDVTrdAlarmRelDlyIndex)
    
    msg->byte_cnt = index;
}

void bcu_chg_temp_alarm_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_CHG_TEMP_ALARM_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgHTFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_HT_FST_ALARM_REL, msg->byte_max);//config_get(kChgHTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HT_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgHTSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_HT_SND_ALARM_REL, msg->byte_max);//config_get(kChgHTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HT_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgHTTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgHTTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_HT_TRD_ALARM_REL, msg->byte_max);//config_get(kChgHTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_HT_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kChgHTTrdAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgLTFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLTFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_LT_FST_ALARM_REL, msg->byte_max);//config_get(kChgLTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LT_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgLTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgLTSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLTSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_LT_SND_ALARM_REL, msg->byte_max);//config_get(kChgLTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LT_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgLTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgLTTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgLTTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_LT_TRD_ALARM_REL, msg->byte_max);//config_get(kChgLTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_LT_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kChgLTTrdAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgDTFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgDTFstAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_DT_FST_ALARM_REL, msg->byte_max);//config_get(kChgDTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_DT_FST_ALARM_REL_DLY, msg->byte_max);//config_get(kChgDTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgDTSndAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgDTSndAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_DT_SND_ALARM_REL, msg->byte_max);//config_get(kChgDTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_DT_SND_ALARM_REL_DLY, msg->byte_max);//config_get(kChgDTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgDTTrdAlarmIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kChgDTTrdAlarmDlyIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_DT_TRD_ALARM_REL, msg->byte_max);//config_get(kChgDTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, CHG_DT_TRD_ALARM_REL_DLY, msg->byte_max);//config_get(kChgDTTrdAlarmRelDlyIndex)
    
    msg->byte_cnt = index;
}

void bcu_base_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U temp,macro_temp;

    FILL_MSG_HEADER(BCU_BASE_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    //WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, OSCPUUsage * 100, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_SOC(), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_SOH(), msg->byte_max);
    //WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, get_tick_count(), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_V(bcu_get_total_voltage()), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_current(), msg->byte_max);
    temp = config_get(kTotalCapIndex) - config_get(kLeftCapIndex);
    if (temp >= 0x8000) temp = 0;
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kNominalCapIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kLeftCapIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgTotalCapLowIndex), msg->byte_max);//TODO:累计充电电量
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgTotalCapHighIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgTotalCapLowIndex), msg->byte_max);//TODO:累计放电电量
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, config_get(kDChgTotalCapHighIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_total_voltage_num(), msg->byte_max);//当前在线电池数
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_total_temperature_num(), msg->byte_max);//当前在线温感数
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_average_voltage(), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (INT8U)bcu_get_average_temperature(), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_difference_voltage(), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bcu_get_difference_temperature(), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, system_voltage_get(), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, board_temperature_get(), msg->byte_max); //板载温度1
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kBCUCommModeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bms_relay_diagnose_get_fault_num(kRelayTypeCharging), msg->byte_max); //充电继电器自诊断状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bms_relay_diagnose_get_fault_num(kRelayTypeDischarging), msg->byte_max); //放电继电器自诊断状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bcu_get_soc_full_uncalib_reason(), msg->byte_max); //SOC满自校正诊断状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bcu_get_soc_empty_uncalib_reason(), msg->byte_max); //SOC空自校正诊断状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, self_check_charger_get_communication_status(), msg->byte_max); //自检充放电通信状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, guobiao_charger_get_selfcheck_fault_num(), msg->byte_max); //国标充电机诊断状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, board_temperature2_get(), msg->byte_max); //板载温度2
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, hardware_io_revision_get(), msg->byte_max); //硬件IO版本
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, DEVICE_SW_SUPPORT_HW_VER, msg->byte_max); //软件支持最高IO版本
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bms_get_power_trigger_signal(), msg->byte_max); //软件支持最高IO版本

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_current1(), msg->byte_max);//////////////////

    msg->byte_cnt = index;
}

void bcu_statistic_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U i, index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_STATISTIC_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    
    msg->data[index++] = 0x00;
    msg->data[index++] = BMU_STAT_HIGH_VOLTAGE_NUM;//高电压个数
    for(i=0; i < BMU_STAT_HIGH_VOLTAGE_NUM; i++)
    {
        WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bmu_stat_get_high_voltage(i), msg->byte_max);//高电压
        WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 
            ((INT16U)bmu_stat_get_high_voltage_id(i) << 8) + bmu_stat_get_high_voltage_index(i), msg->byte_max);//高电压编号
    }
    msg->data[index++] = BMU_STAT_LOW_VOLTAGE_NUM;//低电压个数
    for(i=0; i < BMU_STAT_LOW_VOLTAGE_NUM; i++)
    {
        WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bmu_stat_get_low_voltage(i), msg->byte_max);//低电压
        WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 
            ((INT16U)bmu_stat_get_low_voltage_id(i) << 8) + bmu_stat_get_low_voltage_index(i), msg->byte_max);//低电压编号
    }
    msg->data[index++] = BMU_STAT_HIGH_TEMPERATURE_NUM;//高温度个数
    for(i=0; i < BMU_STAT_HIGH_TEMPERATURE_NUM; i++)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_stat_get_high_temperature(i), msg->byte_max);//高温度
        WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 
            ((INT16U)bmu_stat_get_high_temperature_id(i) << 8) + bmu_stat_get_high_temperature_index(i), msg->byte_max);//高温度编号
    }
    msg->data[index++] = BMU_STAT_LOW_TEMPERATURE_NUM;//低温度个数
    for(i=0; i < BMU_STAT_LOW_TEMPERATURE_NUM; i++)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_stat_get_low_temperature(i), msg->byte_max);//低温度
        WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, 
            ((INT16U)bmu_stat_get_low_temperature_id(i) << 8) + bmu_stat_get_low_temperature_index(i), msg->byte_max);//低温度编号
    }
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kCycleCntIndex), msg->byte_max);//电池组充放电计数
    //WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kCycleStsIndex), msg->byte_max);//电池组充放电计数辅助状态
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kDeepDisChgCntIndex), msg->byte_max);//电池组深度放电计数

    msg->byte_cnt = index;
}

#define BCU_THREE_ALARM_TO_UPPER_ALARM(ALARM)   (ALARM != 0 ? (ALARM + 9) : 0)
void bms_alarm_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_ALARM_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);

    msg->data[index++] = 0x00;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_battery_insulation_state()), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_charge_state()), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_discharge_state()), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_chg_oc_state()), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_dchg_oc_state()), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_high_temperature_state()), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_low_temperature_state()), msg->byte_max);
    if(bcu_get_low_total_volt_state() != kAlarmNone) macro_temp = BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_low_total_volt_state());
    else if(bcu_get_high_total_volt_state() != kAlarmNone) macro_temp = BCU_THREE_ALARM_TO_UPPER_ALARM((bcu_get_high_total_volt_state() + kAlarmMax - 1));
    else macro_temp = 0;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, macro_temp, msg->byte_max);//总压报警状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_delta_temperature_state()), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);//TODO:温升过快报警状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);//TODO:短路保护报警状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_slave_communication_state()), msg->byte_max); // 从机通讯异常
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_delta_voltage_state()), msg->byte_max); // 压差状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_voltage_exception_state()), msg->byte_max); // 电压排线脱落
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_temp_exception_state()||bcu_get_chgr_outlet_temp_line_state()), msg->byte_max); // 温度排线脱落
    if(bcu_get_low_soc_state() != kAlarmNone) macro_temp = BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_low_soc_state());
    else if(bcu_get_high_soc_state() != kAlarmNone) macro_temp = BCU_THREE_ALARM_TO_UPPER_ALARM((bcu_get_high_soc_state() + kAlarmMax - 1));
    else macro_temp = 0;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, macro_temp, msg->byte_max); // soc状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bcu_get_system_state(), msg->byte_max); // 系统状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bcu_get_insulation_work_state(), msg->byte_max); // 绝缘工作状态
#if BMS_SUPPORT_DATA_SAVE
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, data_save_is_sd_fault(), msg->byte_max); // SD卡异常
#else
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max); // SD卡异常
#endif
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bcu_get_io_revision_alarm(), msg->byte_max); // IO版本匹配报警

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, current_signal_is_valid(),msg->byte_max);//电流传感器是否正常  1没有连接  0正常   2超出量程
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, current1_signal_is_valid(),msg->byte_max);//第二路电流传感器是否正常  1没有连接  0正常   2超出量程
    
    WRITE_LT_INT16U_WITH_FUNC_DATA(msg->data, index, crc_check(msg->data, index));
    msg->byte_cnt = index;
}

void bms_relay_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U i, index = 0;

    FILL_MSG_HEADER(BCU_RELAY_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypePreCharging), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypeCharging), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypeDischarging), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypeChargingSignal), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypeDischargingSignal), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypeHeating), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypeCooling), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypeReserved), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);//放电继电器异常状态
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, input_signal_get_count(), msg->byte_max);
    for(i=1; i<=input_signal_get_count(); i++)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, input_signal_is_high(i), msg->byte_max);
    }
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypePositive), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypeNegative), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, relay_control_is_on(kRelayTypeRelay10), msg->byte_max);

    msg->byte_cnt = index;
}

void bms_insu_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U macro_temp;

    FILL_MSG_HEADER(BCU_INSU_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_positive_insulation_resistance(), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_negative_insulation_resistance(), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_system_insulation_resistance(), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_100MV(bcu_get_insulation_total_voltage()), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bcu_get_insulation_work_state(), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 3, msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_100MV(bcu_get_insulation_pre_total_voltage()), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_100MV(bcu_get_insulation_pch_total_voltage()), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_100MV(bcu_get_insulation_hv3_total_voltage()), msg->byte_max);
    if(insu_type_is_both())
    {
        WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_ext_positive_insulation_resistance(), msg->byte_max);
        WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_ext_negative_insulation_resistance(), msg->byte_max);
        WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, bcu_get_ext_system_insulation_resistance(), msg->byte_max);
        WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_100MV(bcu_get_ext_insulation_total_voltage()), msg->byte_max);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bcu_get_ext_insulation_work_state(), msg->byte_max);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 3, msg->byte_max);
        WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_100MV(bcu_get_ext_insulation_pre_total_voltage()), msg->byte_max);
        WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_100MV(bcu_get_ext_insulation_pch_total_voltage()), msg->byte_max);
        WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, MV_TO_100MV(bcu_get_ext_insulation_hv3_total_voltage()), msg->byte_max);
    }

    msg->byte_cnt = index;
}

void bcu_debug_info_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U temp = 0;
    INT32U macro_temp;

    FILL_MSG_HEADER(BCU_DEBUG_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;
    //WRITE_LT_INT8U(msg->data, index, OSCPUUsage);
    WRITE_LT_INT32U_WITH_FUNC_DATA(msg->data, index, bcu_get_sys_exception_flags());
    WRITE_LT_INT32U_WITH_FUNC_DATA(msg->data, index, bcu_get_running_time());
    WRITE_LT_INT32U_WITH_FUNC_DATA(msg->data, index, soc_get_total_delta_time());

    msg->byte_cnt = index;
}

void bmu_base_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U temp = 0;
    INT32U macro_temp;

    FILL_MSG_HEADER(BMU_BASE_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
  #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1   
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BYU_USE_SLAVE_INDEX + 1, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BYU_USE_SLAVE_INDEX + 1, msg->byte_max);
  #endif
  #if  BMS_SUPPORT_HARDWARE_LTC6804 == 1   
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 1, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 1, msg->byte_max);
  #endif
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_voltage_num(0), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kBYUVoltCorrectIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_temperature_num(0), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_CAN_ADDR_DEF, msg->byte_max);//config_get(kBcuCanAddrIndex)
    
    msg->byte_cnt = index;
}

void bmu_relay_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U temp = 0;

    FILL_MSG_HEADER(BMU_RELAY_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
   #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1   
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BYU_USE_SLAVE_INDEX + 1, msg->byte_max);
   #endif
  #if  BMS_SUPPORT_HARDWARE_LTC6804 == 1   
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index,  1, msg->byte_max);
  #endif 
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 2, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    
    msg->byte_cnt = index;
}

void bmu_alarm_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U temp = 0;

    FILL_MSG_HEADER(BMU_ALARM_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
   #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1  
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BYU_USE_SLAVE_INDEX + 1, msg->byte_max);
   #endif
  #if  BMS_SUPPORT_HARDWARE_LTC6804 == 1   
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index,  1, msg->byte_max);
  #endif 
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    
    msg->byte_cnt = index;
}

void bmu_balance_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT16U temp = 0;
    INT32U macro_temp;

    FILL_MSG_HEADER(BMU_BALANCE_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
  #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1   
    msg->data[index++] = BYU_USE_SLAVE_INDEX + 1;
    #endif
  #if  BMS_SUPPORT_HARDWARE_LTC6804 == 1   
    msg->data[index++] =  1;
  #endif  
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kBYUBalDiffVoltMaxIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kBYUBalStartVoltIndex), msg->byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(msg->data, index, config_get(kBYUBalDiffVoltMinIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kBYUBalNumIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChgHTFstAlarmIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kBYUTotalBalEnIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, CHG_HT_FST_ALARM_REL, msg->byte_max);//config_get(kChgHTFstAlarmRelIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kBYUBalTypeIndex), msg->byte_max);
    
    msg->byte_cnt = index;
}

void bmu_5160_config_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    INT8U i = 0;
    INT16U tmp_16u = 0;

    FILL_MSG_HEADER(BMU_5160_CONFIG_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1   
    msg->data[index++] = BYU_USE_SLAVE_INDEX + 1;
    #endif
  #if  BMS_SUPPORT_HARDWARE_LTC6804 == 1   
    msg->data[index++] =  1;
  #endif  
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, PARAM_BSU_NUM_MAX, msg->byte_max);
    for(i=0; i<PARAM_BSU_NUM_MAX; i++)
    {
        tmp_16u = config_get(kBYUVoltTempCnt1Index+i);
        tmp_16u = (tmp_16u>>8);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (INT8U)tmp_16u, msg->byte_max);
    }
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, PARAM_BSU_NUM_MAX, msg->byte_max);
    for(i=0; i<PARAM_BSU_NUM_MAX; i++)
    {
        tmp_16u = config_get(kBYUVoltTempCnt1Index+i);
        tmp_16u = tmp_16u&0x00FF;
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (INT8U)tmp_16u, msg->byte_max);
    }
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kBYUBalTypeIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, PARAM_BSU_NUM_MAX, msg->byte_max);
    for(i=0; i<PARAM_BSU_NUM_MAX; i++)
    {
        tmp_16u = config_get(kBSU1VoltSampleNoUseBitIndex + i);
        tmp_16u = tmp_16u&0x0FFF;
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, tmp_16u, msg->byte_max);
    }
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kBsuRelay1CfgType), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kBsuRelay2CfgType), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, get_main_bsu_relay_force_cmd(1), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, get_main_bsu_relay_force_cmd(2), msg->byte_max);
    msg->byte_cnt = index;
}

void bmu_base_status_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U slave_num = 0;
    INT8U index = 0;
    INT8U i = 0;
    INT16U volt_num;
    INT8U balance_current_num = 0;

    FILL_MSG_HEADER(BMU_BASE_STATUS_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);

    slave_num = rec_msg->data[3] - 1;
    if(slave_num >= BMU_MAX_SLAVE_COUNT) slave_num = 0;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, slave_num+1, msg->byte_max);
    volt_num = bmu_get_voltage_num(slave_num);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, volt_num, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_heat_status(slave_num), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_cool_status(slave_num), msg->byte_max);
    
    volt_num = BMS_BMU_BALANCE_STATE_NUM(volt_num);
    for(i=0; i<volt_num; i++)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_balance_state(slave_num, i), msg->byte_max);
    }
    if(config_get(kBYUBalTypeIndex) == BSU_ACTIVE_BALANCE_TYPE)
        balance_current_num = 1;
    else
        balance_current_num = 0;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, balance_current_num, msg->byte_max);
    for(i=0; i<balance_current_num; i++)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_balance_current_item(slave_num, i), msg->byte_max);
    }
    if(config_get(kBYUBalTypeIndex) == BSU_ACTIVE_BALANCE_TYPE)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, is_active_balance_error(slave_num), msg->byte_max);
    }
    
    msg->byte_cnt = index;
}

void bmu_volt_status_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U slave_num = 0;
    INT8U index = 0;
    INT8U i = 0, volt_num;

    FILL_MSG_HEADER(BMU_VOLT_STATUS_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);

    slave_num = rec_msg->data[3] - 1;
    if(slave_num >= BMU_MAX_SLAVE_COUNT) slave_num = 0;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, slave_num+1, msg->byte_max);
    volt_num = (INT8U)bmu_get_voltage_num(slave_num);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, volt_num, msg->byte_max);
    for(i=0; i<volt_num; i++)
    {
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, bmu_get_voltage_item(slave_num, i), msg->byte_max);
    }
    
    msg->byte_cnt = index;
}

void bmu_temperature_status_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U slave_num = 0;
    INT8U index = 0;
    INT8U i = 0, temp_num;

    FILL_MSG_HEADER(BMU_TEMP_STATUS_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);

    slave_num = rec_msg->data[3] - 1;
    if(slave_num >= BMU_MAX_SLAVE_COUNT) slave_num = 0;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, slave_num+1, msg->byte_max);
    temp_num = bmu_get_total_temperature_num(slave_num);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, temp_num, msg->byte_max);
    
    for(i=0; i<temp_num; i++)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_temperature_item(slave_num, i), msg->byte_max);
    }
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    
    msg->byte_cnt = index;
}

void bmu_relay_status_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U  slave_num = 0;
    INT8U index = 0, flag = 0;

    FILL_MSG_HEADER(BMU_RELAY_STATUS_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);

    slave_num = rec_msg->data[3] - 1;
    if(slave_num >= BMU_MAX_SLAVE_COUNT) slave_num = 0;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, slave_num+1, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 2, msg->byte_max);
    if(bmu_get_relay_state(slave_num) & 0x01) flag = 1;
    else flag = 0;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, flag, msg->byte_max);
    if(bmu_get_relay_state(slave_num) & 0x02) flag = 1;
    else flag = 0;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, flag, msg->byte_max);
    
    msg->byte_cnt = index;
}

void bmu_alarm_status_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U  slave_num = 0;
    INT8U index = 0, bal_num = 0;
    INT8U i = 0;

    FILL_MSG_HEADER(BMU_ALARM_STATUS_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);

    slave_num = rec_msg->data[3] - 1;
    if(slave_num >= BMU_MAX_SLAVE_COUNT) slave_num = 0;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, slave_num+1, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_high_volt_state(slave_num), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_low_volt_state(slave_num), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_high_temp_state(slave_num), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_commabort_state(slave_num), msg->byte_max);
    
    bal_num = bmu_get_balance_num(slave_num);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bal_num, msg->byte_max);
    if (bal_num == 1)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_balance_except_state(slave_num), msg->byte_max);
    }
    else if (bal_num == 2)
    {
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, bmu_get_balance_except_state(slave_num), msg->byte_max);
    }
    else if (bal_num == 3)
    {
        WRITE_LT_INT24U_WITH_BUFF_SIZE(msg->data, index, bmu_get_balance_except_state(slave_num), msg->byte_max);
    }
    else if (bal_num == 4)
    {
        WRITE_LT_INT32U_WITH_BUFF_SIZE(msg->data, index, bmu_get_balance_except_state(slave_num), msg->byte_max);
    }

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bmu_get_low_temp_state(slave_num), msg->byte_max);
    
    msg->byte_cnt = index;
}

void bms_charger_info_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0, status, i;
    INT16U temp = 0;
    OS_CPU_SR cpu_sr = 0;
    FILL_MSG_HEADER(BCU_CHARGER_INFO_MESSAGE_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->data[index++] = 0x00;
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, config_get(kChargerProtocolIndex), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, charger_is_charging(), msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, charger_is_connected(), msg->byte_max);
    // 电流
    temp = 0; charger_get_output_current(&temp);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);
    // 电压
    temp = 0; charger_get_output_voltage(&temp);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);
    // 已充电电量
    temp = 0; charger_get_charged_energy(&temp);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);
    // 已充电时间
    temp = 0; charger_get_charge_time_eclipse(&temp);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);
    // 剩余充电时间
    temp = 0; charger_get_charg_time_require(&temp);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);
    // 状态
    temp = charger_get_except_status_buff_num();
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);
    for(i=0; i<temp; i++)
    {
        status = charger_get_except_status_with_buff_num(i);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, status, msg->byte_max);
    }
    //CC2信号电压值
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, g_guoBiaoCC2Value, msg->byte_max);
    //pwm占空比
    OS_ENTER_CRITICAL();
    temp = g_guobiao_pwm_duty_cycle;
    OS_EXIT_CRITICAL();
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max);
    //CC2信号电压值
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, index, g_guoBiaoCCValue, msg->byte_max);
    // 慢充温度点个数
    temp = config_get(kChgrAcTempNumIndex);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (INT8U)temp, msg->byte_max);
    if(temp != 0)
    {
        // 慢充温感排线脱落故障等级 
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_chgr_ac_outlet_temp_exception_state()), msg->byte_max);
        // 慢充电口温度报警等级 
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_chgr_ac_outlet_ht_state()), msg->byte_max);
        for(i=0; i<temp; i++)
        {
            // 慢充温度点温度   
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bms_get_chgr_ac_outlet_temperature(i), msg->byte_max);
        }   
    }
    // 快充温度点个数
    temp = config_get(kChgrDcTempNumIndex);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (INT8U)temp, msg->byte_max);
    if(temp != 0)
    {
        // 快充温感排线脱落故障等级
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_chgr_dc_outlet_temp_exception_state()), msg->byte_max);
        // 快充电口温度报警等级 
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, BCU_THREE_ALARM_TO_UPPER_ALARM(bcu_get_chgr_dc_outlet_ht_state()), msg->byte_max);
        for(i=0; i<temp; i++)
        {
            // 快充温度点温度   
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, bms_get_chgr_dc_outlet_temperature(i), msg->byte_max);
        }   
    }
    msg->byte_cnt = index;
}

void bms_date_time_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U index = 0;
    DateTime tm;

    memset(&tm, 0, sizeof(tm));
    bcu_get_system_time(&tm);

    FILL_MSG_HEADER(DATE_TIME_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, tm.second, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, tm.minute, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, tm.hour, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, tm.month, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, tm.day * 4, msg->byte_max); // 参照J1939规定, 1天分4个数表示
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, (tm.year + 15), msg->byte_max); // 1939的日期按1985年表示
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0, msg->byte_max);

    msg->byte_cnt = index;
}


//static INT8U data[300];
//static INT16U data_len = 0;

void bms_rule_read_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U pri, dest_addr;
    RuleItem* _PAGED item;
    int offset = 0;
    int len;
    INT16U index;
    INT16U crc;
    INT8U cmd = rec_msg->data[1];
    Result res = RES_OK;

    // 设备编号
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, cmd, msg->byte_max);

    if (cmd == BCU_RULE_READER_READ_CMD)
    {
        index = GET_LT_INT16U(rec_msg->data, 2);
        item = rule_engine_get_rule(index);

        /*
        if (data_len >0)
        {
            safe_memcpy((PINT8U)msg->data, (PINT8U)data, data_len);
            msg->byte_cnt = data_len;
            FILL_MSG_HEADER(BCU_RULE_READ_RSP_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, config_get(kUpperComputerCanAddrIndex), context->can_info->addr);
            return;
        }
        */

        if (item)
        {
            WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max); // 结果码
            WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, item->id, msg->byte_max)
            WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, item->id, msg->byte_max);  // 规则ID
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, item->type, msg->byte_max);    // 类型
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, item->status, msg->byte_max);

            len = safe_strlen(item->name);
            WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, len + 1, msg->byte_max);
            safe_memcpy((PINT8U)msg->data + offset, (PINT8U)item->name, len);
            offset += len;
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);

            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, item->tigger.type, msg->byte_max);  // 触发器类型
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 1, msg->byte_max);  // 触发器个数
            WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, item->tigger.data.event, msg->byte_max); // 触发器时间

            len = safe_strlen(item->condition.expression);
            WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, len + 1, msg->byte_max);
            safe_memcpy((PINT8U)msg->data + offset, (PINT8U)item->condition.expression, len);
            offset += len;
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);

            len = safe_strlen(item->action.expression);
            WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, len + 1, msg->byte_max);
            safe_memcpy((PINT8U)msg->data + offset, (PINT8U)item->action.expression, len);
            offset += len;
            WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);
        }
        else
        {
            res = RES_NOT_FOUND;
        }
    }
    else if (cmd == BCU_RULE_READER_COUNT_CMD)
    {
        INT16U num = rule_engine_get_rule_num();
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, num, msg->byte_max);
    }
    else
    {
        res = RES_ERROR;
    }

    if (res != RES_OK)
    {
        offset = 0;
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, cmd, msg->byte_max);
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, res, msg->byte_max);
    }

    crc = crc_check((PINT8U)msg->data, offset);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, crc, msg->byte_max);

    /*
    if (data_len == 0 && cmd == BCU_RULE_READER_READ_CMD)
    {
        safe_memcpy((PINT8U)data, (PINT8U)msg->data, offset);
        data_len = offset;
    }
    */
    msg->byte_cnt = offset;
    FILL_MSG_HEADER(BCU_RULE_READ_RSP_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
}

void bms_rule_write_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U pri, dest_addr;
    int offset = 0;
    int read_pos = 0;
    Result res;
    INT16U len = 0;
    INT16U crc;
    INT16U data_crc;
    PCSTR name = NULL;
    PCSTR cond = NULL;
    PCSTR action = NULL;
    INT16U pack_index = 0;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, rec_msg->data[1], msg->byte_max);

    if (rec_msg->byte_cnt < 2) return;

    crc = crc_check(rec_msg->data, rec_msg->byte_cnt - 2);
    data_crc = GET_LT_INT16U(rec_msg->data, rec_msg->byte_cnt - 2);

    if (crc != data_crc)
    {
        res = RES_INVALID_CRC;
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, res, msg->byte_max);
    }
    if (rec_msg->data[1] == BCU_RULE_WRITER_OPEN_CMD)
    {
        rule_engine_stop();
        sleep(10);
        res = rule_config_writer_open();
        res = RES_OK;
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, res, msg->byte_max);
        watch_dog_disable();
        g_rule_writer_last_pack_index = 0;
    }
    else if (rec_msg->data[1] == BCU_RULE_WRITER_CLOSE_CMD)
    {
        res = rule_config_writer_close();
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, res, msg->byte_max);
        watch_dog_enable();
        g_rule_writer_last_pack_index = 0;
    }
    else if (rec_msg->data[1] == BCU_RULE_WRITER_WRITE_CMD)
    {
        safe_memset(&g_upper_computer_rule_item, 0, sizeof(g_upper_computer_rule_item));

        read_pos = 2;
        READ_DATA(pack_index, INT16U, read_pos);
        if (pack_index > g_rule_writer_last_pack_index)
        {
            READ_DATA(g_upper_computer_rule_item.id, INT16U, read_pos);
            READ_DATA(g_upper_computer_rule_item.type, INT8U, read_pos);
            READ_DATA(g_upper_computer_rule_item.status, INT8U, read_pos);
            READ_STRING(g_upper_computer_rule_item.name, read_pos);
            READ_DATA(g_upper_computer_rule_item.tigger.type, INT8U, read_pos);
            READ_DATA(len, INT8U, read_pos);
            READ_DATA(g_upper_computer_rule_item.tigger.data.event, INT16U, read_pos); // TODO:
            READ_STRING(g_upper_computer_rule_item.condition.expression, read_pos);
            g_upper_computer_rule_item.condition.type = kRuleConditionTypeExpression;
            READ_STRING(g_upper_computer_rule_item.action.expression, read_pos);
            g_upper_computer_rule_item.action.type = kRuleActionTypeExpression;

            if (g_upper_computer_rule_item.type == kRuleTypeDefault)
            {
                res = rule_config_writer_write_rule_status(g_upper_computer_rule_item.id,
                        g_upper_computer_rule_item.status);
            }
            else
            {
                res = rule_config_writer_write_rule(&g_upper_computer_rule_item);
            }
            if (res == RES_OK) g_rule_writer_last_pack_index = pack_index;
        }
        else if (pack_index == g_rule_writer_last_pack_index)
        {
            res = RES_OK;
        }
        else
        {
            res = RES_ERROR;
        }

        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, res, msg->byte_max);
    }
    else
    {
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, 0xFFFF, msg->byte_max);
    }

    msg->byte_cnt = offset < 8 ? 8 : offset;
    FILL_MSG_HEADER(BCU_COMM_RSP_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
}

void bms_expression_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U pri, dest_addr;
    INT16U offset = 0;
    INT16U read_pos = 0;
    Result res;
    INT16U len = 0;
    VmValue result;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, rec_msg->data[1], msg->byte_max);

    if (rec_msg->data[1] == BCU_RULE_VM_COMPILE_CMD)
    {
        read_pos = 2;
        len = READ_LT_INT16U(rec_msg->data, read_pos);
        if (len > 0 && *((PCSTR)rec_msg->data + read_pos + len - 1) == '\0')
        {
            res = vm_compile((PCSTR)rec_msg->data + read_pos, NULL, 0, NULL);
            //LG_ASSERT(res == RES_OK);
        }
        else
        {
            res = RES_ERROR;
        }
        read_pos += len;
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, res, msg->byte_max);
    }
    else if (rec_msg->data[1] == BCU_RULE_VM_EXECUTE_CMD)
    {
        read_pos = 2;
        len = READ_LT_INT16U(rec_msg->data, read_pos);
        if (len > 0 && *((PCSTR)rec_msg->data + read_pos + len - 1) == '\0')
        {
            res = vm_interpret((PCSTR)rec_msg->data + read_pos, &result);
        }
        else
        {
            res = RES_ERROR;
        }
        read_pos += len;
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, res, msg->byte_max);
    }
    else
    {
        WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, RES_ERROR, msg->byte_max);
    }

    msg->byte_cnt = offset;
    FILL_MSG_HEADER(BCU_COMM_RSP_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
}

//volatile INT8U test_ack = 0;
void bms_confirm_message_fill(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, INT8U ack, INT8U group, INT32U pgn, INT8U dest)
{
    INT8U index = 0;
    INT8U temp = 0;
    //if(ack != 0) 
    //{
    //   test_ack = 1;
    //}
    FILL_MSG_HEADER(MESSAGE_CONFIRM_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, dest, context->can_info->addr);

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, ack, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, group, msg->byte_max);
    if(pgn == BMS_DEVICE_COMM_CTL_PGN)
    {
        if(config_get(kDetectFlagIndex) == 0x12ab)
        {
            temp = 1;
        }
        else
        {
            temp = 0;
        }
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, temp, msg->byte_max); 
    }
    else
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0xFF, msg->byte_max);
    }
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0xFF, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, index, 0xFF, msg->byte_max);
    WRITE_LT_INT24U_WITH_BUFF_SIZE(msg->data, index, pgn, msg->byte_max);

    msg->byte_cnt = index;
}

void slave_task_run_test_rx(void* pdata);

void upper_computer_task_create(char is_test_mode)
{
    if (!is_test_mode) {
    if(mode != RUN_MODE_READ)
    {
    OSTaskCreate(slave_task_run_tx, (void *) 0, (OS_STK *) &g_slave_task_stack.stack.tx[SLAVE_TX_STK_SIZE - 1],
            SLAVECAN_TX_TASK_PRIO);
    }

    OSTaskCreate(slave_task_run_rx, (void *) 0, (OS_STK *) &g_slave_task_stack.stack.rx[SLAVE_RX_STK_SIZE - 1],
            SLAVECAN_RX_TASK_PRIO);
/*    } else {
    OSTaskCreate(slave_task_run_test_rx, (void *) 0, (OS_STK *) &g_slave_rx_task_stack[SLAVE_RX_STK_SIZE - 1],
            SLAVECAN_RX_TASK_PRIO);
*/    }
}

void bms_handle_test_info(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, J1939RecMessage* _PAGED rec_msg)
{
    INT8U pri, dest_addr;
    INT16U crc, expected_crc;
    INT16U num;
    INT16U offset = 0;

    num = GET_LT_INT16U(rec_msg->data, 2);
    if (num < 6) num = 6;

    // verify the package
    crc = crc_check((PINT8U)rec_msg->data, rec_msg->byte_cnt - 2);
    expected_crc = GET_LT_INT16U(rec_msg->data, rec_msg->byte_cnt - 2);
    if (crc != expected_crc)
    {
        bms_confirm_message_fill(context, msg, 1, 0, rec_msg->pgn, rec_msg->id.data[3]);
        return;
    }

    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, 0, msg->byte_max);
    for (; offset < num - 2; offset++)
    {
        msg->data[offset] = (INT8U)offset;
    }

    crc = crc_check((PINT8U)msg->data, offset);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(msg->data, offset, crc, msg->byte_max);

    FILL_MSG_HEADER(BMS_PACK_TEST_PGN, BCU_CAN_MESSAGE_PRI_DEFAULT, rec_msg->id.data[3], context->can_info->addr);
    msg->byte_cnt = offset;
}

/*
void bms_handle_debug_req(J1939CanContext* _PAGED context, J1939RecMessage* _PAGED receive_msg, J1939SendMessageBuff* _PAGED send_msg)
{
    INT16U offset = 1, write_offset = 0 ;
    INT8U cmd = 0;
    READ_DATA(cmd, INT8U, offset);
    if (cmd == BCU_DEBUG_CMD_REQ_INFO)
    {
        INT16U index, start_index = 0;
        INT16U len = 0;
        READ_DATA(index, INT16U, offset);
        READ_DATA(len, INT16U, offset);

        WRITE_LT_INT8U(send_msg->data, write_offset, 0);
        WRITE_LT_INT8U(send_msg->data, write_offset, cmd);
        WRITE_LT_INT16U(send_msg->data, write_offset, start_index);
        WRITE_LT_INT16U(send_msg->data, write_offset, len);
        for (index = 0; index < len; index++)
        {
            switch (index + start_index)
            {
            case BCU_DEBUG_INFO_CPU_USAGE_INDEX:
                WRITE_LT_INT16U(send_msg->data, write_offset, OSCPUUsage);
                break;
            default:
                WRITE_LT_INT16U(send_msg->data, write_offset, 0xFFFF);
                break;
            }
        }
    }

    receive_msg->byte_cnt = write_offset;
}
*/

#endif

#endif