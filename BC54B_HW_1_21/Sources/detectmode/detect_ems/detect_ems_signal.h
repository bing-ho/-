/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www.ligoo.cn
 **文 件 名:detect_ems_signal.h
 **作    者:
 **创建日期:2017.11.1
 **文件说明:客户协议信号集
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef DETECT_EMS_SIGNAL_H_
#define DETECT_EMS_SIGNAL_H_

#include "bms_defs.h"
#include "ems_signal.h"

/**********************客户协议信号集声明（根据DBC提取）************************/
#pragma CONST_SEG PAGED_EMS_CONFIG

extern const Signal_t __far sg_detect_precharge_relay_state; //预充电继电器状态
extern const Signal_t __far sg_detect_charge_relay_state; //充电继电器状态
extern const Signal_t __far sg_detect_discharge_relay_state; //放电继电器状态
extern const Signal_t __far sg_detect_SOH; //电池组SOH
extern const Signal_t __far sg_detect_negative_insulation_resistance; //负极绝缘阻值
extern const Signal_t __far sg_detect_positive_insulation_resistance; //正极绝缘阻值
extern const Signal_t __far sg_detect_low_temperature_id; //最低单体温度串号
extern const Signal_t __far sg_detect_low_temperature; //最低单体温度
extern const Signal_t __far sg_detect_high_temperature_id; //最高单体温度串号
extern const Signal_t __far sg_detect_high_temperature; //最高单体温度
extern const Signal_t __far sg_detect_low_voltage_id; //最低单体电压串号
extern const Signal_t __far sg_detect_low_voltage; //最低单体电压
extern const Signal_t __far sg_detect_high_voltage_id; //最高单体电压串号
extern const Signal_t __far sg_detect_high_voltage; //最高单体电压
extern const Signal_t __far sg_detect_battery_short; //短路保护
extern const Signal_t __far sg_detect_battery_temp_rise; //温升过快
extern const Signal_t __far sg_detect_battery_ODV; //压差过大
extern const Signal_t __far sg_detect_battery_ODT; //温差过大
extern const Signal_t __far sg_detect_battery_OLT; //温度过低
extern const Signal_t __far sg_detect_battery_OCC; //充电过流
extern const Signal_t __far sg_detect_battery_OLTV; //总压过低
extern const Signal_t __far sg_detect_battery_OHTV; //总压过高
extern const Signal_t __far sg_detect_battery_comm_abort; //通讯中断
extern const Signal_t __far sg_detect_battery_Uleak; //绝缘过低
extern const Signal_t __far sg_detect_battery_Leak; //绝缘低
extern const Signal_t __far sg_detect_battery_LSOC; //SOC过低
extern const Signal_t __far sg_detect_battery_ODC; //放电过流
extern const Signal_t __far sg_detect_battery_OHT; //单体过温
extern const Signal_t __far sg_detect_battery_OLV; //单体过放
extern const Signal_t __far sg_detect_battery_OHV; //单体过充
extern const Signal_t __far sg_detect_SOC; //电池组SOC
extern const Signal_t __far sg_detect_total_current; //电池组总电流（充电为负，放电为正）
extern const Signal_t __far sg_detect_total_voltage; //电池组总压


#pragma CONST_SEG DEFAULT	


#endif
