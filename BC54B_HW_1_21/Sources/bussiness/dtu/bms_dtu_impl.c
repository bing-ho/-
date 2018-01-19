/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_dtu_impl.c
 * @brief
 * @note
 * @author
 * @date 2012-5-25
 *
 */
#include "bms_dtu_impl.h"

#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#if BMS_SUPPORT_DTU == BMS_DTU_RS485


#define DTU_PDU_INTERVAL   10
#define DTU_PDU_VERSION    0x01
#define DTU_HEAD_VERSION   2

#define DTU_WRITE_DATA(DATA, TYPE, SLAVE) DTU_WRITE_DATA_EX(DATA, TYPE, index, SLAVE)

#define DTU_WRITE_DATA_EX(DATA, TYPE, OFFSET, SLAVE) \
        if (OFFSET + sizeof(TYPE) >= DTU_DEV_FRAME_SIZE) \
        {\
            dtu_send_data(OFFSET, 0x01, SLAVE); \
            OFFSET = DTU_HEAD_LEN; sleep(DTU_PDU_INTERVAL);\
        }\
        WRITE_BT_##TYPE(g_dtu_context.send_buffer, OFFSET, (DATA)); \

OS_STK g_dtu_tx_task_stack[DTU_TX_STK_SIZE] =
{ 0 };

const char g_dtu_header[] = {'L', 'I', 'G', 'O', 'O', 0, DTU_HEAD_VERSION, 0};
#define DTU_HEAD_LEN    12

#define DTU_OTHER_SIZE  22

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU
static INT8U g_dtu_pack_id = 0;
DtuContext g_dtu_context;
#pragma DATA_SEG DEFAULT

void dtu_init(void)
{
    safe_memset(&g_dtu_context, 0, sizeof(g_dtu_context));
                                   //config_get(kDtuRS485ChannelIndex)
    //g_dtu_context.dev = rs485_init(DTU_DEV_NAME, rs485_get_bps((INT8U)RS485_DTU_BPS_DEF), NULL, NULL);//config_get(kRS4851BpsIndex + config_get(kDtuRS485ChannelIndex))
    Rs485Param rs485_param = {DTU_DEV_BANDRATE, DTU_DEV_DATAFORMAT, DTU_DEV_PARITY_ENABLE, DTU_DEV_PARITY_TYPE};
    g_dtu_context.dev = Rs485_Hardware_Init(DTU_DEV_NAME, &rs485_param, NULL, NULL);   
    safe_memcpy(g_dtu_context.send_buffer, (PINT8U)g_dtu_header, sizeof(g_dtu_header) / sizeof(char));

    dtu_task_create();
}

void dtu_uninit(void)
{
    rs485_uninit(g_dtu_context.dev);
    g_dtu_context.dev = NULL;
}

void dtu_task_create(void)
{
    OSTaskCreate(dtu_task_tx_run, (void *) NULL, (OS_STK *) &g_dtu_tx_task_stack[DTU_TX_STK_SIZE - 1],
            DTU_TX_TASK_PRIO);
}

void dtu_task_tx_run(void* pdata)
{
    for (;;)
    {
        sleep((INT32U)config_get(kDTUSendIntervalIndex) * 1000);  // second -> ms

        dtu_task_tx_bmu_message();
        dtu_task_tx_bcu_message();
    }
}

void dtu_send_data(INT8U index, INT8U control, INT8U slave_id)
{
    INT8U i;
    INIT_WRITE_BUFFFER();
    g_dtu_context.send_buffer[5] = index + 2; //数据长度
    g_dtu_context.send_buffer[6] = DTU_HEAD_VERSION;
    g_dtu_context.send_buffer[7] = g_dtu_pack_id++; //序列号
    g_dtu_context.send_buffer[8] = slave_id; // 从机地址
    g_dtu_context.send_buffer[9] = control; //控制字
    g_dtu_context.send_buffer[10] = g_dtu_context.total_frame; //总帧数
    g_dtu_context.send_buffer[11] = g_dtu_context.current_frame++; //帧地址
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, crc_check_bt((PINT8U)g_dtu_context.send_buffer, index));

    for (i = 0; i < index; i++)
    {
        rs485_send(g_dtu_context.dev, g_dtu_context.send_buffer[i]);
    }
}

void dtu_task_tx_bmu_message(void)
{
    INT8U index, slave_index, i;
    INT16U slave_num = config_get(kSlaveNumIndex);
    INT8U voltage_num, temp_num, balance_num, balance_status_num;

    for (slave_index = 0; slave_index < slave_num; slave_index++)
    {
        index = DTU_HEAD_LEN;

        voltage_num = (INT8U)bmu_get_voltage_num(slave_index);
        temp_num = bmu_get_temperature_num(slave_index);
        balance_num = BMS_BMU_BALANCE_NUM;
        balance_status_num = (INT8U)(voltage_num + 7) / 8;

        if (voltage_num * 2 + temp_num + balance_num + balance_status_num  + DTU_OTHER_SIZE > DTU_DEV_FRAME_SIZE)
        {
            g_dtu_context.total_frame = (INT8U)((voltage_num * 2 + temp_num + balance_num + balance_status_num  + DTU_OTHER_SIZE + DTU_DEV_FRAME_SIZE - 1)/ DTU_DEV_FRAME_SIZE);
            g_dtu_context.current_frame = 1;
        }
        else
        {
            g_dtu_context.total_frame = 1;
            g_dtu_context.current_frame = 1;
        }

        DTU_WRITE_DATA(DTU_PDU_VERSION, INT8U, slave_index+1);
        DTU_WRITE_DATA(voltage_num, INT8U, slave_index+1); //电池串数
        DTU_WRITE_DATA(temp_num, INT8U, slave_index+1); //温感个数
        DTU_WRITE_DATA(balance_num, INT8U, slave_index+1); //均衡电流个数
        for (i = 0; i < voltage_num; i++)
        {
            DTU_WRITE_DATA(bmu_get_voltage_item(slave_index, i), INT16U, slave_index+1);
        }

        for (i = 0; i < temp_num; i++)
        {
            DTU_WRITE_DATA(TEMP_TO_40_OFFSET(bmu_get_temperature_item(slave_index, i)), INT8U, slave_index+1);
        }
        
        temp_num = 0;
        if(bmu_get_heat_status(slave_index))temp_num |= 0x01;
        if(bmu_get_cool_status(slave_index))temp_num |= 0x02;
        DTU_WRITE_DATA(temp_num, INT8U, slave_index+1); //热管理状态

        for (i = 0; i < balance_num; i++)
        {
            DTU_WRITE_DATA(bmu_get_balance_current_item(slave_index, i), INT16U, slave_index+1);
        }

        for (i = 0; i < balance_status_num; i++)
        {
            DTU_WRITE_DATA(bmu_get_balance_state(slave_index, i), INT8U, slave_index+1);
        }

        dtu_send_data(index, 0x01, slave_index + 1);

        sleep(DTU_PDU_INTERVAL);
    }
}

//此函数如果需要两帧及以上，需调整相关参数
void dtu_task_tx_bcu_message(void)
{
    INT16U temp = 0;
    INT8U index;
    INT16U slave_num = config_get(kSlaveNumIndex);
    INIT_WRITE_BUFFFER();

    index = DTU_HEAD_LEN;
    g_dtu_context.total_frame = 1;
    g_dtu_context.current_frame = 1;
    g_dtu_context.send_buffer[index++] = DTU_PDU_VERSION; //数据版本号V1.01
    g_dtu_context.send_buffer[index++] = SOC_TO_INT8U(bcu_get_SOC()); // SOC
    temp = 0;
    if (charger_is_charging() && charger_is_connected()) temp |= 0x01;
    if (bcu_get_battery_insulation_state() >= kAlarmSecondLevel) temp |= 0x02;
    if (bcu_get_slave_communication_state() != kAlarmNone) temp |= 0x04;
    if (bcu_get_high_temperature_state() >= kAlarmSecondLevel || bcu_get_low_temperature_state() >= kAlarmSecondLevel) temp |= 0x08;
    if (bcu_get_discharge_state() >= kAlarmSecondLevel) temp |= 0x10;
    if (bcu_get_charge_state() >= kAlarmSecondLevel) temp |= 0x20;
    if (bcu_get_low_soc_state() >= kAlarmSecondLevel) temp |= 0x40;
    if (bcu_get_SOC() >= SOC_MAX_VALUE) temp |= 0x80;
    if (bmu_is_balance_state_actived() == RES_TRUE) temp |= 0x100;
    if (bcu_get_chg_oc_state() >= kAlarmSecondLevel ||
            bcu_get_dchg_oc_state() >= kAlarmSecondLevel)
            temp |= 0x200;
    if (bcu_get_delta_temperature_state() >= kAlarmSecondLevel)
            temp |= 0x400;
    if (bcu_get_delta_voltage_state() >= kAlarmSecondLevel)
            temp |= 0x800;
    if (bcu_get_voltage_exception_state() != kAlarmNone)
            temp |= 0x1000;
    if (bcu_get_temp_exception_state() != kAlarmNone)
            temp |= 0x2000;
    if (bcu_get_high_total_volt_state() >= kAlarmSecondLevel)
            temp |= 0x4000;
    else if (bcu_get_low_total_volt_state() >= kAlarmSecondLevel)
            temp |= 0x8000;
    g_dtu_context.send_buffer[index++] = 0;
    WRITE_BT_INT16U(g_dtu_context.send_buffer, index, temp);
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, config_get(kTotalCapIndex));
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, config_get(kLeftCapIndex));
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, MV_TO_V(bcu_get_total_voltage()));
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, bcu_get_current() + DTU_CURRENT_OFFSET);
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, config_get(kCycleCntIndex));
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, bcu_get_positive_insulation_resistance());
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, bcu_get_negative_insulation_resistance());
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, bcu_get_system_insulation_resistance());
    WRITE_BT_INT8U(g_dtu_context.send_buffer, index, dtu_insu_state_to_data());
    WRITE_BT_INT8U(g_dtu_context.send_buffer, index, SOH_TO_INT8U(bcu_get_SOH()));
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, bcu_get_high_voltage());
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, VOLT_4_DISPLAY(bcu_get_low_voltage()));
    WRITE_BT_INT8U(g_dtu_context.send_buffer, index, TEMP_TO_40_OFFSET((INT8U)bcu_get_high_temperature()));
    WRITE_BT_INT8U(g_dtu_context.send_buffer, index, TEMP_4_DISPLAY(TEMP_TO_40_OFFSET((INT8U)bcu_get_low_temperature())));
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, bcu_get_high_voltage_id());
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, bcu_get_low_voltage_id());
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, bcu_get_high_temperature_id());
    WRITE_BT_INT16U_WITH_FUNC_DATA(g_dtu_context.send_buffer, index, bcu_get_low_temperature_id());

    dtu_send_data(index, 0x01, 0);

    sleep(DTU_PDU_INTERVAL);
}

#elif  BMS_SUPPORT_DTU == BMS_DTU_CAN

#define DTU_CAN_START_RECEIVE_TIMEOUT_CHECK()   g_dtuContext.start_receive_flag = 1;g_dtuContext.start_receive_tick = get_tick_count();
#define DTU_CAN_STOP_RECEIVE_TIMEOUT_CHECK()    g_dtuContext.start_receive_flag = 0;

OS_STK g_dtu_can_tx_task_stack[DTU_TX_STK_SIZE] =
{ 0 };
OS_STK g_dtu_can_rx_task_stack[DTU_RX_STK_SIZE] =
{ 0 };

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU

CanMessage g_dtu_can_send_message;
CanMessage g_dtu_can_receive_message;

J1939SendMessageBuff g_dtuCanSendMessage;
unsigned char g_dtuCanSendMessageDataBuff[DTU_CAN_SENT_MESSAGE_LENGTH_MAX];
J1939RecMessage g_dtuCanRecMessage;
unsigned char g_dtuCanRecMessageDataBuff[DTU_CAN_REC_MESSAGE_LENGTH_MAX];

DtuCanContext g_dtuContext;
J1939CanContext g_dtuCanContext;
J1939Event g_dtuCanSendEventSem;
J1939Event g_dtuCanRecEventSem;
J1939CanInfo g_dtuCanInfo;
J1939CanTranPro g_dtuCanSendTpIndex;
J1939CanTPBuff g_dtuCanRecTpBuffIndex;
J1939TPChain g_dtuCanRecTpBuffs[DTU_CAN_TP_REC_MAX];
J1939CanFrameBuff g_dtuCanSendFrameBuffIndex;
J1939CanFrame g_dtuCanSendFrameBuffs[DTU_CAN_SENT_FRAME_BUFF];
J1939CanFrameBuff g_dtuCanRecPduBuffIndex;
J1939CanFrame g_dtuCanRecPduBuffs[DTU_CAN_REC_PDU_BUFF];
J1939CanFrameBuff g_dtuCanRecFrameBuffIndex;
J1939CanFrame g_dtuCanRecFrameBuffs[DTU_CAN_REC_FRAME_BUFF];
J1939SendMessageBuff g_dtuCanSendMessageBuff;
unsigned char g_dtuCanSendMessageDataBuff[DTU_CAN_SENT_MESSAGE_LENGTH_MAX];
J1939RecTempMessageBuff g_dtuCanRecTempMessageBuffIndex;
J1939RecMessage g_dtuCanRecTempMessageBuffs[DTU_CAN_TP_REC_MAX];
unsigned char g_dtuCanRecTempMessageDataBuff[DTU_CAN_TP_REC_MAX][DTU_CAN_REC_MESSAGE_LENGTH_MAX];
J1939RecMessageBuff g_dtuCanRecMessageBuffIndex;
J1939RecMessage g_dtuCanRecMessageBuffs[DTU_CAN_REC_MESSAGE_BUFF_MAX];
unsigned char g_dtuCanRecMessageDataBuffs[DTU_CAN_REC_MESSAGE_BUFF_MAX][DTU_CAN_REC_MESSAGE_LENGTH_MAX];
J1939CanRecMaskGroupItem g_dtuCanRecMaskGroupItem;

#pragma DATA_SEG DEFAULT

INT8U dtu_can_send_is_enable(void);

/**********************************************
 *
 * Implementation
 *
 ***********************************************/

INT16U dtu_can_computer_callback(can_t context, CanMessage* msg, void* userdata)
{
	INT8U i;
	J1939CanFrame frame;
	UNUSED(userdata);

    (void)context;
	frame.id.value = can_id_to_extend_id(msg->id.value);
	frame.byte_cnt = msg->len;
	for(i=0; i<msg->len; i++)
		frame.data[i] = msg->data[i];
	J1939CanRecFrame(g_dtuContext.can_cxt, &frame);
	return 1;
}

void dtu_init(void)
{
    INT8U i;
    
    g_dtuCanInfo.addr = (unsigned char)BCU_CAN_ADDR_DEF;//config_get(kBcuCanAddrIndex);
    g_dtuCanInfo.dev = (INT8U)DTU_CAN_DEV;//config_get(kDtuCanChannelIndex);
	if(can_channel_is_valid(g_dtuCanInfo.dev) == FALSE)
	{
	    g_dtuCanInfo.dev = DTU_CAN_DEV;
	    bcu_reset_all_can_channel();
	}
    if(g_dtuCanInfo.dev == kCanDev3) g_dtuCanInfo.bps = can_get_bps((INT8U)config_get(kCAN5BpsIndex));
    else g_dtuCanInfo.bps = can_get_bps((INT8U)config_get(kCAN1BpsIndex + g_dtuCanInfo.dev));
	
	g_dtuCanInfo.id_style = DTU_CAN_MODE;
	g_dtuCanInfo.mask_id.value = DTU_CAN_RECEIVE_MASK_ID;
	g_dtuCanInfo.receive_id.value = DTU_CAN_RECEIVE_ID;
    
    g_dtuCanRecMaskGroupItem.next = NULL;
    g_dtuCanRecMaskGroupItem.flag = 1;
    g_dtuCanRecMaskGroupItem.start_addr = 0xFF;
    g_dtuCanRecMaskGroupItem.stop_addr = 0xFF;
    g_dtuCanInfo.rec_mask_group_item.next = (LIST_ITEM* _PAGED)(&g_dtuCanRecMaskGroupItem);
    
    g_dtuCanSendMessage.byte_max = DTU_CAN_SENT_MESSAGE_LENGTH_MAX;
    g_dtuCanSendMessage.data = g_dtuCanSendMessageDataBuff;

    g_dtuCanRecMessage.byte_max = DTU_CAN_REC_MESSAGE_LENGTH_MAX;
    g_dtuCanRecMessage.data = g_dtuCanRecMessageDataBuff;
    if(g_dtuCanSendEventSem.event == NULL)
        g_dtuCanSendEventSem.event = OSSemCreate(0);
	//J1939 driver init
	g_dtuCanContext.send_event_sem = &g_dtuCanSendEventSem;
	if(g_dtuCanContext.send_event_sem->event == NULL)
    	g_dtuCanContext.send_event_sem->event = J1939CreateSem(1);

	g_dtuCanContext.receive_event_sem = &g_dtuCanRecEventSem;
	if(g_dtuCanContext.receive_event_sem->event == NULL)
	    g_dtuCanContext.receive_event_sem->event = J1939CreateSem(0);

	g_dtuCanContext.can_info = &g_dtuCanInfo;

	g_dtuCanContext.send_tp_index = &g_dtuCanSendTpIndex;

	g_dtuCanContext.rec_tp_buff = &g_dtuCanRecTpBuffIndex;
	g_dtuCanContext.rec_tp_buff->tp_max = DTU_CAN_TP_REC_MAX;
	g_dtuCanContext.rec_tp_buff->buffer = g_dtuCanRecTpBuffs;

	g_dtuCanContext.send_frame_buff = &g_dtuCanSendFrameBuffIndex;
	g_dtuCanContext.send_frame_buff->buffer_cnt = DTU_CAN_SENT_FRAME_BUFF;
	g_dtuCanContext.send_frame_buff->buffer = g_dtuCanSendFrameBuffs;

	g_dtuCanContext.rec_pdu_buff = &g_dtuCanRecPduBuffIndex;
	g_dtuCanContext.rec_pdu_buff->buffer_cnt = DTU_CAN_REC_PDU_BUFF;
	g_dtuCanContext.rec_pdu_buff->buffer = g_dtuCanRecPduBuffs;

	g_dtuCanContext.rec_frame_buff = &g_dtuCanRecFrameBuffIndex;
	g_dtuCanContext.rec_frame_buff->buffer_cnt = DTU_CAN_REC_FRAME_BUFF;
	g_dtuCanContext.rec_frame_buff->buffer = g_dtuCanRecFrameBuffs;

	g_dtuCanContext.send_message_buff = &g_dtuCanSendMessageBuff;
	g_dtuCanContext.send_message_buff->byte_max = DTU_CAN_SENT_MESSAGE_LENGTH_MAX;
	g_dtuCanContext.send_message_buff->data = g_dtuCanSendMessageDataBuff;

	g_dtuCanContext.rec_temp_mess_buff = &g_dtuCanRecTempMessageBuffIndex;
	g_dtuCanContext.rec_temp_mess_buff->buffer = g_dtuCanRecTempMessageBuffs;
	for(i=0; i<DTU_CAN_TP_REC_MAX; i++)
	{
	    g_dtuCanContext.rec_temp_mess_buff->buffer[i].data = g_dtuCanRecTempMessageDataBuff[i];
	    g_dtuCanContext.rec_temp_mess_buff->buffer[i].byte_max = DTU_CAN_REC_MESSAGE_LENGTH_MAX;
	}
	g_dtuCanContext.rec_temp_mess_buff->buffer_cnt = DTU_CAN_TP_REC_MAX;

	g_dtuCanContext.rec_message_buff = &g_dtuCanRecMessageBuffIndex;
	g_dtuCanContext.rec_message_buff->buffer = g_dtuCanRecMessageBuffs;
	for(i=0; i<DTU_CAN_REC_MESSAGE_BUFF_MAX; i++)
	{
	    g_dtuCanContext.rec_message_buff->buffer[i].data = g_dtuCanRecMessageDataBuffs[i];
	    g_dtuCanContext.rec_message_buff->buffer[i].byte_max = DTU_CAN_REC_MESSAGE_LENGTH_MAX;
	}

	g_dtuCanContext.rec_message_buff->buffer_cnt = DTU_CAN_TP_REC_MAX;
	g_dtuCanInfo.receive_callback = dtu_can_computer_callback;
	g_dtuContext.can_cxt = &g_dtuCanContext;
	g_dtuContext.work_start = TRUE;
    //end of J1939 driver init
	J1939InitWithBuff(g_dtuContext.can_cxt,(J1939CanInfo* _PAGED)(&g_dtuCanInfo));
    
    dtu_can_task_create();
}

void dtu_uninit(void)
{
    J1939UnInit(g_dtuContext.can_cxt);
}

void dtu_can_task_create(void)
{
    OSTaskCreate(dtu_can_task_tx_run, (void *) NULL, (OS_STK *) &g_dtu_can_tx_task_stack[DTU_TX_STK_SIZE - 1],
            DTU_TX_TASK_PRIO);
            
    OSTaskCreate(dtu_can_task_rx_run, (void *) NULL, (OS_STK *) &g_dtu_can_rx_task_stack[DTU_RX_STK_SIZE - 1],
            DTU_RX_TASK_PRIO);
}

void dtu_can_start(void)
{
    g_dtuContext.work_start = TRUE;
}

void dtu_can_stop(void)
{
    g_dtuContext.work_start = FALSE;
}

INT8U dtu_can_is_dtu_pgn(INT32U pgn)
{
    if(pgn == DTU_CAN_ACK_PGN) return TRUE;
    
    return FALSE;
}

void dtu_can_task_tx_run(void* pdata)
{
    for (;;)
    {
        if(dtu_can_send_is_enable() == TRUE)
        {
            dtu_can_send(&g_dtuCanSendMessage);
        }
        
        sleep(50);
    }
}

void dtu_can_task_rx_run(void* pdata)
{
    INT8U send_msg_flag = 0;
    INT32U send_pgn = 0;

    for (;;)
    {
        if(J1939_OK == J1939CanRecMessageWithWait(g_dtuContext.can_cxt, &g_dtuCanRecMessage, 0))
        {
            switch(g_dtuCanRecMessage.pgn)
            {
            case DTU_CAN_ACK_PGN:
                dtu_can_receive_ack(&g_dtuCanRecMessage);
                break;
            default:
                break;
            }
        }
    }
}

void dtu_can_receive_ack(J1939RecMessage* _PAGED msg)
{
    INT32U rec_pgn = 0;
    
    if(g_dtuContext.start_receive_flag == 0) return;
    if(g_dtuContext.send_index > config_get(kSlaveNumIndex)) return;
    
    rec_pgn = J1939_GET_PGN(msg->data[5], msg->data[4], msg->data[3]);
    
    if(g_dtuContext.send_index == 0)
    {
        if(rec_pgn != DTU_CAN_BCU_FRAME_PGN) return;
        if(msg->data[0]) return;
        
        DTU_CAN_STOP_RECEIVE_TIMEOUT_CHECK();
        g_dtuContext.send_index = 1;
    }
    else
    {
        if(rec_pgn != DTU_CAN_BMU_FRAME_PGN) return;
        if(msg->data[0]) return;
        
        DTU_CAN_STOP_RECEIVE_TIMEOUT_CHECK();
        g_dtuContext.send_index = (INT8U)((g_dtuContext.send_index + 1) % (config_get(kSlaveNumIndex) + 1));
    }
}


INT8U dtu_can_send_is_enable(void)
{
    INT32U interval = DTU_CAN_SEND_INTERVAL;
    
    if(g_dtuContext.work_start == FALSE) return FALSE;
    
    if(g_dtuContext.start_receive_flag)
    {
        if(get_interval_by_tick(g_dtuContext.start_receive_tick, get_tick_count()) >= DTU_CAN_RECEIVE_TIMEOUT)
        {
            DTU_CAN_STOP_RECEIVE_TIMEOUT_CHECK();
            return TRUE;
        }
        return FALSE;
    }
    else
    {
        if(g_dtuContext.send_index != 0) return TRUE; //启动本轮下一个设备数据发送
    }
    
    if(g_dtuContext.send_index == 0)
    {
        interval = (INT32U)config_get(kDTUSendIntervalIndex) * 1000;
    }
    if(get_interval_by_tick(g_dtuContext.send_interval_tick, get_tick_count()) >= interval)
    {
        g_dtuContext.send_interval_tick += interval;
        return TRUE;
    }

    return FALSE;
}

INT8U dtu_can_send_bcu_data(J1939SendMessageBuff* _PAGED buff)
{
    INT8U slave_num, pri, dest_addr;
    INT16U i, index = 0, crc = 0xFFFF;//, pos;
    INT32U temp = 0;
    INIT_WRITE_BUFFFER();
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, DTU_CAN_PROTOCOL_VERSION, buff->byte_max);
    //WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, DTU_CAN_MASTER_NUM, buff->byte_max);//主机个数
    //WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, 0, buff->byte_max);//长度
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kUUIDPart1), buff->byte_max);//UUID 1
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kUUIDPart2), buff->byte_max);//UUID 2
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kUUIDPart3), buff->byte_max);//UUID 3
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kUUIDPart4), buff->byte_max);//UUID 4
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kUUIDPart5), buff->byte_max);//UUID 5
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kUUIDPart6), buff->byte_max);//UUID 6
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kUUIDPart7), buff->byte_max);//UUID 7
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kUUIDPart8), buff->byte_max);//UUID 8
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, slave_num, buff->byte_max);//从机个数
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, SOC_TO_INT8U(bcu_get_SOC()), buff->byte_max);//SOC
    
    if (charger_is_charging() && charger_is_connected()) temp |= 0x01;
    if (bcu_get_battery_insulation_state() >= kAlarmSecondLevel) temp |= 0x02;
    if (bcu_get_slave_communication_state() != kAlarmNone) temp |= 0x04;
    if (bcu_get_high_temperature_state() >= kAlarmSecondLevel || bcu_get_low_temperature_state() >= kAlarmSecondLevel) temp |= 0x08;
    if (bcu_get_discharge_state() >= kAlarmSecondLevel) temp |= 0x10;
    if (bcu_get_charge_state() >= kAlarmSecondLevel) temp |= 0x20;
    if (bcu_get_low_soc_state() >= kAlarmSecondLevel) temp |= 0x40;
    if (bcu_get_high_soc_state() >= kAlarmSecondLevel) temp |= 0x80;
    if (bmu_is_balance_state_actived() == RES_TRUE) temp |= 0x100;
    if (bcu_get_chg_oc_state() >= kAlarmSecondLevel ||
            bcu_get_dchg_oc_state() >= kAlarmSecondLevel)
            temp |= 0x200;
    if (bcu_get_delta_temperature_state() >= kAlarmSecondLevel)
            temp |= 0x400;
    if (bcu_get_delta_voltage_state() >= kAlarmSecondLevel)
            temp |= 0x800;
    if (bcu_get_voltage_exception_state() != kAlarmNone)
            temp |= 0x1000;
    if (bcu_get_temp_exception_state() != kAlarmNone)
            temp |= 0x2000;
    if (bcu_get_high_total_volt_state() >= kAlarmSecondLevel)
            temp |= 0x4000;
    else if (bcu_get_low_total_volt_state() >= kAlarmSecondLevel)
            temp |= 0x8000;
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, (INT8U)(temp >> 16), buff->byte_max);
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, (INT16U)temp, buff->byte_max);//电池状态
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kTotalCapIndex), buff->byte_max);//总容量
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kLeftCapIndex), buff->byte_max);//剩余容量
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, (INT16U)MV_TO_V(bcu_get_total_voltage()), buff->byte_max);//总压
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, bcu_get_current() + DTU_CURRENT_OFFSET, buff->byte_max);//电流
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, config_get(kCycleCntIndex), buff->byte_max);//电池组充放电次数
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, bcu_get_positive_insulation_resistance(), buff->byte_max);//绝缘正极电阻
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, bcu_get_negative_insulation_resistance(), buff->byte_max);//绝缘负极电阻
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, bcu_get_system_insulation_resistance(), buff->byte_max);//电池组绝缘电阻
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, dtu_insu_state_to_data(), buff->byte_max);//绝缘状态
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, SOH_TO_INT8U(bcu_get_SOH()), buff->byte_max);//SOH
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, bcu_get_high_voltage(), buff->byte_max);//最高单体电压
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, VOLT_4_DISPLAY(bcu_get_low_voltage()), buff->byte_max);//最低单体电压
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, TEMP_TO_40_OFFSET((INT8U)bcu_get_high_temperature()), buff->byte_max);//最高单体温度
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, TEMP_4_DISPLAY(TEMP_TO_40_OFFSET((INT8U)bcu_get_low_temperature())), buff->byte_max);//最低单体温度
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, bcu_get_high_voltage_id() >> 8, buff->byte_max);//最高单体电压箱体号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, (INT8U)bcu_get_high_voltage_id(), buff->byte_max);//最高单体电压串号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, bcu_get_low_voltage_id() >> 8, buff->byte_max);//最低单体电压箱体号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, (INT8U)bcu_get_low_voltage_id(), buff->byte_max);//最低单体电压串号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, bcu_get_high_temperature_id() >> 8, buff->byte_max);//最高单体温度箱体号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, (INT8U)bcu_get_high_temperature_id(), buff->byte_max);//最高单体温度串号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, bcu_get_low_temperature_id() >> 8, buff->byte_max);//最低单体温度箱体号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, (INT8U)bcu_get_low_temperature_id(), buff->byte_max);//最低单体温度串号
    
    for(i=index; i<9; i++)
    {
        WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, 0xFF, buff->byte_max);//保证使用多帧传输
    }
    //pos = DTU_CAN_BCU_FRAME_SIZE_POS;
    //WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, pos, index, buff->byte_max);//主机数据长度
    crc = crc_check(buff->data, index);
    WRITE_BT_INT16U_WITH_BUFF_SIZE(buff->data, index, crc, buff->byte_max);//CRC16
    
    buff->byte_cnt = index;
    dest_addr = J1939_BROADCAST_ID;
    buff->pgn = DTU_CAN_BCU_FRAME_PGN;
    pri = 6;
    buff->type = kBroadcastType;
    buff->id.value = ((INT32U)pri << 26)+((INT32U)buff->pgn << 8)+((INT16U)dest_addr<<8)+g_dtuCanContext.can_info->addr;
    
    if(J1939CanSendMessage(g_dtuContext.can_cxt, &g_dtuCanSendMessage) == J1939_OK)
    {
        return RES_OK; 
    }
    else 
    {
        return RES_ERR;
    }   
}

INT8U dtu_send_bmu_data(J1939SendMessageBuff* _PAGED buff, INT8U bmu_id)
{
    INT8U i, index = 0, pos, pri, dest_addr;
    INT8U bat_num, temp_num, bal_cur_num;
    INT16U crc = 0xFFFF;
    INT32U temp;
    INIT_WRITE_BUFFFER();
        
    if(config_get(kSlaveNumIndex) <= bmu_id) return RES_ERR;
    
    bal_cur_num = DTU_CAN_BMU_BAL_CUR_NUM;
    bat_num = (INT8U)bmu_get_voltage_num(bmu_id);
    temp_num = bmu_get_temperature_num(bmu_id);
    
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, bmu_id+1, buff->byte_max);//从机编号
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, 0, buff->byte_max);//从机数据长度
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, bat_num, buff->byte_max);//监控电池数目
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, temp_num, buff->byte_max);//监控温感数目
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, bal_cur_num, buff->byte_max);//监控均衡电流数目
    
    for(i=0; i<bat_num; i++) 
        WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, bmu_get_voltage_item(bmu_id, i), buff->byte_max);//单体电压
    for(i=0; i<temp_num; i++) 
        WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, TEMP_TO_40_OFFSET(bmu_get_temperature_item(bmu_id, i)), buff->byte_max);//单体温度
    
    temp = 0;
    if(bmu_get_heat_status(bmu_id))temp |= 0x01;
    if(bmu_get_cool_status(bmu_id))temp |= 0x02;
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, (INT8U)temp, buff->byte_max);//热管理状态
    
    for(i=0; i<bal_cur_num; i++) 
        WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, index, 0, buff->byte_max);//均衡电流
    
    for(i=0; i<(bat_num+7)/8; i++)
        WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, bmu_get_balance_state(bmu_id, i), buff->byte_max);//均衡状态
    
    for(i=index; i<9; i++)
    {
        WRITE_BT_INT8U_WITH_BUFF_SIZE(buff->data, index, 0xFF, buff->byte_max);//保证使用多帧传输
    }
    pos = DTU_CAN_BMU_FRAME_SIZE_POS;
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buff->data, pos, index, buff->byte_max);//从机数据长度
    
    crc = crc_check(buff->data, index);
    WRITE_BT_INT16U_WITH_BUFF_SIZE(buff->data, index, crc, buff->byte_max);//CRC16
    
    buff->byte_cnt = index;
    dest_addr = J1939_BROADCAST_ID;
    buff->pgn = DTU_CAN_BMU_FRAME_PGN;
    pri = 6;
    buff->type = kBroadcastType;
    buff->id.value = ((INT32U)pri << 26)+((INT32U)buff->pgn << 8)+((INT16U)dest_addr<<8)+g_dtuCanContext.can_info->addr;
    
    if(J1939CanSendMessage(g_dtuContext.can_cxt, &g_dtuCanSendMessage) == J1939_OK)
    {
        return RES_OK; 
    }
    else 
    {
        return RES_ERR;
    }    
}

void dtu_can_send(J1939SendMessageBuff* _PAGED buff)
{
    if(g_dtuContext.send_index == 0)
    {
        dtu_can_send_bcu_data(buff);
        DTU_CAN_START_RECEIVE_TIMEOUT_CHECK();
    }
    else
    {
        if(g_dtuContext.send_index > config_get(kSlaveNumIndex))
        {
            g_dtuContext.send_index = 0;
        }
        else
        {
            if(RES_OK == dtu_send_bmu_data(buff, g_dtuContext.send_index - 1))
            {
                DTU_CAN_START_RECEIVE_TIMEOUT_CHECK();
            } 
        }
    }
}

#endif

INT8U dtu_insu_state_to_data(void)
{
    InsulationWorkState state = bcu_get_insulation_work_state();
    switch (state)
    {
    case kInsulationWorkStateNormal:
        return bcu_get_battery_insulation_state() >= kAlarmSecondLevel ? 1 : 0;
        break;
    case kInsulationWorkStateVoltExcept:
        return 3;
        break;
    case kInsulationWorkStateResOverLow:
        return 1;
        break;
    case kInsulationWorkStateNotConnected:
        return 2;
        break;
    default:
        return 1;
        break;
    }
}



