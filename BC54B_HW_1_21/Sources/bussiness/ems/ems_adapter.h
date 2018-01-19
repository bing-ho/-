/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www.ligoo.cn
 **文 件 名:ems_adapter.h
 **作    者:
 **创建日期:2017.7.12
 **文件说明:整车协议适配层接口
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef EMS_ADAPTER_H_
#define EMS_ADAPTER_H_

#include "bms_bcu.h"
#include "ems_message.h"


//******BMS信号原始精度、偏移量定义*****//
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

#define TEMP_RAW_FACTOR_0_1DEG  1     // 0.1℃
#define TEMP_RAW_FACTOR_1DEG    10    // 1℃
#define TEMP_RAW_OFFSET_N40DEG  -400  // -40℃
#define TEMP_RAW_OFFSET_N50DEG  -500  // -50℃


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

#define RES_RAW_FACTOR_1OHM      1     //1Ω
#define RES_RAW_FACTOR_10OHM     10    //10Ω
#define RES_RAW_FACTOR_100OHM    100   //100Ω
#define RES_RAW_FACTOR_1KOHM     1000  //1kΩ
//*******************************************//


//******获取value bit_pos开始的bit_len位*****//
INT8U get_bits_from_value(INT32U value, INT8U bit_pos, INT8U bit_len);

//******将信号值根据描述信息写入message数据域*****//
INT8U write_data_with_bit_addr(INT8U *__far data, const SignalInMsg_t *__far msg_signal, INT32U value, INT8U max_len);

//******根据信号的起始位和长度从message数据域提取信号值*****//
INT32U read_data_with_bit_addr(INT8U *__far data, const SignalInMsg_t *__far msg_signal, INT8U max_len);


/*--------------------------------BMS_message_4-------------------------------------*/
INT8U ems_get_precharge_relay_state(void); //预充电继电器状态
INT8U ems_get_charge_relay_state(void); //充电继电器状态
INT8U ems_get_discharge_relay_state(void); //放电继电器状态
INT16U ems_get_SOH(float factor, float offset); //电池组SOH
INT16U ems_get_negative_insulation_resistance(float factor, float offset); //负极绝缘阻值

/*--------------------------------BMS_message_3-------------------------------------*/
INT16U ems_get_positive_insulation_resistance(float factor, float offset); //正极绝缘阻值
INT16U ems_get_low_temperature_id(void); //最低单体温度串号
INT8U ems_get_low_temperature(float factor, float offset); //最低单体温度
INT16U ems_get_high_temperature_id(void); //最高单体温度串号
INT8U ems_get_high_temperature(float factor, float offset); //最高单体温度

/*--------------------------------BMS_message_2-------------------------------------*/
INT16U ems_get_low_voltage_id(void); //最低单体电压串号
INT16U ems_get_low_voltage(float factor, float offset); //最低单体电压
INT16U ems_get_high_voltage_id(void); //最高单体电压串号
INT16U ems_get_high_voltage(float factor, float offset); //最高单体电压

/*--------------------------------BMS_message_1-------------------------------------*/
INT8U ems_get_battery_short(void); //短路保护
INT8U ems_get_battery_temp_rise(void); //温升过快
INT8U ems_get_battery_ODV(void); //压差过大
INT8U ems_get_battery_ODT(void); //温差过大
INT8U ems_get_battery_OLT(void); //温度过低
INT8U ems_get_battery_OCC(void); //充电过流
INT8U ems_get_battery_OLTV(void); //总压过低
INT8U ems_get_battery_OHTV(void); //总压过高
INT8U ems_get_battery_comm_abort(void); //通讯中断
INT8U ems_get_battery_Uleak(void); //绝缘过低
INT8U ems_get_battery_Leak(void); //绝缘低
INT8U ems_get_battery_LSOC(void); //SOC过低
INT8U ems_get_battery_ODC(void); //放电过流
INT8U ems_get_battery_OHT(void); //单体过温
INT8U ems_get_battery_OLV(void); //单体过放
INT8U ems_get_battery_OHV(void); //单体过充
INT16U ems_get_SOC(float factor, float offset); //电池组SOC
INT16U ems_get_total_current(float factor, float offset); //电池组总电流（充电为负，放电为正）
INT16U ems_get_total_voltage(float factor, float offset); //电池组总压


#endif 