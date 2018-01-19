/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www.ligoo.cn
 **�� �� ��:ems_signal.h
 **��    ��:
 **��������:2017.7.12
 **�ļ�˵��:�ͻ�Э���źż�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef EMS_SIGNAL_H_
#define EMS_SIGNAL_H_

#include "bms_defs.h"



/************************�ź������ṹ�������������̶����䣩*************************/

typedef struct _Signal{
  INT8U signal_size; //�źų���
  INT8U byte_order; //1=little endian(Intel), 0=big endian(Motorola);
  float factor; //�ֱ���
  float offset; //ƫ����(physical_value = raw_value * factor + offset);
  float minimum; //��Сֵ
  float maximum; //���ֵ
}Signal_t; 

/**********************�ͻ�Э���źż�����������DBC��ȡ��************************/
#pragma CONST_SEG PAGED_EMS_CONFIG

extern const Signal_t __far sg_precharge_relay_state; //Ԥ���̵���״̬
extern const Signal_t __far sg_charge_relay_state; //���̵���״̬
extern const Signal_t __far sg_discharge_relay_state; //�ŵ�̵���״̬
extern const Signal_t __far sg_SOH; //�����SOH
extern const Signal_t __far sg_negative_insulation_resistance; //������Ե��ֵ
extern const Signal_t __far sg_positive_insulation_resistance; //������Ե��ֵ
extern const Signal_t __far sg_low_temperature_id; //��͵����¶ȴ���
extern const Signal_t __far sg_low_temperature; //��͵����¶�
extern const Signal_t __far sg_high_temperature_id; //��ߵ����¶ȴ���
extern const Signal_t __far sg_high_temperature; //��ߵ����¶�
extern const Signal_t __far sg_low_voltage_id; //��͵����ѹ����
extern const Signal_t __far sg_low_voltage; //��͵����ѹ
extern const Signal_t __far sg_high_voltage_id; //��ߵ����ѹ����
extern const Signal_t __far sg_high_voltage; //��ߵ����ѹ
extern const Signal_t __far sg_battery_short; //��·����
extern const Signal_t __far sg_battery_temp_rise; //��������
extern const Signal_t __far sg_battery_ODV; //ѹ�����
extern const Signal_t __far sg_battery_ODT; //�²����
extern const Signal_t __far sg_battery_OLT; //�¶ȹ���
extern const Signal_t __far sg_battery_OCC; //������
extern const Signal_t __far sg_battery_OLTV; //��ѹ����
extern const Signal_t __far sg_battery_OHTV; //��ѹ����
extern const Signal_t __far sg_battery_comm_abort; //ͨѶ�ж�
extern const Signal_t __far sg_battery_Uleak; //��Ե����
extern const Signal_t __far sg_battery_Leak; //��Ե��
extern const Signal_t __far sg_battery_LSOC; //SOC����
extern const Signal_t __far sg_battery_ODC; //�ŵ����
extern const Signal_t __far sg_battery_OHT; //�������
extern const Signal_t __far sg_battery_OLV; //�������
extern const Signal_t __far sg_battery_OHV; //�������
extern const Signal_t __far sg_SOC; //�����SOC
extern const Signal_t __far sg_total_current; //������ܵ��������Ϊ�����ŵ�Ϊ����
extern const Signal_t __far sg_total_voltage; //�������ѹ


#pragma CONST_SEG DEFAULT	









#endif
