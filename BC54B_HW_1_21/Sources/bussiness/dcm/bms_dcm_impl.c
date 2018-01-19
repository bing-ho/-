/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_dcm_rs485_impl.c
 * @brief
 * @note
 * @author
 * @date 2012-5-25
 *
 */
#include "bms_dcm_impl.h"


#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE C2705 // Possible loss of data
#pragma MESSAGE DISABLE C4301  // Inline expansion done for function call
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

// TODO: 本文件中存在大量的Magic Number, 需要清理

#define LOOP_MIN_INTERVAL   1000
#define MIN_FRAME_INTERVAL  50

#if BMS_SUPPORT_DCM

OS_STK g_dcm_tx_task_stack[DCM_TX_STK_SIZE] = { 0 };

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DCM

/** for dcm */
DcmRs485Context g_dcm_context;

/** for ranges */
#define MAX_SUPPORT_RANGES 2
INT8U g_dcm_ranges[MAX_SUPPORT_RANGES][2] = { 0 };
INT8U g_dcm_range_count = 0;

static INT32U g_last_receive_tick = 0;
static INT32U g_dcm_req_last_tick = 0;

/**********************************************
 * Ranges Function
 ***********************************************/
#define IS_IN_RANGE(INDEX, VALUE) (VALUE >= g_dcm_ranges[index][0] && VALUE <= g_dcm_ranges[index][1])
#define IS_NOT_IN_RANGE(INDEX, VALUE) (!IS_IN_RANGE(VALUE, INDEX))

void dcm_ranges_reset(void)
{
    g_dcm_range_count = 0;
}

void dcm_ranges_add(INT8U left, INT8U right)
{
    if (left > right) return;
    if (g_dcm_range_count >= MAX_SUPPORT_RANGES) return;
    g_dcm_ranges[g_dcm_range_count][0] = left;
    g_dcm_ranges[g_dcm_range_count][1] = right;
    ++g_dcm_range_count;
}

INT8U dcm_ranges_get_next(INT8U value)
{
    INT8U index, next_index, next_value = 0;

    for (index = 0; index < g_dcm_range_count; ++index)
    {
        if (IS_IN_RANGE(index, value))
        {
            if (value == g_dcm_ranges[index][1])
            {
                next_index = (INT8U)((index + 1) % g_dcm_range_count);
                next_value = g_dcm_ranges[next_index][0];
            }
            else
            {
                next_value = value + 1;

            }
            return next_value;
        }
    }

    /** 如果数值不在任何区间，返回第一个数据的第一数值  */
    if (g_dcm_range_count > 0) return g_dcm_ranges[0][0];

    /** 如果都没有找到，返回缺省值 0 */
    return 0;
}

void dcm_init(void)
{
    safe_memset(&g_dcm_context, 0, sizeof(g_dcm_context));
    g_dcm_context.dev = rs485_init(BMS_DCM_RS485_DEV_NAME, BMS_DCM_RS485_DEV_BAND, dcm_task_rs485_receive, NULL);
    watch_dog_register(WDT_DCM_TX_ID, WDT_DCM_TIME);;
    ALLOCAL_STATIC_BUFFERS(g_dcm_context.buffers, DCM_MAX_RECEIVE_BUFFER_COUNT, DCM_MAX_RECEIVE_BUFFER_SIZE);
    g_dcm_context.framer = framer_create(g_dcm_context.buffers.buffers, DCM_MAX_RECEIVE_BUFFER_SIZE,
            DCM_MAX_RECEIVE_BUFFER_COUNT);

    g_dcm_range_count = 0;

    dcm_task_create();
}
#pragma DATA_SEG DEFAULT

void dcm_uninit(void)
{
    rs485_uninit(g_dcm_context.dev);
    g_dcm_context.dev = NULL;
}

int dcm_task_rs485_receive(rs485_t context, char chr, void* user_data)
{
    INT32U now = get_tick_count();
    if (chr == DCM_START_FRAME_BYTE && get_interval_by_tick(g_last_receive_tick, now) >= MIN_FRAME_INTERVAL)
        framer_write_frame_eof(g_dcm_context.framer);
    g_last_receive_tick = now;
    framer_write(g_dcm_context.framer, (PINT8U)&chr, 1);

    return 0;
}

void dcm_task_create(void)
{
    OSTaskCreate(dcm_task_run, (void *) NULL, (OS_STK *) &g_dcm_tx_task_stack[DCM_TX_STK_SIZE - 1], DCM_TX_TASK_PRIO);
}

void dcm_task_run(void* pdata)
{
    INT8U sent = 0;
    INT32U diff;
    INT32U now;

    g_dcm_req_last_tick = get_tick_count();

    for (;;)
    {
        watch_dog_feed(WDT_DCM_TX_ID);

#if BMS_SUPPORT_DCM_SLAVE
        dcm_task_process_slave();
        sleep(BMS_DCM_REQUEST_SLAVE_INTERVAL);
        sent = 1;
#endif

#if BMS_SUPPORT_DCM_INSULATION
        if(insu_type_is_rs485())
        {
        	dcm_task_process_insulation();
			sleep(BMS_DCM_REQUEST_SLAVE_INTERVAL);
			sent = 1;
        }
#endif
        now = get_tick_count();
        diff = get_interval_by_tick(g_dcm_req_last_tick, now);
        g_dcm_req_last_tick = now;
        if (diff < LOOP_MIN_INTERVAL)
        {
            sleep (LOOP_MIN_INTERVAL - diff);
        }
    }
}

void dcm_task_process_slave(void)
{
    dcm_task_request_slave_frame();

    dcm_task_update_slave_id();

    dcm_task_receive_response_frame();
}

void dcm_task_process_insulation(void)
{
    dcm_task_request_insulation_frame();

    dcm_task_receive_response_frame();
}

void dcm_task_request_slave_frame(void)
{
    INT8U index;

    //发送从机数据请求信号
    g_dcm_context.send_buffer[0] = DCM_START_FRAME_BYTE;
    g_dcm_context.send_buffer[1] = g_dcm_context.current_index;
    g_dcm_context.send_buffer[2] = 0x01;
    g_dcm_context.send_buffer[3] = DCM_FID;
    if ((bcu_get_pack_state() & PACK_CHG_BIT))
        g_dcm_context.send_buffer[4] = 0x01; //充电状态
    else
        g_dcm_context.send_buffer[4] = 0x00;

    //在此增加模块均衡判断,g_dcm_context.send_buffer[5]=0x00表示模块可以均衡
    g_dcm_context.send_buffer[5] = 0x01; //表示模块不能开启均衡

    g_dcm_context.send_buffer[6] = acc_check(g_dcm_context.send_buffer, 6);
    g_dcm_context.send_buffer[7] = 0x16;
    for (index = 0; index < 8; index++)
        rs485_send(g_dcm_context.dev, g_dcm_context.send_buffer[index]);
}

void dcm_task_request_insulation_frame(void)
{
    INT8U index;

    g_dcm_context.send_buffer[0] = DCM_START_FRAME_BYTE;
    g_dcm_context.send_buffer[1] = BMS_DCM_ISOLATION_ID;
    g_dcm_context.send_buffer[2] = 0x01;
    g_dcm_context.send_buffer[3] = DCM_FID;
    g_dcm_context.send_buffer[4] = 0x00;
    g_dcm_context.send_buffer[5] = 0x00; //表示模块不能开启均衡
    g_dcm_context.send_buffer[6] = acc_check(g_dcm_context.send_buffer, 6);
    g_dcm_context.send_buffer[7] = 0x16;

    for (index = 0; index < 8; index++)
        rs485_send(g_dcm_context.dev, g_dcm_context.send_buffer[index]);

    start_bcu_insu_heart_beat_tick();
}

void dcm_task_update_slave_id(void)
{
    INT8U found = 0;
    if (g_dcm_context.current_index < config_get(kSlaveNumIndex))
    {
        start_bmu_heart_beat_tick(g_dcm_context.current_index);
    }
    else if ((g_dcm_context.current_index == BMS_DCM_ISOLATION_ID)&&
    		(insu_type_is_rs485()))
    {
        start_bcu_insu_heart_beat_tick();
    }

    dcm_ranges_reset();

#if BMS_SUPPORT_DCM_INSULATION
    dcm_ranges_add(0, (BmuId)config_get(kSlaveNumIndex));
#endif

#if BMS_SUPPORT_DCM_INSULATION
    if(insu_type_is_rs485())
    	dcm_ranges_add(BMS_DCM_ISOLATION_ID, BMS_DCM_ISOLATION_ID);
#endif

    g_dcm_context.current_index = dcm_ranges_get_next(g_dcm_context.current_index);
}

Result dcm_task_process_frame(PINT8U frame, INT8U size)
{
    INT8U function;

    if (size != BMS_DCM_RS485_FRAME_SIZE) return RES_ERROR;
    if (frame[0] != 0x68 || frame[BMS_DCM_RS485_FRAME_SIZE - 1] != 0x16) return RES_ERROR;
    if (dcm_task_verify_frame(frame, size) != RES_OK) return RES_ERROR;

    function = frame[2];

    switch (function)
    {
    case 0x01:
        return dcm_task_process_response_frame(frame, size);
        break;
    default:
        break;
    }

    return RES_ERROR;
}

Result dcm_task_verify_frame(PINT8U frame, INT8U size)
{
    INT8U check_sum = acc_check(frame, size - 2);

    if (frame[size - 2] != check_sum) return RES_ERROR;

    return RES_OK;
}

Result dcm_task_process_response_frame(PINT8U frame, INT8U size)
{
    INT8U slave_index = frame[1];
    INT8U fid = frame[3];
    OS_CPU_SR cpu_sr = 0;

    if (fid != DCM_FID) return RES_ERROR;

#if BMS_SUPPORT_DCM_SLAVE
    if (slave_index < config_get(kSlaveNumIndex))
    {
        bmu_set_voltage_num(slave_index, frame[5]);
        bmu_set_temperature_num(slave_index, BMU_TMEPERATURE_INVALID_VALUE);
        bmu_set_pack_state(slave_index, frame[4]);
        voltage_num = bmu_get_voltage_num(slave_index);

        for (index = 0; index < voltage_num; index++) /*解析电压数据*/
        {
            temp = ((INT16U) frame[6 + 2 * index] << 8) + frame[7 + 2 * index];
            if (temp > DCM_MAX_VOLTAGE) temp = 0;
            bmu_set_voltage_item(slave_index, index, temp);
        }
        for (index = 0; index < 6; index++)/*解析温度数据*/
        {
            if(frame[38 + index] != 0)
                bmu_set_temperature_item(slave_index, index, frame[38 + index]+10);
            else
                bmu_set_temperature_item(slave_index, index, 0);
        }
        //bmu_set_balance_state(slave_index, ((INT16U) frame[45] << 8) + frame[44]);
        bmu_set_balance_state(slave_index, 0, frame[44]);
        bmu_set_balance_state(slave_index, 0, frame[45]);

        bmu_heart_beat_tick_update(slave_index);

        return RES_OK;
    }
#endif

#if BMS_SUPPORT_DCM_INSULATION
    if (insu_type_is_rs485() && (slave_index == BMS_DCM_ISOLATION_ID))
    { //绝缘模块信息
        if ((frame[12] == 0x00) || (frame[12] == 0x02)) //2010-7-28 dlw修改
        {
            bcu_set_positive_insulation_resistance(((INT16U) frame[4] << 8) + frame[5]);
            bcu_set_negative_insulation_resistance(((INT16U) frame[6] << 8) + frame[7]);
            bcu_set_insulation_total_voltage(((INT32U) frame[8] << 24) + ((INT32U) frame[9] << 16) + ((INT32U) frame[10] << 8)
                    + frame[11]);
        }
        bcu_set_insulation_work_state(frame[12]);
        bcu_set_insulation_error(frame[13]);
        bcu_set_insulation_online(1);

        bcu_insu_heart_beat_tick_update();

        return RES_OK;
    }
#endif

    return RES_ERROR;
}

void dcm_task_receive_response_frame(void)
{
    int res;
    PINT8U frame;
    INT16U size;
    INT32U last, now;

    last = get_tick_count();

    do
    {
        res = framer_read_frame(g_dcm_context.framer, &frame, &size, DCM_TIMEOUT);
        if (res != RES_OK) break;

        res = dcm_task_process_frame(frame, (INT8U)size);
        framer_free_frame(g_dcm_context.framer, frame);

        if (res == RES_OK) break;

        now = get_tick_count();
    } while (get_interval_by_tick(last, now) < DCM_TIMEOUT);
}

#endif

