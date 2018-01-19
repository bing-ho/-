/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file app_defs.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-6-8
*
*/

#ifndef APP_DEFS_H_
#define APP_DEFS_H_


/**********************************************
 *
 * 模块定义
 *
 ***********************************************/
/** common */
#define BMS_MODULE_DISABLE      0
#define BMS_MODULE_ENABLE       1
#define BMS_MODULE_DEFAULT      BMS_MODULE_ENABLE

/** charger */
#define BMS_CHARGER_TIECHENG    1 // 铁城充电机
#define BMS_CHARGER_GB          2 // 国标充电机

/** HMI */
#define BMS_HMI_DEFAULT         1

/** upper computer */
#define BMS_UPPER_COMPUTER_DEFAULT  1 //缺省上位机
#define BMS_UPPER_COMPUTER_J1939    2 //J1939通信上位机

/** ems */
#define BMS_EMS_DEFAULT         1

/** MOTOR */
#define BMS_MOTOR_DEFAULT       1

/** DTU */
#define BMS_DTU_DEFAULT         BMS_DTU_BUILDIN
#define BMS_DTU_CAN             1
#define BMS_DTU_RS485           2
#define BMS_DTU_BUILDIN         3

/** SLAVE */
#define BMS_SLAVE_CAN                   1
#define BMS_SLAVE_RS485                 2
#define BMS_SLAVE_DEFAULT               BMS_SLAVE_CAN

/** SOC Type */
#define BMS_SOC_INTEGRATION             1

#define BMS_SOH_DEFAULT                 1

/** Data Save */
#define BMS_DATA_SAVE_SDCARD            1
#define BMS_DATA_SAVE_FLASH             2
/** Battery TYpe */
#define BMS_BATTERY_LIFEPO4             1  //用于磷酸铁锂电池管理
#define BMS_BATTERY_LIMNO2              2  //用于锰酸锂电池管理

/** Communication Mode */
#define BMS_COMM_SLAVE_MODE             0
#define BMS_COMM_BYU_MODE               1
#define BMS_COMM_SLAVE_AND_BYU_MODE     2
#define BMS_SELF_CHECK_MODE             3

/** Insulation Comm Mode */
#define INSULATION_ON_BOARD_MODE        0
#define INSULATION_SLAVE_MODE           1

#endif /* APP_DEFS_H_ */
