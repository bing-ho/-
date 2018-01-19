/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file app_cfg.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-6-8
*
*/

#ifndef APP_CFG_H_
#define APP_CFG_H_                                          
#include "app_defs.h"
#include "bms_unit.h"
#include "bms_input_signal_control.h"
/*********************************************************************************
 *
 * UDS Enabled/Disabled
 *
*********************************************************************************/
#define SWITCH_ON       (1)                                       
#define SWITCH_OFF      (0)
#define UDS_SWITCH_CONFIG   SWITCH_OFF

/**********************************************            
 *
 * Module Enabled/Disabled
 *
 ***********************************************/
#define BMS_SUPPORT_BATTERY             BMS_BATTERY_LIFEPO4 // 电池类型

#define BMS_COMMUNICATION_MODE          BMS_COMM_SLAVE_MODE // 主从通信模式

#define BMS_SUPPORT_HARDWARE_BEFORE     0                   // 主机支持硬件向前兼容 yang 20161213

#define BMS_SUPPORT_LTC6804_ON_BOARD_CHIP_NUM    4
#define BMS_SUPPORT_HARDWARE_LTC6804       1                   // 主机支持6804 cao 20170108
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1
    #define BMS_SUPPORT_HARDWARE_LTC6803   0
#else 
    #define BMS_SUPPORT_HARDWARE_LTC6803   1
#endif

#define BMS_SUPPORT_SOC0_DISCHG_FACTOR  1 

#define BMS_SUPPORT_SOC                 BMS_SOC_INTEGRATION// // SOC检测模块
#define BMS_SUPPORT_SOH                 BMS_SOH_DEFAULT
#define BMS_SUPPORT_DATA_SAVE           1//BMS_DATA_SAVE_FLASH //BMS_DATA_SAVE_SDCARD // 数据存储模块//
#define BMS_SUPPORT_SYSTEM_VOLTAGE      1
#define BMS_SUPPORT_BOARD_TEMPERATURE   1

#define BMS_SUPPORT_HMI                 BMS_HMI_DEFAULT//BMS_HMI_DEFAULT // BMS_HMI_DEFAULT  // 屏程序
#define BMS_SUPPORT_UPPER_COMPUTER      BMS_UPPER_COMPUTER_J1939 // 上位机模块
#define BMS_SUPPORT_DTU                 BMS_DTU_DEFAULT // DTU模块

#define BMS_SUPPORT_EMS                 BMS_EMS_DEFAULT //整车系统
#define BMS_SUPPORT_CHARGER             BMS_CHARGER_TIECHENG //BMS_CHARGER_TIECHENG //BMS_CHARGER_TIECHENG // BMS_CHARGER_TIECHENG //充电机模块
#define BMS_SUPPORT_RELAY               1 // 继电器模块
#define BMS_SUPPORT_INPUT_SIGNAL        1 // 输入开关信号
#define BMS_SUPPORT_MOTOR               0 //BMS_MOTOR_DEFAULT  // 电机模块

#define BMS_SUPPORT_INSULATION_DIAGNOSE 1 // 绝缘检查争端模块

#define BMS_SUPPORT_PRE_DISCHARGE       1           //启用预充电功能
#define BMS_SUPPORT_HVCM_CTL            0           //强电控制盒功能

#define BMS_SUPPORT_J1939_COMM            1    //J1939通信支持

#define BMS_SUPPORT_LED_CONTROL         0

#define BMS_SUPPORT_SLEEP_MODE          0
#define BMS_SUPPORT_STAT                1  // 支持统计功能

#define BMS_SUPPORT_LOAD_NODE_DETCTION  0  // 负载检测， 仅C32平台支持

#define BMS_SUPPORT_STAT_DEBUG_INFO     0  // 支持统计功能

#define BMS_SUPPORT_DOG_WATCHER         1

#define BMS_SUPPORT_CAN_RE_INIT         0

#define BMS_SUPPORT_PCM_MODE            0

#define BMS_DATASAVE_SUPPORT_ENGLISH    0 //SD卡使用英文数据存储

#define BMS_FORTH_ALARM_SUPPORT         0 //四级报警支持，为0时仅为三级报警

#define BMU_CHR_OUTLET_TEMP_SUPPORT     0 //从机支持充电插座温度检测

/* ****************硬件版本号宏开关************************** */
#define BMS_SUPPORT_1_21                     1

#define BMS_SUPPORT_BC5XB_N                  0 //是否支持BC54BN版本

#define BMS_SUPPORT_1_21_B                   0

#define BMS_SUPPORT_NEGATIVE_ADHESION_1_23   0//BMS_SUPPORT_NEGATIVE_ADHESION //是否支持1.23总负粘连检测

#define BMS_SUPPORT_NEGATIVE_ADHESION_1_23B  0

#define BMS_SUPPORT_BY5248D                  0 //如果支持1.1A版本需要将BMS_SUPPORT_BY5248D_1_1A设置为1


#if (BMS_SUPPORT_1_21 ||  BMS_SUPPORT_BC5XB_N) //支持1.21或1.21N硬件版本

   #define BMS_SUPPORT_NEGATIVE_ADHESION   0 //硬件是否支持总负粘连功能

   #define BMS_SUPPORT_CP_DOWN_POWER       0 //硬件是否支持CP充满下电功能

   #define BMS_SUPPORT_CURRENT_DIVIDER     0  //硬件是否支持分流器(单独使用IIC1通道)

   #define BMS_SUPPORT_CAN_WKUP            0  //硬件是否支持CAN唤醒

   #define BMS_SUPPORT_MLOCK               1  //硬件是否支持电子锁
   
#endif

#if BMS_SUPPORT_1_21_B      //支持1.21B硬件版本       

   #define BMS_SUPPORT_NEGATIVE_ADHESION   0 //硬件是否支持总负粘连功能

   #define BMS_SUPPORT_CP_DOWN_POWER       1 //硬件是否支持CP充满下电功能

   #define BMS_SUPPORT_CURRENT_DIVIDER     0  //硬件是否支持分流器(单独使用IIC1通道)

   #define BMS_SUPPORT_CAN_WKUP            0  //硬件是否支持CAN唤醒

   #define BMS_SUPPORT_MLOCK               1  //硬件是否支持电子锁
   
#endif

#if BMS_SUPPORT_NEGATIVE_ADHESION_1_23  //支持1.23硬件版本

   #define BMS_SUPPORT_NEGATIVE_ADHESION   1 //硬件是否支持总负粘连功能

   #define BMS_SUPPORT_CP_DOWN_POWER       0 //硬件是否支持CP充满下电功能

   #define BMS_SUPPORT_CURRENT_DIVIDER     0  //硬件是否支持分流器(单独使用IIC1通道)

   #define BMS_SUPPORT_CAN_WKUP            0  //硬件是否支持CAN唤醒

   #define BMS_SUPPORT_MLOCK               1  //硬件是否支持电子锁
   
#endif

#if BMS_SUPPORT_NEGATIVE_ADHESION_1_23B  //支持1.23B硬件版本

   #define BMS_SUPPORT_NEGATIVE_ADHESION   1 //硬件是否支持总负粘连功能

   #define BMS_SUPPORT_CP_DOWN_POWER       1 //硬件是否支持CP充满下电功能

   #define BMS_SUPPORT_CURRENT_DIVIDER     0  //硬件是否支持分流器(单独使用IIC1通道)

   #define BMS_SUPPORT_CAN_WKUP            0  //硬件是否支持CAN唤醒

   #define BMS_SUPPORT_MLOCK               1  //硬件是否支持电子锁
   
#endif

#if BMS_SUPPORT_BY5248D                 //支持高精度一体机硬件版本

   #define BMS_SUPPORT_NEGATIVE_ADHESION   1 //硬件是否支持总负粘连功能

   #define BMS_SUPPORT_CP_DOWN_POWER       0 //硬件是否支持CP充满下电功能

   #define BMS_SUPPORT_CURRENT_DIVIDER     1  //硬件是否支持分流器(单独使用IIC1通道)

   #define BMS_SUPPORT_CAN_WKUP            1  //硬件是否支持CAN唤醒

   #define BMS_SUPPORT_MLOCK               0  //硬件是否支持电子锁
   
   #define BMS_SUPPORT_BY5248D_1_1A        1//高精度一体机硬件支持1.1A版本 
    
#endif

#if   (BMS_SUPPORT_1_21+BMS_SUPPORT_BC5XB_N+BMS_SUPPORT_1_21_B+BMS_SUPPORT_NEGATIVE_ADHESION_1_23+BMS_SUPPORT_NEGATIVE_ADHESION_1_23B+BMS_SUPPORT_BY5248D)!=1
#error "错误"
#endif



/*********************************************************************************
 *
 *                          任务优先级分配
 *
*********************************************************************************/
#define  OS_TASK_START_PRIO                     1
#define  CURRENT_SCAN_TASK_PRIO                 2
#define  SLAVECAN_RX_TASK_PRIO                  4
#define  J1939_POLL_TASK_PRIO                   3
#define  SLAVECAN_TX_TASK_PRIO                  5
#define  CHARGER_TX_TASK_PRIO                   6
#define  CHARGER_RX_TASK_PRIO                   7
#define  DBDCAN_TX_TASK_PRIO                    8
#define  ECUCAN_TX_TASK_PRIO                    9
#define  ECUCAN_RX_TASK_PRIO                    10  
#define  BMS_MAIN_JOB_TASK_PRIO                 11
#define  HMI_RX_TASK_PRIO                       12
#define  MCU_RX_TASK_PRIO                       13
#define  MCU_TX_TASK_PRIO                       14
#define  DTU_TX_TASK_PRIO                       15
#define  DTU_RX_TASK_PRIO                       16
//#define  BAT_BALANCE_TASK_PRIO                17 
#define  SYSVOLT_SCAN_TASK_PRIO                 17
#define  RULE_ENGINE_TASK_PRIO                  18
#define  RES_SCAN_TASK_PRIO                     19
#define  VOLT_SCAN_TASK_PRIO                    20
#define  AD_SAMPLE_TASK_PRIO                    21
#define  SD_DETECT_TASK_PRIO                    22
#define  DATA_SAVE_TASK_PRIO                    23
#define  GPRS_TASK_PRIO                         24
#define  UDS_TASK_PRIO                          25


/*********************************************************************************
 *
 *                           任务堆栈大小分配
 *
*********************************************************************************/
#define OS_TASK_START_STK_SIZE                  300
#define CURRENT_SCAN_STK_SIZE                   200
#define HMI_RX_STK_SIZE                         210
#define MCU_RX_TASK_STK_SIZE                    180
#define MCU_TX_TASK_STK_SIZE                    180
#define DTU_TX_STK_SIZE                         180
#define DTU_RX_STK_SIZE                         180
#define BMS_MAIN_JOB_TASK_STK_SIZE              280//230
#define CHARGER_RX_STK_SIZE                     180
#define CHARGER_TX_STK_SIZE                     220
#define DATA_SAVE_STK_SIZE                      400
#define DBD_TX_STK_SIZE                         180
#define ECU_TX_STK_SIZE                         230//180
#define ECU_RX_STK_SIZE                         450//180 
#define SLAVE_TX_STK_SIZE                       250
#define SLAVE_RX_STK_SIZE                       260//210  添加CAN读FLASH功能后需增加此堆栈大小
//#define SD_DETECT_STK_SIZE                      180
#define RULE_ENGINE_STK_SIZE                    400//650
#define J1939_POLL_TASK_STK_SIZE                220
#define AD_SAMPLE_TASK_STK_SIZE                 220
#define BAT_BALANCE_TASK_STK_SIZE               200
#define RES_SAMPLE_TASK_STK_SIZE                300
#define VOLT_SAMPLE_TASK_STK_SIZE               220
//#define UDS_TASK_STK_SIZE                       350                



/*********************************************************************************
 *
 *                           任务周期，单位ms
 *
*********************************************************************************/
#define CURRENT_SAMPLE_MIN_PERIODIC       10          /*最小采样电流周期， 指采集一组电流的最小间隔 */
#define BAT_SCAN_PERIODIC                 50          /*电池扫描周期BCP=BAT_SCAN_PERIODIC+ALERT_BEEP_ON_PERIODIC*/

#define CHECK_STATUS_PERIODIC             200
#define MAIN_JOB_CHECK_PERIODIC           20   /* 主job检查周期 */
#define MAIN_JOB_COMMON_PERIODIC          100  /* 一般job的时延 */

#define ChgLock_JOB_STATUS_PERIODIC       20  

#define CURRENT_AD_JOB_CHECK_PERIODIC     10  
#define BCU_UPDATE_JOB_PERIODIC           50   /* bcu job时延 */
#define CHECK_SYSTE_STATE_PERIODIC        600
#define STAT_REFRESH_PERIDOIC             500
#define ALERT_JOB_PERIODIC                50
#define FEEDDOG_JOB_PERIODIC              200
#define DTU_SEND_DATA_JOB_PERIODIC       200
#define CAN_RE_INIT_JOB_PERIODIC          1000 /*CAN重新初始化检测周期*/
/*********************************************************************************
 *
 *                     系统参数默认值
 *
*********************************************************************************/
#define CURRENT_DIVIDER_TYPE           3             /* 电流采集类型 - 分流器编号3*/

#define CURRENT_RES_DEF                1875          /* 分流器默认阻值1875uΩ*/
#define CURRENT_RES_MIN                0
#define CURRENT_RES_MAX                65535

#define CAN_WKUP_ENABLE                1
#define CAN_WKUP_DISABLE               0

#define CAN_WKUP_DEF                   CAN_WKUP_DISABLE     /* CAN唤醒默认不使能 */
#define CAN_WKUP_MIN                   CAN_WKUP_DISABLE
#define CAN_WKUP_MAX                   CAN_WKUP_ENABLE

#define CLOCK_NOT_SYNC                 0
#define CLOCK_IS_SYNC                  1

#define CLOCK_SYNC_DEF                 CLOCK_NOT_SYNC
#define CLOCK_SYNC_MIN                 CLOCK_NOT_SYNC
#define CLOCK_SYNC_MAX                 CLOCK_IS_SYNC

#define CURRENT_CALIB_K_DEF  1000   /*电流标定参数k初始化值，0.001分辨率*/ 
#define CURRENT_CALIB_K_MIN  800    /*电流标定参数k最小值，0.001分辨率  */ 
#define CURRENT_CALIB_K_MAX  1200   /*电流标定参数k最大值，0.001分辨率  */ 

#define CURRENT_CALIB_B_DEF  0      /*电流标定参数b初始化值，0.001分辨率*/ 
#define CURRENT_CALIB_B_MIN  -10000 /*电流标定参数b最小值，0.001分辨率  */ 
#define CURRENT_CALIB_B_MAX  10000  /*电流标定参数b最大值，0.001分辨率  */      

#define MAX_ERROR_CURRENT               20              /*霍尔电流传感器最大误差电流*/
#define ISENSOR_ZERO_AD_VALUE           2048            /*霍尔电流传感器零点AD值*/

#define ISENSOR_TYPE_DEF                kCurrent500Type /*霍尔电流传感器的量程选择150，350，600几档*/
#define ISENSOR_TYPE_MIN_DEF            kCurrentTypeStart /*霍尔电流传感器最小量程*/
#define ISENSOR_TYPE_MAX_DEF            kCurrentTypeStop/*霍尔电流传感器最大量程*/

#define ISENSOR_RANGE_TYPE_DEF          kCurrentRangeTypeSingle /*霍尔电流传感器的量程类型，0-单量程，1-双量程*/
#define ISENSOR_RANGE_TYPE_MIN          kCurrentRangeTypeSingle
#define ISENSOR_RANGE_TYPE_MAX          CURRENT_DIVIDER_TYPE//kCurrentRangeTypeTwoSingle//kCurrentRangeTypeDouble

#define BalaceEnVolt                    50              //跨箱体均衡开启压差
#define EEEPROM_FORMAT_FLAG_DEF         0               /*定义增强型EEPROM格式化标志默认值*/

#define TOTALCAP_DEF                    1000 // 0.1Ah/bit   #define TOTALCAP_DEF                    100             /*总容量默认值 1Ah/bit*/
#define LEFTCAP_DEF                     1000 // 0.1Ah/bit   #define LEFTCAP_DEF                     100             /*剩余容量默认值 1Ah/bit*/
#define CAP_MIN_DEF                        0                /*容量最小值定义 1Ah/bit*/
#define MAXCAP_DEF                      (TOTALCAP_DEF + MAX_CAP_OFFSET) // 0.1Ah/bit /*定义最大可能容量值*/
#define NOMINAL_CAP_DEF                 1000 // 0.1Ah/bit #define NOMINAL_CAP_DEF                 100             /**标称容量 */
#define NOMINAL_CAP_MAX_DEF             11910// 0.1Ah/bit #define NOMINAL_CAP_MAX_DEF             1191
#define CAP_MAX_DEF                     (NOMINAL_CAP_MAX_DEF + MAX_CAP_OFFSET)  // 0.1Ah/bit /*容量最大值定义 1Ah/bit*/

#define SYS_DEFAULT_DLY_MIN_DEF            0                /*定义系统延时最小值*/
#define SYS_DEFAULT_DLY_MAX_DEF            0xFFFF            /*定义系统延时最大值*/

#define SYS_DEFAULT_ALARM_DLY_MIN_DEF    1000            /*定义报警和保护延时最小值*/
#define SYS_DEFAULT_ALARM_DLY_MAX_DEF    0xFFFF            /*定义报警和保护延时最大值*/

#define SYS_DEFAULT_VOLT_MIN_DEF        0               /*定义电压报警和保护最小值*/
#define SYS_DEFAULT_VOLT_MAX_DEF        5000            /*定义电压报警和保护最大值*/

#define SYS_DEFAULT_TEMP_MIN_DEF        0                /*定义温度报警和保护最小值，偏移量为-50℃*/
#define SYS_DEFAULT_TEMP_MAX_DEF        175                /*定义温度报警和保护最大值，偏移量为-50℃*/

#define SYS_DEFAULT_CUR_MIN_DEF         0               /*定义电流报警和保护最小值*/
#define SYS_DEFAULT_CUR_MAX_DEF         0x7FFF          /*定义电流报警和保护最大值*/

#define SYS_DEFAULT_DT_MIN_DEF            0                /*定义温差报警最小值*/
#define SYS_DEFAULT_DT_MAX_DEF            1000            /*定义温差报警最大值*/

#define SYS_DEFAULT_DV_MIN_DEF            5                /*定义压差报警最小值   1mV/bit*/
#define SYS_DEFAULT_DV_MAX_DEF            5000            /*定义压差报警最大值   1mV/bit*/

#define SYS_DEFAULT_SOC_MIN_DEF            PERCENT_TO_SOC(0)    /*SOC报警最小值*/
#define SYS_DEFAULT_SOC_MAX_DEF            PERCENT_TO_SOC(150)    /*SOC报警最大值*/

#define SYS_DEFAULT_TV_MIN_DEF            0                /*定义总压报警最小值*/
#define SYS_DEFAULT_TV_MAX_DEF            0xFFFF            /*定义总压报警最大值*/

#define SYS_DEFAULT_PINSU_MIN_DEF        0                /*定义严重漏电报警释放最小值 1ohm/v/bit*/
#define SYS_DEFAULT_PINSU_MAX_DEF        5000            /*定义严重漏电报警释放最大值 1ohm/v/bit*/

#define SYS_CAN_CHANNEL_MIN_DEF         kCanDev0
#define SYS_CAN_CHANNEL_MAX_DEF         kCanDevMaxCount - 1

#define SYS_RS485_CHANNEL_MIN_DEF       kRs485Dev0
#define SYS_RS485_CHANNEL_MAX_DEF       kRs485Dev1

#define DCHG_HV_FST_ALARM_DEF           3600            /*定义放电高压一级报警阀值*/
#define DCHG_HV_FST_ALARM_REL_DEF       3400            /*定义放电高压一级报警释放阀值*/
#define DCHG_HV_SND_ALARM_DEF           3700            /*定义放电高压二级报警阀值*/
#define DCHG_HV_SND_ALARM_REL_DEF       3600            /*定义放电高压二级报警释放阀值*/
#define DCHG_HV_TRD_ALARM_DEF           3900            /*定义放电高压三级报警阀值*/
#define DCHG_HV_TRD_ALARM_REL_DEF       3700            /*定义放电高压三级报警释放阀值*/

#define DCHG_HV_FST_ALARM_DLY_DEF       5000            /*定义放电高压一级报警延时*/
#define DCHG_HV_FST_ALARM_REL_DLY_DEF   6000            /*定义放电高压一级报警释放延时*/
#define DCHG_HV_SND_ALARM_DLY_DEF       3000            /*定义放电高压二级报警延时*/
#define DCHG_HV_SND_ALARM_REL_DLY_DEF   4000            /*定义放电高压二级报警释放延时*/
#define DCHG_HV_TRD_ALARM_DLY_DEF       3000            /*定义放电高压三级报警延时*/
#define DCHG_HV_TRD_ALARM_REL_DLY_DEF   4000            /*定义放电高压三级报警释放延时*/

#define CHG_HV_FST_ALARM_DEF            3600            /*定义充电高压一级报警阀值*/
#define CHG_HV_FST_ALARM_REL_DEF        3400            /*定义充电高压一级报警释放阀值*/
#define CHG_HV_SND_ALARM_DEF            3700            /*定义充电高压二级报警阀值*/
#define CHG_HV_SND_ALARM_REL_DEF        3600            /*定义充电高压二级报警释放阀值*/
#define CHG_HV_TRD_ALARM_DEF            3900            /*定义充电高压三级报警阀值*/
#define CHG_HV_TRD_ALARM_REL_DEF        3700            /*定义充电高压三级报警释放阀值*/

#define CHG_HV_FST_ALARM_DLY_DEF        5000            /*定义充电高压一级报警延时*/
#define CHG_HV_FST_ALARM_REL_DLY_DEF    6000            /*定义充电高压一级报警释放延时*/
#define CHG_HV_SND_ALARM_DLY_DEF        3000            /*定义充电高压二级报警延时*/
#define CHG_HV_SND_ALARM_REL_DLY_DEF    4000            /*定义充电高压二级报警释放延时*/
#define CHG_HV_TRD_ALARM_DLY_DEF        3000            /*定义充电高压三级报警延时*/
#define CHG_HV_TRD_ALARM_REL_DLY_DEF    4000            /*定义充电高压三级报警释放延时*/

#define DCHG_LV_FST_ALARM_DEF           2900            /*定义放电低压一级报警阀值*/
#define DCHG_LV_FST_ALARM_REL_DEF       3000            /*定义放电低压一级报警释放阀值*/
#define DCHG_LV_SND_ALARM_DEF           2600            /*定义放电低压二级报警阀值*/
#define DCHG_LV_SND_ALARM_REL_DEF       2800            /*定义放电低压二级报警释放阀值*/
#define DCHG_LV_TRD_ALARM_DEF           2500            /*定义放电低压三级报警阀值*/
#define DCHG_LV_TRD_ALARM_REL_DEF       2600            /*定义放电低压三级报警释放阀值*/

#define DCHG_LV_FST_ALARM_DLY_DEF       5000            /*定义放电低压一级报警延时*/
#define DCHG_LV_FST_ALARM_REL_DLY_DEF   6000            /*定义放电低压一级报警释放延时*/
#define DCHG_LV_SND_ALARM_DLY_DEF       5000            /*定义放电低压二级报警延时*/
#define DCHG_LV_SND_ALARM_REL_DLY_DEF   6000            /*定义放电低压二级报警释放延时*/
#define DCHG_LV_TRD_ALARM_DLY_DEF       5000            /*定义放电低压三级报警延时*/
#define DCHG_LV_TRD_ALARM_REL_DLY_DEF   6000            /*定义放电低压三级报警释放延时*/

#define CHG_LV_FST_ALARM_DEF            2900            /*定义充电低压一级报警阀值*/
#define CHG_LV_FST_ALARM_REL_DEF        3000            /*定义充电低压一级报警释放阀值*/
#define CHG_LV_SND_ALARM_DEF            2600            /*定义充电低压二级报警阀值*/
#define CHG_LV_SND_ALARM_REL_DEF        2800            /*定义充电低压二级报警释放阀值*/
#define CHG_LV_TRD_ALARM_DEF            2500            /*定义充电低压三级报警阀值*/
#define CHG_LV_TRD_ALARM_REL_DEF        2600            /*定义充电低压三级报警释放阀值*/

#define CHG_LV_FST_ALARM_DLY_DEF        5000            /*定义充电低压一级报警延时*/
#define CHG_LV_FST_ALARM_REL_DLY_DEF    6000            /*定义充电低压一级报警释放延时*/
#define CHG_LV_SND_ALARM_DLY_DEF        5000            /*定义充电低压二级报警延时*/
#define CHG_LV_SND_ALARM_REL_DLY_DEF    6000            /*定义充电低压二级报警释放延时*/
#define CHG_LV_TRD_ALARM_DLY_DEF        5000            /*定义充电低压三级报警延时*/
#define CHG_LV_TRD_ALARM_REL_DLY_DEF    6000            /*定义充电低压三级报警释放延时*/

#define DCHG_OC_FST_ALARM_DEF           0x7FFF            /*定义放电过流一级报警阀值 0.1A/bit*/
#define DCHG_OC_FST_ALARM_REL_DEF       0x7FFF            /*定义放电过流一级报警释放阀值 0.1A/bit*/
#define DCHG_OC_SND_ALARM_DEF           0x7FFF            /*定义放电过流二级报警阀值 0.1A/bit*/
#define DCHG_OC_SND_ALARM_REL_DEF       0x7FFF            /*定义放电过流二级报警释放阀值 0.1A/bit*/
#define DCHG_OC_TRD_ALARM_DEF           0x7FFF            /*定义放电过流三级报警阀值 0.1A/bit*/
#define DCHG_OC_TRD_ALARM_REL_DEF       0x7FFF            /*定义放电过流三级报警释放阀值 0.1A/bit*/

#define DCHG_OC_FST_ALARM_DLY_DEF       5000            /*定义放电过流一级报警延时*/
#define DCHG_OC_FST_ALARM_REL_DLY_DEF   6000            /*定义放电过流一级报警释放延时*/
#define DCHG_OC_SND_ALARM_DLY_DEF       3000            /*定义放电过流二级报警延时*/
#define DCHG_OC_SND_ALARM_REL_DLY_DEF   4000            /*定义放电过流二级报警释放延时*/
#define DCHG_OC_TRD_ALARM_DLY_DEF       3000            /*定义放电过流三级报警延时*/
#define DCHG_OC_TRD_ALARM_REL_DLY_DEF   4000            /*定义放电过流三级报警释放延时*/

#define CHG_OC_FST_ALARM_DEF            0x7FFF            /*定义充电过流一级报警阀值 0.1A/bit*/
#define CHG_OC_FST_ALARM_REL_DEF        0x7FFF            /*定义充电过流一级报警释放阀值 0.1A/bit*/
#define CHG_OC_SND_ALARM_DEF            0x7FFF            /*定义充电过流二级报警阀值 0.1A/bit*/
#define CHG_OC_SND_ALARM_REL_DEF        0x7FFF            /*定义充电过流二级报警释放阀值 0.1A/bit*/
#define CHG_OC_TRD_ALARM_DEF            0x7FFF            /*定义充电过流三级报警阀值 0.1A/bit*/
#define CHG_OC_TRD_ALARM_REL_DEF        0x7FFF            /*定义充电过流三级报警释放阀值 0.1A/bit*/

#define CHG_OC_FST_ALARM_DLY_DEF        5000            /*定义充电过流一级报警延时*/
#define CHG_OC_FST_ALARM_REL_DLY_DEF    6000            /*定义充电过流一级报警释放延时*/
#define CHG_OC_SND_ALARM_DLY_DEF        3000            /*定义充电过流二级报警延时*/
#define CHG_OC_SND_ALARM_REL_DLY_DEF    4000            /*定义充电过流二级报警释放延时*/
#define CHG_OC_TRD_ALARM_DLY_DEF        3000            /*定义充电过流三级报警延时*/
#define CHG_OC_TRD_ALARM_REL_DLY_DEF    4000            /*定义充电过流三级报警释放延时*/

#define DCHG_HT_FST_ALARM_DEF           100             /*定义放电高温一级报警阀值，偏移量为-50℃*/
#define DCHG_HT_FST_ALARM_REL_DEF       90              /*定义放电高温一级报警释放阀值，偏移量为-50℃*/
#define DCHG_HT_SND_ALARM_DEF           110             /*定义放电高温二级报警阀值，偏移量为-50℃*/
#define DCHG_HT_SND_ALARM_REL_DEF       100             /*定义放电高温二级报警释放阀值，偏移量为-50℃*/
#define DCHG_HT_TRD_ALARM_DEF           115             /*定义放电高温三级报警阀值，偏移量为-50℃*/
#define DCHG_HT_TRD_ALARM_REL_DEF       110             /*定义放电高温三级报警释放阀值，偏移量为-50℃*/

#define DCHG_HT_FST_ALARM_DLY_DEF       5000            /*定义放电高温一级报警延时*/
#define DCHG_HT_FST_ALARM_REL_DLY_DEF   6000            /*定义放电高温一级报警释放延时*/
#define DCHG_HT_SND_ALARM_DLY_DEF       3000            /*定义放电高温二级报警延时*/
#define DCHG_HT_SND_ALARM_REL_DLY_DEF   4000            /*定义放电高温二级报警释放延时*/
#define DCHG_HT_TRD_ALARM_DLY_DEF       1000            /*定义放电高温三级报警延时*/
#define DCHG_HT_TRD_ALARM_REL_DLY_DEF   2000            /*定义放电高温三级报警释放延时*/

#define CHG_HT_FST_ALARM_DEF            95              /*定义充电高温一级报警阀值，偏移量为-50℃*/
#define CHG_HT_FST_ALARM_REL_DEF        90              /*定义充电高温一级报警释放阀值，偏移量为-50℃*/
#define CHG_HT_SND_ALARM_DEF            105             /*定义充电高温二级报警阀值，偏移量为-50℃*/
#define CHG_HT_SND_ALARM_REL_DEF        100             /*定义充电高温二级报警释放阀值，偏移量为-50℃*/
#define CHG_HT_TRD_ALARM_DEF            115             /*定义充电高温三级报警阀值，偏移量为-50℃*/
#define CHG_HT_TRD_ALARM_REL_DEF        110             /*定义充电高温三级报警释放阀值，偏移量为-50℃*/

#define CHG_HT_FST_ALARM_DLY_DEF        5000            /*定义充电高温一级报警延时*/
#define CHG_HT_FST_ALARM_REL_DLY_DEF    6000            /*定义充电高温一级报警释放延时*/
#define CHG_HT_SND_ALARM_DLY_DEF        3000            /*定义充电高温二级报警延时*/
#define CHG_HT_SND_ALARM_REL_DLY_DEF    4000            /*定义充电高温二级报警释放延时*/
#define CHG_HT_TRD_ALARM_DLY_DEF        1000            /*定义充电高温三级报警延时*/
#define CHG_HT_TRD_ALARM_REL_DLY_DEF    2000            /*定义充电高温三级报警释放延时*/

#define DCHG_LT_FST_ALARM_DEF           50              /*定义放电低温一级报警阀值，偏移量为-50℃*/
#define DCHG_LT_FST_ALARM_REL_DEF       55              /*定义放电低温一级报警释放阀值，偏移量为-50℃*/
#define DCHG_LT_SND_ALARM_DEF           40              /*定义放电低温二级报警阀值，偏移量为-50℃*/
#define DCHG_LT_SND_ALARM_REL_DEF       50              /*定义放电低温二级报警释放阀值，偏移量为-50℃*/
#define DCHG_LT_TRD_ALARM_DEF           30              /*定义放电低温三级报警阀值，偏移量为-50℃*/
#define DCHG_LT_TRD_ALARM_REL_DEF       45              /*定义放电低温三级报警释放阀值，偏移量为-50℃*/

#define DCHG_LT_FST_ALARM_DLY_DEF       5000            /*定义放电低温一级报警延时*/
#define DCHG_LT_FST_ALARM_REL_DLY_DEF   6000            /*定义放电低温一级报警释放延时*/
#define DCHG_LT_SND_ALARM_DLY_DEF       5000            /*定义放电低温二级报警延时*/
#define DCHG_LT_SND_ALARM_REL_DLY_DEF   6000            /*定义放电低温二级报警释放延时*/
#define DCHG_LT_TRD_ALARM_DLY_DEF       5000            /*定义放电低温三级报警延时*/
#define DCHG_LT_TRD_ALARM_REL_DLY_DEF   6000            /*定义放电低温三级报警释放延时*/

#define CHG_LT_FST_ALARM_DEF            50              /*定义充电低温一级报警阀值，偏移量为-50℃*/
#define CHG_LT_FST_ALARM_REL_DEF        55              /*定义充电低温一级报警释放阀值，偏移量为-50℃*/
#define CHG_LT_SND_ALARM_DEF            45              /*定义充电低温二级报警阀值，偏移量为-50℃*/
#define CHG_LT_SND_ALARM_REL_DEF        50              /*定义充电低温二级报警释放阀值，偏移量为-50℃*/
#define CHG_LT_TRD_ALARM_DEF            35              /*定义充电低温三级报警阀值，偏移量为-50℃*/
#define CHG_LT_TRD_ALARM_REL_DEF        45              /*定义充电低温三级报警释放阀值，偏移量为-50℃*/

#define CHG_LT_FST_ALARM_DLY_DEF        5000            /*定义充电低温一级报警延时*/
#define CHG_LT_FST_ALARM_REL_DLY_DEF    6000            /*定义充电低温一级报警释放延时*/
#define CHG_LT_SND_ALARM_DLY_DEF        5000            /*定义充电低温二级报警延时*/
#define CHG_LT_SND_ALARM_REL_DLY_DEF    6000            /*定义充电低温二级报警释放延时*/
#define CHG_LT_TRD_ALARM_DLY_DEF        5000            /*定义充电低温三级报警延时*/
#define CHG_LT_TRD_ALARM_REL_DLY_DEF    6000            /*定义充电低温三级报警释放延时*/

#define DCHG_DT_FST_ALARM_DEF           10              /*定义放电温差一级报警阀值*/
#define DCHG_DT_FST_ALARM_REL_DEF       5               /*定义放电温差一级报警释放阀值*/
#define DCHG_DT_SND_ALARM_DEF           15              /*定义放电温差二级报警阀值*/
#define DCHG_DT_SND_ALARM_REL_DEF       12              /*定义放电温差二级报警释放阀值*/
#define DCHG_DT_TRD_ALARM_DEF           20              /*定义放电温差三级报警阀值*/
#define DCHG_DT_TRD_ALARM_REL_DEF       15              /*定义放电温差三级报警释放阀值*/

#define DCHG_DT_FST_ALARM_DLY_DEF       5000            /*定义放电温差一级报警延时*/
#define DCHG_DT_FST_ALARM_REL_DLY_DEF   6000            /*定义放电温差一级报警释放延时*/
#define DCHG_DT_SND_ALARM_DLY_DEF       5000            /*定义放电温差二级报警延时*/
#define DCHG_DT_SND_ALARM_REL_DLY_DEF   6000            /*定义放电温差二级报警释放延时*/
#define DCHG_DT_TRD_ALARM_DLY_DEF       5000            /*定义放电温差三级报警延时*/
#define DCHG_DT_TRD_ALARM_REL_DLY_DEF   6000            /*定义放电温差三级报警释放延时*/

#define CHG_DT_FST_ALARM_DEF            10              /*定义充电温差一级报警阀值*/
#define CHG_DT_FST_ALARM_REL_DEF        5               /*定义充电温差一级报警释放阀值*/
#define CHG_DT_SND_ALARM_DEF            15              /*定义充电温差二级报警阀值*/
#define CHG_DT_SND_ALARM_REL_DEF        12              /*定义充电温差二级报警释放阀值*/
#define CHG_DT_TRD_ALARM_DEF            20              /*定义充电温差三级报警阀值*/
#define CHG_DT_TRD_ALARM_REL_DEF        15              /*定义充电温差三级报警释放阀值*/

#define CHG_DT_FST_ALARM_DLY_DEF        5000            /*定义充电温差一级报警延时*/
#define CHG_DT_FST_ALARM_REL_DLY_DEF    6000            /*定义充电温差一级报警释放延时*/
#define CHG_DT_SND_ALARM_DLY_DEF        5000            /*定义充电温差二级报警延时*/
#define CHG_DT_SND_ALARM_REL_DLY_DEF    6000            /*定义充电温差二级报警释放延时*/
#define CHG_DT_TRD_ALARM_DLY_DEF        5000            /*定义充电温差三级报警延时*/
#define CHG_DT_TRD_ALARM_REL_DLY_DEF    6000            /*定义充电温差三级报警释放延时*/

#define COMM_FST_ALARM_DLY_DEF          15              /*通信中断延时时间默认值,单位：S */
#define COMM_FST_ALARM_REL_DLY_DEF        5                /*通信中断释放延时时间默认值*/
#define COMM_SND_ALARM_DLY_DEF          25              /*通信中断二级报警延时时间默认值,单位：S */
#define COMM_SND_ALARM_REL_DLY_DEF        5                /*通信中断二级报警释放延时时间默认值*/
#define COMM_TRD_ALARM_DLY_DEF          35              /*通信中断三级报警延时时间默认值,单位：S */
#define COMM_TRD_ALARM_REL_DLY_DEF        5                /*通信中断三级报警释放延时时间默认值*/
#define COMM_ABORT_ALARM_DLY_MIN        5               /*通信中断延时时间最小值,单位：S */
#define COMM_ABORT_ALARM_DLY_MAX        65              /*通信中断延时时间最大值,单位：S */
#define COMM_ABORT_ALARM_REL_DLY_MIN    1               /*通信中断释放延时时间最小值,单位：S*/
#define COMM_ABORT_ALARM_REL_DLY_MAX    65              /*通信中断释放延时时间最大值,单位：S*/

#define TEMP_LINE_FST_ALARM_DLY_DEF        10000            /*定义温感线脱落一级报警延时默认值*/
#define TEMP_LINE_FST_ALARM_REL_DLY_DEF    3000            /*定义温感脱落一级报警释放延时默认值*/
#define TEMP_LINE_SND_ALARM_DLY_DEF        20000            /*定义温感线脱落二级报警延时默认值*/
#define TEMP_LINE_SND_ALARM_REL_DLY_DEF    3000            /*定义温感脱落二级报警释放延时默认值*/
#define TEMP_LINE_TRD_ALARM_DLY_DEF        30000            /*定义温感线脱落三级报警延时默认值*/
#define TEMP_LINE_TRD_ALARM_REL_DLY_DEF    3000            /*定义温感脱落三级报警释放延时默认值*/

#define VOLT_LINE_FST_ALARM_DLY_DEF        10000            /*定义电压检测排线脱落一级报警延时默认值*/
#define VOLT_LINE_FST_ALARM_REL_DLY_DEF    3000            /*定义电压检测排线脱落一级报警释放延时默认值*/
#define VOLT_LINE_SND_ALARM_DLY_DEF        20000            /*定义电压检测排线脱落二级报警延时默认值*/
#define VOLT_LINE_SND_ALARM_REL_DLY_DEF    3000            /*定义电压检测排线脱落二级报警释放延时默认值*/
#define VOLT_LINE_TRD_ALARM_DLY_DEF        30000            /*定义电压检测排线脱落三级报警延时默认值*/
#define VOLT_LINE_TRD_ALARM_REL_DLY_DEF    3000            /*定义电压检测排线脱落三级报警释放延时默认值*/

#define DCHG_DV_FST_ALARM_DEF           500             /*定义放电压差一级报警阀值*/
#define DCHG_DV_FST_ALARM_REL_DEF       300             /*定义放电压差一级报警释放阀值*/
#define DCHG_DV_SND_ALARM_DEF           800             /*定义放电压差二级报警阀值*/
#define DCHG_DV_SND_ALARM_REL_DEF       600             /*定义放电压差二级报警释放阀值*/
#define DCHG_DV_TRD_ALARM_DEF           1000            /*定义放电压差三级报警阀值*/
#define DCHG_DV_TRD_ALARM_REL_DEF       700             /*定义放电压差三级报警释放阀值*/

#define DCHG_DV_FST_ALARM_DLY_DEF       5000            /*定义放电压差一级报警延时*/
#define DCHG_DV_FST_ALARM_REL_DLY_DEF   6000            /*定义放电压差一级报警释放延时*/
#define DCHG_DV_SND_ALARM_DLY_DEF       5000            /*定义放电压差二级报警延时*/
#define DCHG_DV_SND_ALARM_REL_DLY_DEF   6000            /*定义放电压差二级报警释放延时*/
#define DCHG_DV_TRD_ALARM_DLY_DEF       5000            /*定义放电压差三级报警延时*/
#define DCHG_DV_TRD_ALARM_REL_DLY_DEF   6000            /*定义放电压差三级报警释放延时*/

#define CHG_DV_FST_ALARM_DEF            300             /*定义充电压差一级报警阀值*/
#define CHG_DV_FST_ALARM_REL_DEF        200             /*定义充电压差一级报警释放阀值*/
#define CHG_DV_SND_ALARM_DEF            600             /*定义充电压差二级报警阀值*/
#define CHG_DV_SND_ALARM_REL_DEF        500             /*定义充电压差二级报警释放阀值*/
#define CHG_DV_TRD_ALARM_DEF            900             /*定义充电压差三级报警阀值*/
#define CHG_DV_TRD_ALARM_REL_DEF        800             /*定义充电压差三级报警释放阀值*/

#define CHG_DV_FST_ALARM_DLY_DEF        5000            /*定义充电压差一级报警延时*/
#define CHG_DV_FST_ALARM_REL_DLY_DEF    6000            /*定义充电压差一级报警释放延时*/
#define CHG_DV_SND_ALARM_DLY_DEF        5000            /*定义充电压差二级报警延时*/
#define CHG_DV_SND_ALARM_REL_DLY_DEF    6000            /*定义充电压差二级报警释放延时*/
#define CHG_DV_TRD_ALARM_DLY_DEF        5000            /*定义充电压差三级报警延时*/
#define CHG_DV_TRD_ALARM_REL_DLY_DEF    6000            /*定义充电压差三级报警释放延时*/

//TODO
#define TEMP_RISE_FAST_DEF                0xFF            /*定义温升快报警默认值*/
#define TEMP_RISE_FAST_MIN_DEF            0                /*定义温升快报警最小值*/
#define TEMP_RISE_FAST_MAX_DEF            0xFF            /*定义温升快报警最大值*/

#define TEMP_RISE_FAST_FREE_DEF            0xFF            /*定义温升快报警释放默认值*/
#define TEMP_RISE_FAST_FREE_MIN_DEF        0                /*定义温升快报警释放最小值*/
#define TEMP_RISE_FAST_FREE_MAX_DEF        0xFF            /*定义温升快报警释放最大值*/

#define TEMP_RISE_OVER_DEF                0xFF            /*定义温升过快报警释放默认值*/
#define TEMP_RISE_OVER_MIN_DEF            0                /*定义温升过快报警最小值*/
#define TEMP_RISE_OVER_MAX_DEF            0xFF            /*定义温升过快报警最大值*/

#define TEMP_RISE_OVER_FREE_DEF            0xFF            /*定义温升过快报警释放默认值*/
#define TEMP_RISE_OVER_FREE_MIN_DEF        0                /*定义温升过快报警释放最小值*/
#define TEMP_RISE_OVER_FREE_MAX_DEF        0xFF            /*定义温升过快报警释放最大值*/

#define TEMP_RISE_FAST_TIME_DEF            5000            /*定义温升快报警检测时间 单位：1ms/bit*/
#define TEMP_RISE_FAST_FREE_TIME_DEF    6000            /*定义温升快报警释放检测时间 单位：1ms/bit*/
#define TEMP_RISE_OVER_TIME_DEF            5000            /*定义温升过快报警检测时间 单位：1ms/bit*/
#define TEMP_RISE_OVER_FREE_TIME_DEF    6000            /*定义温升过快报警释放检测时间 单位：1ms/bit*/

#define TEMP_FALL_FAST_DEF                0xFF            /*定义温降快报警默认值*/
#define TEMP_FALL_FAST_MIN_DEF            0                /*定义温降快报警最小值*/
#define TEMP_FALL_FAST_MAX_DEF            0xFF            /*定义温降快报警最大值*/

#define TEMP_FALL_FAST_FREE_DEF            0xFF            /*定义温降快报警释放默认值*/
#define TEMP_FALL_FAST_FREE_MIN_DEF        0                /*定义温降快报警释放最小值*/
#define TEMP_FALL_FAST_FREE_MAX_DEF        0xFF            /*定义温降快报警释放最大值*/

#define TEMP_FALL_OVER_DEF                0xFF            /*定义温降过快报警释放默认值*/
#define TEMP_FALL_OVER_MIN_DEF            0                /*定义温降过快报警最小值*/
#define TEMP_FALL_OVER_MAX_DEF            0xFF            /*定义温降过快报警最大值*/

#define TEMP_FALL_OVER_FREE_DEF            0xFF            /*定义温降过快报警释放默认值*/
#define TEMP_FALL_OVER_FREE_MIN_DEF        0                /*定义温降过快报警释放最小值*/
#define TEMP_FALL_OVER_FREE_MAX_DEF        0xFF            /*定义温降过快报警释放最大值*/

#define TEMP_FALL_FAST_TIME_DEF            5000            /*定义温降快报警检测时间 单位：1ms/bit*/
#define TEMP_FALL_FAST_FREE_TIME_DEF    6000            /*定义温降快报警释放检测时间 单位：1ms/bit*/
#define TEMP_FALL_OVER_TIME_DEF            5000            /*定义温降过快报警检测时间 单位：1ms/bit*/
#define TEMP_FALL_OVER_FREE_TIME_DEF    6000            /*定义温降过快报警释放检测时间 单位：1ms/bit*/

#define SOC_LOW_FST_ALARM_DEF            PERCENT_TO_SOC(20)    /*SOC低一级报警阈值默认值*/
#define SOC_LOW_FST_ALARM_REL_DEF        PERCENT_TO_SOC(25)  /*SOC低一级报警释放阈值默认值*/
#define SOC_LOW_SND_ALARM_DEF            PERCENT_TO_SOC(10)  /*SOC低二级报警阈值默认值*/
#define SOC_LOW_SND_ALARM_REL_DEF        PERCENT_TO_SOC(15)  /*SOC低二级报警释放阈值默认值*/
#define SOC_LOW_TRD_ALARM_DEF            PERCENT_TO_SOC(5)   /*SOC低三级报警阈值默认值*/
#define SOC_LOW_TRD_ALARM_REL_DEF        PERCENT_TO_SOC(10)  /*SOC低三级报警释放阈值默认值*/

#define SOC_LOW_FST_ALARM_DLY_DEF        5000            /*SOC低一级报警延时默认值*/
#define SOC_LOW_FST_ALARM_REL_DLY_DEF    6000            /*SOC低一级报警释放延时默认值*/
#define SOC_LOW_SND_ALARM_DLY_DEF        5000            /*SOC低二级报警延时默认值*/
#define SOC_LOW_SND_ALARM_REL_DLY_DEF    6000            /*SOC低二级报警释放延时默认值*/
#define SOC_LOW_TRD_ALARM_DLY_DEF        5000            /*SOC低三级报警延时默认值*/
#define SOC_LOW_TRD_ALARM_REL_DLY_DEF    6000            /*SOC低三级报警释放延时默认值*/

#define SOC_HIGH_FST_ALARM_DEF            PERCENT_TO_SOC(101) /*SOC高一级报警阈值默认值*/
#define SOC_HIGH_FST_ALARM_REL_DEF        PERCENT_TO_SOC(100)    /*SOC高一级报警释放阈值默认值*/
#define SOC_HIGH_SND_ALARM_DEF            PERCENT_TO_SOC(102)    /*SOC高二级报警阈值默认值*/
#define SOC_HIGH_SND_ALARM_REL_DEF        PERCENT_TO_SOC(101)    /*SOC高二级报警释放阈值默认值*/
#define SOC_HIGH_TRD_ALARM_DEF            PERCENT_TO_SOC(105) /*SOC高三级报警阈值默认值*/
#define SOC_HIGH_TRD_ALARM_REL_DEF        PERCENT_TO_SOC(102) /*SOC高三级报警释放阈值默认值*/

#define SOC_HIGH_FST_ALARM_DLY_DEF        5000            /*SOC高一级报警延时默认值*/
#define SOC_HIGH_FST_ALARM_REL_DLY_DEF    6000            /*SOC高一级报警释放延时默认值*/
#define SOC_HIGH_SND_ALARM_DLY_DEF        5000            /*SOC高二级报警延时默认值*/
#define SOC_HIGH_SND_ALARM_REL_DLY_DEF    6000            /*SOC高二级报警释放延时默认值*/
#define SOC_HIGH_TRD_ALARM_DLY_DEF        5000            /*SOC高三级报警延时默认值*/
#define SOC_HIGH_TRD_ALARM_REL_DLY_DEF    6000            /*SOC高三级报警释放延时默认值*/

#define DCHG_HTV_FST_ALARM_DEF          3700            /*定义放电总压高一级报警阀值*/
#define DCHG_HTV_FST_ALARM_REL_DEF      3600            /*定义放电总压高一级报警释放阀值*/
#define DCHG_HTV_SND_ALARM_DEF          3750            /*定义放电总压高二级报警阀值*/
#define DCHG_HTV_SND_ALARM_REL_DEF      3700            /*定义放电总压高二级报警释放阀值*/
#define DCHG_HTV_TRD_ALARM_DEF          3800            /*定义放电总压高三级报警阀值*/
#define DCHG_HTV_TRD_ALARM_REL_DEF      3750            /*定义放电总压高三级报警释放阀值*/

#define DCHG_HTV_FST_ALARM_DLY_DEF      5000            /*定义放电总压高一级报警延时*/
#define DCHG_HTV_FST_ALARM_REL_DLY_DEF  6000            /*定义放电总压高一级报警释放延时*/
#define DCHG_HTV_SND_ALARM_DLY_DEF      5000            /*定义放电总压高二级报警延时*/
#define DCHG_HTV_SND_ALARM_REL_DLY_DEF  6000            /*定义放电总压高二级报警释放延时*/
#define DCHG_HTV_TRD_ALARM_DLY_DEF      5000            /*定义放电总压高三级报警延时*/
#define DCHG_HTV_TRD_ALARM_REL_DLY_DEF  6000            /*定义放电总压高三级报警释放延时*/

#define CHG_HTV_FST_ALARM_DEF           3700            /*定义充电总压高一级报警阀值*/
#define CHG_HTV_FST_ALARM_REL_DEF       3600            /*定义充电总压高一级报警释放阀值*/
#define CHG_HTV_SND_ALARM_DEF           3750            /*定义充电总压高二级报警阀值*/
#define CHG_HTV_SND_ALARM_REL_DEF       3700            /*定义充电总压高二级报警释放阀值*/
#define CHG_HTV_TRD_ALARM_DEF           3800            /*定义充电总压高三级报警阀值*/
#define CHG_HTV_TRD_ALARM_REL_DEF       3750            /*定义充电总压高三级报警释放阀值*/

#define CHG_HTV_FST_ALARM_DLY_DEF       5000            /*定义充电总压高一级报警延时*/
#define CHG_HTV_FST_ALARM_REL_DLY_DEF   6000            /*定义充电总压高一级报警释放延时*/
#define CHG_HTV_SND_ALARM_DLY_DEF       5000            /*定义充电总压高二级报警延时*/
#define CHG_HTV_SND_ALARM_REL_DLY_DEF   6000            /*定义充电总压高二级报警释放延时*/
#define CHG_HTV_TRD_ALARM_DLY_DEF       5000            /*定义充电总压高三级报警延时*/
#define CHG_HTV_TRD_ALARM_REL_DLY_DEF   6000            /*定义充电总压高三级报警释放延时*/

#define DCHG_LTV_FST_ALARM_DEF          2800            /*定义放电总压低一级报警阀值*/
#define DCHG_LTV_FST_ALARM_REL_DEF      2900            /*定义放电总压低一级报警释放阀值*/
#define DCHG_LTV_SND_ALARM_DEF          2700            /*定义放电总压低二级报警阀值*/
#define DCHG_LTV_SND_ALARM_REL_DEF      2800            /*定义放电总压低二级报警释放阀值*/
#define DCHG_LTV_TRD_ALARM_DEF          2500            /*定义放电总压低三级报警阀值*/
#define DCHG_LTV_TRD_ALARM_REL_DEF      2600            /*定义放电总压低三级报警释放阀值*/

#define DCHG_LTV_FST_ALARM_DLY_DEF      5000            /*定义放电总压低一级报警延时*/
#define DCHG_LTV_FST_ALARM_REL_DLY_DEF  6000            /*定义放电总压低一级报警释放延时*/
#define DCHG_LTV_SND_ALARM_DLY_DEF      5000            /*定义放电总压低二级报警延时*/
#define DCHG_LTV_SND_ALARM_REL_DLY_DEF  6000            /*定义放电总压低二级报警释放延时*/
#define DCHG_LTV_TRD_ALARM_DLY_DEF      5000            /*定义放电总压低三级报警延时*/
#define DCHG_LTV_TRD_ALARM_REL_DLY_DEF  6000            /*定义放电总压低三级报警释放延时*/

#define CHG_LTV_FST_ALARM_DEF           3000               /*定义充电总压低一级报警阀值*/
#define CHG_LTV_FST_ALARM_REL_DEF       3100               /*定义充电总压低一级报警释放阀值*/
#define CHG_LTV_SND_ALARM_DEF           2800               /*定义充电总压低二级报警阀值*/
#define CHG_LTV_SND_ALARM_REL_DEF       3000               /*定义充电总压低二级报警释放阀值*/
#define CHG_LTV_TRD_ALARM_DEF           2500               /*定义充电总压低三级报警阀值*/
#define CHG_LTV_TRD_ALARM_REL_DEF       2600               /*定义充电总压低三级报警释放阀值*/

#define CHG_LTV_FST_ALARM_DLY_DEF       5000            /*定义充电总压低一级报警延时*/
#define CHG_LTV_FST_ALARM_REL_DLY_DEF   6000            /*定义充电总压低一级报警释放延时*/
#define CHG_LTV_SND_ALARM_DLY_DEF       5000            /*定义充电总压低二级报警延时*/
#define CHG_LTV_SND_ALARM_REL_DLY_DEF   6000            /*定义充电总压低二级报警释放延时*/
#define CHG_LTV_TRD_ALARM_DLY_DEF       5000            /*定义充电总压低三级报警延时*/
#define CHG_LTV_TRD_ALARM_REL_DLY_DEF   6000            /*定义充电总压低三级报警释放延时*/

#define CHG_OUTLET_HT_FST_ALARM_DEF     TEMPERATURE_FROM_C(60)            /*定义充电插座温度高一级报警阀值*/
#define CHG_OUTLET_HT_FST_ALARM_REL_DEF TEMPERATURE_FROM_C(50)            /*定义充电插座温度高一级报警释放阀值*/
#define CHG_OUTLET_HT_SND_ALARM_DEF     TEMPERATURE_FROM_C(90)            /*定义充电插座温度高二级报警阀值*/
#define CHG_OUTLET_HT_SND_ALARM_REL_DEF TEMPERATURE_FROM_C(80)            /*定义充电插座温度高二级报警释放阀值*/
#define CHG_OUTLET_HT_TRD_ALARM_DEF     TEMPERATURE_FROM_C(120)            /*定义充电插座温度高三级报警阀值*/
#define CHG_OUTLET_HT_TRD_ALARM_REL_DEF TEMPERATURE_FROM_C(110)            /*定义充电插座温度高三级报警释放阀值*/

#define CHG_OUTLET_ALARM_MAX_DEF     TEMPERATURE_FROM_C(190)  /////////////////////////////////////



#define BMS_CHGR_AC_OUTLET_NUM_MAX      2
#define BMS_CHGR_AC_OUTLET_NUM_MIN      0
#define BMS_CHGR_AC_OUTLET_NUM_DEF      0
#define BMS_CHGR_DC_OUTLET_NUM_MAX      2
#define BMS_CHGR_DC_OUTLET_NUM_MIN      0
#define BMS_CHGR_DC_OUTLET_NUM_DEF      0

#define CHG_OUTLET_HT_FST_ALARM_DLY_DEF        5000            /*定义充电插座温度高一级报警延时*/
#define CHG_OUTLET_HT_FST_ALARM_REL_DLY_DEF    6000            /*定义充电插座温度高一级报警释放延时*/
#define CHG_OUTLET_HT_SND_ALARM_DLY_DEF        5000            /*定义充电插座温度高二级报警延时*/
#define CHG_OUTLET_HT_SND_ALARM_REL_DLY_DEF    6000            /*定义充电插座温度高二级报警释放延时*/
#define CHG_OUTLET_HT_TRD_ALARM_DLY_DEF        5000            /*定义充电插座温度高三级报警延时*/
#define CHG_OUTLET_HT_TRD_ALARM_REL_DLY_DEF    6000            /*定义充电插座温度高三级报警释放延时*/



#define PINSU_FST_ALARM_DEF                500                /*定义轻微漏电报警阈值默认值 1ohm/v/bit*/
#define PINSU_FST_ALARM_REL_DEF            550                /*定义轻微漏电报警释放阈值默认值 1ohm/v/bit*/
#define PINSU_SND_ALARM_DEF                200                /*定义严重漏电阈值默认值 1ohm/v/bit*/
#define PINSU_SND_ALARM_REL_DEF            250                /*定义严重漏电释放阈值默认值 1ohm/v/bit*/
#define PINSU_TRD_ALARM_DEF             100             /*定义深度漏电报警默认值 1ohm/v/bit*/
#define PINSU_TRD_ALARM_REL_DEF         150             /*定义深度漏电报警释放默认值 1ohm/v/bit*/

#define PINSU_FST_ALARM_DLY_DEF            5000            /*定义轻微漏电报警延时默认值*/
#define PINSU_FST_ALARM_REL_DLY_DEF        6000            /*定义轻微漏电报警释放延时默认值*/
#define PINSU_SND_ALARM_DLY_DEF            1000            /*定义严重漏电延时默认值*/
#define PINSU_SND_ALARM_REL_DLY_DEF        2000            /*定义严重漏电释放延时默认值*/
#define PINSU_TRD_ALARM_DLY_DEF            1000            /*定义深度漏电延时默认值*/
#define PINSU_TRD_ALARM_REL_DLY_DEF        2000            /*定义深度漏电释放延时默认值*/

#define CHG_RELAY_ON_DLY_DEF               1000               /*充电继电器闭合默认延时时间*/
#define CHG_RELAY_OFF_DLY_DEF            5000            /*充电继电器断开默认延时时间*/
#define DISCHG_RELAY_ON_DLY_DEF            0                /*放电继电器闭合默认延时时间 */
#define DISCHG_RELAY_OFF_DLY_DEF        5000            /*放电继电器断开默认延时时间 */
#define PRECHG_RELAY_ON_DLY_DEF            0                /*预充继电器闭合默认延时时间 */
#define PRECHG_RELAY_OFF_DLY_DEF        3000            /*预充继电器断开默认延时时间 */
#define RESERVE_RELAY_ON_DLY_DEF        5000             /*预留继电器闭合默认延时时间 */
#define RESERVE_RELAY_OFF_DLY_DEF        5000            /*预留继电器断开默认延时时间 */

#define CYCLE_DEF                       0               /*循环次数默认值*/
#define CYCLE_FLAG_DEF                  0               /*循环可写标识位默认值*/

#define CHARGER_CUR                     160             /*充电机充电电流0.1A/bit*/
#define CHARGER_CUR_MIN_DEF                0                /*充电机充电电流最小值0.1A/bit*/
#define CHARGER_CUR_MAX_DEF                10000            /*充电机充电电流最大值0.1A/bit*/


#define CAP_CALIB_HIGH_AVER_VOLT_DEF    3300            /*定义容量修正高位平均电压阈值默认值*/
#define CAP_CALIB_HIGH_AVER_VOLT_MIN_DEF 0                /*定义容量修正高位平均电压最小值*/
#define CAP_CALIB_HIGH_AVER_VOLT_MAX_DEF 5000            /*定义容量修正高位平均电压最大值*/

#define CAP_CALIB_LOW_AVER_VOLT_DEF        2800            /*定义容量修正低位平均电压阈值默认值*/
#define CAP_CALIB_LOW_AVER_VOLT_MIN_DEF 0                /*定义容量修正低位平均电压最小值*/
#define CAP_CALIB_LOW_AVER_VOLT_MAX_DEF 5000            /*定义容量修正低位平均电压最大值*/

#define CAP_CALIB_LOW_SOC_DEF            PERCENT_TO_SOC(10)/*定义容量修正低SOC默认值*/
#define CAP_CALIB_LOW_SOC_MIN_DEF        PERCENT_TO_SOC(0)/*定义容量修正低SOC最小值*/
#define CAP_CALIB_LOW_SOC_MAX_DEF        PERCENT_TO_SOC(100)/*定义容量修正低SOC最大值*/

#define LOW_CALIBRATE_SOC_DLY_DEF       60              /*定义容量修正低SOC延时值,单位为s*/
#define LOW_CALIBRATE_SOC_DLY_MIN       0               /*定义容量修正低SOC延时值,单位为s*/
#define LOW_CALIBRATE_SOC_DLY_MAX       0xFFFF          /*定义容量修正低SOC延时值,单位为s*/

#define VICHLE_TYRE_DIAMETER            250             /*汽车轮胎直径尺寸 1cm/bit*/
#define SPEED_RATE                      200             /*速比默认值*/
#define GEAR_RATE                       367             /*减速箱齿轮比*/
#define TYRE_RATE                       100             /*后桥比*/
#define BMS_CHARGER_PROTOCOL_DEFAULT    1               /*定义充电机通信协议，默认为力高协议*/
#define BMS_CHARGER_PROTOCOL_MIN_DEF    0                /*定义充电机通信协议最小值*/
#define BMS_CHARGER_PROTOCOL_MAX_DEF    3//PROTOCOL_MAX-1    /*定义充电机通信协议最大值*/
#define CCHK_DEF                        0               /*定义电流校准默认值*/
#define CCHK_MIN_DEF                    0x8000          /*定义电流校准最小值*/
#define CCHK_MAX_DEF                    0x7FFF          /*定义电流校准最大值*/
#define CCHK_CUR_OUT_VOLT_MAX_DEF       200             /*电流自动校准允许最大电流*/
#define MAX_OVER_CUR_DEF                4500            /*定义放电电流极限值0.1A/bit*/
#define SLAVE_NUM_DEF                   1               /*定义默认从机数目 1/bit*/
#define SLAVE_NUM_MIN_DEF                0                /*定义从机数目最小值*/
#define SLAVE_NUM_MAX_DEF                BMU_MAX_SLAVE_COUNT                /*定义从机数目最大值*/
#define SLAVE_TEMPERA_NUM_DEF           3               /*定义默认从机温感数目 1/bit*/
#define SLAVE_TEMPERA_NUM_MIN_DEF        0                /*定义从机温感数目最小值 1/bit*/
#define SLAVE_TEMPERA_NUM_MAX_DEF        BMS_BMU_TEMPERATURE_MAX_NUM    /*定义从机温感数目最大值 1/bit*/

#define HMI_FRAME_COMM_ABORT_TIMEOUT    HMI_FRAME_TIMEOUT/*HMI通信超时时间定义*/
#define HMI_FRAME_COMM_ABORT_TIMEOUT_MIN 0                /*HMI通信超时最小值1ms/bit*/
#define HMI_FRAME_COMM_ABORT_TIMEOUT_MAX 5000            /*HMI通信超时最大值 1ms/bit*/
#define HMI_COMM_BPS_DEF                HMI_DEV_BAND    /*HMI通信波特率定义*/
#define RS485_BPS_MIN_DEF                1            /*HMI通信波特率最小值*/
#define RS485_BPS_MAX_DEF                4            /*HMI通信波特率最大值*/
#define RS485_DTU_BPS_DEF               DTU_DEV_BAND    /*DTU通信波特率定义*/
#define RS485_SLAVE_BPS_DEF             BMS_DCM_RS485_DEV_BAND
#define DTU_SEND_INTERVAL_DEF            30                /*DTU发送周期定义, 单位为second*/
#define DTU_SEND_INTERVAL_MIN_DEF        5                /*DTU发送周期最小值*/
#define DTU_SEND_INTERVAL_MAX_DEF        60000            /*DTU发送周期最大值*/
#define UPPER_COMPUTER_CAN_ADDR_DEF        161                /*上位机CAN通信地址定义*/
#define UPPER_COMPUTER_CAN_ADDR_MIN        0                /*上位机CAN通信地址最小值*/
#define UPPER_COMPUTER_CAN_ADDR_MAX        255                /*上位机CAN通信地址最大值*/
#define BCU_CAN_ADDR_DEF                244                /*BCU_CAN通信地址定义*/
#define BCU_CAN_ADDR_MIN_DEF            0                /*BCU_CAN通信地址最小值*/
#define BCU_CAN_ADDR_MAX_DEF            255                /*BCU_CAN通信地址最大值*/
#define SYS_STATUS_SAVE_INTERVAL_DEF    60000            /*系统状态信息保存周期*/
#define SYS_STATUS_SAVE_INTERVAL_MIN_DEF    10            /*系统状态信息保存周期最小值*/
#define SYS_STATUS_SAVE_INTERVAL_MAX_DEF    60000        /*系统状态信息保存周期最大值*/
#define DISCHG_RELAY_DLY_DEF            5000            /*放电继电器默认延时时间*/
#define CHG_RELAY_DLY_DEF               5000            /*充电继电器默认延时时间 */
#define OHT_STATE_DLY_DEF               5000            /*过温延时时间默认值*/
#define ODHT_STATE_DLY_DEF              5000            /*温差延时时间默认值  ms*/
#define SOC_OLV_STATE_DLY_DEF           5000            /*SOC过低延时时间默认值 */
#define OLV_STATE_DLY_DEF               5000            /*过放延时时间默认值*/
#define OCV_STATE_DLY_DEF               5000            /*过充延时时间默认值 */
#define PINSU_LEAK_STATE_DLY_DEF        5000            /*绝缘漏电延时时间默认值*/
#define TOTAL_VOLT_MAX_DEF              5000            /*最大总压默认值 1V/bit*/
#define TOTAL_VOLT_MIN_DEF              3000            /*最小总压默认值 1V/bit*/
#define DIFF_VOLT_DEF                   4000            /*压差默认值 1mV/bit*/

//放电积分系数 0.001倍/bit

#define SOC0_DISCHG_FACTOR_DEFAULT_VAULE   1000//1倍
#define SOC0_DISCHG_FACTOR_MIN_VAULE   900//0.9倍
#define SOC0_DISCHG_FACTOR_MAX_VAULE   1100//1.1倍

#define EEPROM_TV_CHECK_DEF             0
#define EEPROM_TV_RANGE_DEF             0
#define EEPROM_NORMAL_CHECK_CYCLE_DEF   10000            /*正常状态下绝缘信息更新周期*/
#define EEPROM_INSU_RES_CALIB_FLAG_DEF  1
#define EEPROM_MOS_ON_DEF               600     /*0表示MOS常闭*/
#define EEPROM_INSU_RES_SWITCH_IN_CHG_DEF  0

#define EEPROM_NORMAL_CHECK_CYCLE_MIN_DEF   1000   
#define EEPROM_NORMAL_CHECK_CYCLE_MAX_DEF   60000  
#define EEPROM_MOS_ON_MIN_DEF    0            /*定义报警和保护延时最小值*/
#define EEPROM_MOS_ON_MAX_DEF    1201 //1.2s

// TODO
#define CFG_HEAT_ON_DEF                 0
#define CFG_HEAT_ON_MIN_DEF                0
#define CFG_HEAT_ON_MAX_DEF                0xFF

#define CFG_HEAT_OFF_DEF                0xFF
#define CFG_HEAT_OFF_MIN_DEF            0
#define CFG_HEAT_OFF_MAX_DEF            0xFF

#define CFG_COOL_ON_DEF                 0xFF
#define CFG_COOL_ON_MIN_DEF                0
#define CFG_COOL_ON_MAX_DEF                0xFF

#define CFG_COOL_OFF_DEF                0
#define CFG_COOL_OFF_MIN_DEF            0
#define CFG_COOL_OFF_MAX_DEF            0xFF

#define CFG_HEAT_ON_DLY_DEF                5000            /*定义加热开启延时时间*/
#define CFG_HEAT_OFF_DLY_DEF            5000            /*定义加热关闭延时时间*/
#define CFG_COOL_ON_DLY_DEF                5000            /*定义制冷开启延时时间*/
#define CFG_COOL_OFF_DLY_DEF            5000            /*定义制冷关闭延时时间*/

// TODO
#define CFG_SYS_VOLT_HIGH_DEF           0
#define CFG_SYS_VOLT_HIGH_MIN_DEF        0
#define CFG_SYS_VOLT_HIGH_MAX_DEF        0
#define CFG_SYS_VOLT_LOW_DEF            0
#define CFG_SYS_VOLT_LOW_MIN_DEF        0
#define CFG_SYS_VOLT_LOW_MAX_DEF        0

#define CFG_SYS_VOLT_HIGH_DLY_DEF        0
#define CFG_SYS_VOLT_HIGH_DLY_MIN_DEF    0
#define CFG_SYS_VOLT_HIGH_DLY_MAX_DEF    0

#define CFG_SYS_VOLT_LOW_DLY_DEF        0
#define CFG_SYS_VOLT_LOW_DLY_MIN_DEF    0
#define CFG_SYS_VOLT_LOW_DLY_MAX_DEF    0

#define CFG_SYS_VOLT_CALIB_DEF            0
#define CFG_SYS_VOLT_CALIB_MIN_DEF        0
#define CFG_SYS_VOLT_CALIB_MAX_DEF        0


#define SAVE_FILE_A_FLAG_DEF            0               /*A类文件存储标志默认值*/

#define BMU_BALANCE_ENABLE_DEF            1                /*从机均衡使能状态*/
#define BMU_BALANCE_ENABLE_MIN_DEF        0                /*从机均衡使能状态最小值*/
#define BMU_BALANCE_ENABLE_MAX_DEF        1                /*从机均衡使能状态最大值*/
#define MCU_WORK_STATE_DELAY_DEF        5000            /*BCU工作状态延时时间，单位：ms*/
#define BCU_WORK_STATE_DELAY_MIN_DEF    0                /*BCU工作状态延时时间，单位：ms*/
#define BCU_WORK_STATE_DELAY_MAX_DEF    0xFFFF            /*BCU工作状态延时时间，单位：ms*/

#define ALARM_BEEP_INTERVAL_DEF         3000            /*蜂鸣器鸣笛时间间隔默认值*/
#define ALARM_BEEP_INTERVAL_MIN_DEF     0                /*蜂鸣器鸣笛时间间隔最小值*/
#define ALARM_BEEP_INTERVAL_MAX_DEF     60000           /*蜂鸣器鸣笛时间间隔最大值*/

#define ALARM_BEEP_MASK_HIGH_DEF        0xFFFF          /* 蜂鸣器鸣笛掩码 高字*/
#define ALARM_BEEP_MASK_HIGH_MIN_DEF    0                /*蜂鸣器吃鸣响掩码高字最小值*/
#define ALARM_BEEP_MASK_HIGH_MAX_DEF    0xFFFF            /*蜂鸣器吃鸣响掩码高字最大值*/
#define ALARM_BEEP_MASK_LOW_DEF            0xFFFF          /* 蜂鸣器鸣笛掩码 低字*/
#define ALARM_BEEP_MASK_LOW_MIN_DEF        0                /* 蜂鸣器鸣笛掩码低字最小值*/
#define ALARM_BEEP_MASK_LOW_MAX_DEF        0xFFFF            /* 蜂鸣器鸣笛掩码低字最小值*/

#define RELAY_DISCHARGE_SAME_PORT_DEF   0
#define RELAY_DISCHARGE_SAME_PORT_MIN_DEF    0
#define RELAY_DISCHARGE_SAME_PORT_MAX_DEF    1

#define RELAY_PRECHARGE_DEF             0
#define RELAY_PRECHARGE_MIN_DEF         0
#define RELAY_PRECHARGE_MAX_DEF         1

#define RELAY_CHARGE_MUTEX_DEF          1
#define RELAY_CHARGE_MUTEX_MIN_DEF      0
#define RELAY_CHARGE_MUTEX_MAX_DEF      1

#define RELAY_TYPE_MIN_DEF                0
#define RELAY_TYPE_MAX_DEF                kRelayTypeMaxCount-1

#define RELAY_RELAY1_TYPE_DEF           kRelayTypeCharging
#define RELAY_RELAY2_TYPE_DEF           kRelayTypeDischarging
#define RELAY_RELAY3_TYPE_DEF           kRelayTypePreCharging
#define RELAY_RELAY4_TYPE_DEF           kRelayTypeCooling
#define RELAY_RELAY5_TYPE_DEF           kRelayTypePositive
#define RELAY_RELAY6_TYPE_DEF           kRelayTypeNegative
#define RELAY_RELAY7_TYPE_DEF           kRelayTypeDischargingSignal
#define RELAY_RELAY8_TYPE_DEF           kRelayTypeChargingSignal
#define RELAY_RELAY9_TYPE_DEF           kRelayTypeHeating   //对应的就是 DO1
#define RELAY_RELAY10_TYPE_DEF          kRelayTypeUnknown   //对应的就是 DO2


#define RELAY_CONTROL_ENABLE_FLAG_DEF   0xFF

#define INSULATION_TYPE_DEF             kInsulationTypeBoard
#define INSULATION_TYPE_MIN_DEF         kInsulationNone
#define INSULATION_TYPE_MAX_DEF         kInsulationTypeMax

#define BATTERY_TOTAL_NUM_DEF           100
#define BATTERY_TOTAL_NUM_MIN_DEF       0
#define BATTERY_TOTAL_NUM_MAX_DEF       5000

#define NOMINAL_TOTAL_VOLTAGE_DEF       3400    //分辨率 0.1v/bit
#define NOMINAL_TOTAL_VOLTAGE_MIN_DEF   0
#define NOMINAL_TOTAL_VOLTAGE_MAX_DEF   65535

#define CHARGE_EFFICIENCY_FACTOR_DEF            1000
#define CHARGE_EFFICIENCY_FACTOR_MIN_DEF        0
#define CHARGE_EFFICIENCY_FACTOR_MAX_DEF        2000

#define DISCHARGE_EFFICIENCY_FACTOR_DEF         1000
#define DISCHARGE_EFFICIENCY_FACTOR_MIN_DEF     0
#define DISCHARGE_EFFICIENCY_FACTOR_MAX_DEF     2000

#define SYSTEM_STATE_AVAILABLE_DELAY_DEF        5000 // 系统状态检查上电或者失败后延迟时间

#define CAN_DEV_1_BPS_DEF               4
#define CAN_DEV_2_BPS_DEF               4
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#define CAN_DEV_3_BPS_DEF               5 ///500k 
#else
#define CAN_DEV_3_BPS_DEF               4
#endif
#define CAN_DEV_5_BPS_DEF               4
#define CAN_DEV_BPS_MIN_DEF             1
#define CAN_DEV_BPS_MAX_DEF             7

#define INPUT_TYPE_MIN_DEF                0
#define INPUT_TYPE_MAX_DEF                kInputTypeMaxCount

#define INPUT_SIGNAL1_TYPE_DEF          kInputTypePowerOn
#define INPUT_SIGNAL2_TYPE_DEF          kInputTypeChargerConnect
#define INPUT_SIGNAL3_TYPE_DEF          kInputTypeSigOne
#define INPUT_SIGNAL4_TYPE_DEF          kInputTypeUnknown
#define INPUT_SIGNAL5_TYPE_DEF          kInputTypeUnknown
#define INPUT_SIGNAL6_TYPE_DEF          kInputTypeUnknown
#define INPUT_SIGNAL7_TYPE_DEF          kInputTypeUnknown
#define INPUT_SIGNAL8_TYPE_DEF          kInputTypeUnknown

#define BMS_COMM_MODE_MIN               BMS_COMM_SLAVE_MODE
#define BMS_COMM_MODE_MAX               BMS_SELF_CHECK_MODE

#define BMS_VIN_LEN                     17
#define BMS_VIN_DEFAULT                 0x3030 //"00"

#define BMS_MLOCK_TYPE_DFT              0x01    //busbar
#define BMS_MLOCK_PARAM_LEN_DFT         5    //ms
#define BMS_MLOCK_LOCK_TIME_DFT         1000    //ms
#define BMS_MLOCK_UNLOCK_TIME_DFT       1000    //ms
#define BMS_MLOCK_LOCK_TIMEOUT_DFT      200     //ms
#define BMS_MLOCK_UNLOCK_TIMEOUT_DFT    200     //ms
#define BMS_MLOCK_FEEDBACK_DIR_DFT      1       //0: low, 1:high


#define PARAM_BSU_NUM_MAX              30
#define LTC6804_MAX_TEMP_NUM           8
/**********************************************
 *
 * BYU setting
 *
 ***********************************************/
#define MAX_LTC6803_NUM                 10         //最大支持采集模块数
#define LTC_CELLV_NUM                   12        //每个采集模块采集最大电池数
#define LTC_CELLT_NUM                   8         //每个采集模块采集最大温度数
#define LTC_CELLT_EXTEND_CNT            4         /*1代表每个采集模块采集2个温感
                                                    2代表每个采集模块采集4个温感,依次类推直到6个温感*/
#define BYU_BALANCE_EN_DEF              1
 //高8位电池数 第8位温度数
#define BYU_VOLT_TEMP_CNT_MIN_DEF            0x0C02
#define BYU_VOLT_TEMP_CNT_MAX_DEF            0x0C02
#define BYU_VOLT_TEMP_CNT_1_DEF              0x0C02
#define BYU_VOLT_TEMP_CNT_2_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_3_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_4_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_5_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_6_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_7_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_8_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_9_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_10_DEF             0x0000
#define BYU_VOLT_TEMP_CNT_11_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_12_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_13_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_14_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_15_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_16_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_17_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_18_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_19_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_20_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_21_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_22_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_23_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_24_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_25_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_26_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_27_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_28_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_29_DEF              0x0000
#define BYU_VOLT_TEMP_CNT_30_DEF             0x0000
#define BYU_VOLT_CORRECT_DEF            0
#define BYU_VOLT_CORRECT_MIN_DEF        -5000
#define BYU_VOLT_CORRECT_MAX_DEF        5000

#define BYU_BAL_START_DV_DEF            10
#define BYU_BAL_STOP_DV_DEF             20

#define BYU_BAL_START_VOLT_DEF          3550

#define BYU_BAL_NUM_DEF                 2
#define BYU_BAL_NUM_MIN_DEF             0
#define BYU_BAL_NUM_MAX_DEF             3

#define BYU_TOTAL_BAL_EN_DEF            1
#define BYU_TOTAL_BAL_EN_MIN_DEF        0
#define BYU_TOTAL_BAL_EN_MAX_DEF        1

#define BYU_BAL_TYPE_DEF                1
#define BYU_BAL_TYPE_MIN_DEF            0
#define BYU_BAL_TYPE_MAX_DEF            2

#define BYU_TEMP_CNT_MIN_DEF            0
#define BYU_TEMP_CNT_MAX_DEF            8
#define BYU_TEMP_CNT_1_DEF              2
#define BYU_TEMP_CNT_2_DEF              0
#define BYU_TEMP_CNT_3_DEF              0
#define BYU_TEMP_CNT_4_DEF              0
#define BYU_TEMP_CNT_5_DEF              0
#define BYU_TEMP_CNT_6_DEF              0
#define BYU_TEMP_CNT_7_DEF              0
#define BYU_TEMP_CNT_8_DEF              0
#define BYU_TEMP_CNT_9_DEF              0
#define BYU_TEMP_CNT_10_DEF             0
#define BYU_TEMP_CNT_11_DEF              0
#define BYU_TEMP_CNT_12_DEF              0
#define BYU_TEMP_CNT_13_DEF              0
#define BYU_TEMP_CNT_14_DEF              0
#define BYU_TEMP_CNT_15_DEF              0
#define BYU_TEMP_CNT_16_DEF              0
#define BYU_TEMP_CNT_17_DEF              0
#define BYU_TEMP_CNT_18_DEF              0
#define BYU_TEMP_CNT_19_DEF              0
#define BYU_TEMP_CNT_20_DEF              0
#define BYU_TEMP_CNT_21_DEF              0
#define BYU_TEMP_CNT_22_DEF              0
#define BYU_TEMP_CNT_23_DEF              0
#define BYU_TEMP_CNT_24_DEF              0
#define BYU_TEMP_CNT_25_DEF              0
#define BYU_TEMP_CNT_26_DEF              0
#define BYU_TEMP_CNT_27_DEF              0
#define BYU_TEMP_CNT_28_DEF              0
#define BYU_TEMP_CNT_29_DEF              0
#define BYU_TEMP_CNT_30_DEF              0
/**********************************************
 *
 * Version
 *
 ***********************************************/
#define BMS_VER_HARDWARE            "HW1.0"
#define BMS_VER_NAME                "C32"
#define BMS_VER_TIME                "2012-06-11"
#define BMS_VER_MAJOR_NUMBER        1
#define BMS_VER_MINOR_NUMBER        0
#define BMS_VER_REVISION_NUMBER     0
#define BMS_VER_BUILD_NUMBER        1

/**********************************************
 *
 * Product Information
 *
 ***********************************************/
#define BMS_PRODUCT_HOST_CODE_NUM   "110702C001"    /*主机唯一码*/
#define BMS_PRODUCT_DATE            "2012-03-12"    /*主机生产日期*/

/**********************************************
 *
 * Range Setting
 *
 ***********************************************/
#define BATTERY_VOLTAGE_MAX_VALUE               5000 // mV
#define BATTERY_VOLTAGE_MIN_VALUE               400  // mV
#define BATTERY_VOLTAGE_INVALID_VALUE           0xFFFF

#define BATTERY_TEMPERATURE_MAX_VALUE           175 // (125) C
#define BATTERY_TEMPERATURE_MIN_VALUE           1   // (-49) C
#define BATTERY_TEMPERATURE_INVALID_VALUE       0xFF// 255

#define CHARGER_TEMPERATURE_MAX_VALUE           240 // (190) C
#define CHARGER_TEMPERATURE_MIN_VALUE           1   // (-49) C
#define CHARGER_TEMPERATURE_INVALID_VALUE       0xFF// 255

/**********************************************
 *
 * System Work State
 *
 ***********************************************/
#define WORK_STATE_STOP_MODE_DELAY   5000 // 进入停止状态的时延, ms
#define WORK_STATE_WORK_MODE_DELAY   2000 // 进入工作状态的时延, ms

/**********************************************
 *
 * System State
 *
 ***********************************************/
#define SYSTEM_STATE_SCAN_PERIODIC      500

#define SYSTEM_STARTUP_BEEP_TIME        50   // 系统启动后长鸣时长

/**********************************************
 *
 * Capacity Setting
 *
 ***********************************************/
#define   MAX_CAP_OFFSET                            20  //0.1/AH  2 // AH, 最大充绒与标称容量的差值，一般厂商提供比标称略大的容量

/**********************************************
 *
 * System Voltage
 *
 ***********************************************/
#define SYSTEM_VOLTAGE_SCAN_PERIOD          500  // ms, 系统电压采样周期
#define SYSTEM_VOLTAGE_AD_CNT               50 // 系统电压采样数
#define SYSTEM_VOLTAGE_AD_CONVERT_FACTOR    20

/**********************************************
 *
 * BMU Setting
 *
 ***********************************************/
#define BMU_MAX_SLAVE_COUNT           25 //最多支持的从机数目

#define BMU_MAX_TOTAL_VOLTAGE_NUM     1000
#define BMU_MAX_TOTAL_TEMP_NUM        800
#define BMU_MAX_TOTAL_BLANCE_STATE    ((BMU_MAX_TOTAL_VOLTAGE_NUM + 7) / 8)

#define BMS_BMU_BALANCE_NUM               2  //从机均衡路数

#define BMS_BMU_BATTERY_MAX_NUM           120  // 从机最大电池数,增加此值要注意J1939通信超时，适当增大kBmuResponseTimeoutIndex，目前此值可以支持70串左右。
#define BMS_BMU_TEMPERATURE_MAX_NUM       60  //从机最大温度数
#define BMS_BMU_BALANCE_STATE_BYTE_NUM    ((BMS_BMU_BATTERY_MAX_NUM + 7) / 8)

#define BMS_BMU_DEFAULT_TEMPERATURE_NUM       22
#define BMS_BMU_DEFAULT_VOLTAGE_NUM           60
#define BMS_BMU_BALANCE_STATE_NUM(VOLT_NUM)   ((VOLT_NUM + 7) / 8)
#define BMS_BMU_DEFAULT_BALANCE_STATE_NUM     BMS_BMU_BALANCE_STATE_NUM(BMS_BMU_DEFAULT_VOLTAGE_NUM)

#define BMU_STAT_HIGH_TEMPERATURE_NUM  1     // 最高温度统计个数
#define BMU_STAT_LOW_TEMPERATURE_NUM   1     // 最低温度统计个数
#define BMU_STAT_HIGH_VOLTAGE_NUM      1     // 最高单体电压统计个数
#define BMU_STAT_LOW_VOLTAGE_NUM       1     // 最低单体电压统计个数

/**********************************************
 *
 * HMI Setting
 *
 ***********************************************/
#define HMI_DEV_NAME                    kRs485Dev1   // RS485设备号
#define HMI_DEV_BAND                    3
#define HMI_MAX_RECEIVE_BUFFER_SIZE     100          // 最大数据包长度
#define HMI_MAX_RECEIVE_BUFFER_COUNT    2            // 接受缓冲个数
#define HMI_FRAME_TIMEOUT               40           // 帧间隔时间
#define HMI_CUSTOM_INFORMATION_NUM      3           // 支持显示的客户信息的个数

/**********************************************
 *
 * DTU Setting
 *
 ***********************************************/
#define DTU_DEV_NAME                    kRs485Dev0
#define DTU_DEV_BAND                    3
#define DTU_DEV_FRAME_SIZE              110
#define DTU_SEND_PERIODIC               10000 // ms, DTU发送周期
#define DTU_CURRENT_OFFSET              32000 // 电流偏移值， 修正负数为正, TODO:建议采用有符号值，去除该偏移

/**********************************************
 *
 * Upper Computer Setting
 *
 ***********************************************/
#define  UPPER_COMPUTER_CAN_DEV               kCanDev0
#define  UPPER_COMPUTER_CAN_RECEIVE_ID        0x18000000
#define  UPPER_COMPUTER_CAN_RECEIVE_MASK_ID   0x1FFF00FF
#define  UPPER_COMPUTER_CAN_MODE              kCanExtendMode
#define  UPPER_COMPUTER_CAN_BUFFER_COUNT      5
#define  UPPER_COMPUTER_UPPER_BMS_CAN_ID      0x18F0FFF4
#define  UPPER_COMPUTER_UPPER_COMPUTER_CAN_ID 0x07ECF450 //0x1CECF450

/**********************************************
 *
 * Motor Setting
 *
 ***********************************************/
#define  MOTOR_CAN_DEV              kCanDev2
#define  MOTOR_CAN_RECEIVE_ID       0x0CF11F05
#define  MOTOR_CAN_RECEIVE_MASK_ID  0x00
#define  MOTOR_CAN_MODE             kCanExtendMode
#define  MOTOR_CAN_BUFFER_COUNT     3
#define  MOTOR_MOTOR_CAN_ID         0x0CF1EFF4
#define  MOTOR_TX_FRAME_INTERVAL    977
#define  MOTOR_RX_FRAME_TIMEOUT     5000

#define  MOTO_VICHLE_TYRE_DIAMETER  250      /*轮胎尺寸 0.1英寸/bit*/
#define  MOTO_SPEED_RATE            350      /*速比 0.01 /bit*/
#define  MOTO_GEAR_RATE             367      /*减速箱齿轮比0.01 /bit*/
#define  MOTO_TYRE_RATE             100      /*后桥比0.01 /bit*/
/**********************************************
 *
 * Slave Setting
 *
 ***********************************************/
#define  BMS_SLAVE_CAN_DEV              kCanDev0
#define  BMS_SLAVE_CAN_RECEIVE_ID       0x1CECF450
#define  BMS_SLAVE_CAN_RECEIVE_MASK_ID  0x000000BF
#define  BMS_SLAVE_CAN_MODE             kCanExtendMode
#define  BMS_SLAVE_CAN_BUFFER_COUNT     5
#define  BMS_SLAVE_CAN_QUERY_SLAVE_DELAY    10
#define  BMS_SLAVE_CAN_QUERY_SLAVE_DLY_MIN    5
#define  BMS_SLAVE_CAN_QUERY_SLAVE_DLY_MAX  10000
#define  BMS_SLAVE_CAN_RECEIVE_RESPONSE_TIMEOUT 150
#define  BMS_SLAVE_CAN_RECEIVE_RESPONSE_TIMEOUT_MIN    5
#define  BMS_SLAVE_CAN_RECEIVE_RESPONSE_TIMEOUT_MAX    10000
#define  BMS_SLAVE_CAN_QUERY_PEROID     1000

#define  BMS_SLAVE_CAN_FRAME_SLAVE_ID   0x18F0FFF4
#define  BMS_SLAVE_CAN_FRAME_MASK_ID    0
#define  BMS_SLAVE_CAN_FRAME_LEN        8

#define  BMS_SLAVE_ISO_SLAVE_CAN_ADDR   0xD0
#define  BMS_SLAVE_HVCM_SLAVE_CAN_ADDR  0xD1


/**********************************************
 *
 * DCM Setting
 * DCM用于RS485模式下的主从通讯和绝缘检测通讯
 *
 ***********************************************/
#define BMS_DCM_RS485_DEV_NAME        kRs485Dev0
#define BMS_DCM_RS485_DEV_BAND        3
#define BMS_DCM_RS485_FRAME_SIZE      48

#define BMS_DCM_ISOLATION_ID            0x42
#define BMS_DCM_REQUEST_SLAVE_INTERVAL  50

#define DCM_MAX_RECEIVE_BUFFER_COUNT    2
#define DCM_MAX_RECEIVE_BUFFER_SIZE     74

#define DCM_TIMEOUT                     200         /*DCM超时等待时间*/


/**********************************************
 *
 * EMS Setting
 *
 ***********************************************/
#define  STANDARD_ID       (0)                                       
#define  EXTEND_ID         (1) 
#define  EMS_CAN_DEV              kCanDev2
#define  EMS_CAN_RECEIVE_ID       0x0CF11F05
#define  EMS_CAN_RECEIVE_MASK_ID  0x0
#define  EMS_CAN_RECEIVE_1_ID       0x0
#define  EMS_CAN_RECEIVE_MASK_1_ID  0x0
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#define  EMS_CAN_MODE             kCanStandardMode
#define  UDS_CAN_ID_TYPE          STANDARD_ID  //EXTEND_ID  STANDARD_ID
#else
#define  EMS_CAN_MODE             kCanExtendMode
#endif
#define  EMS_CAN_BUFFER_COUNT     10

#define EMS_ECU_MESSAGE1_CAN_ID  0x1801FFF4
#define EMS_ECU_MESSAGE2_CAN_ID  0x1802FFF4
#define EMS_ECU_MESSAGE3_CAN_ID  0x1803FFF4
#define EMS_ECU_MESSAGE4_CAN_ID  0x1804FFF4


#define EMS_DBD_BAT_VOLT_BASE_CAN_ID 0x1801D2F4
#define EMS_DBD_BAT_TEMP_BASE_CAN_ID 0x1850D2F4

#define EMS_ECU_TX_INTERVAL       24  // ECU的帧发送时间间隔, 单位为ms

#if( SWITCH_ON == UDS_SWITCH_CONFIG)

#define  EMS_CHECK_UDS_MSG_INTERVAL             10  // need 10ms loop
#define  EMS_CHECK_BUSOFF_INTERVAL              10  // need 10ms loop
#define  EMS_CHECK_DTC_STATUS_INTERVAL          500
#endif
/**********************************************
 *
 * Charger Setting
 *
 ***********************************************/
#define  BMS_CHARGER_CAN_DEV            kCanDev1 //注：实际是kCanDev4，为了上位机显示调整为kCanDev3，应用层自动修正为kCanDev4
#define  BMS_CHARGER_MAX_VOLTAGE        5900 // config_get(kChargerVoltIndex)
#define  BMS_CHARGER_MAX_VOLTAGE_MIN    0
#define  BMS_CHARGER_MAX_VOLTAGE_MAX    0xFFFF

#define  BMS_FULL_CHG_RELEASE_VOLT      5700
#define  BMS_FULL_CHG_RELEASE_VOLT_MIN  0
#define  BMS_FULL_CHG_RELEASE_VOLT_MAX  0xFFFF

#define  BMS_CHARGER_ENABLE_DEFAULT     1
#define  BMS_CHARGER_WIRE_REG           PORTK_PK3

#define  BMS_CHG_CUT_OFF_CUR_CAP_RATE   0.05
#define  BMS_CHARGER_CUT_OFF_CURRENT    TOTALCAP_DEF*BMS_CHG_CUT_OFF_CUR_CAP_RATE*10/*截止电流 0.1A/bit*/
#define  BMS_CHARGER_CUT_OFF_CURRENT_MIN    30 // 0.1A/bit
#define  BMS_CHARGER_CUT_OFF_CURRENT_MAX    1000 // 0.1A/bit

#define  BMS_CHARGER_CUT_OFF_CURRENT_DELAY  10000 // 1ms/bit

#define  BMS_CHARGER_TOTAL_TIME_DEF     15 // 1hour/bit
#define  BMS_CHARGER_TOTAL_TIME_MIN     1 // 1hour/bit
#define  BMS_CHARGER_TOTAL_TIME_MAX     30// 1hour/bit

#define  BMS_VALID_CHARGE_CURRENT_MIN   15

#define  BMS_FULL_CHG_RELEASE_DELAY_DEF 5000 //满充释放延时， 1ms/bit

#define  BMS_GB_CHARGER_CONNECT_MODE_DEF    kGBChargerConnectNone
#define  BMS_GB_CHARGER_CONNECT_MODE_MIN    kGBChargerConnectNone
#define  BMS_GB_CHARGER_CONNECT_MODE_MAX    kGBChargerConnectMax

#define SELF_CHECK_CHARGER_RECEIVE_ID       0x18FF50E5
#define SELF_CHECK_CHARGER_RECEIVE_MASK_ID  0x0
#define SELF_CHECK_CHARGER_CAN_MODE         kCanExtendMode

/**********************************************
 *
 * DTU_CAN Setting
 *
 ***********************************************/
#define  DTU_CAN_DEV              kCanDev3
#define  DTU_CAN_RECEIVE_ID       0x1CECFF00
#define  DTU_CAN_RECEIVE_MASK_ID  0x1FFF00FF
#define  DTU_CAN_MODE             kCanExtendMode
#define  DTU_CAN_BUFFER_COUNT     5

#define  DTU_CAN_PROTOCOL_VERSION 0x02

/**********************************************
 *
 * SOC Daemon Setting
 *
 ***********************************************/
#define SOC_CURRENT_SAMPLE_COUNT            10

#define SOC_CURRENT_SAMPLE_COUNT_MIN        5
#define SOC_CURRENT_SAMPLE_COUNT_MAX        100
#define SOC_CURRENT_SAMPLE_PERIOD           3  // ms
#define SOC_CURRENT_SAMPLE_PERIOD_MIN        2
#define SOC_CURRENT_SAMPLE_PERIOD_MAX        0xFFFF

#define SOC_CURRENT_FILTER_MICRO_CURRENT    15 // 0.1mA
#define SOC_CURRENT_FILTER_MICRO_CURRENT_MIN    0
#define SOC_CURRENT_FILTER_MICRO_CURRENT_MAX    100

#define SOC_CURRENT_SAMPLE_FILTER_NUM 1  // 采样最大最小各自过滤数
#define SOC_CURRENT_FAILURE_DELAY   5000 // 失败延时

#define SOC_CALIB_MIN_SOH  70       // 容量校正最大比例，百分数
#define SOC_CALIB_MAX_RATE  1       // 容量校正单次允许最大比例数，百分数
#define SOC_CALIB_MIN_AH    1       // 容量校正最小允许的安时数，主要用来处理1%不足1AH的情景

#define CAP_CALIB_RESOLUTION_MAX    1000 
#define CAP_CALIB_CNT_RESOLUTION    100      //定义值为1、10、100、1000

#define TOTAL_CAP_DECIMAL_MAX       0xFFFF
#define TOTAL_CAP_DECIMAL_MIN       0

#define SOC_UPWARD_CHECK_SOC_RELEASE    PERCENT_TO_SOC(70) 

/** 用来记录SOC循环，在通过这个区间即认为电池经历过一次循环充放电 */
#define SOC_CYCLE_HIGH_THRESHOLD PERCENT_TO_SOC(90)
#define SOC_CYCLE_LOW_THRESHOLD  PERCENT_TO_SOC(40)

#define SOC_MAX_VALUE PERCENT_TO_SOC(100) // 0.01%
#define SOC_MIN_VALUE PERCENT_TO_SOC(0)

#define SOH_MAX_VALUE PERCENT_TO_SOH(100)
#define SOH_MIN_VALUE PERCENT_TO_SOH(0)

/**********************************************
 *
 * DATA SAVE Setting
 *
 ***********************************************/
#define DATA_SAVE_MAX_LINE_SIZE            2500
#define DATA_SAVE_PATH                     "BMS"

#define DATA_SAVE_CHECK_SDCARD_PERIODIC    10
#define DATA_SAVE_CHECK_SAVE_PERIODIC      100



/**********************************************
 *
 * Insulation Setting
 *
 ***********************************************/
#define INSULATION_MODULE_UNIT              100  // Ω

/**********************************************
 *
 * Relay Control Setting
 *
 ***********************************************/


/**********************************************
 *
 * PROCESSOR SPECIFICS
 *
***********************************************/
#define  OS_TICK_OC                         7           /* Output comp. channel # used to generate OS Tick Int. */
#define  OSCFREQ                            8000000L    /* Define the EVB crystal oscillator frequency          */


/**********************************************
 *
 * ALERT Setting
 *
***********************************************/
#define ALERT_DISABLE_TIMEOUT           86400000     /*报警启动超时定时器, 默认24小时，时间到达后重启启动报警功能*/
#define ALERT_BEEP_ON_PERIODIC          50          /*蜂鸣器鸣笛时间长短*/

/**********************************************
 *
 * Rule Engine Setting
 *
***********************************************/
#define RULE_ENGINE_MAX_RULE_NUM        10
#define RULE_DEFAULT_TEMPERA_STS_DIFF_FROM_DCHG_AND_CHG  0


/**********************************************
 *
 * TODO for Config
 *
***********************************************/
#define TODO_DELAY              2000



/**********************************************
 *
 * Wakeup Config
 *
***********************************************/
#define WAKEUP_INTERVAL_MINUTES (60UL * 24) //休眠唤醒周期24hour
#define WORK_MAX_TIME_AFTER_WAKEUP_SECTONDS (60UL * 2) //上电运行2min


/**********************************************
 *
 * 设备信息
 *
***********************************************/

#define DEVICE_VENDOR_NAME          "Ligoo"
//硬件版本名
#if BMS_SUPPORT_BY5248D 
  #define DEVICE_COMMON_NAME          "BY5XD"
#else
  #define DEVICE_COMMON_NAME          "BC5XB"
#endif
//设备名称
#if BMS_SUPPORT_BY5248D 
  #define DEVICE_NAME                 "BY52D"
  #define DEVICE_NAME_WITH_DTU        "BY54D"
  #define DEVICE_BYU_NAME             "BY52D"
  #define DEVICE_BYU_NAME_WITH_DTU    "BY54D"  
#else  
  #if BMS_SUPPORT_BC5XB_N
    #define DEVICE_NAME                 "BC52BN"
    #define DEVICE_NAME_WITH_DTU        "BC54BN"
    #define DEVICE_BYU_NAME             "BC52BN"
    #define DEVICE_BYU_NAME_WITH_DTU    "BC54BN"
  #else
    #define DEVICE_NAME                 "BC52B"
    #define DEVICE_NAME_WITH_DTU        "BC54B"
    #define DEVICE_BYU_NAME             "BC52B"
    #define DEVICE_BYU_NAME_WITH_DTU    "BC54B"
  #endif
#endif

#define DEVICE_TYPE         0x10

//定义硬件版本号
#if BMS_SUPPORT_BY5248D
  #define DEVICE_HW_MAJOR_VER 1
  #define   DEVICE_HW_MINOR_VER  1
#else
  #define DEVICE_HW_MAJOR_VER 1
  #if (BMS_SUPPORT_NEGATIVE_ADHESION_1_23 || BMS_SUPPORT_NEGATIVE_ADHESION_1_23B)
    #define DEVICE_HW_MINOR_VER 23
  #else
    #define DEVICE_HW_MINOR_VER 21
  #endif 
#endif

#define BMS_BUILD_DATE      __DATE__
#define BMS_BUILD_YEAR      2018  //编译时间
#define BMS_BUILD_MONTH     1
#define BMS_BUILD_DAY       2  
//模板版本                                      
#define DEVICE_SW_MAJOR_VER 2
#define DEVICE_SW_MINOR_VER 1
#define DEVICE_SW_REV_VER   101 
#define DEVICE_SW_BUILD_VER 105                         
//订单版本
#define ORDER_SW_MAJOR_VER  1  
#define ORDER_SW_MINOR_VER  0                  
#define ORDER_SW_REV_VER    0
#define ORDER_SW_BUILD_VER  0




#define DEVICE_SW_SUPPORT_HW_VER    3 //软件支持的硬件版本 由SW_A0_MCU~SW_A2_MCU确定

/***********************************************************************************
 *
 * 内部配置，请不要修改
 *
***********************************************************************************/
#if BMS_SUPPORT_INSULATION_DIAGNOSE > 0
#if BMS_SUPPORT_HVCM_CTL > 0
#       define BMS_TOTAL_SLAVE_NUM      (config_get(kSlaveNumIndex) + 2)
#   else
#       define BMS_TOTAL_SLAVE_NUM      (config_get(kSlaveNumIndex) + 1)
#   endif
#else
#   if BMS_SUPPORT_HVCM_CTL > 0
#       define  BMS_TOTAL_SLAVE_NUM     (config_get(kSlaveNumIndex) + 1)
#   else
#       define  BMS_TOTAL_SLAVE_NUM     (config_get(kSlaveNumIndex))
#   endif
#endif

#if TOTAL_CAP_DECIMAL_MAX < CAP_CALIB_CNT_RESOLUTION
#        error The define of "TOTAL_CAP_DECIMAL_MAX" should be more than the define of "CAP_CALIB_CNT_RESOLUTION" in "bms_soc.h" file
#endif

#if BMS_CHARGER_CUT_OFF_CURRENT <= BMS_VALID_CHARGE_CURRENT_MIN
#   error The define of "BMS_CHARGER_CUT_OFF_CURRENT" should be more than the define of "BMS_VALID_CHARGE_CURRENT_MIN" in "app_cfg.h" file
#elif BMS_CHARGER_CUT_OFF_CURRENT - BMS_VALID_CHARGE_CURRENT_MIN < 10
#   warning The different between "BMS_CHARGER_CUT_OFF_CURRENT" and "BMS_VALID_CHARGE_CURRENT_MIN" is little small in "app_cfg.h" file
#endif

#endif /* APP_CFG_H_ */
