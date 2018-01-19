/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_motor_impl.h
* @brief
* @note
* @author
* @date 2012-5-25
*
*/

#ifndef BMS_MOTOR_IMPL_H_
#define BMS_MOTOR_IMPL_H_
#include "bms_motor.h"

#define MOTOR_CAN_RE_INIT_TIME_WITH_COMM_ABORT      600000//10min ���CANͨ���жϺ�������ʱʱ�� ��λ��ms

#define MOTOR_ROTATION_MAX  12000
#define MOTOR_MILE_MAX 3000
#define MOTOR_SAVE_MILE_MAX 1 // 0.1km/h
#define MOTOR_MILE_CHANGE_MAX_ONCE  20

#define MOTOR_CONTROL_BYTE_HARDWARE_BIT 0 /*BMSӲ������*/
#define MOTOR_CONTROL_BYTE_ODV_BIT      1 /*BMS��⵽����*/
#define MOTOR_CONTROL_BYTE_OHT_BIT      2 /*BMS��⵽����*/
#define MOTOR_CONTROL_BYTE_UDV_BIT      3 /*BMS��⵽Ƿѹ*/

typedef struct{
    INT16U speed;        /*����ٶ� ��λ���棺1kmph��1mph*/
    INT16U rotation;     /*���ת��  1rpm*/
    INT16U mile;         /*��ǰ��ʻ�����(�޼���) ��λ���棺0.1km*/
    INT8U tempera;       /*����������¶� ��λ���棺1���϶� ƫ������-40���϶�*/
    INT16U fault_code;    /*������ϴ���*/
    INT8U comm_state;     /*���������ͨѶ״̬0���� 1����*/
}MotorRxInfo;

typedef struct{
    INT16U voltage;     /*������е���͵�ѹ ��λ���棺0.1V*/
    INT16U current;     /*�������������� ��λ���棺0.1A*/
    INT8U  control_byte; /*�����ĵ�����ж���*/
}MotorTxInfo;

typedef struct
{
    can_t can;

    INT16U tyremm; /*��ֱ̥�� 0.1 inch/bit*/
    INT16U speed_rate; /*�ٱ� 0.01/bit*/
    INT16U gear_rate; /*���ֱ� 0.01/bit*/
    INT16U tyre_rate; /*���ֱ� 0.01/bit*/
    INT16U speed_factor; /*�ٶȱ�������*/
    INT16U temp_mile; /*BMS��¼����ʱ�����0.1 Km/bit*/
    INT32U total_mile; /*�ۻ������Ŀ*/
    INT16U mile_save; /*�����������totalMile����65,535�Ժ������*/
    INT16U mile_cycle; /*���Ȧ��,��totalMile����65,535�ı���*/

    MotorTxInfo tx_info;
    MotorRxInfo rx_info;
} MotorContext;

void motor_task_create(void);
void motor_task_tx_run(void*);
void motor_task_rx_run(void*);

void motor_task_rx_process_message(CanMessage* _PAGED message);

INT8U motor_get_control_byte(void);


#endif /* BMS_MOTOR_IMPL_H_ */
