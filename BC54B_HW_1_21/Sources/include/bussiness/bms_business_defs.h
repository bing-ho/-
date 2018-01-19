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

#define BMS_POWER_ON_VALID_SLAVE_DELAY  450  //�ϵ��ӻ�������Ч��ʱ
#define BMS_SLEF_CHECK_DELAY  500            //�Լ���ʱʱ�䣬��BMS_POWER_ON_VALID_SLAVE_DELAY��
#define BMS_SLEF_CHECK_TIMEOUT_DELAY  1000   //����S�ʹӻ���ͨ�ų�ʱʱ��

/*********************************************************************************
 *                 ������BMS�ⲿ���ݷ��ͺ����������
 *********************************************************************************/
typedef enum
{
    kSystemStatePowerOn = 0, //�ϵ�
    kSystemStateAvailable, //����
    kSystemStateCharging, //���
    kSystemStateFailure, //����
    kSystemStateDiagnose,//���
    kSystemStatePowerOff  //�µ�
} SystemState;

/*PackState*/
#define  TEMP_OLT_ALERT_BIT     16 /*����*/
#define  CELL_ODC_ALERT_BIT     11           //�ŵ����״̬λ
#define  CELL_OCC_ALERT_BIT     10           //������״̬λ
#define  EEPROM_ERR_BIT         9
#define  DATE_TIME_READ_ERR_BIT 8
#define  PACK_COMM_ALERT_BIT    7
#define  CELL_ODV_STOP_BIT       6 /*����*/
#define  TEMP_OHT_ALERT_BIT     5 /*����*/
#define  PACK_CHG_BIT           2
#define  CELL_LV_ALERT_BIT      1 /* Ƿѹ */
#define  CELL_OHV_ALERT_BIT      0 /*��ѹ, ���� */


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
#define  SELFCHECK_OHV          0x01    /*���䱨����־λ0*/
#define  SELFCHECK_OLV          0x02    /*���ű�����־λ1*/
#define  SELFCHECK_OHT          0x04    /*���±�����־λ2*/
#define  SELFCHECK_DT           0x08    /*�²����־λ3*/
#define  SELFCHECK_COMM         0x10    /*����ͨ�ű�����־λ4*/
#define  SELFCHECK_LOW_SOC      0x20    /*SOC������־λ5*/
#define  SELFCHECK_INSU         0x40    /*��Ե��Ե©�籨����־λ6*/
#define  SELFCHECK_CHG_TV_OLV   0x80    /*�����ѹ�ͱ�����־*/
#define  SELFCHECK_CHARGEWIREOFF 0x0100 //TODO
#define  SELFCHECK_CHG_TV_OHV   0x0200  /*�����ѹ�߱�����־*/
#define  SELFCHECK_OLT          0x0400
#define  SELFCHECK_DV           0x0800
#define  SELFCHECK_VOLT_EXCEPTION    0x1000
#define  SELFCHECK_TEMP_EXCEPTION    0x2000
#define  SELFCHECK_TV_OHV               0x4000
#define  SELFCHECK_TV_OLV               0x8000
#define  SELFCHECK_CC                   0x10000 /* ������ */
#define  SELFCHECK_DC                   0x20000 /* �ŵ���� */

#define  SELFCHECK_CURRENT_SAMPLE_FAILURE   0x40000
#define  SELFCHECK_CURRENT_SENSOR_FAILURE   0x80000

#define  SELFCHECK_FULL_CHARGE  0x100000    /*������䱨����־*/
#define  SELFCHECK_HIGH_SOC     0x200000    /*SOC�߱�����־*/
#define  SELFCHECK_CHG_OHV      0x400000    /*����ѹ������־*/
#define  SELFCHECK_CHG_OLV      0x800000    /*����ѹ������־*/
#define  SELFCHECK_CHG_OHT      0x1000000   /*�����±�����־*/
#define  SELFCHECK_CHG_DT       0x2000000   /*����²����־*/
#define  SELFCHECK_CHG_OLT      0x4000000   /*�����±�����־*/
#define  SELFCHECK_CHG_DV       0x8000000   /*���ѹ�����־*/
#define  SELFCHECK_CHG_HEAT_TEMP    0x10000000 /*�����¶ȱ�����־*/
#define  SELFCHECK_CHR_OUTLET_HT    0x20000000 /*�������¶ȱ�����־*/

#define SYS_EXCEPTION_CONFIG_LIMIT_ERROR           0x0001
#define SYS_EXCEPTION_CONFIG_WRITE_ERROR           0x0002
#define SYS_EXCEPTION_CONFIG_READ_EEPROM_ERROR     0x0004
#define SYS_EXCEPTION_CONFIG_READ_EEEPROM_ERROR    0x0008
#define SYS_EXCEPTION_CURRENT_ERROR                0x0010
#define SYS_EXCEPTION_FLASH_ERROR                  0x0020
#define SYS_EXCEPTION_EEEPROM_FORMAT_ERROR         0x0040 //��ǿ��EEPROM��ʼ���쳣

/*WatchdogFlag*/
#define  WDT_MAX_COUNT            5

#define  WDT_CURRENT_ID           0    /*�����ɼ�����λ0*/
#define  WDT_CURRENT_TIME         2

#define  WDT_SLAVECAN_ID          1    /*CAN����ͨ������λ1*/
#define  WDT_SLAVECAN_TIME        6

#define  WDT_SYSTEM_ID            2    /*ϵͳ״̬����λ2*/
#define  WDT_SYSTEM_TIME          6

#define  WDT_DCM_TX_ID            3    /*485����ͨ������λ3*/
#define  WDT_DCM_TIME             6

#define  WDT_RULE_ID              4    /*�����ƶ�����λ4*/
#define  WDT_RULE_TIME            30

#define  WDT_AD_ID                5
#define  WDT_AD_TIME              15


/*Full charge*/
#define CHARGE_TOTAL_TIME_CHECK_EN  1   //�����ʱ����ʹ��

#define HARDWARE_CHECK_FAIL_BEEP_TIME     100

/*λ�����궨��,bit����15ʱ,��Ҫ��1д��1UL*/
#define SET(a,bit)  ( (a) |=  ((INT32U)1<<(bit)) )     /*ĳλ��λ*/
#define CLR(a,bit)  ( (a) &= ~((INT32U)1<<(bit)) )     /*ĳλ����*/
#define GET(a,bit)  ( (a) &   ((INT32U)1<<(bit)) )     /*��ȡĳλ*/
/*
 ********************************************************************************
 *                  BMS���ݶ���
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
} RelayState; //�̵�������״̬

typedef enum
{
    kPInsuStateNormal = 0, //����
    kPInsuStateTLeak = 1, //��΢©��
    kPInsuStateLeak = 2, //�߶�©��
    kPInsuStateULeak = 3 //����©��
} PInsuState; //������Ե���״̬

typedef enum
{
    kInsulationWorkStateNormal = 0, //����
    kInsulationWorkStateVoltExcept = 1, //û��׼����
    kInsulationWorkStateResOverLow = 2, // ����ѹ�������ε���������
    kInsulationWorkStateNotConnected = 5, // û������
}InsulationWorkState;

typedef enum
{
    kConfigStateIdle = 0, /*���ù�������*/
    kPChargeStateRunning = 1/*���ò������ڸ���*/
} ConfigState;

typedef enum _RelayState
{
    kRelayStatusOff = 0,  // �Ͽ�״̬
    kRelayStatusOn, // �պ�״̬
    kRelayStatusPreCharging, // Ԥ��״̬�� Ϊ�ŵ�̵���״̬����
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
    kSOCEmptyCalibNormal=0, //У������
    kSOCEmptyCalibNoTrigger, //δ����У��
    kSOCEmptyCalib3rdLVNotRel, //�ϴ�У��δ�ͷ�
    kSOCEmptyCalibHSOC, //��ǰSOC���ڵ�У��SOC����ֵ
    kSOCEmptyCalibHighAverageVolt, //��ǰƽ����ѹ���ڵ�У��ƽ����ѹ����ֵ
    kSOCEmptyCalibDelay //У����ʱ��
}SOCEmptyUnCalibReason;

typedef enum
{
    kSOCFullCalibNormal=0, //У������
    kSOCFullCalibNoTrigger, //δ����У��
    kSOCFullCalibNotRel, //�ϴ�У��δ�ͷ�
    kSOCFullCalibLowAverageVolt, //��ǰƽ����ѹ���ڸ�У��ƽ����ѹ����ֵ
}SOCFullUnCalibReason;

typedef enum
{
    kPrechargeStateWaitting = 0, //�ȴ�Ԥ��
    kPrechargeStatePrecharging, //Ԥ�����
    kPrechargeStateFinish, //Ԥ�����
    kPrechargeStateFailure //Ԥ��ʧ��
}BMS_PRECHARGE_STATE;

typedef enum
{
    kBmsPowerTriggerNone = 0,
    kBmsPowerTriggerON,
    kBmsPowerTriggerChargerReady,
    kBmsPowerTriggerSignalIn1,
    kBmsPowerTriggerCP,
    kBmsPowerTriggerRTC,
    kBmsPowerTrigger24VIN, //��������
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
