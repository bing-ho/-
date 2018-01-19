/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www.ligoo.cn
 **�� �� ��:detect_ems_adapter.h
 **��    ��:
 **��������:2017.11.1
 **�ļ�˵��:����Э�������ӿ�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef DETECT_EMS_ADAPTER_H_
#define DETECT_EMS_ADAPTER_H_

#include "bms_bcu.h"
#include "detect_ems_message.h"


/*--------------------------------BMS_message_4-------------------------------------*/
INT8U detect_ems_get_precharge_relay_state(void); //Ԥ���̵���״̬
INT8U detect_ems_get_charge_relay_state(void); //���̵���״̬
INT8U detect_ems_get_discharge_relay_state(void); //�ŵ�̵���״̬
INT16U detect_ems_get_SOH(float factor, float offset); //�����SOH
INT16U detect_ems_get_negative_insulation_resistance(float factor, float offset); //������Ե��ֵ

/*--------------------------------BMS_message_3-------------------------------------*/
INT16U detect_ems_get_positive_insulation_resistance(float factor, float offset); //������Ե��ֵ
INT16U detect_ems_get_low_temperature_id(void); //��͵����¶ȴ���
INT8U detect_ems_get_low_temperature(float factor, float offset); //��͵����¶�
INT16U detect_ems_get_high_temperature_id(void); //��ߵ����¶ȴ���
INT8U detect_ems_get_high_temperature(float factor, float offset); //��ߵ����¶�

/*--------------------------------BMS_message_2-------------------------------------*/
INT16U detect_ems_get_low_voltage_id(void); //��͵����ѹ����
INT16U detect_ems_get_low_voltage(float factor, float offset); //��͵����ѹ
INT16U detect_ems_get_high_voltage_id(void); //��ߵ����ѹ����
INT16U detect_ems_get_high_voltage(float factor, float offset); //��ߵ����ѹ

/*--------------------------------BMS_message_1-------------------------------------*/
INT8U detect_ems_get_battery_short(void); //��·����
INT8U detect_ems_get_battery_temp_rise(void); //��������
INT8U detect_ems_get_battery_ODV(void); //ѹ�����
INT8U detect_ems_get_battery_ODT(void); //�²����
INT8U detect_ems_get_battery_OLT(void); //�¶ȹ���
INT8U detect_ems_get_battery_OCC(void); //������
INT8U detect_ems_get_battery_OLTV(void); //��ѹ����
INT8U detect_ems_get_battery_OHTV(void); //��ѹ����
INT8U detect_ems_get_battery_comm_abort(void); //ͨѶ�ж�
INT8U detect_ems_get_battery_Uleak(void); //��Ե����
INT8U detect_ems_get_battery_Leak(void); //��Ե��
INT8U detect_ems_get_battery_LSOC(void); //SOC����
INT8U detect_ems_get_battery_ODC(void); //�ŵ����
INT8U detect_ems_get_battery_OHT(void); //�������
INT8U detect_ems_get_battery_OLV(void); //�������
INT8U detect_ems_get_battery_OHV(void); //�������
INT16U detect_ems_get_SOC(float factor, float offset); //�����SOC
INT16U detect_ems_get_total_current(float factor, float offset); //������ܵ��������Ϊ�����ŵ�Ϊ����
INT16U detect_ems_get_total_voltage(float factor, float offset); //�������ѹ


#endif 