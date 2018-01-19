/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_hmi_impl.h
* @brief
* @note
* @author
* @date 2012-5-12
*
*/

#ifndef BMS_HMI_IMPL_H_
#define BMS_HMI_IMPL_H_

#include "bms_hmi.h"

/**********************************************
 *
 * Definition
 *
 ***********************************************/
#define HMI_COIL_LEN_NAX    2000
#define HMI_COIL_LEN_NIN    1

#define HMI_HOLDING_REGISTER_MAX_LEN 32
#define HMI_HOLDING_REGISTER_MIN_LEN 1

#define HMI_INPUT_REGISTER_MAX_LEN 36
#define HMI_INPUT_REGISTER_MIN_LEN 1

#define HMI_INPUT_REIGSTER_TOTAL_MAX_ADDRESS 100
#define HMI_INPUT_REIGSTER_ENTRY_MAX_ADDRESS 5002
#define HMI_INPUT_REIGSTER_ENTRY_MAX_LENGTH 18
#define HMI_INPUT_REIGSTER_ENTRY_MIN_LENGTH 1

//电池组总体状态
#define HMI_BATTERYS_STATE_ADDRESS  0
#define HMI_BATTERYS_STATE_LENGTH   8

// sdcard状态
#define HMI_SDCARD_STATE_ADDRESS    16
#define HMI_SDCARD_STATE_LENGTH     8

//登录报警消息
#define HMI_LOGIN_WARNING_ADDRESS  32
#define HMI_LOGIN_WARNING_LENGTH   24

#define HMI_COLIS_SDCARD_REQUEST_UNPLUG 1

//充电器信息
#define HMI_CHARGER_INPUT_REGISTER_ADDRESS 5001
#define HMI_CHARGER_INPUT_REGISTER_LENGTH  2

//显示报警使能状态
#define HMI_DISPLAY_WARNING_ENABLE_ADDRESS 8000
#define HMI_DISPLAY_WARNING_ENABLE_LENGTH  8

// 充电器状态
#define HMI_CHARGER_STATE_ADDRESS   600
#define HMI_CHARGER_STATE_LENGTH    8

// 从机电池信息
#define HMI_BMU_VOLTAGE_START_ADDRESS 701
#define HMI_BMU_VOLTAGE_MAX_LENGTH    120
#define HMI_BMU_TEMPERATURE_START_ADDRESS 1101
#define HMI_BMU_TEMPERATURE_MAX_LENGTH    60

#define VOLT_SHOW_CONTROL               501
#define TEMP_SHOW_CONTROL               901
#define TEMP_SHOW_CONTROL_LENGTH        4
#define CELL_BAL_CUR_ADDR_MAX           74
#define SLAVE_ALARM_ADDR                75
#define CELL_BAL_STS_ADDR               81

#define HMI_CUSTOM_INFORMATION_START_ADDRESS    2001

#define CHGR_CHG_CUR_ADDR               5001
#define CHGR_CHG_VOLT_ADDR              5002


#define MODBUS_UNIT 0x01
#define MODBUS_ADDR          0x01


/*MODBUS数据地址定义，功能码：0x03、0x10 电池组配方(参数)信息*/
/*9000-9031*/
#define BAT_INFO_INDEX_MBADDR           1000  /*单体电池信息模块号索引*/
#define SCNT_MBADDR                     9004  /*从机数目地址*/

/*MODBUS数据地址定义，功能码:0x04 电池组实时信息  */
#define TOTAL_VOLT_MBADDR               1   /*电池组总压MODBUS地址*/
#define SOC_MBADDR                      2   /*电池组荷电状态MODBUS地址*/
#define CURRENT_MBADDR                  3   /*电池组电流MODBUS地址*/
#define CELL_VMAX_MBADDR                4   /*单体最高电压地址*/
#define CELL_VMIN_MBADDR                5   /*单体最低电压地址*/
#define CELL_TMAX_MBADDR                6   /*最高温度地址*/
#define TOTAL_CAP_MBADDR                7   /*电池组总容量地址*/
#define LEFT_CAP_MBADDR                 8   /*电池组剩余容量地址*/
#define PACK_CYCLE_MBADDR               9   /*电池组循环次数地址*/
#define VMAX_MODULE_INDEX_MBADDR        10  /*最高电压模块编号地址*/
#define VMAX_CELL_INDEX_MBADDR          11  /*最高电压模块内部编号地址*/
#define VMIN_MODULE_INDEX_MBADDR        12  /*最低电压模块编号地址*/
#define VMIN_CELL_INDEX_MBADDR          13  /*最低电压模块内部编号地址*/
#define TMAX_CELL_INDEX_MBADDR          14  /*最高温度模块内部编号地址*/
#define TMIN_CELL_INDEX_MBADDR          15  /*最低温度模块内部编号地址*/
#define TMAX_MODULE_INDEX_MBADDR        16  /*最高温度模块编号地址*/
#define TMIN_MODULE_INDEX_MBADDR        17  /*最低温度模块编号地址*/
#define ODV_ALERT_MBADDR                18  /*过放地址 */
#define TEMP_HIGH_ALERT_MBADDR          19  /*温度过温报警地址*/
#define TEMP_LOW_ALERT_MBADDR           20  /*温度低温报警地址*/
#define SOC_LOW_ALERT_MBADDR            21  /*SOC低报警地址*/
#define SOC_HIGH_ALERT_MBADDR           22  /*SOC高报警*/
#define CELL_UV_ALERT_MBADDR            23  /*单体欠压报警地址*/
#define CELL_OV_ALERT_MBADDR            24  /*单体过压报警地址*/
#define CHARGER_STATE_MBADDR            25  /*充电器状态地址*/
#define LEAK_ALERT_MBADDR               26  /*高压漏电报警地址*/
#define COMM_ABORT_ALERT_MBADDR         27  /*内部网络通信中断报警地址*/

#define MOTOR_RSPEED_MBADDR             28  /*电机转速地址*/
#define MOTOR_TMILE_MBADDR              29  /*电机里程地址*/
#define MOTOR_TEMPERA_MBADDR            30  /*电机温度地址*/
#define MOTOR_FAULTCODE_MBADDR          31  /*电机故障代码地址*/

#define MOTOR_SPEED_MBADDR              32  /*汽车速度*/
#define MOTOR_MILE_MBADDR_L             33  /*电机累积里程低字节地址*/
#define MOTOR_MILE_MBADDR_H             34  /*电机累积里程高字节地址，必需两个同时请求，否则报错*/

#define COMMON_ALART_MBADDR             35  /*Bit0:总压高报警地址*/
                                            /*Bit1:总压低报警地址*/
                                            /*Bit2:电压排线脱落报警地址*/
                                            /*Bit3:温感排线脱落报警地址*/
                                            /*Bit4:电压差报警地址*/
                                            /*Bit5:温度差报警地址*/
                                            /*Bit6:单体低压报警地址*/
                                            /*Bit7:绝缘漏电报警地址*/
#define INSU_R_SYS_MBADDR               40 /*系统绝缘电阻地址*/
#define INSU_R_P_MBADDR                 41 /*正极绝缘电阻地址*/
#define INSU_R_N_MBADDR                 42 /*负极绝缘电阻地址*/
#define INSU_STATE_MBADDR               43 /*绝缘模块工作状态地址*/
#define INSU_ERROR_MBADDR               44 /*绝缘模块故障代码地址*/
#define INSU_ONLINE_MBADDR              45 /*绝缘模块在线状态*/
#define SYS_VOLT_MBADDR                 46 /*系统电压*/
#define RESIDULE_CAP_MBADDR             47  /*剩余能量 1kWh/bit*/
#define ALARM_CHECK_FLAG_MBADDR         48  /*报警标志显示地址*/
#define CHG_CURRENT_ALERT_MBADDR        49  /*充电电流报警 */
#define DCHG_CURRENT_ALERT_MBADDR       50  /*放电电流报警地址*/
#define SOH_MBADDR                      51
#define DC1_CHARGE_OUTLET_TEMPER_MBADDR 52 //Juson add
#define DC2_CHARGE_OUTLET_TEMPER_MBADDR 53 
#define AC1_CHARGE_OUTLET_TEMPER_MBADDR 54
#define AC2_CHARGE_OUTLET_TEMPER_MBADDR 55 
#define LOCK_STATUS_MBADDR              56 




/**********************************************
 *
 * Task Functions
 *
 ***********************************************/
void hmi_modbus_handler_init(HmiContext* context);
void hmi_task_start(void);
void hmi_task_run(void* pdata);
void hmi_check_rx_frame(HmiContext* context);


/**********************************************
 *
 * Handler Functions
 *
 ***********************************************/
int hmi_handle_read_coils(modbus_t ctx, PINT8U frame, INT16U size);
int hmi_handle_read_holding_registers(modbus_t ctx, PINT8U frame, INT16U size);
int hmi_handle_write_single_coils(modbus_t ctx, PINT8U frame, INT16U size);
int hmi_handle_read_input_registers(modbus_t ctx, PINT8U frame, INT16U size);
int hmi_handle_write_input_register(modbus_t ctx, PINT8U frame, INT16U size);
int hmi_handle_write_registers(modbus_t ctx, PINT8U frame, INT16U size);

int hmi_handle_process_total_input_registers(HmiContext* hmi_context);
int hmi_handle_process_entry_input_registers(HmiContext* hmi_context);
int hmi_handle_process_charger_input_registers(HmiContext* hmi_context);

int hmi_handle_process_entry_voltage_input_registers(HmiContext* hmi_context, BmuId slave_index, INT16U start_address, INT8U length);
int hmi_handle_process_entry_temperate_input_registers(HmiContext* hmi_context, BmuId slave_index, INT16U start_address, INT8U length);

/**********************************************
 *
 * Config Handler Functions
 *
 ***********************************************/
Result hmi_handle_config_changing(ConfigIndex index, INT16U new_value);


/**********************************************
 *
 * Helper Functions
 *
 ***********************************************/
INT16U hmi_query_insu_state(void);
void hmi_set_time(INT8U value, INT8U flag);
void hmi_set_left_cap(INT16U value);
void hmi_set_max_cap(INT16U value);
INT16U hmi_register_to_state_delay(INT16U value);
INT8U hmi_register_to_slave_num(INT16U value);
INT8U hmi_get_current_slave_num(HmiContext* hmi_context);
void hmi_set_current_slave_index(HmiContext* hmi_context, INT16U value);

#endif /* BMS_HMI_IMPL_H_ */
