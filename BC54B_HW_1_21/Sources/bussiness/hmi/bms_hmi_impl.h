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

//���������״̬
#define HMI_BATTERYS_STATE_ADDRESS  0
#define HMI_BATTERYS_STATE_LENGTH   8

// sdcard״̬
#define HMI_SDCARD_STATE_ADDRESS    16
#define HMI_SDCARD_STATE_LENGTH     8

//��¼������Ϣ
#define HMI_LOGIN_WARNING_ADDRESS  32
#define HMI_LOGIN_WARNING_LENGTH   24

#define HMI_COLIS_SDCARD_REQUEST_UNPLUG 1

//�������Ϣ
#define HMI_CHARGER_INPUT_REGISTER_ADDRESS 5001
#define HMI_CHARGER_INPUT_REGISTER_LENGTH  2

//��ʾ����ʹ��״̬
#define HMI_DISPLAY_WARNING_ENABLE_ADDRESS 8000
#define HMI_DISPLAY_WARNING_ENABLE_LENGTH  8

// �����״̬
#define HMI_CHARGER_STATE_ADDRESS   600
#define HMI_CHARGER_STATE_LENGTH    8

// �ӻ������Ϣ
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


/*MODBUS���ݵ�ַ���壬�����룺0x03��0x10 ������䷽(����)��Ϣ*/
/*9000-9031*/
#define BAT_INFO_INDEX_MBADDR           1000  /*��������Ϣģ�������*/
#define SCNT_MBADDR                     9004  /*�ӻ���Ŀ��ַ*/

/*MODBUS���ݵ�ַ���壬������:0x04 �����ʵʱ��Ϣ  */
#define TOTAL_VOLT_MBADDR               1   /*�������ѹMODBUS��ַ*/
#define SOC_MBADDR                      2   /*�����ɵ�״̬MODBUS��ַ*/
#define CURRENT_MBADDR                  3   /*��������MODBUS��ַ*/
#define CELL_VMAX_MBADDR                4   /*������ߵ�ѹ��ַ*/
#define CELL_VMIN_MBADDR                5   /*������͵�ѹ��ַ*/
#define CELL_TMAX_MBADDR                6   /*����¶ȵ�ַ*/
#define TOTAL_CAP_MBADDR                7   /*�������������ַ*/
#define LEFT_CAP_MBADDR                 8   /*�����ʣ��������ַ*/
#define PACK_CYCLE_MBADDR               9   /*�����ѭ��������ַ*/
#define VMAX_MODULE_INDEX_MBADDR        10  /*��ߵ�ѹģ���ŵ�ַ*/
#define VMAX_CELL_INDEX_MBADDR          11  /*��ߵ�ѹģ���ڲ���ŵ�ַ*/
#define VMIN_MODULE_INDEX_MBADDR        12  /*��͵�ѹģ���ŵ�ַ*/
#define VMIN_CELL_INDEX_MBADDR          13  /*��͵�ѹģ���ڲ���ŵ�ַ*/
#define TMAX_CELL_INDEX_MBADDR          14  /*����¶�ģ���ڲ���ŵ�ַ*/
#define TMIN_CELL_INDEX_MBADDR          15  /*����¶�ģ���ڲ���ŵ�ַ*/
#define TMAX_MODULE_INDEX_MBADDR        16  /*����¶�ģ���ŵ�ַ*/
#define TMIN_MODULE_INDEX_MBADDR        17  /*����¶�ģ���ŵ�ַ*/
#define ODV_ALERT_MBADDR                18  /*���ŵ�ַ */
#define TEMP_HIGH_ALERT_MBADDR          19  /*�¶ȹ��±�����ַ*/
#define TEMP_LOW_ALERT_MBADDR           20  /*�¶ȵ��±�����ַ*/
#define SOC_LOW_ALERT_MBADDR            21  /*SOC�ͱ�����ַ*/
#define SOC_HIGH_ALERT_MBADDR           22  /*SOC�߱���*/
#define CELL_UV_ALERT_MBADDR            23  /*����Ƿѹ������ַ*/
#define CELL_OV_ALERT_MBADDR            24  /*�����ѹ������ַ*/
#define CHARGER_STATE_MBADDR            25  /*�����״̬��ַ*/
#define LEAK_ALERT_MBADDR               26  /*��ѹ©�籨����ַ*/
#define COMM_ABORT_ALERT_MBADDR         27  /*�ڲ�����ͨ���жϱ�����ַ*/

#define MOTOR_RSPEED_MBADDR             28  /*���ת�ٵ�ַ*/
#define MOTOR_TMILE_MBADDR              29  /*�����̵�ַ*/
#define MOTOR_TEMPERA_MBADDR            30  /*����¶ȵ�ַ*/
#define MOTOR_FAULTCODE_MBADDR          31  /*������ϴ����ַ*/

#define MOTOR_SPEED_MBADDR              32  /*�����ٶ�*/
#define MOTOR_MILE_MBADDR_L             33  /*����ۻ���̵��ֽڵ�ַ*/
#define MOTOR_MILE_MBADDR_H             34  /*����ۻ���̸��ֽڵ�ַ����������ͬʱ���󣬷��򱨴�*/

#define COMMON_ALART_MBADDR             35  /*Bit0:��ѹ�߱�����ַ*/
                                            /*Bit1:��ѹ�ͱ�����ַ*/
                                            /*Bit2:��ѹ�������䱨����ַ*/
                                            /*Bit3:�¸��������䱨����ַ*/
                                            /*Bit4:��ѹ�����ַ*/
                                            /*Bit5:�¶Ȳ����ַ*/
                                            /*Bit6:�����ѹ������ַ*/
                                            /*Bit7:��Ե©�籨����ַ*/
#define INSU_R_SYS_MBADDR               40 /*ϵͳ��Ե�����ַ*/
#define INSU_R_P_MBADDR                 41 /*������Ե�����ַ*/
#define INSU_R_N_MBADDR                 42 /*������Ե�����ַ*/
#define INSU_STATE_MBADDR               43 /*��Եģ�鹤��״̬��ַ*/
#define INSU_ERROR_MBADDR               44 /*��Եģ����ϴ����ַ*/
#define INSU_ONLINE_MBADDR              45 /*��Եģ������״̬*/
#define SYS_VOLT_MBADDR                 46 /*ϵͳ��ѹ*/
#define RESIDULE_CAP_MBADDR             47  /*ʣ������ 1kWh/bit*/
#define ALARM_CHECK_FLAG_MBADDR         48  /*������־��ʾ��ַ*/
#define CHG_CURRENT_ALERT_MBADDR        49  /*���������� */
#define DCHG_CURRENT_ALERT_MBADDR       50  /*�ŵ����������ַ*/
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
