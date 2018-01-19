/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www.ligoo.cn
 **文 件 名:ems_signal.h
 **作    者:
 **创建日期:2017.7.12
 **文件说明:客户协议信号集
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef EMS_SIGNAL_H_
#define EMS_SIGNAL_H_

#include "bms_defs.h"



/************************信号描述结构体类型声明（固定不变）*************************/

typedef struct _Signal{
  INT8U signal_size; //信号长度
  INT8U byte_order; //1=little endian(Intel), 0=big endian(Motorola);
  float factor; //分辨率
  float offset; //偏移量(physical_value = raw_value * factor + offset);
  float minimum; //最小值
  float maximum; //最大值
}Signal_t; 

/**********************客户协议信号集声明（根据DBC提取）************************/
#pragma CONST_SEG PAGED_EMS_CONFIG

extern const Signal_t __far sg_precharge_relay_state; //预充电继电器状态
extern const Signal_t __far sg_charge_relay_state; //充电继电器状态
extern const Signal_t __far sg_discharge_relay_state; //放电继电器状态
extern const Signal_t __far sg_SOH; //电池组SOH
extern const Signal_t __far sg_negative_insulation_resistance; //负极绝缘阻值
extern const Signal_t __far sg_positive_insulation_resistance; //正极绝缘阻值
extern const Signal_t __far sg_low_temperature_id; //最低单体温度串号
extern const Signal_t __far sg_low_temperature; //最低单体温度
extern const Signal_t __far sg_high_temperature_id; //最高单体温度串号
extern const Signal_t __far sg_high_temperature; //最高单体温度
extern const Signal_t __far sg_low_voltage_id; //最低单体电压串号
extern const Signal_t __far sg_low_voltage; //最低单体电压
extern const Signal_t __far sg_high_voltage_id; //最高单体电压串号
extern const Signal_t __far sg_high_voltage; //最高单体电压
extern const Signal_t __far sg_battery_short; //短路保护
extern const Signal_t __far sg_battery_temp_rise; //温升过快
extern const Signal_t __far sg_battery_ODV; //压差过大
extern const Signal_t __far sg_battery_ODT; //温差过大
extern const Signal_t __far sg_battery_OLT; //温度过低
extern const Signal_t __far sg_battery_OCC; //充电过流
extern const Signal_t __far sg_battery_OLTV; //总压过低
extern const Signal_t __far sg_battery_OHTV; //总压过高
extern const Signal_t __far sg_battery_comm_abort; //通讯中断
extern const Signal_t __far sg_battery_Uleak; //绝缘过低
extern const Signal_t __far sg_battery_Leak; //绝缘低
extern const Signal_t __far sg_battery_LSOC; //SOC过低
extern const Signal_t __far sg_battery_ODC; //放电过流
extern const Signal_t __far sg_battery_OHT; //单体过温
extern const Signal_t __far sg_battery_OLV; //单体过放
extern const Signal_t __far sg_battery_OHV; //单体过充
extern const Signal_t __far sg_SOC; //电池组SOC
extern const Signal_t __far sg_total_current; //电池组总电流（充电为负，放电为正）
extern const Signal_t __far sg_total_voltage; //电池组总压


#pragma CONST_SEG DEFAULT	









#endif
