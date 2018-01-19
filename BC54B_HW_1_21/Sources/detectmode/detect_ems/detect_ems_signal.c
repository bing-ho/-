/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www.ligoo.cn
 **�� �� ��:detect_ems_signal.c
 **��    ��:
 **��������:2017.11.1
 **�ļ�˵��:�ͻ�Э���źż�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef DETECT_EMS_BMS_VAR_IMPL
  #define DETCET_EMS_BMS_VAR_IMPL
#endif
#include "detect_ems_signal.h"
#include "ems_signal.h"


/**********�ͻ�Э���źż����壨����DBC��ȡ��*********/ 
#pragma CONST_SEG PAGED_EMS_CONFIG	
           
const Signal_t __far sg_detect_precharge_relay_state = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_charge_relay_state = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_discharge_relay_state = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_SOH = { 16, 1, 10.0, 0.0, 0.0, 10000.0};
const Signal_t __far sg_detect_negative_insulation_resistance = { 16, 1, 100.0, 0.0, 1000.0, 6553500.0};
const Signal_t __far sg_detect_positive_insulation_resistance = { 16, 1, 100.0, 0.0, 1000.0, 6553500.0};
const Signal_t __far sg_detect_low_temperature_id = { 16, 1, 1.0, 0.0, 0.0, 1000.0};
const Signal_t __far sg_detect_low_temperature = { 8, 1, 1.0, -40.0, -40.0, 210.0};
const Signal_t __far sg_detect_high_temperature_id = { 16, 1, 1.0, 0.0, 0.0, 1000.0};
const Signal_t __far sg_detect_high_temperature = { 8, 1, 1.0, -40.0, -40.0, 210.0};
const Signal_t __far sg_detect_low_voltage_id = { 16, 1, 1.0, 0.0, 0.0, 1000.0};
const Signal_t __far sg_detect_low_voltage = { 16, 1, 1.0, 0.0, 0.0, 10000.0};
const Signal_t __far sg_detect_high_voltage_id = { 16, 1, 1.0, 0.0, 0.0, 1000.0};
const Signal_t __far sg_detect_high_voltage = { 16, 1, 1.0, 0.0, 0.0, 10000.0};
const Signal_t __far sg_detect_battery_short = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_temp_rise = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_ODV = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_ODT = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_OLT = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_OCC = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_OLTV = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_OHTV = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_comm_abort = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_Uleak = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_Leak = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_LSOC = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_ODC = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_OHT = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_OLV = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_battery_OHV = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_detect_SOC = { 16, 1, 10.0, 0.0, 0.0, 100000.0};
const Signal_t __far sg_detect_total_current = { 16, 1, 1.0, -32000.0, -32000.0, 33535.0};
const Signal_t __far sg_detect_total_voltage = { 16, 1, 100.0, 0.0, 0.0, 6553500.0};


#pragma CONST_SEG DEFAULT
			                
