/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www.ligoo.cn
 **文 件 名:ems_signal.c
 **作    者:
 **创建日期:2017.7.12
 **文件说明:客户协议信号集
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef EMS_BMS_VAR_IMPL
  #define EMS_BMS_VAR_IMPL
#endif
#include "ems_signal.h"



/**********客户协议信号集定义（根据DBC提取）*********/ 
#pragma CONST_SEG PAGED_EMS_CONFIG	
           
const Signal_t __far sg_precharge_relay_state = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_charge_relay_state = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_discharge_relay_state = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_SOH = { 16, 1, 10.0, 0.0, 0.0, 10000.0};
const Signal_t __far sg_negative_insulation_resistance = { 16, 1, 100.0, 0.0, 1000.0, 6553500.0};
const Signal_t __far sg_positive_insulation_resistance = { 16, 1, 100.0, 0.0, 1000.0, 6553500.0};
const Signal_t __far sg_low_temperature_id = { 16, 1, 1.0, 0.0, 0.0, 1000.0};
const Signal_t __far sg_low_temperature = { 8, 1, 1.0, -40.0, -40.0, 210.0};
const Signal_t __far sg_high_temperature_id = { 16, 1, 1.0, 0.0, 0.0, 1000.0};
const Signal_t __far sg_high_temperature = { 8, 1, 1.0, -40.0, -40.0, 210.0};
const Signal_t __far sg_low_voltage_id = { 16, 1, 1.0, 0.0, 0.0, 1000.0};
const Signal_t __far sg_low_voltage = { 16, 1, 1.0, 0.0, 0.0, 10000.0};
const Signal_t __far sg_high_voltage_id = { 16, 1, 1.0, 0.0, 0.0, 1000.0};
const Signal_t __far sg_high_voltage = { 16, 1, 1.0, 0.0, 0.0, 10000.0};
const Signal_t __far sg_battery_short = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_temp_rise = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_ODV = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_ODT = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_OLT = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_OCC = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_OLTV = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_OHTV = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_comm_abort = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_Uleak = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_Leak = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_LSOC = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_ODC = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_OHT = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_OLV = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_battery_OHV = { 1, 1, 1.0, 0.0, 0.0, 1.0};
const Signal_t __far sg_SOC = { 16, 1, 10.0, 0.0, 0.0, 100000.0};
const Signal_t __far sg_total_current = { 16, 1, 1.0, -32000.0, -32000.0, 33535.0};
const Signal_t __far sg_total_voltage = { 16, 1, 100.0, 0.0, 0.0, 6553500.0};


#pragma CONST_SEG DEFAULT
			                
