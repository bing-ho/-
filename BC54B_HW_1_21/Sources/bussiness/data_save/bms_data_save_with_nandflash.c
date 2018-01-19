
#include "bms_data_save_impl.h"
#include "nandflash_interface.h"
#include "spi_flash_impl.h"
#include "nandflash_intermediate.h"
#include "bms_event.h"
#include "bms_event_impl.h"
#include "bms_config.h"
#include "main_bsu_relay_ctrl.h"
#include "bms_power_control_impl.h"
#include "bms_charger_temperature.h"
#include "bms_charger_stock.h"
#include "bms_charger_temperature.h"
#include "bsu_active_balance.h"
#include "bms_data_read_impl.h"
#include "bms_stat.h"
#include "run_mode.h"
#include "precharge_process_create.h"
#include "logic_ctrl_lib_func.h"

#define DATA_SAVE_DATA_TYPE_SAMPLE  1
#define DATA_SAVE_DATA_TYPE_ALARM   2
#define DATA_SAVE_DATA_TYPE_PARAM   3


#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_SAVE

static INT16U g_data_save_buff_write_index = 0;
static INT32U g_data_save_counter = 0;
static INT8U g_data_save_request_stopped = 0;
static INT8U g_data_save_status = kDataSaveStatusNormal;
static INT32U g_data_save_d_file_last_tick = 0;
static INT8U g_data_save_record_d_life = 0;

static INT8U g_data_save_status_is_changed = 0;
static INT8U g_data_save_fault_status_is_changed = 0;
static INT8U g_data_save_config_is_changed = 0;
static INT8U g_data_fault_changed_save_count = 0;

#pragma pop

static uint16_t data_save_save_d_file_append_custom(void);


static void data_save_buffer_write_byte(INT8U data)
{
    WRITE_LT_INT8U_WITH_BUFF_SIZE(g_data_save_buffer, g_data_save_buff_write_index, data, DATA_SAVE_MAX_LINE_SIZE);
}

static void data_save_buffer_write_word(INT16U data)
{
    WRITE_LT_INT16U_WITH_BUFF_SIZE(g_data_save_buffer, g_data_save_buff_write_index, data, DATA_SAVE_MAX_LINE_SIZE);
}

#if 0 //removed as no referenced
static void data_save_buffer_write_long(INT32U data)
{
    WRITE_LT_INT32U_WITH_BUFF_SIZE(g_data_save_buffer, g_data_save_buff_write_index, data, DATA_SAVE_MAX_LINE_SIZE);
}
#endif


static INT8U alarm_level_to_alarm_code(AlarmLevel lv) {
    switch(lv){
    case kAlarmFirstLevel: 
    case kAlarmFirstRelLevel: 
        return 1;
    case kAlarmSecondLevel: 
    case kAlarmSecondRelLevel: 
        return 2;
    case kAlarmThirdLevel: 
    case kAlarmThirdRelLevel: 
        return 3;
#if BMS_FORTH_ALARM_SUPPORT
    case kAlarmForthLevel: 
    case kAlarmForthRelLevel: 
        return 4;
#endif
    case kAlarmNone: 
    default: 
        return 0;
    }
}

/**
 * �洢������Ϣ
 */
static Result data_save_save_b_file(void) {
    struct AlarmRecordInfo {
        AlarmLevel (*get_state)(void);
        INT8U alarm_record_code;    // 1~31,��ֵ���������޸�,�������׷��
    };
    static const struct AlarmRecordInfo alarm_record_info_list[] = {
        { bcu_get_charge_state,             1},
        { bcu_get_discharge_state,          2},
        { bcu_get_low_soc_state,            3},
        { bcu_get_high_temperature_state,   4},
        { bcu_get_delta_temperature_state,  5},
        { bcu_get_chg_oc_state,             6},
        { bcu_get_dchg_oc_state,            7},
        { bcu_get_battery_insulation_state, 8},
        { bcu_get_slave_communication_state,9},
        { bcu_get_low_temperature_state,    10},
        { bcu_get_delta_voltage_state,      11},
        { bcu_get_temp_exception_state,     12},
        { bcu_get_voltage_exception_state,  13},
        { bcu_get_high_total_volt_state,    14},
        { bcu_get_low_total_volt_state,     15},
        { NULL}
    };

    struct AlarmRecordInfo *__FAR ptr;
    INT8U ret;
    INT8U list[sizeof(alarm_record_info_list)/sizeof(alarm_record_info_list[0])];
    INT8U index = 0;
    if(g_data_save_status_is_changed) {
        for(ptr = alarm_record_info_list; ptr->get_state; ptr++) {
            ret = alarm_level_to_alarm_code(ptr->get_state());
            if(ret) {
                list[index] = (ptr->alarm_record_code << 3) + ret;
                index++;
            }
        }
        storage_save(&record_storage_obj, DATA_SAVE_DATA_TYPE_ALARM, list, index);
        g_data_save_status_is_changed = 0;
    }
    return RES_OK;
}

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CONFIG
extern INT16U g_bms_config[kSysParamMaxCount];
#pragma pop

static Result data_save_save_c_file(void) {

    if(g_data_save_config_is_changed) {
        if(g_data_save_config_is_changed >= CONFIG_CHANGED_DELAY) {

            storage_save(&record_storage_obj, DATA_SAVE_DATA_TYPE_PARAM, (uint8_t *__FAR)g_bms_config, sizeof(g_bms_config));
            g_data_save_config_is_changed = 0;            
        } else {
            g_data_save_config_is_changed++;
        }
    }
    return RES_OK;
}

static Result data_save_save_d_file(void)
{
    Result res = 0;
    INT8U i, j, offset = 0, balance_temp = 0, balance_state = 0,ac_temp_num=0,dc_temp_num=0,relay_status=0;
    INT16U slave_num = 0, voltage_num = 0, record_type,reqcurrent;
    INT32U temp, tick = 0;
    INT16U interval = config_get(kSysStatusSaveIntervalIndex);
    RPAGE_INIT();
    
    tick = get_tick_count();
    // interval >>= 2;
    if(g_data_save_fault_status_is_changed == 1)
    {
        if(get_interval_by_tick(g_data_save_d_file_last_tick, tick) < 2000 )
        {
            return RES_OK;
        }
        else
        {
            g_data_fault_changed_save_count++;
            if(g_data_fault_changed_save_count > FAULT_CHANGED_DATA_SAVE_COUNT)
            {   
                g_data_fault_changed_save_count = 0;
                g_data_save_fault_status_is_changed = 0;
                return RES_OK;
            }
        }
    }
    else
    {
    if (get_interval_by_tick(g_data_save_d_file_last_tick, tick) < interval)
    {
        return RES_OK;
    }
    }
    
    g_data_save_d_file_last_tick = tick;
    
    RPAGE_RESET();
    g_data_save_buff_write_index = 0;
    
    //Life
    data_save_buffer_write_word(g_data_save_record_d_life++);
    
    //SOC
    data_save_buffer_write_byte(SOC_TO_PERCENT(bcu_get_SOC()));
    
    //SOH
    data_save_buffer_write_byte(SOC_TO_PERCENT(bcu_get_SOH()));
    
    //��ѹ
    data_save_buffer_write_word((INT16U) MV_TO_V(bcu_get_total_voltage()));

    //���� 0.1A/bit
    data_save_buffer_write_word(bcu_get_current());

    //Ǧ�ṩ���ѹ
    DATA_SAVE_SAFE_GET(temp, bat_det_voltage_get()); //DATA_SAVE_SAFE_GET(temp, bcu_get_lead_acid_volt());
    if(temp < BMS_SYSTEM_VOLTAGE_LOW_THRESHOLD) temp = 0;
    data_save_buffer_write_word((INT16U)temp);
    
    //���������ѹ
    DATA_SAVE_SAFE_GET(temp, dc_24V_voltage_get());
    if(temp < BMS_SYSTEM_VOLTAGE_LOW_THRESHOLD) temp = 0;
    data_save_buffer_write_word((INT16U)temp);
    
    //��Դ�����ź�
    DATA_SAVE_SAFE_GET(temp, bms_get_power_trigger_signal());
    data_save_buffer_write_byte((INT8U)temp);
    
    //�����¶�1
    DATA_SAVE_SAFE_GET(temp, TEMPERATURE_TO_C(board_temperature_get()));
    data_save_buffer_write_byte((INT8U)temp);
    
    //�����¶�2
    DATA_SAVE_SAFE_GET(temp, TEMPERATURE_TO_C(board_temperature2_get()));
    data_save_buffer_write_byte((INT8U)temp);
    
    //������Ե��ֵ
    data_save_buffer_write_word((INT16U) bcu_get_positive_insulation_resistance() / 10);

    //������Ե��ֵ
    data_save_buffer_write_word((INT16U) bcu_get_negative_insulation_resistance() / 10);

    //ϵͳ��Ե��ֵ
    data_save_buffer_write_word((INT16U) bcu_get_system_insulation_resistance() / 10);

    //������
    data_save_buffer_write_word(config_get(kCycleCntIndex));

    //�����������
    DATA_SAVE_SAFE_GET(temp, guobiao_charger_get_selfcheck_fault_num());
    data_save_buffer_write_byte((INT8U)temp);
       
    //��������
    DATA_SAVE_SAFE_GET(temp, bms_relay_diagnose_get_fault_num(kRelayTypeCharging));
    data_save_buffer_write_byte((INT8U)temp);
    
    //�ŵ������
    DATA_SAVE_SAFE_GET(temp, bms_relay_diagnose_get_fault_num(kRelayTypeDischarging));
    data_save_buffer_write_byte((INT8U)temp);
    
    //�ӻ�����
    slave_num = config_get(kSlaveNumIndex);
    data_save_buffer_write_byte((INT8U)slave_num);

    //�ӻ������
    for (i=0; i<slave_num; i++)
    {
        data_save_buffer_write_word(bmu_get_voltage_num(i));
    }
    //�ӻ��¸���
    for (i=0; i<slave_num; i++)
    {
        data_save_buffer_write_byte(bmu_get_temperature_num(i));
    }
    //����Ƭ�¶��� 
    for (i=0; i<slave_num; i++)
    {
        data_save_buffer_write_byte(bmu_get_heat_temperature_num(i));
    }
    //�����ѹ
    for (i=0; i<slave_num; i++)
    {
        for (j = 0; j < bmu_get_voltage_num(i); j++)
            data_save_buffer_write_word(bmu_get_voltage_item(i, j));
    }
    //����¶�
    for (i=0; i<slave_num; i++)
    {
        for (j = 0; j < bmu_get_temperature_num(i); j++) //�����¶�
        {
            temp = TEMPERATURE_TO_C(bmu_get_temperature_item(i, j));
            data_save_buffer_write_byte((INT8U)temp);
        }
    }
    //����Ƭ�¶�
    for (i=0; i<slave_num; i++)
    {
        for (j = 0; j < bmu_get_heat_temperature_num(i); j++) 
        {
            temp = TEMPERATURE_TO_C(bmu_get_heat_temperature_item(i, j));
            data_save_buffer_write_byte((INT8U)temp);
        }
    }
   
    //����״̬    
    offset = 0;balance_temp = 0;
    for (i=0; i<slave_num; i++)
    {
        voltage_num = (INT8U)bmu_get_voltage_num(i);
        for(j=0; j<voltage_num; j++)
        {
            if(j % 8 == 0)
            {
                balance_state = bmu_get_balance_state(i, j/8);
            }
            offset++;
            balance_temp >>= 1;
                
            if(balance_state & 0x01) 
            {
                balance_temp |= 0x80;
            }
            balance_state >>= 1;
            if(offset >0 && offset >= 8)
            {
                offset = 0;
                data_save_buffer_write_byte(balance_temp);
                balance_temp = 0;
            }
        }
    }
    if(offset)
    {
        for(; offset<8; offset++) balance_temp >>= 1;
        
        data_save_buffer_write_byte(balance_temp);
    }
     //  New AddBegin
    //�ӿؼ̵���״̬  
     for (i=0; i<slave_num; i++)
    {
      temp=bmu_get_relay_state(i);
      data_save_buffer_write_byte((INT8U)temp);
    }
    //����¶� 
    data_save_buffer_write_byte((INT8U)((TEMPERATURE_TO_C(bcu_get_high_temperature()))));
    //����¶�λ��
    DATA_SAVE_SAFE_GET(temp,bcu_get_high_temperature_id());
    data_save_buffer_write_word((INT16U)temp);
    //����¶�
    data_save_buffer_write_byte((INT8U)((TEMPERATURE_TO_C(bcu_get_low_temperature()))));
    //����¶�λ�� 
    DATA_SAVE_SAFE_GET(temp,bcu_get_low_temperature_id());
    data_save_buffer_write_word((INT16U)temp);
    //��ߵ�ѹ
    data_save_buffer_write_word(bcu_get_high_voltage());
    //��ߵ�ѹλ��
    DATA_SAVE_SAFE_GET(temp,bcu_get_high_voltage_id());
    data_save_buffer_write_word((INT16U)temp);
    //��͵�ѹ
    data_save_buffer_write_word(bcu_get_low_voltage());
    //��͵�ѹλ�� 
    DATA_SAVE_SAFE_GET(temp,bcu_get_low_voltage_id());
    data_save_buffer_write_word((INT16U)temp);
    //��ʱ�ŵ�����������
    data_save_buffer_write_word(bms_get_discharge_current_max());
    //�����ŵ�����������
    data_save_buffer_write_word(bms_get_discharge_continue_current_max());
    //��ʱ�������������
    data_save_buffer_write_word(bms_get_charge_current_max());
    //�����������������
    data_save_buffer_write_word(bms_get_charge_continue_current_max());
    //�ܸ� Ԥ�� �ŵ� ��� ���� ���� ���� �ϲ���1���ֽ� Ԥ��λ
    relay_status |= (relay_control_is_on(kRelayTypeNegative)==1)?0x80:0;
    relay_status |= (relay_control_is_on(kRelayTypePreCharging)==1)?0x40:0;		 
    relay_status |= (relay_control_is_on(kRelayTypeDischarging)==1)?0x20:0;			  
    relay_status |= (relay_control_is_on(kRelayTypeCharging)==1)?0x10:0;							
    relay_status |= (relay_control_is_on(kRelayTypePositive)==1)?0x08:0;
    relay_status |= (relay_control_is_on(kRelayTypeHeating)==1)?0x04:0;
    relay_status |= (relay_control_is_on(kRelayTypeCooling)==1)?0x02:0;
     data_save_buffer_write_byte(relay_status);
    //�������״̬ ����CC��CP��CC2
     data_save_buffer_write_byte(guobiao_charger_cc2_is_connected()<<2|
                                 guobiao_charger_cc_is_connected()<<1|
                                 guobiao_charger_pwm_is_connected()); 
     //���������¸и���
     ac_temp_num=bms_get_chgr_ac_outlet_temperature_num();
     data_save_buffer_write_byte(ac_temp_num);
     //���������¶�
    for (i=0; i<ac_temp_num; i++)
    {
            temp = TEMPERATURE_TO_C(bms_get_chgr_ac_outlet_temperature(i));
            data_save_buffer_write_byte((INT8U)temp);
        
    }
   //�����������
     dc_temp_num=bms_get_chgr_dc_outlet_temperature_num();
     data_save_buffer_write_byte(dc_temp_num);
    //��������¶�
    for (i=0; i<dc_temp_num; i++)
    {
            temp = TEMPERATURE_TO_C(bms_get_chgr_dc_outlet_temperature(i));
            data_save_buffer_write_byte((INT8U)temp);
        
    }  
   
     //�ۼƳ�����
    data_save_buffer_write_word(config_get(kChgTotalCapLowIndex));
    data_save_buffer_write_word(config_get(kChgTotalCapHighIndex));
    //�ۼƷŵ����
    data_save_buffer_write_word(config_get(kDChgTotalCapLowIndex));
    data_save_buffer_write_word(config_get(kDChgTotalCapHighIndex)); 
    //ƽ����ѹ
    data_save_buffer_write_word(bcu_get_average_voltage());
    //�ŵ�����
    data_save_buffer_write_word(config_get(kSOC0DischgFactorIndex));
    //���ĸ�ߵ�ѹ��Ԥ��˵�ѹ(HV1)
    data_save_buffer_write_word((INT16U)BMS_PRECHARGE_TV_ON_MCU);
    //���ȵ���
    data_save_buffer_write_word(bcu_get_current1());
    //����������
    charger_get_current(&reqcurrent);
    data_save_buffer_write_word(reqcurrent);
    //����Ƭ����״̬
    data_save_buffer_write_word(rule_stock_get_var(CHARGER_HEAT_FAULT_BUFF_INDEX));
    //Ӳ������״̬
    data_save_buffer_write_word((INT16U)bcu_get_sys_exception_flags());
    //�̵���ճ��״̬
    data_save_buffer_write_byte(0);  //�õ�����������ӵ��˴��洢��NAND FLASH��
    //��ѹ�����ź�
    data_save_buffer_write_byte(0);  //��⵽���źſ����ӵ��˴��洢��nand flash��
    /* ******************************����*********************************************** */
    //����״̬������
    data_save_buffer_write_word((INT16U)charger_get_control_bits()); 
    data_save_buffer_write_word((INT16U)(charger_get_control_bits()>>16)); 
    //����ͨ��״̬ 
    data_save_buffer_write_byte((INT8U)guobiao_charger_get_CommStage()); 
    //Ԥ��״̬
    data_save_buffer_write_byte((INT8U)bcu_get_precharge_with_tv_state());
    //Ԥ�����
    data_save_buffer_write_byte(bms_get_precharge_cnt());
    //��Ե��ѹ
    data_save_buffer_write_word((INT16U)MV_TO_100MV(bcu_get_insulation_total_voltage()));
    //HV2
    data_save_buffer_write_word((INT16U)MV_TO_100MV(bcu_get_insulation_pch_total_voltage()));
    //HV3
    data_save_buffer_write_word((INT16U)MV_TO_100MV(bcu_get_insulation_hv3_total_voltage()));
    //������
    data_save_buffer_write_word(config_get(kTotalCapIndex));
    //ʣ������
    data_save_buffer_write_word(config_get(kLeftCapIndex)); 
    /* ********************************************************************************* */
    //�����������
    for (i=0; i<slave_num; i++)
    {
        data_save_buffer_write_byte((INT8U)bmu_get_balance_current_item(i,0));
        data_save_buffer_write_byte((INT8U)bmu_get_balance_current_item(i,1));
    }
    //ƽ���¶�
    data_save_buffer_write_byte((INT8U)((TEMPERATURE_TO_C(bcu_get_average_temperature()))));
    //New AddEnd   
    record_type = data_save_save_d_file_append_custom();

    // �ٴ洢һ����¼
    storage_save(&record_storage_obj, record_type, g_data_save_buffer, g_data_save_buff_write_index);
    
    g_data_save_buff_write_index = 0;
    
    g_data_save_counter++;    
    return RES_OK;
}

/**
 * ׷���Զ������ݵ�D���¼
 * @return  ���ؼ�¼����,����ʹ�þ������ö��������ֵ
 * @note �ɹ�ʹ�õĺ�������:
 *       data_save_buffer_write_byte
 *       data_save_buffer_write_word
 *       data_save_buffer_write_long
 */
static uint16_t data_save_save_d_file_append_custom(void) {

    return DATA_SAVE_DATA_TYPE_SAMPLE;
}

static void data_save_save(void)
{
    /** check the disk information */
    data_save_save_b_file();
    data_save_save_c_file();
    data_save_save_d_file();
}

static void private_start(void) {
    g_data_save_request_stopped = 0;
}

void set_data_save_request_stopped(INT8U request_stop) 
{
  if(g_data_save_request_stopped == request_stop) return;  
  g_data_save_request_stopped = request_stop;
}

static void private_stop(void) {
    //g_data_save_request_stopped = 1;
    storage_flush(&record_storage_obj);
}

static INT8U private_is_stop(void) {
    if(g_data_save_request_stopped == 2)
        return TRUE;
    
    return FALSE;
}

static DataSaveStatus private_get_status(void) {
    return g_data_save_status;
}

static INT32U private_get_counter(void) {
    return g_data_save_counter;
}

static INT8U private_is_fault(void) {
    return (g_data_save_status == kDataSaveStatusNotAvaliable);
}

static Result data_save_on_status_changed(EventTypeId event_id, void* event_data, void* user_data)
{
    (void)event_id;
    (void)event_data; 
    (void)user_data;
    g_data_save_status_is_changed = 1;
    g_data_save_fault_status_is_changed = 1;
    return RES_OK;
}
static Result data_save_on_config_changing(ConfigIndex index, INT16U new_value)
{       
    (void)index;     
    (void)new_value;
    g_data_save_config_is_changed = 1;
    return RES_OK;
}

static void private_init(void) {
    static const InfoEventType alarm_event_list[] = {
        kInfoEventBatteryChargeState,
        kInfoEventBatteryDischargeState,
        kInfoEventBatterySocState,
        kInfoEventBatteryHighTemperatureState,
        kInfoEventBatteryDifferenceTemperatureState,
        kInfoEventBatteryOverChargeCurrentState,
        kInfoEventBatteryOverDischargeCurrentState,
        kInfoEventBatteryInsulationState,
        kInfoEventSlaveCommunicationState,
        kInfoEventBatteryLowTemperatureState,
        kInfoEventDifferenceVoltageState,
        kInfoEventTempExceptionState,
        kInfoEventVoltageExceptionState,
        kInfoEventTotalVoltageState,
    };

    INT8U i;
    g_data_save_rpage = (INT8U)((INT32U)g_data_save_buffer / 0x10000);
    RPAGE_RESET();
    for (i = 0; i < (sizeof(alarm_event_list) / sizeof(alarm_event_list[0])); i++) {
        event_observe((EventTypeId)alarm_event_list[i], data_save_on_status_changed, NULL);
    }

    config_register_observer(kEepromParamStart, kEepromParamEnd, data_save_on_config_changing);

    /**
     * NandFlash ��ʼ��ʧ��,���ܵ�ԭ����:
     * 1) spiͨ��ʧ��
     * 2) ��֧�ֵ�ǰ�洢оƬ,��Ҫ�ڴ�������Ӷ�Ӧ���豸ID�Լ��洢����
     */
    if(spi_flash_init(&nand)){
        g_data_save_status = kDataSaveStatusNotAvaliable;
    }
}

static void private_uninit(void) {
    if(g_data_save_status == kDataSaveStatusNotAvaliable){
        return;
    }
    storage_deinit(&record_storage_obj);
}
static INT32U g_bcu_data_save_start_tick = 0;
static INT8U g_data_save_tick_flag = 0;
static void private_task(void *pdata) 
{
    INT16U res = -1;
    INT8U  tdata;
    (void)pdata;
    
    g_bcu_data_save_start_tick = get_tick_count();
    
    //sleep(500);//sleep(3000);
    // �ڳ�ʼ���洢�ṹ��
    if(g_data_save_status == kDataSaveStatusNormal) {
        res = storage_init(&record_storage_obj);
    }
    
    for (;;)
    {
        if((g_data_save_tick_flag==0) && (get_interval_by_tick(g_bcu_data_save_start_tick, get_tick_count()) >= 3000))
        {  
            g_data_save_tick_flag = 1;
        }
        charger_outlet_temperature_update(&tdata);
        
        if((g_data_save_tick_flag)&&(mode != RUN_MODE_READ))
        {
            if (res != 0) //��ʼ��ʧ��
            {
                g_data_save_status = kDataSaveStatusNotAvaliable;
            }
            else if (g_data_save_request_stopped)
            {
                data_save_stop();
                g_data_save_status = kDataSaveStatusStopped;
                g_data_save_request_stopped = 2;
            }
            else
            {
                g_data_save_status = kDataSaveStatusNormal;
            }

        /** ��¼���� */
            if (kDataSaveStatusNormal == g_data_save_status)
            {
                data_save_save();
            //DEBUG("sdcard", "data_save_save");
            }
        }
        else 
        {
            if((mode == RUN_MODE_READ) && (bms_get_nandflash_scan_require() == 1)) 
            {
                page_addr_extract(&record_storage_obj, g_start_date, g_end_date);
    
                g_data_scan_require = 0;     
            }        
        }

        sleep(DATA_SAVE_CHECK_SAVE_PERIODIC);
    }    
}

const struct DataSaveHandler data_save_handler_nandflash = {
    private_task,
    private_init,
    private_uninit,
    private_start,
    private_stop,
    private_is_stop,
    private_get_status,
    private_get_counter,
    private_is_fault,
};
