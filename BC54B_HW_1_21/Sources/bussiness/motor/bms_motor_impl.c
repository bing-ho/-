/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_motor_impl.c
* @brief
* @note
* @author
* @date 2012-5-25
*
*/
#include "bms_motor_impl.h"


#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C5703


#if BMS_SUPPORT_MOTOR == BMS_MOTOR_DEFAULT

static INT8U g_motor_tx_task_stack[MCU_TX_TASK_STK_SIZE];
static INT8U g_motor_rx_task_stack[MCU_RX_TASK_STK_SIZE];

/**********************************************
 * Structures
 ***********************************************/
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
CanMessage g_motor_rx_message;
MotorContext g_motor_context = { NULL };

CanInfo g_motor_can_info;
CanMessage g_motor_can_buffers[MOTOR_CAN_BUFFER_COUNT];

CanMessage g_motor_tx_message;
#pragma DATA_SEG DEFAULT


#define SAFE_GET_VAR(VAR) SAFE_GET_VAR_EX(VAR, INT16U)

#define SAFE_GET_VAR_EX(VAR, TYPE) \
OS_CPU_SR cpu_sr = 0; \
TYPE value; \
OS_ENTER_CRITICAL(); \
value = VAR; \
OS_EXIT_CRITICAL(); \
return value;


/**********************************************
 * Functions
 ***********************************************/
void motor_init(void)
{
    /** init the context */
    safe_memset(&g_motor_context, 0, sizeof(g_motor_context));
    g_motor_context.mile_save = 0;//config_get(kMotorMileLowIndex);
    g_motor_context.mile_cycle = 0;//config_get(kMotorMileHighIndex);
    g_motor_context.tx_info.voltage = 0; /*����������������е�ѹ*/
    g_motor_context.tx_info.current = 0; /*����������������е���*/
    g_motor_context.tx_info.control_byte = 0x00; /*�ؿ�*/

    g_motor_context.rx_info.speed = 0;
    g_motor_context.rx_info.rotation = 0;
    g_motor_context.rx_info.tempera = 0;
    g_motor_context.rx_info.mile = 0;
    g_motor_context.rx_info.fault_code = 0;
    g_motor_context.rx_info.comm_state = 1;
#if !REMOVE_NO_USED_IDX_TO_SAVE_EEPROM
    g_motor_context.speed_rate = config_get(kMotorSpeedRate);
    g_motor_context.tyremm = config_get(kMotorTyremm);
    g_motor_context.gear_rate = config_get(kMotorGearRate);
    g_motor_context.tyre_rate = config_get(kMotorTyreRate);
#else
    g_motor_context.speed_rate = 0;
    g_motor_context.tyremm = 0;
    g_motor_context.gear_rate = 0;
    g_motor_context.tyre_rate = 0;
#endif
    g_motor_context.total_mile =  0+ (0 << 16);//(INT32U)config_get(kMotorMileHighIndex)config_get(kMotorMileLowIndex)

    /** init the can info */
    safe_memset(&g_motor_can_info, 0, sizeof(g_motor_can_info));
    g_motor_can_info.dev = MOTOR_CAN_DEV;
    g_motor_can_info.receive_id = MOTOR_CAN_RECEIVE_ID;
    g_motor_can_info.mask_id = MOTOR_CAN_RECEIVE_MASK_ID;
    g_motor_can_info.mode = MOTOR_CAN_MODE;
    g_motor_can_info.buffers = g_motor_can_buffers;
    g_motor_can_info.buffer_count = MOTOR_CAN_BUFFER_COUNT;
    g_motor_can_info.filtemode=CanFilte32;
    g_motor_context.can = can_init(&g_motor_can_info);

    motor_task_create();
}

void motor_uninit(void)
{
    can_uninit(g_motor_context.can);
    g_motor_context.can = NULL;
}

BOOLEAN motor_is_connected(void)
{
    return (g_motor_context.rx_info.comm_state == 0);
}


Result motor_enable_event(INT16U event)
{
    OS_CPU_SR cpu_sr = 0;

    OS_ENTER_CRITICAL();

    if (event & kMotorEventBmsHardwareException)
        g_motor_context.tx_info.control_byte |= (1 << MOTOR_CONTROL_BYTE_HARDWARE_BIT);

    if (event & kMotorEventBmsOverDischarge)
        g_motor_context.tx_info.control_byte |= (1 << MOTOR_CONTROL_BYTE_ODV_BIT);

    if (event & kMotorEventBmsOverHighTemperature)
        g_motor_context.tx_info.control_byte |= (1 << MOTOR_CONTROL_BYTE_OHT_BIT);

    if (event & kMotorEventBmsUnderVoltage)
        g_motor_context.tx_info.control_byte |= (1 << MOTOR_CONTROL_BYTE_UDV_BIT);

    OS_EXIT_CRITICAL();

    return RES_OK;
}
Result motor_disable_event(INT16U event)
{
    OS_CPU_SR cpu_sr = 0;

    OS_ENTER_CRITICAL();

    if (event & kMotorEventBmsHardwareException)
        g_motor_context.tx_info.control_byte &= ~(1 << MOTOR_CONTROL_BYTE_HARDWARE_BIT);

    if (event & kMotorEventBmsOverDischarge)
        g_motor_context.tx_info.control_byte &= ~(1 << MOTOR_CONTROL_BYTE_ODV_BIT);

    if (event & kMotorEventBmsOverHighTemperature)
        g_motor_context.tx_info.control_byte &= ~(1 << MOTOR_CONTROL_BYTE_OHT_BIT);

    if (event & kMotorEventBmsUnderVoltage)
        g_motor_context.tx_info.control_byte &= ~(1 << MOTOR_CONTROL_BYTE_UDV_BIT);

    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result motor_get_event(INT16U* event)
{
    OS_CPU_SR cpu_sr = 0;
    if (event == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *event = g_motor_context.tx_info.control_byte;
    OS_EXIT_CRITICAL();

    return RES_OK;
}


INT16U motor_get_tyremm(void) /*��ֱ̥�� 0.1 Inch/bit*/
{
    INT16U value;
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    value = g_motor_context.tyremm;
    OS_EXIT_CRITICAL();

    return value;
}

INT16U motor_get_speed_rate(void) /*�ٱ� 0.01/bit*/
{
    INT16U value;
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    value = g_motor_context.speed_rate;
    OS_EXIT_CRITICAL();

    return value;
}

INT16U motor_get_gear_rate(void)  /*��������ֱ� 0.01/bit*/
{
    INT16U value;
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    value = g_motor_context.gear_rate;
    OS_EXIT_CRITICAL();

    return value;
}

INT16U motor_get_tyre_rate(void)  /*���ֱ� 0.01 /bit*/
{
    INT16U value;
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    value = g_motor_context.tyre_rate;
    OS_EXIT_CRITICAL();

    return value;
}

Result motor_set_tyremm(INT16U value)
{
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    g_motor_context.tyremm = value;
    config_save(kMotorTyremm, value);
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result motor_set_speed_rate(INT16U value)
{
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    g_motor_context.speed_rate = value;
    config_save(kMotorSpeedRate, value);
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result motor_set_gear_rate(INT16U value)
{
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    g_motor_context.gear_rate = value;
    config_save(kMotorGearRate, value);
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result motor_set_tyre_rate(INT16U value)
{
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    g_motor_context.tyre_rate = value;
    config_save(kMotorTyreRate, value);
    OS_EXIT_CRITICAL();

    return RES_OK;
}


INT16U motor_get_rotation(void) /*ת��1rpm/bit */
{
    SAFE_GET_VAR(g_motor_context.rx_info.rotation);
}

INT16U motor_get_speed(void) /*ʱ��1kmph/bit */
{
    SAFE_GET_VAR(g_motor_context.rx_info.speed);
}

INT16U motor_get_mile(void) /*��ʱ���0.1km/bit*/
{
    SAFE_GET_VAR(g_motor_context.rx_info.mile);
}

INT32U motor_get_total_mile(void) /*�ۻ����0.1km/bit*/
{
    SAFE_GET_VAR_EX(g_motor_context.total_mile, INT32U);
}

INT16U motor_get_temperature(void) /*����������¶�,1���϶�/bit  */
{
    SAFE_GET_VAR(g_motor_context.rx_info.tempera);
}

INT16S motor_get_fault_code(void) /*���������*/
{
    SAFE_GET_VAR(g_motor_context.rx_info.fault_code);
}

void motor_task_create(void)
{
    OSTaskCreate(motor_task_tx_run, (void *) NULL, (OS_STK *) &g_motor_tx_task_stack[MCU_TX_TASK_STK_SIZE - 1],
            MCU_TX_TASK_PRIO);
    OSTaskCreate(motor_task_rx_run, (void *) NULL, (OS_STK *) &g_motor_rx_task_stack[MCU_RX_TASK_STK_SIZE - 1],
            MCU_RX_TASK_PRIO);
}

void motor_task_tx_run(void* pdata)
{
    g_motor_tx_message.id.value = can_id_from_extend_id(MOTOR_MOTOR_CAN_ID);
    g_motor_tx_message.len = 8;

    for (;;)
    {
        sleep(MOTOR_TX_FRAME_INTERVAL);

        /*���ͳ�������Ʊ���*/
        g_motor_tx_message.data[0] = 0xff;
        g_motor_tx_message.data[1] = 0xff;
        g_motor_tx_message.data[2] = 0xff;
        g_motor_tx_message.data[3] = 0xff;
        g_motor_tx_message.data[4] = motor_get_control_byte();
        g_motor_tx_message.data[5] = 0xff;
        g_motor_tx_message.data[6] = 0xff;
        g_motor_tx_message.data[7] = 0xff;

        can_send(g_motor_context.can, &g_motor_tx_message);
    }
}

void motor_task_rx_run(void* pdata)
{
    Result res;
    
    for (;;)
    {
        res = can_receive(g_motor_context.can, &g_motor_rx_message, MOTOR_RX_FRAME_TIMEOUT);

        if (res != RES_OK)
        {
            g_motor_context.rx_info.rotation = 0;
            g_motor_context.rx_info.speed = 0; /*����kmph*/
            g_motor_context.rx_info.mile = 0;
            g_motor_context.rx_info.tempera = 0;
            g_motor_context.rx_info.fault_code = 0;
            g_motor_context.rx_info.comm_state = 1;
            
            sleep(10);
            continue;
        }
        // ignore the frame
        if (can_id_to_extend_id(g_motor_rx_message.id.value) != MOTOR_CAN_RECEIVE_ID)
        {
            sleep(10);
            continue;
        }

        motor_task_rx_process_message(&g_motor_rx_message);
    }
}

void motor_task_rx_process_message(CanMessage* _PAGED message)
{
    INT16U temp1;
    INT16U temp2;
    OS_CPU_SR cpu_sr = 0;

    /* ����ͨѶ״̬ */
    g_motor_context.rx_info.comm_state = 0;

    /* ��ȡת��*/
    g_motor_context.rx_info.rotation = GET_LT_INT16U(message->data, 0); /*ת��*/
    if (g_motor_context.rx_info.rotation >= MOTOR_ROTATION_MAX) g_motor_context.rx_info.rotation = 0;

    OS_ENTER_CRITICAL();/*�����ж�,��Ϊ�����ܺ���Դ*/
    g_motor_context.speed_factor =(INT16U) ((INT32U)g_motor_context.speed_rate * g_motor_context.gear_rate
            * g_motor_context.tyre_rate * 5 / g_motor_context.tyremm / 762) ;/* h/km */
            
    g_motor_context.rx_info.speed = (INT16U)(((INT32U) g_motor_context.rx_info.rotation * 314)
            / g_motor_context.speed_factor / 100); /*����kmph*/
    OS_EXIT_CRITICAL();

    /* ��ȡ�¶� */
    g_motor_context.rx_info.tempera = message->data[4];

    /*��˿��ĵ��������*/
    // g_motor_context.rx_info.Speed = (MCURxBuf[4] )+((INT16U)MCURxBuf[5] << 8);   /*����kmph*/
    g_motor_context.rx_info.mile = GET_LT_INT16U(message->data, 2);/*���*/
    if (g_motor_context.rx_info.mile >= MOTOR_MILE_MAX) g_motor_context.rx_info.mile = 0;
    if (g_motor_context.rx_info.mile > g_motor_context.temp_mile) /*ϵͳһֱ���л�BMS����*/
    {
        if (g_motor_context.rx_info.mile - g_motor_context.temp_mile <= MOTOR_MILE_CHANGE_MAX_ONCE)
        {
            g_motor_context.total_mile += (g_motor_context.rx_info.mile - g_motor_context.temp_mile);
        }
        g_motor_context.temp_mile = g_motor_context.rx_info.mile;
    }
    else if (g_motor_context.temp_mile > g_motor_context.rx_info.mile)/*���������������*/
    {
        g_motor_context.total_mile += g_motor_context.rx_info.mile;
        g_motor_context.temp_mile = g_motor_context.rx_info.mile;
    }

    /*������̼�¼*/
    temp1 = (INT16U) (g_motor_context.total_mile >> 16);
    temp2 = (INT16U) (g_motor_context.total_mile);
    if (temp2 > g_motor_context.mile_save)
    {
        if(temp2 - g_motor_context.mile_save >= MOTOR_SAVE_MILE_MAX)
        {
            if (temp1 == (g_motor_context.mile_cycle + 1))/*����ʻ��6553.5km*/
            {
                g_motor_context.mile_cycle = temp1;
                //config_save(kMotorMileHighIndex, temp1);
            }
            g_motor_context.mile_save = temp2;
            //config_save(kMotorMileLowIndex, temp2); /*������̼�¼*/
        }
    }
    else
        g_motor_context.mile_save = temp2;

    /** ��ȡ������ */
    g_motor_context.rx_info.fault_code = GET_LT_INT16U(message->data, 6);;
}

INT8U motor_get_control_byte(void)
{
    OS_CPU_SR cpu_sr = 0;
    INT8U byte;

    OS_ENTER_CRITICAL();
    byte = g_motor_context.tx_info.control_byte;
    OS_EXIT_CRITICAL();

    return byte;
}

#endif

