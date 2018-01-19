/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www.ligoo.cn
 **�� �� ��:ems_adapter.h
 **��    ��:
 **��������:2017.7.12
 **�ļ�˵��:����Э�������ӿ�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef EMS_ADAPTER_H_
#define EMS_ADAPTER_H_

#include "bms_bcu.h"
#include "ems_message.h"


//******BMS�ź�ԭʼ���ȡ�ƫ��������*****//
#define VOLT_RAW_FACTOR_1MV     1
#define VOLT_RAW_FACTOR_10MV    10
#define VOLT_RAW_FACTOR_100MV   100
#define VOLT_RAW_FACTOR_1V      1000

#define CURRENT_RAW_FACTOR_1MA     1
#define CURRENT_RAW_FACTOR_10MA    10
#define CURRENT_RAW_FACTOR_100MA   100
#define CURRENT_RAW_FACTOR_1A      1000

#define SOC_RAW_FACTOR_0_01P  1       // 0.01%
#define SOC_RAW_FATOR_0_1P    10      // 0.1%
#define SOC_RAW_FACTOR_1P     100     // 1%

#define TEMP_RAW_FACTOR_0_1DEG  1     // 0.1��
#define TEMP_RAW_FACTOR_1DEG    10    // 1��
#define TEMP_RAW_OFFSET_N40DEG  -400  // -40��
#define TEMP_RAW_OFFSET_N50DEG  -500  // -50��


#define POWER_RAW_FACTOR_1W     1
#define POWER_RAW_FACTOR_10W    10
#define POWER_RAW_FACTOR_100W   100
#define POWER_RAW_FACTOR_1KW    1000


#define CAP_RAW_FACTOR_1MAH     1
#define CAP_RAW_FACTOR_10MAH    10
#define CAP_RAW_FACTOR_100MAH   100
#define CAP_RAW_FACTOR_1AH      1000

#define ENERGY_RAW_FACTOR_1WH    1
#define ENERGY_RAW_FACTOR_10WH   10
#define ENERGY_RAW_FACTOR_100WH  100
#define ENERGY_RAW_FACTOR_1KWH   1000

#define RES_RAW_FACTOR_1OHM      1     //1��
#define RES_RAW_FACTOR_10OHM     10    //10��
#define RES_RAW_FACTOR_100OHM    100   //100��
#define RES_RAW_FACTOR_1KOHM     1000  //1k��
//*******************************************//


//******��ȡvalue bit_pos��ʼ��bit_lenλ*****//
INT8U get_bits_from_value(INT32U value, INT8U bit_pos, INT8U bit_len);

//******���ź�ֵ����������Ϣд��message������*****//
INT8U write_data_with_bit_addr(INT8U *__far data, const SignalInMsg_t *__far msg_signal, INT32U value, INT8U max_len);

//******�����źŵ���ʼλ�ͳ��ȴ�message��������ȡ�ź�ֵ*****//
INT32U read_data_with_bit_addr(INT8U *__far data, const SignalInMsg_t *__far msg_signal, INT8U max_len);


/*--------------------------------BMS_message_4-------------------------------------*/
INT8U ems_get_precharge_relay_state(void); //Ԥ���̵���״̬
INT8U ems_get_charge_relay_state(void); //���̵���״̬
INT8U ems_get_discharge_relay_state(void); //�ŵ�̵���״̬
INT16U ems_get_SOH(float factor, float offset); //�����SOH
INT16U ems_get_negative_insulation_resistance(float factor, float offset); //������Ե��ֵ

/*--------------------------------BMS_message_3-------------------------------------*/
INT16U ems_get_positive_insulation_resistance(float factor, float offset); //������Ե��ֵ
INT16U ems_get_low_temperature_id(void); //��͵����¶ȴ���
INT8U ems_get_low_temperature(float factor, float offset); //��͵����¶�
INT16U ems_get_high_temperature_id(void); //��ߵ����¶ȴ���
INT8U ems_get_high_temperature(float factor, float offset); //��ߵ����¶�

/*--------------------------------BMS_message_2-------------------------------------*/
INT16U ems_get_low_voltage_id(void); //��͵����ѹ����
INT16U ems_get_low_voltage(float factor, float offset); //��͵����ѹ
INT16U ems_get_high_voltage_id(void); //��ߵ����ѹ����
INT16U ems_get_high_voltage(float factor, float offset); //��ߵ����ѹ

/*--------------------------------BMS_message_1-------------------------------------*/
INT8U ems_get_battery_short(void); //��·����
INT8U ems_get_battery_temp_rise(void); //��������
INT8U ems_get_battery_ODV(void); //ѹ�����
INT8U ems_get_battery_ODT(void); //�²����
INT8U ems_get_battery_OLT(void); //�¶ȹ���
INT8U ems_get_battery_OCC(void); //������
INT8U ems_get_battery_OLTV(void); //��ѹ����
INT8U ems_get_battery_OHTV(void); //��ѹ����
INT8U ems_get_battery_comm_abort(void); //ͨѶ�ж�
INT8U ems_get_battery_Uleak(void); //��Ե����
INT8U ems_get_battery_Leak(void); //��Ե��
INT8U ems_get_battery_LSOC(void); //SOC����
INT8U ems_get_battery_ODC(void); //�ŵ����
INT8U ems_get_battery_OHT(void); //�������
INT8U ems_get_battery_OLV(void); //�������
INT8U ems_get_battery_OHV(void); //�������
INT16U ems_get_SOC(float factor, float offset); //�����SOC
INT16U ems_get_total_current(float factor, float offset); //������ܵ��������Ϊ�����ŵ�Ϊ����
INT16U ems_get_total_voltage(float factor, float offset); //�������ѹ


#endif 