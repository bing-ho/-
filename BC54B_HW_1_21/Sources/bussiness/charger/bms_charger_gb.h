/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_charger_gb.h
* @brief
* @note
* @author
* @date 2012-5-14
*
*/
#include "bms_charger.h"
#include "includes.h"
#include "can_intermediate.h"
#include "ads1013.h"
#include "bms_slow_charger.h"

#ifndef BMS_CHARGER_GB_H_
#define BMS_CHARGER_GB_H_

/*******************************���������ò���*********************************/
/**
* ����ʵ������޸� "�������"
* 1:Ǧ���� 
* 2:������ 
* 3:������﮵�� 
* 4:����﮵�� 
* 5:����﮵�� 
* 6:��Ԫ���ϵ�� 
* 7:�ۺ�������ӵ�� 
* 8:����﮵�� 
* FF:�������
**/
#define GUOBIAO_BATTERY_TYPE                3 //�������
#define GUOBIAO_RELAY_ADHESION_CHECK_EN     0 //�̵���ճ�����ʹ��
#define GUOBIAO_AC_VOLT_DEFAULT             GUOBIAO_SINGLE_PHASE_VOLT // v/bit ������繩���ѹ�˵�ѹֵ�����ڹ���ת�� ��ѡ 380v

//ע�⣺Ŀǰʵ��ճ���������˷ŵ�̵�����Ԥ��̵�������Ͽ���Ҫ��
#define GUOBIAO_RELAY_ADHESION_CHECK_TV     MV_TO_V(bcu_get_insulation_pch_total_voltage()) // V/bit

/*******************************���������ò���end********************************/

#define GUOBIAO_CHARGER_CAN_MODE kCanExtendMode

#define GUOBIAO_CHARGER_PROTOCOL_VERSION_DEFAULT    GUOBIAO_CHARGER_PROTOCOL_VERSION_V11
#define GUOBIAO_CHARGER_PROTOCOL_VERSION_V10        "V1.0"
#define GUOBIAO_CHARGER_PROTOCOL_VERSION_V11        "V1.1"


#define GUOBIAO_CHARGER_ENABLE_CONTROL_BYTE 0
#define GUOBIAO_CHARGER_DISABLE_CONTROL_BYTE 1

#define GUOBIAO_CHARGER_ID              0x56

#define GUOBIAO_CC_DETECT_JOB_PERIODIC  30   /* bcu jobʱ�� */
#define GUOBIAO_PWM_DETECT_JOB_PERIODIC 200   /* bcu jobʱ�� */

#define GUOBIAO_MESSAGE_PRI_DEFAULT 	6
#define GUOBIAO_BRO_PGN_2304_PRI        4
#define GUOBIAO_BST_PGN_6400_PRI        4
#define GUOBIAO_BEM_PGN_7680_PRI        2

#define GUOBIAO_2015_BRM_PGN_PRI        7
#define GUOBIAO_2015_BCP_PGN_PRI        7
#define GUOBIAO_2015_BCS_PGN_PRI        7
#define GUOBIAO_2015_BMV_PGN_PRI        7
#define GUOBIAO_2015_BMT_PGN_PRI        7
#define GUOBIAO_2015_BSP_PGN_PRI        7


#define GUOBIAO_BYTE_VALUE_DEFAULT      0xFF
#define GUOBIAO_WORD_VALUE_DEFAULT      0xFFFF
#define GUOBIAO_3_BYTES_VALUE_DEFAULT   0xFFFFFF
#define GUOBIAO_LONG_VALUE_DEFAULT      0xFFFFFFFF
#define GUOBIAO_NOMINAL_TV			    5000

#define GUOBIAO_MESSAGE_RE_SEND_TIME    1

#define GUOBIAO_PROTOCOL_VERSION_2011   0x100
#define GUOBIAO_PROTOCOL_VERSION_2015   0x101

#define GUOBIAO_MESSAGE_CHM_PGN_9728    0x2600
#define GUOBIAO_MESSAGE_BHM_PGN_9984    0x2700

#define GUOBIAO_MESSAGE_CRM_PGN_256     0x100
#define GUOBIAO_MESSAGE_BRM_PGN_512     0x200
#define GUOBIAO_MESSAGE_BCP_PGN_1536    0x600
#define GUOBIAO_MESSAGE_CTS_PGN_1792    0x700
#define GUOBIAO_MESSAGE_CML_PGN_2048    0x800
#define GUOBIAO_MESSAGE_BRO_PGN_2304    0x900
#define GUOBIAO_MESSAGE_CRO_PGN_2560    0xA00
#define GUOBIAO_MESSAGE_BCL_PGN_4096    0x1000
#define GUOBIAO_MESSAGE_BCS_PGN_4352    0x1100
#define GUOBIAO_MESSAGE_CCS_PGN_4608    0x1200
#define GUOBIAO_MESSAGE_BSM_PGN_4864    0x1300
#define GUOBIAO_MESSAGE_BMV_PGN_5376    0x1500
#define GUOBIAO_MESSAGE_BMT_PGN_5632    0x1600
#define GUOBIAO_MESSAGE_BSP_PGN_5888    0x1700
#define GUOBIAO_MESSAGE_BST_PGN_6400    0x1900
#define GUOBIAO_MESSAGE_CST_PGN_6656    0x1A00
#define GUOBIAO_MESSAGE_BSD_PGN_7168    0x1C00
#define GUOBIAO_MESSAGE_CSD_PGN_7424    0x1D00
#define GUOBIAO_MESSAGE_BEM_PGN_7680    0x1E00
#define GUOBIAO_MESSAGE_CEM_PGN_7936    0x1F00

#define GUOBIAO_SHAKE_HAND_TIME         250
#define GUOBIAO_PARA_CONFIG_TIME        500
#define GUOBIAO_CONFIG_READY_TIME       250
#define GUOBIAO_CHARGING_TIME           5
#define GUOBIAO_CHARGING_TIME_4352      250 //��λ��ms
#define GUOBIAO_CHARGING_TIME_5376      1000 //���壺ms
#define GUOBIAO_2015_CHARGING_TIME_5376 10000
#define GUOBIAO_CHARGE_STOP_TIME        10
#define GUOBIAO_CHARGE_ERROR_TIME       250
#define GUOBIAO_STATISTICS_TIME         250

#define GUOBIAO_TIMEOUT_DEFAULT         5000
#define GUOBIAO_SHAKE_HAND_TIMEOUT      60000
#define GUOBIAO_SHAKE_HAND_CHS_TIMEOUT  30000
#define GUOBIAO_SHAKE_HAND_CHS_AA_TIMEOUT   5000
#define GUOBIAO_PARA_CONFIG_TIMEOUT     5000
#define GUOBIAO_CONFIG_READY_TIMEOUT    5000
#define GUOBIAO_CHARGING_TIMEOUT        1000
#define GUOBIAO_CHARGE_STOP_TIMEOUT     5000
#define GUOBIAO_CHARGE_ERROR_TIMEOUT    30000
#define GUOBIAO_STATISTICS_TIMEOUT      10000

#define CHG_UN_IDENTIFICATION_NUM       0x00
#define CHG_IDENTIFICATION_NUM          0xAA

#define BMS_CHARGE_READY                0xAA
#define BMS_CHARGE_NOT_READY            0x00

#define GUOBIAO_CHARGER_BMS_RECEIVE_MASK_ID     0x1FFF0000
#define GUOBIAO_CHARGER_BMS_RECEIVE_ID  0x1801F456

#define GUOBIAO_REC_SPN2560_00_TIMEOUT_VALUE        0x01
#define GUOBIAO_REC_SPN2560_AA_TIMEOUT_VALUE        0x04
#define GUOBIAO_REC_CAPABILITY_TIMEOUT_VALUE        0x01
#define GUOBIAO_REC_CHARGER_READY_TIMEOUT_VALUE     0x04
#define GUOBIAO_REC_CHARGER_STATUS_TIMEOUT_VALUE    0x01
#define GUOBIAO_REC_CHARGER_STOP_TIMEOUT_VALUE      0x04
#define GUOBIAO_REC_CHARGER_STATIS_TIMEOUT_VALUE    0x01

#define GUOBIAO_CC_CC2_OFFSET           0//700 //mv

#define GUOBIAO_CC_1500OHM_VALUE_MIN    1910//2080//2690 //mv
#define GUOBIAO_CC_1500OHM_VALUE_MAX    2370//2202//3220 //mv
#define GUOBIAO_CC_680OHM_VALUE_MIN     1060//1220//1860 //mv
#define GUOBIAO_CC_680OHM_VALUE_MAX     1480//1315//2430 //mv
#define GUOBIAO_CC_220OHM_VALUE_MIN     380//473//830  //mv
#define GUOBIAO_CC_220OHM_VALUE_MAX     610//518//1270 //mv
#define GUOBIAO_CC_100OHM_VALUE_MIN     80//226//400  //mv
#define GUOBIAO_CC_100OHM_VALUE_MAX     310//250//710  //mv

#define GUOBIAO_CC2_VALUE_MIN           950//1610//1350 //mv
#define GUOBIAO_CC2_VALUE_MAX           2250//1721//2730 //mv

#define GUOBIAO_PWM_COMM_TIMEOUT        5000 //pwm�źż�ⳬʱ

#define CP_PWM_FREQUENCE_MIN            900 //HZ
#define CP_PWM_FREQUENCE_MAX            1100 //HZ
 
#define GUOBIAO_SINGLE_PHASE_VOLT       220 // v/bit �����ѹֵ
#define GUOBIAO_THREE_PHASE_VOLT        380 // v/bit �����ѹֵ

#define GUOBIAO_CURRENT_DECREASE_MAX_PER_S          200 // 0.1A/bit ������������

#define GUOBIAO_COMM_ABORT_TIMEOUT_TIMES            3 //��Ϊ��ͨ���жϵĳ�ʱ����
#define GUOBIAO_RELAY_ADHESION_CHECK_TIMEOUT        3000 //ms/bit ���̵���ճ����ⳬʱʱ��
#define GUOBIAO_CURRENT_OFFSET           4000 // 0.1A/bit�������ƫ����

#define GUOBIAO_CHG_TIMEOUT_BYTE_NUM     4
#define GUOBIAO_BMS_TIMEOUT_BYTE_NUM     4

typedef enum
{
    kPhysicalConnectStage = 0,
    kShakeHandsStage,
    kParaConfigStage,
    kChargeReadyStage,
    kChargeStage,
    kChargeStopStage,
    kChargeStatisticsStage,
    kChargeErrorStage,
    kChargeEndStage
}GuoBiaoChargerCommStage;

typedef enum
{
    kGBChargerConnectNone = 0,
    kGBChargerConnectInputSignal,
    kGBChargerConnectAC16A3A,
    kGBChargerConnectAC16A3B,
    kGBChargerConnectAC16A3C,
    kGBChargerConnectAC16A2B,
    kGBChargerConnectAC16A1B,
    kGBChargerConnectAC32A3A,
    kGBChargerConnectAC32A3B,
    kGBChargerConnectAC32A3C,
    kGBChargerConnectAC32A2B,
    kGBChargerConnectAC32A1B,
    kGBChargerConnectDC,
    kGBChargerConnectAuto,
    kGBChargerConnectMax
}GBChargerConnectMode;

typedef struct
{
    can_t can_context;
    INT16U max_voltage;
    INT16U current;
    INT8U  control_byte;

    INT16U output_voltage;
    INT16S output_current;
    INT16U status;
}GuoBiaoCharger;

typedef enum
{
    kGuoBiaoNoneStop = 0,
    kGuoBiaoCharger,
    kGuoBiaoBms
}GuoBiaoStopChgDev;

typedef enum
{
    kGBChgFaultNone = 0, //����
    kGBChgFaultLineOff, //���������δ����
    kGBChgFaultFullCharge, //����
    kGBChgFaultHTV, //��ѹ����
    kGBChgFaultHV, //�������
    kGBChgFaultLTV, //��ѹ����
    kGBChgFaultLV, //�������
    kGBChgFaultDV, //����ѹ�����
    kGBChgFaultDT, //�����²����
    kGBChgFaultCommAbort, //����ͨ���쳣
    kGBChgFaultVoltLine, //��ѹ��������
    kGBChgFaultTempLine, //�¸���������
    kGBChgFaultLSOC, //SOC����
    kGBChgFaultHSOC, //SOC����
    kGBChgFaultLeak, //��Ե����
    kGBChgFaultBatteryHT, //������¶ȹ���
    kGBChgFaultBatteryLT, //������¶ȹ���
    kGBChgFaultOCC, //������
    kGBChgFaultODC, //�ŵ����
    kGBChgFaultTVException, //��ѹ�쳣
    kGBChgFaultChargerStop, //����ֹͣ���
    kGBChgFaultSPN3901, //���ճ���δ������ʶ���ĳ�ʱ
    kGBChgFaultSPN3902, //���ճ���������ʶ���ĳ�ʱ
    kGBChgFaultSPN3903, //���ճ����������������ĳ�ʱ
    kGBChgFaultSPN3904, //���ճ�����ɳ��׼�����ĳ�ʱ
    kGBChgFaultSPN3905, //���ճ������״̬���ĳ�ʱ
    kGBChgFaultSPN3906, //���ճ�����ֹ��籨�ĳ�ʱ
    kGBChgFaultSPN3907, //���ճ������ͳ�Ʊ��ĳ�ʱ
    kGBChgFaultSPN3921, //����BMS��ʶ���ĳ�ʱ
    kGBChgFaultSPN3922, //����BMS���������ĳ�ʱ
    kGBChgFaultSPN3923, //����BMS��ɳ��׼�����ĳ�ʱ
    kGBChgFaultSPN3924, //����BMS�����״̬���ĳ�ʱ
    kGBChgFaultSPN3925, //����BMS���Ҫ���ĳ�ʱ
    kGBChgFaultSPN3926, //����BMS��ֹ��籨�ĳ�ʱ
    kGBChgFaultSPN3927, //����BMS���ͳ�Ʊ��ĳ�ʱ
    kGBChgFaultAdhesion, //�̵���ճ������
    kGBChgFaultOverTimeoutTimes, //��ʱ��������
    kGBChgFaultOthers //��������
}GuoBiaoChgSelfCheckFault;

typedef enum
{
    kShakeHandStageNone=0,
    kShakeHandStartStage,
    kShakeHandIdentifyStage
}GuoBiaoShakeHandStage;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER
extern GuoBiaoCharger g_guobiao_charger;
extern INT16U g_guoBiaoCCValue;
extern INT16U g_guoBiaoCC2Value;
extern INT16U g_guobiao_pwm_duty_cycle;
#pragma DATA_SEG DEFAULT

void guobiao_charger_init(void);

GuoBiaoChgSelfCheckFault guobiao_charger_get_selfcheck_fault_num(void);
void guobiao_cc_detection_init(void);
void guobiao_cc_cc2_detect(void* data);
void guobiao_pwm_detect(void* data);
//INT8U guobiao_charger_is_pwm_over_current(void);
INT8U guobiao_charger_protocol_version_is_v11(void);
void guobiao_charger_protocol_version_set(PINT8U str);

GuoBiaoChargerCommStage guobiao_charger_get_CommStage(void);

#endif /* BMS_CHARGER_GB_H_ */
