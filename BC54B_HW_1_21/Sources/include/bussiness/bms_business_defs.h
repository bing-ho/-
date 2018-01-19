/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_business_defs.h
* @brief
* @note
* @author
* @date 2012-5-31
*
*/

#ifndef BMS_BUSINESS_DEFS_H_
#define BMS_BUSINESS_DEFS_H_
#include "bms_defs.h"
//#include "includes.h"

#ifdef   BMS_GLOBALS
#define  BMS_EXT
#else
#define  BMS_EXT  extern
#endif

#define BMS_POWER_ON_VALID_SLAVE_DELAY  450  //上电后从机数据有效延时
#define BMS_SLEF_CHECK_DELAY  500            //自检延时时间，比BMS_POWER_ON_VALID_SLAVE_DELAY大
#define BMS_SLEF_CHECK_TIMEOUT_DELAY  1000   //连接S型从机，通信超时时间

/*********************************************************************************
 *                 可用于BMS外部数据发送和请求的数据
 *********************************************************************************/
typedef enum
{
    kSystemStatePowerOn = 0, //上电
    kSystemStateAvailable, //可用
    kSystemStateCharging, //充电
    kSystemStateFailure, //错误
    kSystemStateDiagnose,//诊断
    kSystemStatePowerOff  //下电
} SystemState;

/*PackState*/
#define  TEMP_OLT_ALERT_BIT     16 /*低温*/
#define  CELL_ODC_ALERT_BIT     11           //放电过流状态位
#define  CELL_OCC_ALERT_BIT     10           //充电过流状态位
#define  EEPROM_ERR_BIT         9
#define  DATE_TIME_READ_ERR_BIT 8
#define  PACK_COMM_ALERT_BIT    7
#define  CELL_ODV_STOP_BIT       6 /*过放*/
#define  TEMP_OHT_ALERT_BIT     5 /*过温*/
#define  PACK_CHG_BIT           2
#define  CELL_LV_ALERT_BIT      1 /* 欠压 */
#define  CELL_OHV_ALERT_BIT      0 /*过压, 过充 */


/*BmsStatus*/
#define  BMS_OHV_STS_BIT         0
#define  BMS_OLV_STS_BIT         1
#define  BMS_OHT_STS_BIT         2
#define  BMS_ODC_STS_BIT         3
#define  BMS_LSOC_STS_BIT        4
#define  BMS_LEAK_STS_BIT        5
#define  BMS_ULEAK_STS_BIT       6
#define  BMS_COMM_ABORT_STS_BIT  7
#define  BMS_OHTV_STS_BIT        8
#define  BMS_OLTV_STS_BIT        9
#define  BMS_OCC_STS_BIT         10
#define  BMS_OLT_STS_BIT         11
#define  BMS_DIFFT_STS_BIT       12
#define  BMS_DIFFV_STS_BIT       13
#define  BMS_QRT_STS_BIT         14
#define  BMS_SHORT_STS_BIT       15

/*AlarmCheckFlag*/
#define  SELFCHECK_OHV          0x01    /*过充报警标志位0*/
#define  SELFCHECK_OLV          0x02    /*过放报警标志位1*/
#define  SELFCHECK_OHT          0x04    /*过温报警标志位2*/
#define  SELFCHECK_DT           0x08    /*温差报警标志位3*/
#define  SELFCHECK_COMM         0x10    /*内网通信报警标志位4*/
#define  SELFCHECK_LOW_SOC      0x20    /*SOC报警标志位5*/
#define  SELFCHECK_INSU         0x40    /*绝缘绝缘漏电报警标志位6*/
#define  SELFCHECK_CHG_TV_OLV   0x80    /*充电总压低报警标志*/
#define  SELFCHECK_CHARGEWIREOFF 0x0100 //TODO
#define  SELFCHECK_CHG_TV_OHV   0x0200  /*充电总压高报警标志*/
#define  SELFCHECK_OLT          0x0400
#define  SELFCHECK_DV           0x0800
#define  SELFCHECK_VOLT_EXCEPTION    0x1000
#define  SELFCHECK_TEMP_EXCEPTION    0x2000
#define  SELFCHECK_TV_OHV               0x4000
#define  SELFCHECK_TV_OLV               0x8000
#define  SELFCHECK_CC                   0x10000 /* 充电电流 */
#define  SELFCHECK_DC                   0x20000 /* 放电电流 */

#define  SELFCHECK_CURRENT_SAMPLE_FAILURE   0x40000
#define  SELFCHECK_CURRENT_SENSOR_FAILURE   0x80000

#define  SELFCHECK_FULL_CHARGE  0x100000    /*充电满充报警标志*/
#define  SELFCHECK_HIGH_SOC     0x200000    /*SOC高报警标志*/
#define  SELFCHECK_CHG_OHV      0x400000    /*充电高压报警标志*/
#define  SELFCHECK_CHG_OLV      0x800000    /*充电低压报警标志*/
#define  SELFCHECK_CHG_OHT      0x1000000   /*充电过温报警标志*/
#define  SELFCHECK_CHG_DT       0x2000000   /*充电温差报警标志*/
#define  SELFCHECK_CHG_OLT      0x4000000   /*充电低温报警标志*/
#define  SELFCHECK_CHG_DV       0x8000000   /*充电压差报警标志*/
#define  SELFCHECK_CHG_HEAT_TEMP    0x10000000 /*加热温度报警标志*/
#define  SELFCHECK_CHR_OUTLET_HT    0x20000000 /*充电插座温度报警标志*/

#define SYS_EXCEPTION_CONFIG_LIMIT_ERROR           0x0001
#define SYS_EXCEPTION_CONFIG_WRITE_ERROR           0x0002
#define SYS_EXCEPTION_CONFIG_READ_EEPROM_ERROR     0x0004
#define SYS_EXCEPTION_CONFIG_READ_EEEPROM_ERROR    0x0008
#define SYS_EXCEPTION_CURRENT_ERROR                0x0010
#define SYS_EXCEPTION_FLASH_ERROR                  0x0020
#define SYS_EXCEPTION_EEEPROM_FORMAT_ERROR         0x0040 //增强型EEPROM初始化异常

/*WatchdogFlag*/
#define  WDT_MAX_COUNT            5

#define  WDT_CURRENT_ID           0    /*电流采集任务位0*/
#define  WDT_CURRENT_TIME         2

#define  WDT_SLAVECAN_ID          1    /*CAN主从通信任务位1*/
#define  WDT_SLAVECAN_TIME        6

#define  WDT_SYSTEM_ID            2    /*系统状态任务位2*/
#define  WDT_SYSTEM_TIME          6

#define  WDT_DCM_TX_ID            3    /*485主从通信任务位3*/
#define  WDT_DCM_TIME             6

#define  WDT_RULE_ID              4    /*规则制定任务位4*/
#define  WDT_RULE_TIME            30

#define  WDT_AD_ID                5
#define  WDT_AD_TIME              15


/*Full charge*/
#define CHARGE_TOTAL_TIME_CHECK_EN  1   //充电总时间检查使能

#define HARDWARE_CHECK_FAIL_BEEP_TIME     100

/*位操作宏定义,bit大于15时,需要把1写成1UL*/
#define SET(a,bit)  ( (a) |=  ((INT32U)1<<(bit)) )     /*某位置位*/
#define CLR(a,bit)  ( (a) &= ~((INT32U)1<<(bit)) )     /*某位清零*/
#define GET(a,bit)  ( (a) &   ((INT32U)1<<(bit)) )     /*读取某位*/
/*
 ********************************************************************************
 *                  BMS数据定义
 ********************************************************************************
 */
typedef enum{
    kBcuV4_21 = 0x01,
    kBcuBC52A,
    kBcuBY31B,
    kBcuV4_22,
    kBcuBC5xB,
    kBcuBY5xS,
    kBcuBY5xD,
    kBmuV4_20 = 0x80,
    kBmuM1216,
    kBmu2125,
    kBmu5160,
    kBroadcastDevice = 0xFF
}BtDeviceType;

typedef enum
{
    kRelayStateNormal = 0, //
    kRelayStateError = 1 //
} RelayState; //继电器故障状态

typedef enum
{
    kPInsuStateNormal = 0, //正常
    kPInsuStateTLeak = 1, //轻微漏电
    kPInsuStateLeak = 2, //高度漏电
    kPInsuStateULeak = 3 //严重漏电
} PInsuState; //电池组绝缘检测状态

typedef enum
{
    kInsulationWorkStateNormal = 0, //正常
    kInsulationWorkStateVoltExcept = 1, //没有准备好
    kInsulationWorkStateResOverLow = 2, // 检测电压出错，本次电阻计算出错
    kInsulationWorkStateNotConnected = 5, // 没有连接
}InsulationWorkState;

typedef enum
{
    kConfigStateIdle = 0, /*配置功能闲置*/
    kPChargeStateRunning = 1/*配置参数正在更新*/
} ConfigState;

typedef enum _RelayState
{
    kRelayStatusOff = 0,  // 断开状态
    kRelayStatusOn, // 闭合状态
    kRelayStatusPreCharging, // 预充状态， 为放电继电器状态所有
}RelayState;

typedef enum
{
    kMCUWorkStateStop = 0,
    kMCUWorkStateWork = 1,
}MCUWorkState;

typedef enum
{
    kInsulationNone = 0,
    kInsulationTypeCan,
    kInsulationTypeRS485,
    kInsulationTypeBoard,
    kInsulationTypeBoth,
    kInsulationTypeMax = kInsulationTypeBoth
}InsulationType;

typedef enum
{
    kStartFailureNone=0,
    kStartFailureOccure,
    kStartFailureRecovery
}StartFailureState;

typedef enum
{
    kSOCEmptyCalibNormal=0, //校正正常
    kSOCEmptyCalibNoTrigger, //未触发校正
    kSOCEmptyCalib3rdLVNotRel, //上次校正未释放
    kSOCEmptyCalibHSOC, //当前SOC高于低校正SOC配置值
    kSOCEmptyCalibHighAverageVolt, //当前平均电压高于低校正平均电压配置值
    kSOCEmptyCalibDelay //校正延时中
}SOCEmptyUnCalibReason;

typedef enum
{
    kSOCFullCalibNormal=0, //校正正常
    kSOCFullCalibNoTrigger, //未触发校正
    kSOCFullCalibNotRel, //上次校正未释放
    kSOCFullCalibLowAverageVolt, //当前平均电压低于高校正平均电压配置值
}SOCFullUnCalibReason;

typedef enum
{
    kPrechargeStateWaitting = 0, //等待预充
    kPrechargeStatePrecharging, //预充电中
    kPrechargeStateFinish, //预充完成
    kPrechargeStateFailure //预充失败
}BMS_PRECHARGE_STATE;

typedef enum
{
    kBmsPowerTriggerNone = 0,
    kBmsPowerTriggerON,
    kBmsPowerTriggerChargerReady,
    kBmsPowerTriggerSignalIn1,
    kBmsPowerTriggerCP,
    kBmsPowerTriggerRTC,
    kBmsPowerTrigger24VIN, //充电机供电
    kBmsPowerTriggerSoftwareLatch
}BMSPowerTriggerSignal;

#if 0
/**********************************************
 * Definitions
 ***********************************************/

/** Type */
typedef INT16U VoltageType;  // unit: 1mV
typedef INT32U TotalVoltageType; // unit: 1mV
typedef INT16U CurrentType;  // unit: 1mA
typedef INT16U SocType;      // unit: 0.01%
typedef INT16U CapType;      // unit: 1mAH
typedef INT8U  TemperatureType; // unit: 1

#endif


#endif /* BMS_BUSINESS_DEFS_H_ */
