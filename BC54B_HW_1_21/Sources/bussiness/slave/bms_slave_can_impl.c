/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file slave.c
 * @brief
 * @note
 * @author
 * @date 2012-5-11
 *
 */

#include "bms_slave_can_impl.h"


#if BMS_SUPPORT_SLAVE == 0

#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

/**********************************************
 *
 * Macros
 *
 ***********************************************/
#define READ_NEXT_INT8U() READ_LT_INT8U(frame->data, index)
#define READ_NEXT_INT16U() READ_LT_INT16U(frame->data, index)
#define READ_NEXT_INT24U() READ_LT_INT24U(frame->data, index)
#define READ_NEXT_INT32U() READ_LT_INT32U(frame->data, index)

#define SLAVE_RESPONSE_VOLTAGE_NUM_PER_FRAME    3

/**********************************************
 *
 * Global Variable
 *
 ***********************************************/
OS_STK g_slave_tx_task_stack[SLAVE_TX_STK_SIZE] = { 0 };
OS_STK g_slave_rx_task_stack[SLAVE_RX_STK_SIZE] = { 0 };
SlaveNetResponseFrame g_slave_response_frame;
SlaveNetQueryFrame g_slave_query_frame;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
slave_net_t g_slave_net_context;
CanMessage g_slave_can_buffers[BMS_SLAVE_CAN_BUFFER_COUNT];
INT8U g_slave_id = SLAVE_START_ID;
#pragma DATA_SEG DEFAULT

/**********************************************
 *
 * Implementation
 *
 ***********************************************/
void slave_init(void)
{
    dog_watcher_register(SLAVECAN_TASK);
    CanInfo slave_net_can_info;
    safe_memset(&slave_net_can_info, 0, sizeof(slave_net_can_info));
    slave_net_can_info.dev = BMS_SLAVE_CAN_DEV;
    slave_net_can_info.receive_id = BMS_SLAVE_CAN_RECEIVE_ID;
    slave_net_can_info.mask_id = BMS_SLAVE_CAN_RECEIVE_MASK_ID;
    slave_net_can_info.mode = BMS_SLAVE_CAN_MODE;
    slave_net_can_info.buffers = g_slave_can_buffers;
    slave_net_can_info.buffer_count = BMS_SLAVE_CAN_BUFFER_COUNT;

    g_slave_net_context = slave_net_create_can(&slave_net_can_info);

    slave_task_create();
}

void slave_uninit(void)
{
    slave_net_destroy(g_slave_net_context);
    g_slave_net_context = NULL;
}

void slave_task_create(void)
{
    OSTaskCreate(slave_task_run_tx, (void *) 0, (OS_STK *) &g_slave_tx_task_stack[SLAVE_TX_STK_SIZE - 1],
            SLAVECAN_RX_TASK_PRIO);

    OSTaskCreate(slave_task_run_rx, (void *) 0, (OS_STK *) &g_slave_rx_task_stack[SLAVE_RX_STK_SIZE - 1],
            SLAVECAN_TX_TASK_PRIO);
}

void slave_task_run_tx(void* data)
{
    INT32U last_tick;
    INT32U now_tick, diff;
    for (;;)
    {
        last_tick = get_tick_count();
        slave_query_slaves();

        /* delay until next period */
        now_tick = get_tick_count();
        diff = get_interval_by_tick(last_tick, now_tick);
        if (diff <= BMS_SLAVE_CAN_QUERY_PEROID)
        {
            sleep(BMS_SLAVE_CAN_QUERY_PEROID - (INT16U)diff);
        }
        else
        {
            sleep(BMS_SLAVE_CAN_QUERY_SLAVE_DELAY);
        }
    }
}

void slave_task_run_rx(void* data)
{
    for (;;)
    {
        slave_receive_next_frame();
    }
}

void slave_receive_next_frame(void)
{
    Result res;

    res = slave_net_receive(g_slave_net_context, &g_slave_response_frame, BMS_SLAVE_CAN_RECEIVE_RESPONSE_TIMEOUT);
    if (res != 0) return;

    if (g_slave_response_frame.function_code == kSlaveNetRequestStatusCode)
        slave_handle_status_response(&g_slave_response_frame);
}

void slave_query_slaves(void)
{
    do
    {
        slave_query_next_slave();
    }while(g_slave_id > SLAVE_START_ID);
}


void slave_query_next_slave(void)
{
    INT16U slave_num = config_get(kSlaveNumIndex);
    slave_query_slave(g_slave_id);

    if (g_slave_id < slave_num + SLAVE_START_ID)
    {
        if (bmu_get_heart_beat_tick(g_slave_id - SLAVE_START_ID) > config_get(kCommAbortDlyIndex))
        {
            bmu_set_heart_beat_tick(g_slave_id - SLAVE_START_ID, config_get(kCommAbortDlyIndex) + 10); // TODO: ??????
        }
        else
        {
            bmu_set_heart_beat_tick(g_slave_id - SLAVE_START_ID, bmu_get_heart_beat_tick(g_slave_id - SLAVE_START_ID) + 1);
        }
    }

    slave_next_slave();
}

void slave_next_slave(void)
{
    ++g_slave_id;

#if BMS_SUPPORT_HVCM_CTL > 0
    if (g_slave_id == (g_sysParam[SLAVE_NUM_INDEX] + SLAVE_START_ID))
    {
        g_slave_id = BMS_SLAVE_HVCM_SLAVE_CAN_ADDR;
    }
    else if (g_slave_id > BMS_SLAVE_HVCM_SLAVE_CAN_ADDR)
    {
        g_slave_id = SLAVE_START_ID;
    }
#else
    if (g_slave_id == (config_get(kSlaveNumIndex) + SLAVE_START_ID))
    {
        g_slave_id = SLAVE_START_ID;
    }
#endif
}

void slave_query_slave(INT8U slave)
{
    OS_CPU_SR cpu_sr = 0;
    SlaveType slave_type;
    INT8U temp;
    safe_memset(&g_slave_query_frame, 0, sizeof(g_slave_query_frame));
    g_slave_query_frame.receive_id = BMS_SLAVE_CAN_FRAME_SLAVE_ID;
    g_slave_query_frame.mask_id = BMS_SLAVE_CAN_FRAME_MASK_ID;
    g_slave_query_frame.len = BMS_SLAVE_CAN_FRAME_LEN;
    g_slave_query_frame.function_code = kSlaveNetRequestStatusCode;
    g_slave_query_frame.slave_id = slave;

    OS_ENTER_CRITICAL();
    slave_type = slave_get_type(slave);
    switch (slave_type)
    {
    case kSlaveISOType:
        if ((charger_is_charging()) || ((bcu_get_current() > SOC_CURRENT_FILTER_MICRO_CURRENT) && (bcu_get_current() < 0x8000)))
            g_slave_query_frame.data[0] = SLAVE_REQUEST_FLAG_CHARGE;
        else
            g_slave_query_frame.data[0] = SLAVE_REQUEST_FLAG_NOT_BANLANCE;
        break;
    case kSlaveHVCMType:
        temp = 0;
        if (relay_control_main_relay_is_on()) temp |= SLAVE_REQUEST_FLAG_CHARGE;
        if (relay_control_charger_relay_is_on()) temp |= SLAVE_REQUEST_FLAG_NOT_BANLANCE;
        g_slave_query_frame.data[0] = temp;
        g_slave_query_frame.data[1] = 10; // TODO: ?????
        break;
    default:
        break;
    }
    OS_EXIT_CRITICAL();

    slave_net_query(g_slave_net_context, &g_slave_query_frame);
}

SlaveType slave_get_type(INT8U slave_id)
{
    if (slave_id < BMS_SLAVE_ISO_SLAVE_CAN_ADDR)
        return kSlaveISOType;
    else if (slave_id == BMS_SLAVE_HVCM_SLAVE_CAN_ADDR)
        return kSlaveHVCMType;
    else
        return kSlaveNoneType;
}

void slave_handle_status_response(SlaveNetResponseFrame* frame)
{
    INT8U index = 0;
    INT8U slave = frame->receive_id - SLAVE_START_ID;
    INT8U volt_start = 0,temp;
    if (slave >= BMU_MAX_SLAVE_COUNT) return;

    bmu_set_heart_beat_tick(slave, 0);

    if ((slave < config_get(kSlaveNumIndex)))
    {
        /** handle the voltage frame id */
        if (frame->frame_id >= kSlaveVoltageStartFrameId && frame->frame_id <= kSlaveVoltageEndFrameId)
        {
            //volatile INT8U _index = 0;
            //volatile INT16U value1 = READ_BT_INT16U(frame->data, _index);
            //volatile INT16U value2 = READ_LT_INT16U(frame->data, _index);
            //volatile INT16U value3 = frame->data[_index++] + ((INT16U)(frame->data[_index++]) << 8);
            volt_start = (frame->frame_id - kSlaveVoltageStartFrameId) * SLAVE_RESPONSE_VOLTAGE_NUM_PER_FRAME;
            bmu_set_voltage_item(slave, volt_start++, READ_NEXT_INT16U());
            bmu_set_voltage_item(slave, volt_start++, READ_NEXT_INT16U());
            bmu_set_voltage_item(slave, volt_start++, READ_NEXT_INT16U());

            return;
        }

        /** handle other frame */
        switch (frame->frame_id)
        {
        case kSlaveSummaryFrameId:
            bmu_set_voltage_num(slave, READ_NEXT_INT8U());
            bmu_set_pack_state(slave, READ_NEXT_INT8U() & 0x01);
            bmu_set_balance_state(slave, 0, READ_NEXT_INT8U());
            bmu_set_balance_state(slave, 1, READ_NEXT_INT8U());
            bmu_set_balance_state(slave, 2, READ_NEXT_INT8U());
            bmu_set_alarm_status(slave, READ_NEXT_INT8U());
            break;
        case kSlaveBalanceCurrentFrameId:
            bmu_set_balance_current_item(slave, 0, READ_NEXT_INT16U());
            bmu_set_balance_current_item(slave, 1, READ_NEXT_INT16U());
            break;
        case kSlaveTemperatureFrameId:
            while(index < 6)
            {
                temp = READ_NEXT_INT8U();
                if(temp != 0)
                    bmu_set_temperature_item(slave, index-1, temp)+10;
                else
                    bmu_set_temperature_item(slave, index-1, 0);
            }
            bmu_set_temperature_num(slave, BMU_TMEPERATURE_INVALID_VALUE);
            break;
        default:
            break;
        }
    }
}

#endif


