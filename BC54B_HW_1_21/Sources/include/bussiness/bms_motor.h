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
    kMotorEventBmsHardwareException     = 0x01, /*BMSӲ������*/
    kMotorEventBmsOverDischarge         = 0x02, /*BMS��⵽����*/
    kMotorEventBmsOverHighTemperature   = 0x04, /*BMS��⵽����*/
    kMotorEventBmsUnderVoltage          = 0x08, /*BMS��⵽Ƿѹ*/
    kMotorEventAllEvents = 0xFF
}MotorEventFlag;

void motor_init(void);
void motor_uninit(void);

BOOLEAN motor_is_connected(void);

Result motor_enable_event(INT16U event);
Result motor_disable_event(INT16U event);
Result motor_get_event(INT16U* event);


INT16U motor_get_tyremm(void); /*��ֱ̥�� 0.1 Inch/bit*/
INT16U motor_get_speed_rate(void); /*�ٱ� 0.01/bit*/
INT16U motor_get_gear_rate(void);  /*��������ֱ� 0.01/bit*/
INT16U motor_get_tyre_rate(void);  /*���ֱ� 0.01 /bit*/
Result motor_set_tyremm(INT16U value);
Result motor_set_speed_rate(INT16U value);
Result motor_set_gear_rate(INT16U value);
Result motor_set_tyre_rate(INT16U value);

INT16U motor_get_rotation(void); /*ת��1rpm/bit */
INT16U motor_get_speed(void); /*ʱ��1kmph/bit */
INT16U motor_get_mile(void); /*��ʱ���0.1km/bit*/
INT32U motor_get_total_mile(void); /*�ۻ����0.1km/bit*/
INT16U motor_get_temperature(void); /*����������¶�,1���϶�/bit  */
INT16S motor_get_fault_code(void); /*���������*/


#endif /* BMS_MOTOR_H_ */
