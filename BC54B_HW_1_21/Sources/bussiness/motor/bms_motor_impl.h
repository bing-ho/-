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

#define MOTOR_CAN_RE_INIT_TIME_WITH_COMM_ABORT      600000//10min 电机CAN通信中断后重启延时时间 单位：ms

#define MOTOR_ROTATION_MAX  12000
#define MOTOR_MILE_MAX 3000
#define MOTOR_SAVE_MILE_MAX 1 // 0.1km/h
#define MOTOR_MILE_CHANGE_MAX_ONCE  20

#define MOTOR_CONTROL_BYTE_HARDWARE_BIT 0 /*BMS硬件故障*/
#define MOTOR_CONTROL_BYTE_ODV_BIT      1 /*BMS检测到过放*/
#define MOTOR_CONTROL_BYTE_OHT_BIT      2 /*BMS检测到过温*/
#define MOTOR_CONTROL_BYTE_UDV_BIT      3 /*BMS检测到欠压*/

typedef struct{
    INT16U speed;        /*电机速度 单位增益：1kmph或1mph*/
    INT16U rotation;     /*电机转速  1rpm*/
    INT16U mile;         /*当前行驶的里程(无记忆) 单位增益：0.1km*/
    INT8U tempera;       /*电机控制器温度 单位增益：1摄氏度 偏移量：-40摄氏度*/
    INT16U fault_code;    /*电机故障代码*/
    INT8U comm_state;     /*电机控制器通讯状态0正常 1离线*/
}MotorRxInfo;

typedef struct{
    INT16U voltage;     /*电机运行的最低电压 单位增益：0.1V*/
    INT16U current;     /*电机允许的最大电流 单位增益：0.1A*/
    INT8U  control_byte; /*期望的电机运行动作*/
}MotorTxInfo;

typedef struct
{
    can_t can;

    INT16U tyremm; /*轮胎直径 0.1 inch/bit*/
    INT16U speed_rate; /*速比 0.01/bit*/
    INT16U gear_rate; /*齿轮比 0.01/bit*/
    INT16U tyre_rate; /*后轮比 0.01/bit*/
    INT16U speed_factor; /*速度比例因子*/
    INT16U temp_mile; /*BMS记录的临时里程数0.1 Km/bit*/
    INT32U total_mile; /*累积里程数目*/
    INT16U mile_save; /*里程余数，是totalMile除以65,535以后的余数*/
    INT16U mile_cycle; /*里程圈数,是totalMile除以65,535的倍数*/

    MotorTxInfo tx_info;
    MotorRxInfo rx_info;
} MotorContext;

void motor_task_create(void);
void motor_task_tx_run(void*);
void motor_task_rx_run(void*);

void motor_task_rx_process_message(CanMessage* _PAGED message);

INT8U motor_get_control_byte(void);


#endif /* BMS_MOTOR_IMPL_H_ */
