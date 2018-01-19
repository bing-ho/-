/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www.ligoo.cn
 **�� �� ��:detect_ems_signal.h
 **��    ��:
 **��������:2017.11.1
 **�ļ�˵��:�ͻ�Э���źż�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef DETECT_EMS_SIGNAL_H_
#define DETECT_EMS_SIGNAL_H_

#include "bms_defs.h"
#include "ems_signal.h"

/**********************�ͻ�Э���źż�����������DBC��ȡ��************************/
#pragma CONST_SEG PAGED_EMS_CONFIG

extern const Signal_t __far sg_detect_precharge_relay_state; //Ԥ���̵���״̬
extern const Signal_t __far sg_detect_charge_relay_state; //���̵���״̬
extern const Signal_t __far sg_detect_discharge_relay_state; //�ŵ�̵���״̬
extern const Signal_t __far sg_detect_SOH; //�����SOH
extern const Signal_t __far sg_detect_negative_insulation_resistance; //������Ե��ֵ
extern const Signal_t __far sg_detect_positive_insulation_resistance; //������Ե��ֵ
extern const Signal_t __far sg_detect_low_temperature_id; //��͵����¶ȴ���
extern const Signal_t __far sg_detect_low_temperature; //��͵����¶�
extern const Signal_t __far sg_detect_high_temperature_id; //��ߵ����¶ȴ���
extern const Signal_t __far sg_detect_high_temperature; //��ߵ����¶�
extern const Signal_t __far sg_detect_low_voltage_id; //��͵����ѹ����
extern const Signal_t __far sg_detect_low_voltage; //��͵����ѹ
extern const Signal_t __far sg_detect_high_voltage_id; //��ߵ����ѹ����
extern const Signal_t __far sg_detect_high_voltage; //��ߵ����ѹ
extern const Signal_t __far sg_detect_battery_short; //��·����
extern const Signal_t __far sg_detect_battery_temp_rise; //��������
extern const Signal_t __far sg_detect_battery_ODV; //ѹ�����
extern const Signal_t __far sg_detect_battery_ODT; //�²����
extern const Signal_t __far sg_detect_battery_OLT; //�¶ȹ���
extern const Signal_t __far sg_detect_battery_OCC; //������
extern const Signal_t __far sg_detect_battery_OLTV; //��ѹ����
extern const Signal_t __far sg_detect_battery_OHTV; //��ѹ����
extern const Signal_t __far sg_detect_battery_comm_abort; //ͨѶ�ж�
extern const Signal_t __far sg_detect_battery_Uleak; //��Ե����
extern const Signal_t __far sg_detect_battery_Leak; //��Ե��
extern const Signal_t __far sg_detect_battery_LSOC; //SOC����
extern const Signal_t __far sg_detect_battery_ODC; //�ŵ����
extern const Signal_t __far sg_detect_battery_OHT; //�������
extern const Signal_t __far sg_detect_battery_OLV; //�������
extern const Signal_t __far sg_detect_battery_OHV; //�������
extern const Signal_t __far sg_detect_SOC; //�����SOC
extern const Signal_t __far sg_detect_total_current; //������ܵ��������Ϊ�����ŵ�Ϊ����
extern const Signal_t __far sg_detect_total_voltage; //�������ѹ


#pragma CONST_SEG DEFAULT	


#endif
