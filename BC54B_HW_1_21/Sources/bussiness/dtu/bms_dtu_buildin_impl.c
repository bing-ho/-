#include "bms_dtu_buildin_impl.h"
#include "bms_bmu.h"
#include "bms_bcu.h"
#include "bms_config.h"
#include "bms_buffer.h"
#include "bms_charger_temperature.h"

#include "bms_util.h"
#define memset safe_memset
#define memcmp safe_memcmp
#define memcpy safe_memcpy
#define strcpy safe_strcpy
#define strlen safe_strlen
#define strcmp safe_strcmp

#define DTU_CAN_BMU_BAL_CUR_NUM     2
#define DTU_CAN_BMU_FRAME_SIZE_POS  1

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU

can_t g_dtu_self_check_context;
CanInfo g_dtu_self_check_can_info;
CanMessage g_dtu_self_check_message;

#define ARRAY_SIZE(a) ((sizeof(a))/sizeof(a[0]))
/// 计算结构体中成员变量的便宜量的宏.
#define STRUCT_OFFSET(__struct, __menmber) ((int)(&(((__struct *)0)->__menmber)))


//static unsigned short short_host_to_transfer(unsigned short dat) {
//    return (dat >> 8) | ((dat & 0x00ff)<< 8);
//}

//unsigned char mPackDataBuffer[512 + 196];

#pragma DATA_SEG DEFAULT

void dtu_self_check_init(void);

static INT8U dtu_insu_state_to_data(void) {
    InsulationWorkState state = bcu_get_insulation_work_state();
    switch (state) {
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
#if 0
/// 参考"BMS与DTU无限传输CAN协议."
typedef struct {
    unsigned char version;
    unsigned short uuid[8];
    unsigned char slave_num;
    unsigned char soc;
    unsigned char reserved_status;
    union {
        unsigned short combined_status;
        struct {
            unsigned short is_total_voltage_too_low: 1;
            unsigned short is_charging: 1;
            unsigned short is_battery_insulation_state_err: 1;
            unsigned short is_slave_commulication_err: 1;
            unsigned short is_over_temperature: 1;
            unsigned short is_over_discharge: 1;
            unsigned short is_over_charge: 1;
            unsigned short is_soc_too_low: 1;

            unsigned short is_soc_too_high: 1;
            unsigned short is_balance_active: 1;
            unsigned short is_over_current: 1;
            unsigned short is_delta_temperature_too_large: 1;
            unsigned short is_delta_voltage_too_large: 1;
            unsigned short is_voltage_detect_execption: 1;
            unsigned short is_temperature_detect_execption: 1;
            unsigned short is_total_voltage_too_high: 1;
        } bits;
    } status;
    unsigned short total_battery_cap;
    unsigned short left_battery_cap;
    unsigned short total_battery_voltage;
    unsigned short total_battery_current;
    unsigned short battery_charge_cycle;
    unsigned short positive_insulation_resistance;
    unsigned short negative_insulation_resistance;
    unsigned short total_insulation_resistance;
    unsigned char insulation_state;
    unsigned char soh;
    unsigned short highest_unit_voltage;
    unsigned short lowest_unit_voltage;
    unsigned char highest_unit_temperature;
    unsigned char lowest_unit_temprature;
    unsigned char highest_unit_voltage_box_id;
    unsigned char highest_unit_voltage_unit_id;
    unsigned char lowest_unit_voltage_box_id;
    unsigned char lowest_unit_voltage_unit_id;
    unsigned char highest_unit_temperature_box_id;
    unsigned char highest_unit_temperature_unit_id;
    unsigned char lowest_unit_temperature_box_id;
    unsigned char lowest_unit_temperature_unit_id;
    unsigned short crc;
} bcu_data_t;

void pack_bcu_data(bcu_data_t *dat) {
    static const ConfigIndex key_uuids[] = {
        kUUIDPart1,
        kUUIDPart2,
        kUUIDPart3,
        kUUIDPart4,
        kUUIDPart5,
        kUUIDPart6,
        kUUIDPart7,
        kUUIDPart8,
    }

    INT16U i;
    INT16U crc;
    INIT_WRITE_BUFFFER();

    dat->version = DTU_CAN_PROTOCOL_VERSION;
    for (i = 0; i < ARRAY_SIZE(dat->uuid); ++i) {
        dat->uuid[i] = short_host_to_transfer(config_get(key_uuids[i]));
    }
    dat->slave_num = config_get(kSlaveNumIndex);
    dat->soc = SOC_TO_INT8U(bcu_get_SOC());
    dat->reserved_status = 0;
    dat->status.combined_status = 0;
    if (charger_is_charging() && charger_is_connected()) {
        dat->status.bits.is_charging = 1;
    }
    if (bcu_get_battery_insulation_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_battery_insulation_state_err = 1;
    }
    if (bcu_get_slave_communication_state() != kAlarmNone) {
        dat->status.bits.is_slave_commulication_err = 1;
    }
    if (bcu_get_high_temperature_state() >= kAlarmSecondLevel || bcu_get_low_temperature_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_over_temperature = 1;
    }
    if (bcu_get_discharge_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_over_discharge = 1;
    }
    if (bcu_get_charge_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_over_charge = 1;
    }
    if (bcu_get_low_soc_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_soc_too_low = 1;
    }
    if (bcu_get_high_soc_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_soc_too_high = 1;
    }
    if (bmu_is_balance_state_actived() == RES_TRUE) {
        dat->status.bits.is_balance_active = 1;
    }
    if (bcu_get_chg_oc_state() >= kAlarmSecondLevel || bcu_get_dchg_oc_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_over_current = 1;
    }
    if (bcu_get_delta_temperature_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_delta_voltage_too_large = 1;
    }
    if (bcu_get_delta_voltage_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_delta_temperature_too_large = 1;
    }
    if (bcu_get_voltage_exception_state() != kAlarmNone) {
        dat->status.bits.is_voltage_detect_execption = 1;
    }
    if (bcu_get_temp_exception_state() != kAlarmNone) {
        dat->status.bits.is_temperature_detect_execption = 1;
    }
    if (bcu_get_high_total_volt_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_total_voltage_too_high = 1;
    } else if (bcu_get_low_total_volt_state() >= kAlarmSecondLevel) {
        dat->status.bits.is_total_voltage_too_low = 1;
    }

    dat->total_battery_cap = short_host_to_transfer(config_get(kTotalCapIndex));
    dat->left_battery_cap = short_host_to_transfer(config_get(kLeftCapIndex));
    dat->total_battery_voltage = short_host_to_transfer((INT16U)MV_TO_V(bcu_get_total_voltage()));
    dat->total_battery_current = short_host_to_transfer(bcu_get_current() + DTU_CURRENT_OFFSET);
    dat->battery_charge_cycle = short_host_to_transfer(config_get(kCycleCntIndex));
    dat->positive_insulation_resistance = short_host_to_transfer(bcu_get_positive_insulation_resistance());
    dat->negative_insulation_resistance = short_host_to_transfer(bcu_get_negative_insulation_resistance());
    dat->total_insulation_resistance = short_host_to_transfer(bcu_get_system_insulation_resistance());
    dat->insulation_state = dtu_insu_state_to_data();
    dat->soh = SOH_TO_INT8U(bcu_get_SOH());
    dat->highest_unit_voltage = short_host_to_transfer(bcu_get_high_voltage());
    dat->lowest_unit_voltage = short_host_to_transfer(bcu_get_low_voltage());
    dat->highest_unit_temperature = TEMP_TO_40_OFFSET((INT8U)bcu_get_high_temperature())
                                    dat->lowest_unit_temprature = TEMP_4_DISPLAY(TEMP_TO_40_OFFSET((INT8U)bcu_get_low_temperature()));
    i = bcu_get_high_voltage_id();
    dat->highest_unit_voltage_box_id = i >> 8;
    dat->highest_unit_voltage_unit_id = i;
    i = bcu_get_low_voltage_id();
    dat->lowest_unit_voltage_box_id = i >> 8;
    dat->lowest_unit_voltage_unit_id = i;
    i = bcu_get_high_temperature_id();
    dat->highest_unit_temperature_box_id = i >> 8;
    dat->highest_unit_temperature_unit_id = i;
    i = bcu_get_low_temperature_id();
    dat->lowest_unit_temperature_box_id = i >> 8;
    dat->lowest_unit_temperature_unit_id = i;

    dat->crc = short_host_to_transfer(crc_check((unsigned char *)buf, STRUCT_OFFSET(bcu_data_t, crc)));
}

#else
unsigned short pack_bcu_data(unsigned char *__FAR buf, unsigned short bufLen) {
    INT16U index = 0;
    INT32U temp = 0;
    INT16U crc;
    INIT_WRITE_BUFFFER();

    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, DTU_CAN_PROTOCOL_VERSION, bufLen);
    //WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, DTU_CAN_MASTER_NUM, bufLen);//主机个数
    //WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, bufLen);//长度
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kUUIDPart1), bufLen);//UUID 1
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kUUIDPart2), bufLen);//UUID 2
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kUUIDPart3), bufLen);//UUID 3
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kUUIDPart4), bufLen);//UUID 4
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kUUIDPart5), bufLen);//UUID 5
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kUUIDPart6), bufLen);//UUID 6
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kUUIDPart7), bufLen);//UUID 7
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kUUIDPart8), bufLen);//UUID 8
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, (INT8U)(config_get(kSlaveNumIndex)), bufLen);//从机个数
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, SOC_TO_INT8U(bcu_get_SOC()), bufLen);//SOC

    if (charger_is_connected()) {
        temp |= 0x01;
    }
    if (bcu_get_battery_insulation_state() >= kAlarmSecondLevel) {
        temp |= 0x02;
    }
    if (bcu_get_slave_communication_state() != kAlarmNone) {
        temp |= 0x04;
    }
    if (bcu_get_high_temperature_state() >= kAlarmSecondLevel || bcu_get_low_temperature_state() >= kAlarmSecondLevel) {
        temp |= 0x08;
    }
    if (bcu_get_discharge_state() >= kAlarmSecondLevel) {
        temp |= 0x10;
    }
    if (bcu_get_charge_state() >= kAlarmSecondLevel) {
        temp |= 0x20;
    }
    if (bcu_get_low_soc_state() >= kAlarmSecondLevel) {
        temp |= 0x40;
    }
    if (bcu_get_high_soc_state() >= kAlarmSecondLevel) {
        temp |= 0x80;
    }
    if (bmu_is_balance_state_actived() == RES_TRUE) {
        temp |= 0x100;
    }
    if (bcu_get_chg_oc_state() >= kAlarmSecondLevel || bcu_get_dchg_oc_state() >= kAlarmSecondLevel) {
        temp |= 0x200;
    }
    if (bcu_get_delta_temperature_state() >= kAlarmSecondLevel) {
        temp |= 0x400;
    }
    if (bcu_get_delta_voltage_state() >= kAlarmSecondLevel) {
        temp |= 0x800;
    }
    if (bcu_get_voltage_exception_state() != kAlarmNone) {
        temp |= 0x1000;
    }
    if (bcu_get_temp_exception_state() != kAlarmNone) {
        temp |= 0x2000;
    }
    if (bcu_get_high_total_volt_state() >= kAlarmSecondLevel) {
        temp |= 0x4000;
    } else if (bcu_get_low_total_volt_state() >= kAlarmSecondLevel) {
        temp |= 0x8000;
    }
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, (INT8U)(temp >> 16), bufLen);
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, (INT16U)temp, bufLen);//电池状态
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kTotalCapIndex), bufLen);//总容量
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kLeftCapIndex), bufLen);//剩余容量
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, (INT16U)MV_TO_V(bcu_get_total_voltage()), bufLen);//总压
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, bcu_get_current() + DTU_CURRENT_OFFSET, bufLen);//电流
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kCycleCntIndex), bufLen);//电池组充放电次数
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, bcu_get_positive_insulation_resistance(), bufLen);//绝缘正极电阻
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, bcu_get_negative_insulation_resistance(), bufLen);//绝缘负极电阻
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, bcu_get_system_insulation_resistance(), bufLen);//电池组绝缘电阻
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, dtu_insu_state_to_data(), bufLen);//绝缘状态
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, SOH_TO_INT8U(bcu_get_SOH()), bufLen);//SOH
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, bcu_get_high_voltage(), bufLen);//最高单体电压
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, VOLT_4_DISPLAY(bcu_get_low_voltage()), bufLen);//最低单体电压
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, TEMP_TO_40_OFFSET((INT8U)bcu_get_high_temperature()), bufLen);//最高单体温度
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, TEMP_4_DISPLAY(TEMP_TO_40_OFFSET((INT8U)bcu_get_low_temperature())), bufLen);//最低单体温度
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, bcu_get_high_voltage_id() >> 8, bufLen);//最高单体电压箱体号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, (INT8U)bcu_get_high_voltage_id(), bufLen);//最高单体电压串号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, bcu_get_low_voltage_id() >> 8, bufLen);//最低单体电压箱体号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, (INT8U)bcu_get_low_voltage_id(), bufLen);//最低单体电压串号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, bcu_get_high_temperature_id() >> 8, bufLen);//最高单体温度箱体号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, (INT8U)bcu_get_high_temperature_id(), bufLen);//最高单体温度串号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, bcu_get_low_temperature_id() >> 8, bufLen);//最低单体温度箱体号
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, (INT8U)bcu_get_low_temperature_id(), bufLen);//最低单体温度串号

    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, TEMP_TO_40_OFFSET(bms_get_chgr_dc_outlet_temperature(0)), bufLen);//快充温度1
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, TEMP_TO_40_OFFSET(bms_get_chgr_dc_outlet_temperature(1)), bufLen);//快充温度2
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, TEMP_TO_40_OFFSET(bms_get_chgr_ac_outlet_temperature(0)), bufLen);//慢充温度1
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, TEMP_TO_40_OFFSET(bms_get_chgr_ac_outlet_temperature(1)), bufLen);//慢充温度2
    
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kNominalCapIndex), bufLen);//标称容量
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgTotalCapLowIndex), bufLen);//累计充电电量
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgTotalCapHighIndex), bufLen);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgTotalCapLowIndex), bufLen);//累计放电电量
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgTotalCapHighIndex), bufLen);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kSOC0DischgFactorIndex), bufLen);//放电因子                             
  
    //for(i=index; i<9; i++)
    //{
    //    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, 0xFF, bufLen);//保证使用多帧传输
    //}
    //pos = DTU_CAN_BCU_FRAME_SIZE_POS;
    //WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, pos, index, bufLen);//主机数据长度
    crc = crc_check(buf, index);
    WRITE_BT_INT16U_WITH_BUFF_SIZE(buf, index, crc, bufLen);//CRC16

    return index;
}
#endif

unsigned short pack_bmu_data(unsigned char bmuId, unsigned char *__FAR buf, unsigned short bufLen) {
    INT8U i, index = 0, pos;
    INT8U bat_num, temp_num;
    INT16U crc;
    INT32U temp;
    INIT_WRITE_BUFFFER();

    if (config_get(kSlaveNumIndex) <= bmuId) {
        return RES_ERR;
    }

    bat_num = (INT8U)bmu_get_voltage_num(bmuId);
    temp_num = bmu_get_temperature_num(bmuId);

    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, bmuId + 1, bufLen); //从机编号
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, bufLen);//从机数据长度
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, bat_num, bufLen);//监控电池数目
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, temp_num, bufLen);//监控温感数目
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, DTU_CAN_BMU_BAL_CUR_NUM, bufLen);//监控均衡电流数目

    for (i = 0; i < bat_num; i++) {
        WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, bmu_get_voltage_item(bmuId, i), bufLen);    //单体电压
    }
    for (i = 0; i < temp_num; i++) {
        WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, TEMP_TO_40_OFFSET(bmu_get_temperature_item(bmuId, i)), bufLen);    //单体温度
    }

    temp = 0;
    if (bmu_get_heat_status(bmuId)) {
        temp |= 0x01;
    }
    if (bmu_get_cool_status(bmuId)) {
        temp |= 0x02;
    }
    WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, (INT8U)temp, bufLen);//热管理状态

    for (i = 0; i < DTU_CAN_BMU_BAL_CUR_NUM; i++) {
        WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, bufLen);    //均衡电流
    }

    for (i = 0; i < (bat_num + 7) / 8; i++) {
        WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, bmu_get_balance_state(bmuId, i), bufLen);    //均衡状态
    }

    for (i = index; i < 9; i++) {
        WRITE_BT_INT8U_WITH_BUFF_SIZE(buf, index, 0xFF, bufLen);//保证使用多帧传输
    }
    pos = DTU_CAN_BMU_FRAME_SIZE_POS;
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, pos, index, bufLen);//从机数据长度

    crc = crc_check(buf, index);
    WRITE_BT_INT16U_WITH_BUFF_SIZE(buf, index, crc, bufLen);//CRC16
    return index;
}


#include "dtu_m35_impl.h"


struct sent_data_private {
    INT16U data_size_once;
    INT16U got_byte_index;
    INT16U packed_byte_size;
    INT8U next_id;
    INT8U buffer[200];
    INT8U sending_timeout_count_down;
};


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU

static struct sent_data_private sent_data_private;


unsigned short pack_data(unsigned char Id, unsigned char *__FAR buf, unsigned short bufLen) {
    INT16U data_length;
    INT16U header_length = DTUM35_ProtocolGetHeaderLength();
    if (Id == 0) { // BCU
        data_length = pack_bcu_data(&buf[header_length], bufLen - header_length);
        DTUM35_ProtocolGetBCUHeader(buf, data_length);
    } else { // BMU
        data_length = pack_bmu_data(Id-1, &buf[header_length], bufLen - header_length);
        DTUM35_ProtocolGetBMUHeader(buf, data_length);
    }
    return data_length + header_length;
}


// 计算一次发送的数据的总长度.
static void calculate_onetime_data_size(void) {
    INT8U id;
    sent_data_private.data_size_once = 0;    
    for (id = 0; id <= config_get(kSlaveNumIndex); ++id) {
        sent_data_private.data_size_once += pack_data(id, sent_data_private.buffer, sizeof(sent_data_private.buffer));
    }
}


INT16U get_data(void *__FAR private_data, char *buf, INT16U len) {
    (void)private_data;
    if (sent_data_private.got_byte_index >= sent_data_private.packed_byte_size) {
        sent_data_private.got_byte_index = 0;
        sent_data_private.packed_byte_size = pack_data(sent_data_private.next_id, sent_data_private.buffer, sizeof(sent_data_private.buffer));
        ++sent_data_private.next_id;
    }
    
    if (len > sent_data_private.packed_byte_size - sent_data_private.got_byte_index) {
        len = sent_data_private.packed_byte_size - sent_data_private.got_byte_index;
    }
    
    safe_memcpy((unsigned char *)buf, &sent_data_private.buffer[sent_data_private.got_byte_index], len);
    sent_data_private.got_byte_index += len;
    return len;
}

void finished_sent(char is_ok) {
    OS_CPU_SR cpu_sr = 0;
    (void)is_ok;        
    OS_ENTER_CRITICAL();
    sent_data_private.sending_timeout_count_down = 0;
    OS_EXIT_CRITICAL();
}

static void send_data(void *nouse) {
    static INT32U lastSendTick = 0;
    struct DTUM35_SendDataInfo sendinfo;
    
    
    INT32U howlong;
    INT32U interval = (INT32U)config_get(kDTUSendIntervalIndex) * 1000UL; 
    
    (void)nouse;

    howlong = get_interval_by_tick(lastSendTick, get_tick_count());

    if (howlong <= interval) {
        return;
    }

    if (howlong > (interval * 3 / 2)) {
        lastSendTick += interval * 2;
    } else {
        lastSendTick += interval;
    }

    if (sent_data_private.sending_timeout_count_down > 0) {
        if (--sent_data_private.sending_timeout_count_down == 0) {
            finished_sent(0);
        }
        return;
    }
    
    calculate_onetime_data_size();     
    sendinfo.finished = finished_sent;
    sendinfo.private_data = &sent_data_private;
    sendinfo.get_data = get_data;
    sendinfo.len = sent_data_private.data_size_once;
    sent_data_private.got_byte_index = 0;
    sent_data_private.packed_byte_size = 0;
    sent_data_private.next_id = 0;    
    if (DTUM35_SendDataWithCallback(&sendinfo)) {
        sent_data_private.sending_timeout_count_down = 5;
    }
}


void dtu_init(void) {

    unsigned char type;
    
    if(config_get(kBCUCommModeIndex) == BMS_SELF_CHECK_MODE)
    {
        dtu_self_check_init();
    }
    
    if(input_signal_is_high_by_name("GSM_ID"))
    {
        return;
    }
    
    // 默认DTU类型
    if(input_signal_is_high_by_name("GSM_ID"))
    {
        type = 0; // 无DTU
    } 
    else 
    {
        type = 1; // 板载DTU   
    }
    // 获取配置的DTU类型
    config_get_dtu_type(&type);     
    
    if (type == 1) {      
        DTUM35_Init(&g_DTUM35_BspInterface, 0);
        job_schedule(MAIN_JOB_GROUP, DTU_SEND_DATA_JOB_PERIODIC, send_data, NULL);
    }
}

// self check function
void dtu_task_self_check_message(void* pdata)
{
    
    UNUSED(pdata);
    g_dtu_self_check_message.data[0] = 0x10;
    g_dtu_self_check_message.data[1] = 0x01;
    g_dtu_self_check_message.data[2] = 0x02;
    g_dtu_self_check_message.data[3] = 0x03;
    g_dtu_self_check_message.data[4] = 0x04;   
    g_dtu_self_check_message.data[5] = 0x05;
    g_dtu_self_check_message.data[6] = 0x06;
    g_dtu_self_check_message.data[7] = 0x07;

    g_dtu_self_check_message.id.value = can_id_from_extend_id(SELF_CHECK_CHARGER_RECEIVE_ID);
    g_dtu_self_check_message.len = CAN_DATA_MAX_LEN;
    can_send(g_dtu_self_check_context, &g_dtu_self_check_message);
}

void dtu_self_check_init(void)
{
    /** init the information */
    safe_memset(&g_dtu_self_check_can_info, 0, sizeof(g_dtu_self_check_can_info));
    g_dtu_self_check_can_info.dev = (INT8U)DTU_CAN_DEV;//EMS_CAN_DEV;  //config_get(kDtuCanChannelIndex);
    //if(g_dtu_self_check_can_info.dev >= kCanDev3) g_dtu_self_check_can_info.dev = kCanDev4;
    
    if(g_dtu_self_check_can_info.dev == kCanDev3) g_dtu_self_check_can_info.bps = can_get_bps((INT8U)config_get(kCAN5BpsIndex));
    else g_dtu_self_check_can_info.bps = can_get_bps((INT8U)config_get(kCAN1BpsIndex + g_dtu_self_check_can_info.dev));
	
    g_dtu_self_check_can_info.receive_id = 0;
    g_dtu_self_check_can_info.mask_id = 0;
    g_dtu_self_check_can_info.mode = DTU_CAN_MODE;
    g_dtu_self_check_can_info.buffers = NULL;
    g_dtu_self_check_can_info.buffer_count = 0;
    g_dtu_self_check_can_info.filtemode=CanFilte32;
    g_dtu_self_check_context = can_init(&g_dtu_self_check_can_info);
    job_schedule(MAIN_JOB_GROUP, 200, dtu_task_self_check_message, NULL);
}

