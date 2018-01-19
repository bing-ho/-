/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_hmi.c
 * @brief
 * @note 其他任务调用此协议修改说明见本文档末尾
 * @author
 * @date 2012-4-26
 *
 */

#include "bms_hmi_impl.h"
#include "bms_charger_temperature.h"

#if BMS_SUPPORT_HMI

#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#define HMI_TX_FRAME_SIZE               74
#define HMI_MAX_REGISTER_SIZES          64
#define VOLT_SHOW_CONTROL_LENGTH        8

OS_STK g_hmi_recv_task_stack[HMI_RX_STK_SIZE] = { 0 };

HmiContext g_hmi_context;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_HMI

static INT8U g_hmi_send_buffer[HMI_TX_FRAME_SIZE] = { 0 };
static INT8U g_hmi_register_buffers[HMI_MAX_REGISTER_SIZES] = { 0 };
static INT16U g_hmi_volt_temp_show_bits[VOLT_SHOW_CONTROL_LENGTH] = {0};

#pragma DATA_SEG DEFAULT


/******************************************
 * Helper Macros
 ******************************************/
#define READ_REGISTER_MAP_BEGIN() \
switch(index) \
{

#define READ_REGISTER_MAP_END() \
        default: \
           data = 0x00; \
           break; \
}

#define READ_REGISTER_MAP_ITEM(INDEX, VARIABLE) \
        case INDEX: \
           data = VARIABLE; \
           break;

#define WRITE_REGISTER_MAP_BEGIN() \
switch(index) \
{

#define WRITE_REGISTER_MAP_END() \
}

#define WRITE_REGISTER_MAP_ITEM(INDEX, VARIABLE) WRITE_REGISTER_MAP_ITEM_EX(INDEX, VARIABLE, value)

#define WRITE_REGISTER_MAP_ITEM_EX(INDEX, VARIABLE, VALUE) \
        case INDEX: \
             VARIABLE = VALUE; \
             break;

#define WRITE_REGISTER_MAP_STATEMENT(INDEX, STATEMENT) \
        case INDEX: \
             STATEMENT; \
             break;

#define WRITE_REGISTER_MAP_ITEM_CONFIG(INDEX, CONFIG) WRITE_REGISTER_MAP_ITEM_CONFIG_EX(INDEX, CONFIG, value)

#define WRITE_REGISTER_MAP_ITEM_CONFIG_EX(INDEX, CONFIG, VALUE) \
        case INDEX: \
             config_save(CONFIG, VALUE); \
             break;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
/******************************************
 * Function Implementation
 ******************************************/
void hmi_init()
{
    Buffers buffers;
    HmiContext* hmi_context = &g_hmi_context;
    
    safe_memset(hmi_context, 0, sizeof(HmiContext));
    
    hmi_context->send_buffer = g_hmi_send_buffer;
    hmi_context->register_buffer = g_hmi_register_buffers;
    hmi_context->volt_temp_show_bits = g_hmi_volt_temp_show_bits;
    
    /* ready the buffers */
    ALLOCAL_STATIC_BUFFERS(buffers, HMI_MAX_RECEIVE_BUFFER_COUNT, HMI_MAX_RECEIVE_BUFFER_SIZE);

    hmi_context->modbus_context = modbus_create(HMI_DEV_NAME, rs485_get_bps((INT8U)RS485_SLAVE_BPS_DEF), HMI_FRAME_TIMEOUT, &buffers, hmi_context);//config_get(kRS4851BpsIndex + config_get(kHmiRS485ChannelIndex))
                                                //config_get(kHmiRS485ChannelIndex)
    hmi_modbus_handler_init(hmi_context);
    
    config_register_observer(kSlaveNumIndex, kSlaveNumIndex, hmi_handle_config_changing);

    hmi_task_start();
}

void hmi_modbus_handler_init(HmiContext* context)
{
    
    if(context == NULL) return;
    
    context->modbus_handler = modbus_handler_create(context->modbus_context);
    /* register handing function */
    modbus_handler_register(context->modbus_handler, MODBUS_FUNCTION_READ_COILS, hmi_handle_read_coils);
    modbus_handler_register(context->modbus_handler, MODBUS_FUNCTION_READ_HOLDING_REGISTERS,
            hmi_handle_read_holding_registers);

    modbus_handler_register(context->modbus_handler, MODBUS_FUNCTION_READ_INPUT_REGISTERS,
            hmi_handle_read_input_registers);
    modbus_handler_register(context->modbus_handler, MODBUS_FUNCTION_WRITE_SINGLE_COILS, hmi_handle_write_single_coils);
    modbus_handler_register(context->modbus_handler, MODBUS_FUNCTION_WRITE_SINGLE_REGISTER,
            hmi_handle_write_input_register);
    modbus_handler_register(context->modbus_handler, MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS,
            hmi_handle_write_registers);
}

#pragma DATA_SEG DEFAULT

void hmi_uninit()
{
    HmiContext* hmi_context = &g_hmi_context;
    
    modbus_destroy(hmi_context->modbus_context);
    hmi_context->modbus_context = NULL;
}

void hmi_task_start()
{
    OSTaskCreate(hmi_task_run, (void *) NULL, (OS_STK *) &g_hmi_recv_task_stack[HMI_RX_STK_SIZE - 1], HMI_RX_TASK_PRIO);
}

void hmi_task_run(void* pdata)
{
    if(pdata == NULL) return;
    
    for (;;)
    {
        hmi_check_rx_frame(&g_hmi_context);
        sleep(20);
    }
}

void hmi_check_rx_frame(HmiContext* context)
{
    int err;

    if(context == NULL) return;
    /** receive a frame */
    err = modbus_receive(context->modbus_context, &context->frame, &context->frame_size);
    if (err != 0)
    {
        return;
    }

    /** check the address */
    if (modbus_frame_parse_unit_id(context->modbus_context, context->frame) != MODBUS_UNIT)
    {
        modbus_reply_error(context->modbus_context, context->send_buffer, MODBUS_UNIT, MODBUS_FUNCTION_READ_COILS,
                MODBUS_EXECPTION_CANNOT_READ);
        return;
    }

    context->address = modbus_frame_parse_start_address(context->modbus_context, context->frame);
    context->length = modbus_frame_parse_length(context->modbus_context, context->frame);

    /** handle the frame */
    modbus_handler_dispatch(context->modbus_handler, context->frame, context->frame_size);
}

int hmi_handle_read_coils(modbus_t ctx, PINT8U frame, INT16U size)
{
    HmiContext* hmi_context = NULL;
    INT16U length = modbus_frame_parse_length(ctx, frame);
    INT16U address = modbus_frame_parse_start_address(ctx, frame);
    
    hmi_context = (HmiContext*)modbus_get_user_data(ctx);
    /** check the size */
    if ((length < HMI_COIL_LEN_NIN || length > HMI_COIL_LEN_NAX))
    {
        modbus_reply_error(ctx, hmi_context->send_buffer, MODBUS_UNIT, MODBUS_FUNCTION_READ_COILS,
                MODBUS_EXECPTION_OUT_OF_RANGE_NUMBER);
        return RES_FRAME_INVALID_FORMAT;
    }

    if (address == HMI_BATTERYS_STATE_ADDRESS && length == HMI_BATTERYS_STATE_LENGTH)
    {
        // 电池组总体状态
        INT8U value = (INT8U) bcu_get_hmi_bms_state();
        modbus_reply_read_coils(ctx, hmi_context->send_buffer, MODBUS_UNIT, &value, 1);
    }
    else if (address == HMI_SDCARD_STATE_ADDRESS && length == HMI_SDCARD_STATE_LENGTH)
    {
#if BMS_SUPPORT_DATA_SAVE
        INT8U value = (data_save_get_status() == kDataSaveStatusNormal) ? 1 : 0;
#else
        INT8U value = 0;
#endif
        modbus_reply_read_coils(ctx, hmi_context->send_buffer, MODBUS_UNIT, &value, 1);
    }
    else if (address == HMI_LOGIN_WARNING_ADDRESS && length == HMI_LOGIN_WARNING_LENGTH)
    {
        // 登陆报警信息
        static INT8U data[3] =  { 0x00, 0x00, 0x00 };
        data[0] = (INT8U) bcu_get_hmi_bms_state() & (~(1 << 1));
        modbus_reply_read_coils(ctx, hmi_context->send_buffer, MODBUS_UNIT, data, sizeof(data));
    }
    else if (address == HMI_DISPLAY_WARNING_ENABLE_ADDRESS && length == HMI_DISPLAY_WARNING_ENABLE_LENGTH)
    {
        // 显示报警使能状态
        INT8U state = alert_is_enabled() ? 1 : 0;
        modbus_reply_read_coils(ctx, hmi_context->send_buffer, MODBUS_UNIT, &state, sizeof(state));
    }
    else if (address == HMI_CHARGER_STATE_ADDRESS && length == HMI_CHARGER_STATE_LENGTH)
    { // 充电器状态
        INT8U temp, status = 0x08;
        
        charger_get_status(&temp);
        if(charger_get_charge_except_status_with_num(CHARGER_HARDWARE_FAULT_NUM) == 0x01)
            status |= 0x01;
        if(charger_get_charge_except_status_with_num(CHARGER_OVER_TEMPERATURE_EXCEPT_NUM) == 0x01)
            status |= 0x02;
        if(charger_get_charge_except_status_with_num(CHARGER_INPUT_VOLT_EXCEPT_NUM) == 0x01)
            status |= 0x04;
        if(charger_get_charge_except_status_with_num(CHARGER_RUN_STATUS_NUM) == 0x00)
            status &= ~0x08;
        if(charger_get_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_BMS_NUM) == 0x00)
            status |= 0x20;
        modbus_reply_read_coils(ctx, hmi_context->send_buffer, MODBUS_UNIT, &status, 1);
    }
    else
    {
        modbus_reply_error(ctx, hmi_context->send_buffer, MODBUS_UNIT, MODBUS_FUNCTION_READ_COILS,
                MODBUS_EXECPTION_INVALID_ADDRESS_LENGTH);
    }

    return 0;
}

int hmi_handle_write_single_coils(modbus_t ctx, PINT8U frame, INT16U size)
{
    INT16U address = modbus_frame_parse_start_address(ctx, frame);
    INT16U value = modbus_frame_parse_input_single_value(ctx, frame);
    HmiContext* hmi_context = NULL;
    
    hmi_context = (HmiContext*)modbus_get_user_data(ctx);
    
    if (address == HMI_COLIS_SDCARD_REQUEST_UNPLUG)
    {
        if (value == 0xFF00)
        {
#if BMS_SUPPORT_DATA_SAVE
            data_save_stop();
#endif
        }

        return modbus_reply_write_single_colis(ctx, hmi_context->send_buffer, MODBUS_UNIT, address, value);
    }

    return modbus_reply_error(ctx, hmi_context->send_buffer, MODBUS_UNIT, MODBUS_FUNCTION_WRITE_SINGLE_COILS,
                    MODBUS_EXECPTION_INVALID_ADDRESS_LENGTH);
}


int hmi_handle_read_holding_registers(modbus_t ctx, PINT8U frame, INT16U size)
{
    INT16U index;
    INT16U length = modbus_frame_parse_length(ctx, frame);
    INT16U address = modbus_frame_parse_start_address(ctx, frame);
    INT16U data = 0;
    INT8U data_index = 0;
    HmiContext* hmi_context = NULL;
    
    hmi_context = (HmiContext*)modbus_get_user_data(ctx);
    
    /** check the size */
    if (length < HMI_HOLDING_REGISTER_MIN_LEN || length > HMI_HOLDING_REGISTER_MAX_LEN)
    {
        modbus_reply_error(ctx, hmi_context->send_buffer, MODBUS_UNIT, MODBUS_FUNCTION_READ_HOLDING_REGISTERS,
                MODBUS_EXECPTION_OUT_OF_RANGE_NUMBER);
        return RES_FRAME_INVALID_FORMAT;
    }

    for (index = address; index < address + length; ++index)
    {

        READ_REGISTER_MAP_BEGIN() //
        READ_REGISTER_MAP_ITEM(BAT_INFO_INDEX_MBADDR, hmi_context->current_slave_index + 1)
        READ_REGISTER_MAP_ITEM(SCNT_MBADDR, config_get(kSlaveNumIndex))
        READ_REGISTER_MAP_END()

        WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, data_index, data, HMI_MAX_REGISTER_SIZES);
    }

    return modbus_reply_read_holding_registers(ctx, hmi_context->send_buffer, MODBUS_UNIT, hmi_context->register_buffer, data_index);
}

INT8U hmi_get_battery_lv_icon_status(void)
{
    INT16U low_volt = bcu_get_low_voltage();
    INT16U cfg_lv = config_get(kDChgLVFstAlarmIndex);
  	INT16U cfg_lv_re = DCHG_LV_FST_ALARM_REL;//config_get(kDChgLVFstAlarmRelIndex);
  	INT16U cfg_olv = config_get(kDChgLVSndAlarmIndex);
  	INT16U cfg_olv_re = DCHG_LV_SND_ALARM_REL;//config_get(kDChgLVSndAlarmRelIndex);
    AlarmLevel discharge_status = bcu_get_discharge_state();
    
    if((low_volt > cfg_olv)&&(low_volt <= cfg_olv_re))
  	{
    	if(!(discharge_status >= kAlarmSecondLevel))
    	{
    	  	return TRUE;
    	}
  	}
  	else if(low_volt > cfg_olv_re && low_volt < cfg_lv)
  	{                
    	return TRUE;
  	}
  	else if(low_volt >= cfg_lv && low_volt < cfg_lv_re)
  	{
    	if(discharge_status != kAlarmNone)
    	{
    	   	return TRUE;	
    	}
  	}
  	return FALSE;
}

INT8U hmi_get_battery_olv_icon_status(void)
{
    INT16U low_volt = bcu_get_low_voltage();
    INT16U cfg_lv = config_get(kDChgLVFstAlarmIndex);
  	INT16U cfg_lv_re = DCHG_LV_FST_ALARM_REL;//config_get(kDChgLVFstAlarmRelIndex);
  	INT16U cfg_olv = config_get(kDChgLVSndAlarmIndex);
  	INT16U cfg_olv_re = DCHG_LV_SND_ALARM_REL;//config_get(kDChgLVSndAlarmRelIndex);
    AlarmLevel discharge_status = bcu_get_discharge_state();
    
    if(low_volt <= cfg_olv )
  	{
  	  return TRUE;
  	}
  	else if((low_volt>cfg_olv)&&(low_volt<=cfg_olv_re))
  	{
    	if(discharge_status >= kAlarmSecondLevel)
    	{
    	  	return TRUE;
    	}
  	}
  	return FALSE;
}

int hmi_handle_process_total_input_registers(HmiContext* hmi_context)
{
    INT16U index;
    INT16U data_index = 0;
    INT16U data;
    
    if (hmi_context == NULL) return -1;
    
    for (index = hmi_context->address; index < (hmi_context->address + hmi_context->length); ++index)
    {
        data = 0;
        READ_REGISTER_MAP_BEGIN()
        // TODO: bcu_get_high_temperature_id有被
        READ_REGISTER_MAP_ITEM(TOTAL_VOLT_MBADDR, (INT16U)MV_TO_V(bcu_get_total_voltage())) /*总电压*/
        //READ_REGISTER_MAP_ITEM(TOTAL_VOLT_MBADDR, OSCPUUsage)
        READ_REGISTER_MAP_ITEM(SOC_MBADDR, SOC_TO_INT8U(bcu_get_SOC())) /*SOC*/
        READ_REGISTER_MAP_ITEM(SOH_MBADDR, SOH_TO_INT8U(bcu_get_SOH())) /*SOH*/
        READ_REGISTER_MAP_ITEM(CURRENT_MBADDR, bcu_get_current() + 5000) /*总电流*/
        READ_REGISTER_MAP_ITEM(ALARM_CHECK_FLAG_MBADDR, (INT16U)bcu_get_alarm_check_flag()) /*报警状态*/ //TODO:溢出
        READ_REGISTER_MAP_ITEM(CELL_VMAX_MBADDR, bcu_get_high_voltage()) /*单体最高电压*/
        READ_REGISTER_MAP_ITEM(CELL_VMIN_MBADDR, VOLT_4_DISPLAY(bcu_get_low_voltage())) /*单体最低电压*/
        READ_REGISTER_MAP_ITEM(CELL_TMAX_MBADDR, ((INT16S)TEMPERATURE_TO_C(bcu_get_high_temperature()))) /*单体最高温度*/
        READ_REGISTER_MAP_ITEM(TOTAL_CAP_MBADDR, config_get(kTotalCapIndex)) /*总容量*/
        READ_REGISTER_MAP_ITEM(LEFT_CAP_MBADDR, config_get(kLeftCapIndex))
        READ_REGISTER_MAP_ITEM(PACK_CYCLE_MBADDR, config_get(kCycleCntIndex))
        READ_REGISTER_MAP_ITEM(RESIDULE_CAP_MBADDR, (INT16U)(DIVISION(config_get(kLeftCapIndex),10) * MV_TO_V(bcu_get_total_voltage()) / 1000))//(INT32U)config_get(kLeftCapIndex)
        READ_REGISTER_MAP_ITEM(VMAX_MODULE_INDEX_MBADDR, bcu_get_high_voltage_id() >> 8) /*最高电压模块编号*/
        READ_REGISTER_MAP_ITEM(VMAX_CELL_INDEX_MBADDR, bcu_get_high_voltage_id() & 0xFF) /*最高电压内部模块编号*/
        READ_REGISTER_MAP_ITEM(VMIN_MODULE_INDEX_MBADDR, bcu_get_low_voltage_id() >> 8) /*最低电压模块编号*/
        READ_REGISTER_MAP_ITEM(VMIN_CELL_INDEX_MBADDR, bcu_get_low_voltage_id() & 0xFF) /*最低电压模块内部编号*/
        READ_REGISTER_MAP_ITEM(TMAX_MODULE_INDEX_MBADDR, bcu_get_high_temperature_id() >> 8) /*最高温度模块编号*/
        READ_REGISTER_MAP_ITEM(TMIN_MODULE_INDEX_MBADDR, bcu_get_low_temperature_id() >> 8) /*最低温度模块编号*/
        READ_REGISTER_MAP_ITEM(TMAX_CELL_INDEX_MBADDR, bcu_get_high_temperature_id() & 0xFF) /*最高温度模块内部编号*/
        READ_REGISTER_MAP_ITEM(TMIN_CELL_INDEX_MBADDR, bcu_get_low_temperature_id() & 0xFF) /*最低温度模块内部编号*/
        READ_REGISTER_MAP_ITEM(ODV_ALERT_MBADDR, hmi_get_battery_olv_icon_status()) // 单体过放
        READ_REGISTER_MAP_ITEM(DCHG_CURRENT_ALERT_MBADDR, (bcu_get_dchg_oc_state() != kAlarmNone) ? 1 : 0) // 放电过流
        READ_REGISTER_MAP_ITEM(CHG_CURRENT_ALERT_MBADDR, (bcu_get_chg_oc_state() != kAlarmNone) ? 1 : 0) //充电过流
        READ_REGISTER_MAP_ITEM(CELL_UV_ALERT_MBADDR, hmi_get_battery_lv_icon_status()) //单体欠压报警
        READ_REGISTER_MAP_ITEM(SOC_LOW_ALERT_MBADDR, (bcu_get_low_soc_state() >= kAlarmFirstLevel) ? 1 : 0) //soc过低
        READ_REGISTER_MAP_ITEM(SOC_HIGH_ALERT_MBADDR, (bcu_get_high_soc_state() >= kAlarmFirstLevel) ? 1 : 0) //soc过高
        READ_REGISTER_MAP_ITEM(TEMP_HIGH_ALERT_MBADDR, (bcu_get_high_temperature_state() != kAlarmNone) ? 1 : 0) //高温报警
        READ_REGISTER_MAP_ITEM(TEMP_LOW_ALERT_MBADDR, (bcu_get_low_temperature_state() != kAlarmNone) ? 1 : 0) //低温报警
        READ_REGISTER_MAP_ITEM(CELL_OV_ALERT_MBADDR, (bcu_get_charge_state() != kAlarmNone) ? 1 : 0)         //单体过充
        READ_REGISTER_MAP_ITEM(CHARGER_STATE_MBADDR, (bcu_get_pack_state() & (1 << PACK_CHG_BIT)) ? 1 : 0)     //充电状态
        READ_REGISTER_MAP_ITEM(COMM_ABORT_ALERT_MBADDR, (bcu_get_pack_state() & (1 << PACK_COMM_ALERT_BIT)) == 0x80 ? 1 : 0) /*通讯异常报警*/
        //TODOs
        //READ_REGISTER_MAP_ITEM(MOTOR_RSPEED_MBADDR, g_batteryLTState >= TEMP_OHT ? 1 : 0) /*转速1rpm/bit */

        // TODO:
#if BMS_SUPPORT_MOTOR
        READ_REGISTER_MAP_ITEM(MOTOR_RSPEED_MBADDR, motor_get_rotation()) /*转速1rpm/bit */
        READ_REGISTER_MAP_ITEM(MOTOR_SPEED_MBADDR, motor_get_speed()) /*时速1kmph/bit */
        READ_REGISTER_MAP_ITEM(MOTOR_TMILE_MBADDR, motor_get_mile()) /*临时里程0.1km/bit*/
        READ_REGISTER_MAP_ITEM(MOTOR_MILE_MBADDR_H, (INT16U)(motor_get_total_mile() >> 16)) /*累积里程0.1km/bit*/
        READ_REGISTER_MAP_ITEM(MOTOR_MILE_MBADDR_L, (INT16U)motor_get_total_mile()) /*电机控制器温度,1摄氏度/bit  */
        READ_REGISTER_MAP_ITEM(MOTOR_TEMPERA_MBADDR, TEMP_C_FROM_40_OFFSET(motor_get_temperature()))
        READ_REGISTER_MAP_ITEM(MOTOR_FAULTCODE_MBADDR, motor_get_fault_code()) /*电机故障码*/
#endif
        READ_REGISTER_MAP_ITEM(COMMON_ALART_MBADDR, ((bcu_get_high_total_volt_state() != kAlarmNone) ? (1 << 0) : 0) |
                                                    ((bcu_get_low_total_volt_state() != kAlarmNone) ? (1 << 1) : 0) |
                                                    ((bcu_get_voltage_exception_state() != kAlarmNone) ? (1 << 2) : 0) |
                                                    ((bcu_get_temp_exception_state() != kAlarmNone) ? (1 << 3) : 0) |
                                                    ((bcu_get_delta_voltage_state() != kAlarmNone) ? (1 << 4) : 0) |
                                                    ((bcu_get_delta_temperature_state() != kAlarmNone) ? (1 << 5) : 0) |
                                                    ((bcu_get_discharge_state() != kAlarmNone) ? (1 << 6) : 0) |
                                                    ((bcu_get_battery_insulation_state() != kAlarmNone) ? (1 << 7) : 0))
        READ_REGISTER_MAP_ITEM(INSU_R_SYS_MBADDR, bcu_get_system_insulation_resistance())
        READ_REGISTER_MAP_ITEM(INSU_R_P_MBADDR, bcu_get_positive_insulation_resistance())
        READ_REGISTER_MAP_ITEM(INSU_R_N_MBADDR, bcu_get_negative_insulation_resistance())
        READ_REGISTER_MAP_ITEM(INSU_ERROR_MBADDR, bcu_get_insulation_error())
        READ_REGISTER_MAP_ITEM(INSU_ONLINE_MBADDR, bcu_get_insulation_online())
        READ_REGISTER_MAP_ITEM(INSU_STATE_MBADDR, hmi_query_insu_state())
        READ_REGISTER_MAP_ITEM(SYS_VOLT_MBADDR, bcu_get_system_voltage())
        //Juson add                                                                  
        READ_REGISTER_MAP_ITEM(DC1_CHARGE_OUTLET_TEMPER_MBADDR, TEMPERATURE_TO_C(bms_get_chgr_dc_outlet_temperature(0)))
        READ_REGISTER_MAP_ITEM(DC2_CHARGE_OUTLET_TEMPER_MBADDR, TEMPERATURE_TO_C(bms_get_chgr_dc_outlet_temperature(1)))
        READ_REGISTER_MAP_ITEM(AC1_CHARGE_OUTLET_TEMPER_MBADDR, TEMPERATURE_TO_C(bms_get_chgr_ac_outlet_temperature(0)))
        READ_REGISTER_MAP_ITEM(AC2_CHARGE_OUTLET_TEMPER_MBADDR, TEMPERATURE_TO_C(bms_get_chgr_ac_outlet_temperature(1)))
        READ_REGISTER_MAP_ITEM(LOCK_STATUS_MBADDR,chglock_get_logic_state());  
        READ_REGISTER_MAP_END()

        hmi_context->register_buffer[data_index++] = (INT8U)(data >> 8);
        hmi_context->register_buffer[data_index++] = (INT8U)(data);
    }

    return modbus_reply_read_input_registers(hmi_context->modbus_context, hmi_context->send_buffer, MODBUS_UNIT,
            hmi_context->register_buffer, data_index);
}

void hmi_set_show_bit(INT16U* _PAGED show_bytes, INT8U offset)
{
    INT8U byte_index = offset / 16;
    INT8U byte_offset = offset % 16;
    show_bytes[byte_index] |= (1 << byte_offset);
}

int hmi_handle_process_entry_input_registers(HmiContext* hmi_context)
{
    INT8U slave_index = hmi_get_current_slave_num(hmi_context);
    INT16U start_address = hmi_context->address;
    INT16U max_address = start_address + hmi_context->length;
    INT8U length, i;

  //  if (hmi_context->current_slave_index > 0) slave_index = (INT8U)hmi_context->current_slave_index;
    if (hmi_context == NULL) return -1;
    
    if (slave_index >= config_get(kSlaveNumIndex))
    {
        modbus_reply_error(hmi_context->modbus_context, hmi_context->send_buffer, MODBUS_UNIT,
                MODBUS_FUNCTION_READ_INPUT_REGISTERS, MODBUS_EXECPTION_INVALID_ADDRESS_LENGTH);
        return RES_FRAME_INVALID_FORMAT;
    }

    hmi_context->register_buffer_pos = 0;

    while(start_address < max_address)
    {
        if (start_address >= HMI_BMU_VOLTAGE_START_ADDRESS && start_address < HMI_BMU_VOLTAGE_START_ADDRESS + HMI_BMU_VOLTAGE_MAX_LENGTH)
        {
            length = (INT8U)(HMI_BMU_VOLTAGE_START_ADDRESS + HMI_BMU_VOLTAGE_MAX_LENGTH - start_address);
            if (length > (max_address - start_address)) length = max_address - start_address;
            hmi_handle_process_entry_voltage_input_registers(hmi_context, slave_index, start_address, length);
            start_address += length;
        }
        else if(start_address >= HMI_BMU_TEMPERATURE_START_ADDRESS && start_address < HMI_BMU_TEMPERATURE_START_ADDRESS + HMI_BMU_TEMPERATURE_MAX_LENGTH)
        {
            length = (INT8U)(HMI_BMU_TEMPERATURE_START_ADDRESS + HMI_BMU_TEMPERATURE_MAX_LENGTH - start_address);
            if (length > (max_address - start_address)) length = max_address - start_address;
            hmi_handle_process_entry_temperate_input_registers(hmi_context, slave_index, start_address, length);
            start_address += length;
        }
        else if(start_address >= VOLT_SHOW_CONTROL && start_address < VOLT_SHOW_CONTROL + VOLT_SHOW_CONTROL_LENGTH)
        {
            INT8U voltage_num = 0;
            
            voltage_num = (INT8U)bmu_get_voltage_num(slave_index);
            safe_memset(hmi_context->volt_temp_show_bits, 0, sizeof(INT16U)*VOLT_SHOW_CONTROL_LENGTH);
            while(voltage_num > 0)
            {
                if (voltage_num < VOLT_SHOW_CONTROL_LENGTH * 16)
                    hmi_set_show_bit(hmi_context->volt_temp_show_bits, voltage_num - 1);
                --voltage_num;
            }
            length = (INT8U)(max_address - start_address);
            if(length > VOLT_SHOW_CONTROL_LENGTH)length = VOLT_SHOW_CONTROL_LENGTH;
            i=(INT8U)(start_address - VOLT_SHOW_CONTROL);
            while(length--)
            {
                if(i < VOLT_SHOW_CONTROL_LENGTH) 
                {
                    WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, hmi_context->register_buffer_pos, hmi_context->volt_temp_show_bits[i], HMI_MAX_REGISTER_SIZES);
                    i++;
                }
                else
                    WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, hmi_context->register_buffer_pos, 0, HMI_MAX_REGISTER_SIZES);
            }
            start_address += length;
        }
        else if(start_address >= TEMP_SHOW_CONTROL && start_address < TEMP_SHOW_CONTROL + TEMP_SHOW_CONTROL_LENGTH)
        {
            INT8U temperature_num=0;
            
            temperature_num = bmu_get_temperature_num(slave_index);
            safe_memset(hmi_context->volt_temp_show_bits, 0, sizeof(INT16U)*VOLT_SHOW_CONTROL_LENGTH);
            while(temperature_num > 0)
            {
                if (temperature_num < VOLT_SHOW_CONTROL_LENGTH * 16)
                    hmi_set_show_bit(hmi_context->volt_temp_show_bits, temperature_num - 1);
                --temperature_num;
            }
            length = (INT8U)(max_address - start_address);
            if(length > TEMP_SHOW_CONTROL_LENGTH)length = TEMP_SHOW_CONTROL_LENGTH;
            i=(INT8U)(start_address - TEMP_SHOW_CONTROL);
            while(length--)
            {
                if(i < TEMP_SHOW_CONTROL_LENGTH) 
                {
                    WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, hmi_context->register_buffer_pos, hmi_context->volt_temp_show_bits[i], HMI_MAX_REGISTER_SIZES);
                    i++;
                } 
                else
                    WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, hmi_context->register_buffer_pos, 0, HMI_MAX_REGISTER_SIZES);
            }
            start_address += length;
        }
        else if(start_address >= HMI_CUSTOM_INFORMATION_START_ADDRESS && start_address < HMI_CUSTOM_INFORMATION_START_ADDRESS + HMI_CUSTOM_INFORMATION_NUM)
        {//客户信息显示区
            length = (INT8U)(max_address - start_address);
            if(length > HMI_CUSTOM_INFORMATION_NUM) length = HMI_CUSTOM_INFORMATION_NUM;
            i = (INT8U)(start_address - HMI_CUSTOM_INFORMATION_START_ADDRESS);
            while(length--)
            {
                if(i < HMI_CUSTOM_INFORMATION_NUM)
                {
                    WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, hmi_context->register_buffer_pos, bcu_get_custom_information(i), HMI_MAX_REGISTER_SIZES);
                    i++;
                }
                else
                    WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, hmi_context->register_buffer_pos, 0, HMI_MAX_REGISTER_SIZES);
            }
            start_address += length;
        }
        else
        {
            WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, hmi_context->register_buffer_pos, 0, HMI_MAX_REGISTER_SIZES);
            ++start_address;
        }
    }

    return modbus_reply_read_input_registers(hmi_context->modbus_context, hmi_context->send_buffer, MODBUS_UNIT,
            hmi_context->register_buffer, hmi_context->register_buffer_pos);
}

int hmi_handle_process_entry_voltage_input_registers(HmiContext* hmi_context, BmuId slave_index, INT16U start_address, INT8U length)
{
    INT8U data_index = 0;
    INT8U index;
    INT8U battery_index_offset = (INT8U)(start_address - HMI_BMU_VOLTAGE_START_ADDRESS);
    INT8U battery_index = 0;
    INT16U voltage;

    if (hmi_context == NULL) return -1;
    /* 电压数据 */
    for (index = 0; index < length; index++)
    {
        battery_index = battery_index_offset + index;
        voltage = bmu_get_voltage_item(slave_index, battery_index);
        WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, hmi_context->register_buffer_pos, voltage, HMI_MAX_REGISTER_SIZES);
    }

    return 0;
}

int hmi_handle_process_entry_temperate_input_registers(HmiContext* hmi_context, BmuId slave_index, INT16U start_address, INT8U length)
{
    INT8U data_index = 0;
    INT8U index;
    INT8U temperature_index_offset = (INT8U)(start_address - HMI_BMU_TEMPERATURE_START_ADDRESS);
    INT8U temperature_index = 0;
    INT16S temperature;

    /* 电压数据 */
    for (index = 0; index < length; index++)
    {
        temperature_index = temperature_index_offset + index;
        temperature = (INT16S)TEMPERATURE_TO_C(bmu_get_temperature_item(slave_index, temperature_index));
        WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, hmi_context->register_buffer_pos, temperature, HMI_MAX_REGISTER_SIZES);
    }

    return 0;
}

int hmi_handle_process_charger_input_registers(HmiContext* hmi_context)
{
    /*********************************************************************************
     **充电器的编号为50XX,xx表示数据编号，依据不同的充电器而定
     *********************************************************************************/
    INT8U data_index = 0;
    //OS_CPU_SR cpu_sr = 0;
    INT16U output_voltage = 0, output_current = 0;
    
    if (hmi_context == NULL) return -1;
    //OS_ENTER_CRITICAL();
    charger_get_output_voltage(&output_voltage);
    charger_get_output_current(&output_current);
    WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, data_index, output_voltage, HMI_MAX_REGISTER_SIZES);
    WRITE_BT_INT16U_WITH_BUFF_SIZE(hmi_context->register_buffer, data_index, output_current, HMI_MAX_REGISTER_SIZES);
    //OS_EXIT_CRITICAL();

    return modbus_reply_read_input_registers(hmi_context->modbus_context, hmi_context->send_buffer, MODBUS_UNIT,
            hmi_context->register_buffer, data_index);
}

int hmi_handle_write_input_register(modbus_t ctx, PINT8U frame, INT16U size)
{
    HmiContext* hmi_context = NULL;
    INT16U index = modbus_frame_parse_start_address(ctx, frame);
    INT16U value = modbus_frame_parse_input_single_value(ctx, frame);
    
    hmi_context = (HmiContext*)modbus_get_user_data(ctx);
    if(hmi_context == NULL) return -1;
    
    WRITE_REGISTER_MAP_BEGIN()  //
    WRITE_REGISTER_MAP_STATEMENT(BAT_INFO_INDEX_MBADDR, hmi_set_current_slave_index(hmi_context, value)) /*单体过充阈值*/
    WRITE_REGISTER_MAP_END()

    return modbus_reply_write_input_register(hmi_context->modbus_context, hmi_context->send_buffer, MODBUS_UNIT,
            index, value);
}

int hmi_handle_read_input_registers(modbus_t ctx, PINT8U frame, INT16U size)
{
    INT16U data = 0;
    HmiContext* hmi_context = NULL;
    
    hmi_context = (HmiContext*)modbus_get_user_data(ctx);
    
    /** check the size */
    if (hmi_context->address < HMI_INPUT_REGISTER_MIN_LEN || hmi_context->address > HMI_INPUT_REIGSTER_ENTRY_MAX_ADDRESS)
    {
        modbus_reply_error(ctx, hmi_context->send_buffer, MODBUS_UNIT, MODBUS_FUNCTION_READ_INPUT_REGISTERS,
                MODBUS_EXECPTION_OUT_OF_RANGE_NUMBER);
        return RES_FRAME_INVALID_FORMAT;
    }

    /** check the max address */
    if (hmi_context->address + hmi_context->length <= HMI_INPUT_REIGSTER_TOTAL_MAX_ADDRESS)
    {
        return hmi_handle_process_total_input_registers(hmi_context);
    }
   /* else if (hmi_context->address == BAT_INFO_INDEX_MBADDR)         //TODO
    {
        return hmi_handle_process_current_slave_index_input_registers();
    }    */
    else if (hmi_context->address + hmi_context->length < HMI_INPUT_REIGSTER_ENTRY_MAX_ADDRESS)
    {
        return hmi_handle_process_entry_input_registers(hmi_context);
    }
    else if (hmi_context->address == HMI_CHARGER_INPUT_REGISTER_ADDRESS
            && hmi_context->length == HMI_CHARGER_INPUT_REGISTER_LENGTH)
    {
        return hmi_handle_process_charger_input_registers(hmi_context);
    }
    else
    {
        modbus_reply_error(hmi_context->modbus_context, hmi_context->send_buffer, MODBUS_UNIT,
                MODBUS_FUNCTION_READ_INPUT_REGISTERS, MODBUS_EXECPTION_INVALID_ADDRESS_LENGTH);
        return RES_FRAME_INVALID_FORMAT;
    }
}

int hmi_handle_write_registers(modbus_t ctx, PINT8U frame, INT16U size)
{
    OS_CPU_SR cpu_sr = 0;
    INT16U index;
    PINT8U data = modbus_frame_parse_data(ctx, frame);
    INT16U value;
    INT8U data_index = 0;
    HmiContext* hmi_context = NULL;
    
    hmi_context = (HmiContext*)modbus_get_user_data(ctx);
    
    /* check the size */
    if (data == NULL || data + 2 * hmi_context->length > hmi_context->frame + hmi_context->frame_size)
    {
        modbus_reply_error(hmi_context->modbus_context, hmi_context->send_buffer, MODBUS_UNIT,
                MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS, MODBUS_EXECPTION_OUT_OF_RANGE_NUMBER);
        return RES_FRAME_INVALID_FORMAT;
    }

    if (bcu_get_config_state() != kConfigStateIdle || hmi_context->length < 1 || hmi_context->length > 32)     //TODO
    {
        modbus_reply_error(hmi_context->modbus_context, hmi_context->send_buffer, MODBUS_UNIT,
                MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS, MODBUS_EXECPTION_OUT_OF_RANGE_NUMBER);
        return RES_FRAME_INVALID_FORMAT;
    }

    //OS_ENTER_CRITICAL();
    for (index = hmi_context->address; index < hmi_context->address + hmi_context->length; index++)
    {
        value = GET_INT16U(data, data_index);
        data_index += 2;
        WRITE_REGISTER_MAP_BEGIN()  //

        WRITE_REGISTER_MAP_STATEMENT(BAT_INFO_INDEX_MBADDR, hmi_set_current_slave_index(hmi_context, value)) 
        
        WRITE_REGISTER_MAP_END()
    }

    //bcu_set_config_state(kPChargeStateRunning);
    //OS_EXIT_CRITICAL();

    return modbus_reply_write_input_registers(hmi_context->modbus_context, hmi_context->send_buffer, MODBUS_UNIT,
            hmi_context->address, hmi_context->length);
}

INT16U hmi_query_insu_state()
{
    INT16U status, data = bcu_get_insulation_work_state();
    
    if(insu_type_is_none())
        return 0x00;
    
    if ((data == kInsulationWorkStateResOverLow)
            || (data == kInsulationWorkStateVoltExcept))
    {
        if(data == kInsulationWorkStateResOverLow)
            status = 0x01;
        else
            status = 0x03;
    }
    else if (data == kInsulationWorkStateNormal)
    {
        status = (bcu_get_battery_insulation_state() > kAlarmFirstLevel) ? 0x01 : 0x00;
    }
    else
    {
        status = 0x02;
    }
    return status;
}


void hmi_set_max_cap(INT16U value)
{
    config_save(kNominalCapIndex, value);
}

void hmi_set_left_cap(INT16U value)
{
    //OS_INIT_CRITICAL();
    //OS_ENTER_CRITICAL();
    config_save(kLeftCapIndex, value);
    bcu_set_left_cap_interm(CAP_TENFOLD_AH_TO_MAS(value)); //剩余容量：1Ah==>1Ams
    //OS_EXIT_CRITICAL();
}

INT16U hmi_register_to_state_delay(INT16U value)
{
    return (INT16U) value;
}
INT8U hmi_register_to_slave_num(INT16U value)
{
    if (value > BMU_MAX_SLAVE_COUNT) return BMU_MAX_SLAVE_COUNT;
    if (value == 0) return 1;
    return (INT8U) value;
}

void hmi_set_current_slave_index(HmiContext* hmi_context, INT16U value)
{
    BmuId slave = (BmuId)config_get(kSlaveNumIndex);
    if (slave == 0) return;

    hmi_context->current_slave_index = (value + slave - 1) % slave;
}

INT8U hmi_get_current_slave_num(HmiContext* hmi_context)
{
    BmuId slave = (BmuId)config_get(kSlaveNumIndex);
    INT8U currnet_slave = (INT8U)hmi_context->current_slave_index;
    if (currnet_slave > slave) return 0;

    return currnet_slave;
}
Result hmi_handle_config_changing(ConfigIndex index, INT16U new_value)
{
    if (index == kSlaveNumIndex)
    {
        hmi_set_current_slave_index(&g_hmi_context, new_value);
    }

    return RES_OK;
}

/* 移植本协议，请将以下内容拷贝过去并修改相关名称 */

#if 0

#define HMI_TX_FRAME_SIZE               74
#define HMI_MAX_REGISTER_SIZES          64
#define VOLT_SHOW_CONTROL_LENGTH        8

DtuCanContext g_dtuContext;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_HMI

static INT8U g_dtu_send_buffer[HMI_TX_FRAME_SIZE] = { 0 };
static INT8U g_dtu_register_buffers[HMI_MAX_REGISTER_SIZES] = { 0 };
static INT16U g_dtu_volt_temp_show_bits[VOLT_SHOW_CONTROL_LENGTH] = {0};

#pragma DATA_SEG DEFAULT

void dtu_can_task_create(void); //需要修改dtu_can_task_create名称
/**********************************************
 *
 * Implementation
 *
 ***********************************************/

Result dtu_handle_config_changing(ConfigIndex index, INT16U new_value)  //需要修改dtu_handle_config_changing名称
{
    if (index == kSlaveNumIndex)
    {
        hmi_set_current_slave_index(&g_dtu_context, new_value); //需要修改g_dtu_context名称
    }

    return RES_OK;
}

void dtu_init(void)
{
    Buffers buffers;
    HmiContext* hmi_context = &g_dtu_context;  //需要修改g_dtu_context名称
    
    safe_memset(hmi_context, 0, sizeof(HmiContext));
    hmi_context->send_buffer = g_dtu_send_buffer; //需要修改g_dtu_send_buffer名称
    hmi_context->register_buffer = g_dtu_register_buffers; //需要修改g_dtu_register_buffers名称
    hmi_context->volt_temp_show_bits = g_dtu_volt_temp_show_bits; //需要修改g_dtu_volt_temp_show_bits名称
    
    /* ready the buffers */
    ALLOCAL_STATIC_BUFFERS(buffers, HMI_MAX_RECEIVE_BUFFER_COUNT, HMI_MAX_RECEIVE_BUFFER_SIZE);

    hmi_context->modbus_context = modbus_create(DTU_DEV_NAME, rs485_get_bps((INT8U)RS485_DTU_BPS_DEF), HMI_FRAME_TIMEOUT, &buffers, hmi_context);//config_get(kRS4851BpsIndex + config_get(kDtuRS485ChannelIndex))
                                                //config_get(kDtuRS485ChannelIndex)
    hmi_modbus_handler_init(hmi_context);
    
    
    config_register_observer(kSlaveNumIndex, kSlaveNumIndex, dtu_handle_config_changing); //需要修改dtu_handle_config_changing名称
    dtu_can_task_create(); //需要修改dtu_can_task_create名称
}

void dtu_can_task_tx_run(void* pdata)  //需要修改dtu_can_task_tx_run名称
{
    for (;;)
    {
        hmi_check_rx_frame(&g_dtu_context);  //需要修改g_dtu_context名称
        sleep(20);
    }
}

void dtu_can_task_create(void) //需要修改dtu_can_task_create名称
{
    OSTaskCreate(dtu_can_task_tx_run, (void *) NULL, (OS_STK *) &g_dtu_can_tx_task_stack[DTU_TX_STK_SIZE - 1],
            DTU_TX_TASK_PRIO); //需要修改dtu_can_task_tx_run名称
            
}

#endif

#endif

