/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_motor.h
* @brief
* @note
* @author
* @date 2012-5-9
*
*/

#ifndef BMS_MOTOR_H_
#define BMS_MOTOR_H_
#include "bms_defs.h"
#include "app_cfg.h"
#include "bms_system.h"
#include "bms_buffer.h"
#include "bms_util.h"
#include "bms_can.h"
#include "bms_config.h"

typedef enum
{
    kMotorEventBmsHardwareException     = 0x01, /*BMS硬件故障*/
    kMotorEventBmsOverDischarge         = 0x02, /*BMS检测到过放*/
    kMotorEventBmsOverHighTemperature   = 0x04, /*BMS检测到过温*/
    kMotorEventBmsUnderVoltage          = 0x08, /*BMS检测到欠压*/
    kMotorEventAllEvents = 0xFF
}MotorEventFlag;

void motor_init(void);
void motor_uninit(void);

BOOLEAN motor_is_connected(void);

Result motor_enable_event(INT16U event);
Result motor_disable_event(INT16U event);
Result motor_get_event(INT16U* event);


INT16U motor_get_tyremm(void); /*轮胎直径 0.1 Inch/bit*/
INT16U motor_get_speed_rate(void); /*速比 0.01/bit*/
INT16U motor_get_gear_rate(void);  /*减速箱齿轮比 0.01/bit*/
INT16U motor_get_tyre_rate(void);  /*后轮比 0.01 /bit*/
Result motor_set_tyremm(INT16U value);
Result motor_set_speed_rate(INT16U value);
Result motor_set_gear_rate(INT16U value);
Result motor_set_tyre_rate(INT16U value);

INT16U motor_get_rotation(void); /*转速1rpm/bit */
INT16U motor_get_speed(void); /*时速1kmph/bit */
INT16U motor_get_mile(void); /*临时里程0.1km/bit*/
INT32U motor_get_total_mile(void); /*累积里程0.1km/bit*/
INT16U motor_get_temperature(void); /*电机控制器温度,1摄氏度/bit  */
INT16S motor_get_fault_code(void); /*电机故障码*/


#endif /* BMS_MOTOR_H_ */
