/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_charger.h
* @brief
* @note
* @author
* @date 2012-5-8
*
*/

#ifndef BMS_CHARGER_H_
#define BMS_CHARGER_H_

#include "bms_defs.h"
#include "bms_bcu.h"
#include "bms_charger_common.h"

#define SLOW_CHARGER_WORK_WITH_GUOBIAO_CHARGER_EN       0

#define GUOBIAO_CC_1500OHM_CURRENT_MAX      100 //0.1A CC 1.5K欧电阻连接允许最大电流
#define GUOBIAO_CC_680OHM_CURRENT_MAX       160 //0.1A CC 680欧电阻连接允许最大电流
#define GUOBIAO_CC_220OHM_CURRENT_MAX       320 //0.1A CC 220欧电阻连接允许最大电流
#define GUOBIAO_CC_100OHM_CURRENT_MAX       630 //0.1A CC 100欧电阻连接允许最大电流

typedef enum
{
    kChargerStatusUnknown = 0x0,
    kChargerStatusHardwareException = 0x01,
    kChargerStatusTemperatureException = 0x02,
    kChargerStatusInputVoltageException = 0x04,
    kChargerStatusNotCharging = 0x08,
    kChargerStatusCommunicationException = 0x10,
    kChargerStatusOnline = 0x20,
} ChargerStatus;

#define MAX_CHARGER_NAME_LEN   10

typedef struct
{
    INT16U type;
    INT16U flag;
    char name[MAX_CHARGER_NAME_LEN];
    char version[MAX_CHARGER_NAME_LEN];
    char vendor[MAX_CHARGER_NAME_LEN];
    char protocol[MAX_CHARGER_NAME_LEN];
}ChargerDeviceInfo;

extern void charger_init(void);

extern void charger_uninit(void);

extern Result charger_on(void);

extern Result charger_off(void);

extern BOOLEAN charger_is_charging(void);

/** 充电机是否连接工作 */
extern Result charger_is_connected(void);

extern Result charger_get_status(INT8U* status);

extern Result charger_get_device_info(ChargerDeviceInfo* info);

extern Result charger_set_current(INT16U current);

extern Result charger_get_current(INT16U* current);

extern Result charger_set_voltage(INT16U voltage);

extern Result charger_get_voltage(INT16U* voltage);

extern Result charger_get_output_voltage(INT16U* voltage);

extern Result charger_get_output_current(INT16U* current);

extern Result charger_set_pulse_current(INT16U current);

extern Result charger_get_pulse_current(INT16U* current);

extern Result charger_set_pulse_charge_time(INT16U time);

extern Result charger_get_pulse_charge_time(INT16U* time);

extern Result charger_constant_current_voltage_ctl(void);

extern Result charger_get_charge_voltage_max(INT16U* voltage);

extern Result charger_get_charge_voltage_min(INT16U* voltage);

extern Result charger_get_charge_current_max(INT16U* current);

extern Result charger_get_charge_current_min(INT16U* current);

extern Result charger_get_charge_pulse_current_max(INT16U* current);

extern Result charger_get_charged_energy(INT16U* energy);

extern Result charger_get_charge_time_eclipse(INT16U* time);

extern Result charger_get_charg_time_require(INT16U* time);

extern Result charger_get_charge_cycle(INT16U* cycle);

extern Result charger_get_charge_ready_status(void);

extern INT8U charger_is_defective(void);

extern INT8U charger_charging_is_enable(void);

extern Result charger_is_communication(void);

/** 用于带有条件的开启关闭充电
 * */
extern Result charger_control_enable_charger(INT32U flag);
extern Result charger_control_disable_charger(INT32U flag);
extern INT8U guobiao_charger_cc_680OHM_is_connected(void);
extern INT8U guobiao_charger_cc_220OHM_is_connected(void);
extern INT8U guobiao_charger_cc_is_connected(void);
extern INT8U guobiao_charger_cc2_is_connected(void);
extern INT8U guobiao_charger_pwm_is_connected(void);

extern INT8U self_check_charger_get_communication_status(void);

#endif /* BMS_CHARGER_H_ */
