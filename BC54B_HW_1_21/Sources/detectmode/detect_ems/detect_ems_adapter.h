/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www.ligoo.cn
 **文 件 名:detect_ems_adapter.h
 **作    者:
 **创建日期:2017.11.1
 **文件说明:整车协议适配层接口
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef DETECT_EMS_ADAPTER_H_
#define DETECT_EMS_ADAPTER_H_

#include "bms_bcu.h"
#include "detect_ems_message.h"


/*--------------------------------BMS_message_4-------------------------------------*/
INT8U detect_ems_get_precharge_relay_state(void); //预充电继电器状态
INT8U detect_ems_get_charge_relay_state(void); //充电继电器状态
INT8U detect_ems_get_discharge_relay_state(void); //放电继电器状态
INT16U detect_ems_get_SOH(float factor, float offset); //电池组SOH
INT16U detect_ems_get_negative_insulation_resistance(float factor, float offset); //负极绝缘阻值

/*--------------------------------BMS_message_3-------------------------------------*/
INT16U detect_ems_get_positive_insulation_resistance(float factor, float offset); //正极绝缘阻值
INT16U detect_ems_get_low_temperature_id(void); //最低单体温度串号
INT8U detect_ems_get_low_temperature(float factor, float offset); //最低单体温度
INT16U detect_ems_get_high_temperature_id(void); //最高单体温度串号
INT8U detect_ems_get_high_temperature(float factor, float offset); //最高单体温度

/*--------------------------------BMS_message_2-------------------------------------*/
INT16U detect_ems_get_low_voltage_id(void); //最低单体电压串号
INT16U detect_ems_get_low_voltage(float factor, float offset); //最低单体电压
INT16U detect_ems_get_high_voltage_id(void); //最高单体电压串号
INT16U detect_ems_get_high_voltage(float factor, float offset); //最高单体电压

/*--------------------------------BMS_message_1-------------------------------------*/
INT8U detect_ems_get_battery_short(void); //短路保护
INT8U detect_ems_get_battery_temp_rise(void); //温升过快
INT8U detect_ems_get_battery_ODV(void); //压差过大
INT8U detect_ems_get_battery_ODT(void); //温差过大
INT8U detect_ems_get_battery_OLT(void); //温度过低
INT8U detect_ems_get_battery_OCC(void); //充电过流
INT8U detect_ems_get_battery_OLTV(void); //总压过低
INT8U detect_ems_get_battery_OHTV(void); //总压过高
INT8U detect_ems_get_battery_comm_abort(void); //通讯中断
INT8U detect_ems_get_battery_Uleak(void); //绝缘过低
INT8U detect_ems_get_battery_Leak(void); //绝缘低
INT8U detect_ems_get_battery_LSOC(void); //SOC过低
INT8U detect_ems_get_battery_ODC(void); //放电过流
INT8U detect_ems_get_battery_OHT(void); //单体过温
INT8U detect_ems_get_battery_OLV(void); //单体过放
INT8U detect_ems_get_battery_OHV(void); //单体过充
INT16U detect_ems_get_SOC(float factor, float offset); //电池组SOC
INT16U detect_ems_get_total_current(float factor, float offset); //电池组总电流（充电为负，放电为正）
INT16U detect_ems_get_total_voltage(float factor, float offset); //电池组总压


#endif 