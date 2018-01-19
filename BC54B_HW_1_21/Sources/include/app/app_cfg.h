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
#define BMS_SUPPORT_BATTERY             BMS_BATTERY_LIFEPO4 // �������

#define BMS_COMMUNICATION_MODE          BMS_COMM_SLAVE_MODE // ����ͨ��ģʽ

#define BMS_SUPPORT_HARDWARE_BEFORE     0                   // ����֧��Ӳ����ǰ���� yang 20161213

#define BMS_SUPPORT_LTC6804_ON_BOARD_CHIP_NUM    4
#define BMS_SUPPORT_HARDWARE_LTC6804       1                   // ����֧��6804 cao 20170108
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1
    #define BMS_SUPPORT_HARDWARE_LTC6803   0
#else 
    #define BMS_SUPPORT_HARDWARE_LTC6803   1
#endif

#define BMS_SUPPORT_SOC0_DISCHG_FACTOR  1 

#define BMS_SUPPORT_SOC                 BMS_SOC_INTEGRATION// // SOC���ģ��
#define BMS_SUPPORT_SOH                 BMS_SOH_DEFAULT
#define BMS_SUPPORT_DATA_SAVE           1//BMS_DATA_SAVE_FLASH //BMS_DATA_SAVE_SDCARD // ���ݴ洢ģ��//
#define BMS_SUPPORT_SYSTEM_VOLTAGE      1
#define BMS_SUPPORT_BOARD_TEMPERATURE   1

#define BMS_SUPPORT_HMI                 BMS_HMI_DEFAULT//BMS_HMI_DEFAULT // BMS_HMI_DEFAULT  // ������
#define BMS_SUPPORT_UPPER_COMPUTER      BMS_UPPER_COMPUTER_J1939 // ��λ��ģ��
#define BMS_SUPPORT_DTU                 BMS_DTU_DEFAULT // DTUģ��

#define BMS_SUPPORT_EMS                 BMS_EMS_DEFAULT //����ϵͳ
#define BMS_SUPPORT_CHARGER             BMS_CHARGER_TIECHENG //BMS_CHARGER_TIECHENG //BMS_CHARGER_TIECHENG // BMS_CHARGER_TIECHENG //����ģ��
#define BMS_SUPPORT_RELAY               1 // �̵���ģ��
#define BMS_SUPPORT_INPUT_SIGNAL        1 // ���뿪���ź�
#define BMS_SUPPORT_MOTOR               0 //BMS_MOTOR_DEFAULT  // ���ģ��

#define BMS_SUPPORT_INSULATION_DIAGNOSE 1 // ��Ե�������ģ��

#define BMS_SUPPORT_PRE_DISCHARGE       1           //����Ԥ��繦��
#define BMS_SUPPORT_HVCM_CTL            0           //ǿ����ƺй���

#define BMS_SUPPORT_J1939_COMM            1    //J1939ͨ��֧��

#define BMS_SUPPORT_LED_CONTROL         0

#define BMS_SUPPORT_SLEEP_MODE          0
#define BMS_SUPPORT_STAT                1  // ֧��ͳ�ƹ���

#define BMS_SUPPORT_LOAD_NODE_DETCTION  0  // ���ؼ�⣬ ��C32ƽ̨֧��

#define BMS_SUPPORT_STAT_DEBUG_INFO     0  // ֧��ͳ�ƹ���

#define BMS_SUPPORT_DOG_WATCHER         1

#define BMS_SUPPORT_CAN_RE_INIT         0

#define BMS_SUPPORT_PCM_MODE            0

#define BMS_DATASAVE_SUPPORT_ENGLISH    0 //SD��ʹ��Ӣ�����ݴ洢

#define BMS_FORTH_ALARM_SUPPORT         0 //�ļ�����֧�֣�Ϊ0ʱ��Ϊ��������

#define BMU_CHR_OUTLET_TEMP_SUPPORT     0 //�ӻ�֧�ֳ������¶ȼ��

/* ****************Ӳ���汾�ź꿪��************************** */
#define BMS_SUPPORT_1_21                     1

#define BMS_SUPPORT_BC5XB_N                  0 //�Ƿ�֧��BC54BN�汾

#define BMS_SUPPORT_1_21_B                   0

#define BMS_SUPPORT_NEGATIVE_ADHESION_1_23   0//BMS_SUPPORT_NEGATIVE_ADHESION //�Ƿ�֧��1.23�ܸ�ճ�����

#define BMS_SUPPORT_NEGATIVE_ADHESION_1_23B  0

#define BMS_SUPPORT_BY5248D                  0 //���֧��1.1A�汾��Ҫ��BMS_SUPPORT_BY5248D_1_1A����Ϊ1


#if (BMS_SUPPORT_1_21 ||  BMS_SUPPORT_BC5XB_N) //֧��1.21��1.21NӲ���汾

   #define BMS_SUPPORT_NEGATIVE_ADHESION   0 //Ӳ���Ƿ�֧���ܸ�ճ������

   #define BMS_SUPPORT_CP_DOWN_POWER       0 //Ӳ���Ƿ�֧��CP�����µ繦��

   #define BMS_SUPPORT_CURRENT_DIVIDER     0  //Ӳ���Ƿ�֧�ַ�����(����ʹ��IIC1ͨ��)

   #define BMS_SUPPORT_CAN_WKUP            0  //Ӳ���Ƿ�֧��CAN����

   #define BMS_SUPPORT_MLOCK               1  //Ӳ���Ƿ�֧�ֵ�����
   
#endif

#if BMS_SUPPORT_1_21_B      //֧��1.21BӲ���汾       

   #define BMS_SUPPORT_NEGATIVE_ADHESION   0 //Ӳ���Ƿ�֧���ܸ�ճ������

   #define BMS_SUPPORT_CP_DOWN_POWER       1 //Ӳ���Ƿ�֧��CP�����µ繦��

   #define BMS_SUPPORT_CURRENT_DIVIDER     0  //Ӳ���Ƿ�֧�ַ�����(����ʹ��IIC1ͨ��)

   #define BMS_SUPPORT_CAN_WKUP            0  //Ӳ���Ƿ�֧��CAN����

   #define BMS_SUPPORT_MLOCK               1  //Ӳ���Ƿ�֧�ֵ�����
   
#endif

#if BMS_SUPPORT_NEGATIVE_ADHESION_1_23  //֧��1.23Ӳ���汾

   #define BMS_SUPPORT_NEGATIVE_ADHESION   1 //Ӳ���Ƿ�֧���ܸ�ճ������

   #define BMS_SUPPORT_CP_DOWN_POWER       0 //Ӳ���Ƿ�֧��CP�����µ繦��

   #define BMS_SUPPORT_CURRENT_DIVIDER     0  //Ӳ���Ƿ�֧�ַ�����(����ʹ��IIC1ͨ��)

   #define BMS_SUPPORT_CAN_WKUP            0  //Ӳ���Ƿ�֧��CAN����

   #define BMS_SUPPORT_MLOCK               1  //Ӳ���Ƿ�֧�ֵ�����
   
#endif

#if BMS_SUPPORT_NEGATIVE_ADHESION_1_23B  //֧��1.23BӲ���汾

   #define BMS_SUPPORT_NEGATIVE_ADHESION   1 //Ӳ���Ƿ�֧���ܸ�ճ������

   #define BMS_SUPPORT_CP_DOWN_POWER       1 //Ӳ���Ƿ�֧��CP�����µ繦��

   #define BMS_SUPPORT_CURRENT_DIVIDER     0  //Ӳ���Ƿ�֧�ַ�����(����ʹ��IIC1ͨ��)

   #define BMS_SUPPORT_CAN_WKUP            0  //Ӳ���Ƿ�֧��CAN����

   #define BMS_SUPPORT_MLOCK               1  //Ӳ���Ƿ�֧�ֵ�����
   
#endif

#if BMS_SUPPORT_BY5248D                 //֧�ָ߾���һ���Ӳ���汾

   #define BMS_SUPPORT_NEGATIVE_ADHESION   1 //Ӳ���Ƿ�֧���ܸ�ճ������

   #define BMS_SUPPORT_CP_DOWN_POWER       0 //Ӳ���Ƿ�֧��CP�����µ繦��

   #define BMS_SUPPORT_CURRENT_DIVIDER     1  //Ӳ���Ƿ�֧�ַ�����(����ʹ��IIC1ͨ��)

   #define BMS_SUPPORT_CAN_WKUP            1  //Ӳ���Ƿ�֧��CAN����

   #define BMS_SUPPORT_MLOCK               0  //Ӳ���Ƿ�֧�ֵ�����
   
   #define BMS_SUPPORT_BY5248D_1_1A        1//�߾���һ���Ӳ��֧��1.1A�汾 
    
#endif

#if   (BMS_SUPPORT_1_21+BMS_SUPPORT_BC5XB_N+BMS_SUPPORT_1_21_B+BMS_SUPPORT_NEGATIVE_ADHESION_1_23+BMS_SUPPORT_NEGATIVE_ADHESION_1_23B+BMS_SUPPORT_BY5248D)!=1
#error "����"
#endif



/*********************************************************************************
 *
 *                          �������ȼ�����
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
 *                           �����ջ��С����
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
#define SLAVE_RX_STK_SIZE                       260//210  ���CAN��FLASH���ܺ������Ӵ˶�ջ��С
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
 *                           �������ڣ���λms
 *
*********************************************************************************/
#define CURRENT_SAMPLE_MIN_PERIODIC       10          /*��С�����������ڣ� ָ�ɼ�һ���������С��� */
#define BAT_SCAN_PERIODIC                 50          /*���ɨ������BCP=BAT_SCAN_PERIODIC+ALERT_BEEP_ON_PERIODIC*/

#define CHECK_STATUS_PERIODIC             200
#define MAIN_JOB_CHECK_PERIODIC           20   /* ��job������� */
#define MAIN_JOB_COMMON_PERIODIC          100  /* һ��job��ʱ�� */

#define ChgLock_JOB_STATUS_PERIODIC       20  

#define CURRENT_AD_JOB_CHECK_PERIODIC     10  
#define BCU_UPDATE_JOB_PERIODIC           50   /* bcu jobʱ�� */
#define CHECK_SYSTE_STATE_PERIODIC        600
#define STAT_REFRESH_PERIDOIC             500
#define ALERT_JOB_PERIODIC                50
#define FEEDDOG_JOB_PERIODIC              200
#define DTU_SEND_DATA_JOB_PERIODIC       200
#define CAN_RE_INIT_JOB_PERIODIC          1000 /*CAN���³�ʼ���������*/
/*********************************************************************************
 *
 *                     ϵͳ����Ĭ��ֵ
 *
*********************************************************************************/
#define CURRENT_DIVIDER_TYPE           3             /* �����ɼ����� - ���������3*/

#define CURRENT_RES_DEF                1875          /* ������Ĭ����ֵ1875u��*/
#define CURRENT_RES_MIN                0
#define CURRENT_RES_MAX                65535

#define CAN_WKUP_ENABLE                1
#define CAN_WKUP_DISABLE               0

#define CAN_WKUP_DEF                   CAN_WKUP_DISABLE     /* CAN����Ĭ�ϲ�ʹ�� */
#define CAN_WKUP_MIN                   CAN_WKUP_DISABLE
#define CAN_WKUP_MAX                   CAN_WKUP_ENABLE

#define CLOCK_NOT_SYNC                 0
#define CLOCK_IS_SYNC                  1

#define CLOCK_SYNC_DEF                 CLOCK_NOT_SYNC
#define CLOCK_SYNC_MIN                 CLOCK_NOT_SYNC
#define CLOCK_SYNC_MAX                 CLOCK_IS_SYNC

#define CURRENT_CALIB_K_DEF  1000   /*�����궨����k��ʼ��ֵ��0.001�ֱ���*/ 
#define CURRENT_CALIB_K_MIN  800    /*�����궨����k��Сֵ��0.001�ֱ���  */ 
#define CURRENT_CALIB_K_MAX  1200   /*�����궨����k���ֵ��0.001�ֱ���  */ 

#define CURRENT_CALIB_B_DEF  0      /*�����궨����b��ʼ��ֵ��0.001�ֱ���*/ 
#define CURRENT_CALIB_B_MIN  -10000 /*�����궨����b��Сֵ��0.001�ֱ���  */ 
#define CURRENT_CALIB_B_MAX  10000  /*�����궨����b���ֵ��0.001�ֱ���  */      

#define MAX_ERROR_CURRENT               20              /*�����������������������*/
#define ISENSOR_ZERO_AD_VALUE           2048            /*�����������������ADֵ*/

#define ISENSOR_TYPE_DEF                kCurrent500Type /*��������������������ѡ��150��350��600����*/
#define ISENSOR_TYPE_MIN_DEF            kCurrentTypeStart /*����������������С����*/
#define ISENSOR_TYPE_MAX_DEF            kCurrentTypeStop/*���������������������*/

#define ISENSOR_RANGE_TYPE_DEF          kCurrentRangeTypeSingle /*�����������������������ͣ�0-�����̣�1-˫����*/
#define ISENSOR_RANGE_TYPE_MIN          kCurrentRangeTypeSingle
#define ISENSOR_RANGE_TYPE_MAX          CURRENT_DIVIDER_TYPE//kCurrentRangeTypeTwoSingle//kCurrentRangeTypeDouble

#define BalaceEnVolt                    50              //��������⿪��ѹ��
#define EEEPROM_FORMAT_FLAG_DEF         0               /*������ǿ��EEPROM��ʽ����־Ĭ��ֵ*/

#define TOTALCAP_DEF                    1000 // 0.1Ah/bit   #define TOTALCAP_DEF                    100             /*������Ĭ��ֵ 1Ah/bit*/
#define LEFTCAP_DEF                     1000 // 0.1Ah/bit   #define LEFTCAP_DEF                     100             /*ʣ������Ĭ��ֵ 1Ah/bit*/
#define CAP_MIN_DEF                        0                /*������Сֵ���� 1Ah/bit*/
#define MAXCAP_DEF                      (TOTALCAP_DEF + MAX_CAP_OFFSET) // 0.1Ah/bit /*��������������ֵ*/
#define NOMINAL_CAP_DEF                 1000 // 0.1Ah/bit #define NOMINAL_CAP_DEF                 100             /**������� */
#define NOMINAL_CAP_MAX_DEF             11910// 0.1Ah/bit #define NOMINAL_CAP_MAX_DEF             1191
#define CAP_MAX_DEF                     (NOMINAL_CAP_MAX_DEF + MAX_CAP_OFFSET)  // 0.1Ah/bit /*�������ֵ���� 1Ah/bit*/

#define SYS_DEFAULT_DLY_MIN_DEF            0                /*����ϵͳ��ʱ��Сֵ*/
#define SYS_DEFAULT_DLY_MAX_DEF            0xFFFF            /*����ϵͳ��ʱ���ֵ*/

#define SYS_DEFAULT_ALARM_DLY_MIN_DEF    1000            /*���屨���ͱ�����ʱ��Сֵ*/
#define SYS_DEFAULT_ALARM_DLY_MAX_DEF    0xFFFF            /*���屨���ͱ�����ʱ���ֵ*/

#define SYS_DEFAULT_VOLT_MIN_DEF        0               /*�����ѹ�����ͱ�����Сֵ*/
#define SYS_DEFAULT_VOLT_MAX_DEF        5000            /*�����ѹ�����ͱ������ֵ*/

#define SYS_DEFAULT_TEMP_MIN_DEF        0                /*�����¶ȱ����ͱ�����Сֵ��ƫ����Ϊ-50��*/
#define SYS_DEFAULT_TEMP_MAX_DEF        175                /*�����¶ȱ����ͱ������ֵ��ƫ����Ϊ-50��*/

#define SYS_DEFAULT_CUR_MIN_DEF         0               /*������������ͱ�����Сֵ*/
#define SYS_DEFAULT_CUR_MAX_DEF         0x7FFF          /*������������ͱ������ֵ*/

#define SYS_DEFAULT_DT_MIN_DEF            0                /*�����²����Сֵ*/
#define SYS_DEFAULT_DT_MAX_DEF            1000            /*�����²�����ֵ*/

#define SYS_DEFAULT_DV_MIN_DEF            5                /*����ѹ�����Сֵ   1mV/bit*/
#define SYS_DEFAULT_DV_MAX_DEF            5000            /*����ѹ������ֵ   1mV/bit*/

#define SYS_DEFAULT_SOC_MIN_DEF            PERCENT_TO_SOC(0)    /*SOC������Сֵ*/
#define SYS_DEFAULT_SOC_MAX_DEF            PERCENT_TO_SOC(150)    /*SOC�������ֵ*/

#define SYS_DEFAULT_TV_MIN_DEF            0                /*������ѹ������Сֵ*/
#define SYS_DEFAULT_TV_MAX_DEF            0xFFFF            /*������ѹ�������ֵ*/

#define SYS_DEFAULT_PINSU_MIN_DEF        0                /*��������©�籨���ͷ���Сֵ 1ohm/v/bit*/
#define SYS_DEFAULT_PINSU_MAX_DEF        5000            /*��������©�籨���ͷ����ֵ 1ohm/v/bit*/

#define SYS_CAN_CHANNEL_MIN_DEF         kCanDev0
#define SYS_CAN_CHANNEL_MAX_DEF         kCanDevMaxCount - 1

#define SYS_RS485_CHANNEL_MIN_DEF       kRs485Dev0
#define SYS_RS485_CHANNEL_MAX_DEF       kRs485Dev1

#define DCHG_HV_FST_ALARM_DEF           3600            /*����ŵ��ѹһ��������ֵ*/
#define DCHG_HV_FST_ALARM_REL_DEF       3400            /*����ŵ��ѹһ�������ͷŷ�ֵ*/
#define DCHG_HV_SND_ALARM_DEF           3700            /*����ŵ��ѹ����������ֵ*/
#define DCHG_HV_SND_ALARM_REL_DEF       3600            /*����ŵ��ѹ���������ͷŷ�ֵ*/
#define DCHG_HV_TRD_ALARM_DEF           3900            /*����ŵ��ѹ����������ֵ*/
#define DCHG_HV_TRD_ALARM_REL_DEF       3700            /*����ŵ��ѹ���������ͷŷ�ֵ*/

#define DCHG_HV_FST_ALARM_DLY_DEF       5000            /*����ŵ��ѹһ��������ʱ*/
#define DCHG_HV_FST_ALARM_REL_DLY_DEF   6000            /*����ŵ��ѹһ�������ͷ���ʱ*/
#define DCHG_HV_SND_ALARM_DLY_DEF       3000            /*����ŵ��ѹ����������ʱ*/
#define DCHG_HV_SND_ALARM_REL_DLY_DEF   4000            /*����ŵ��ѹ���������ͷ���ʱ*/
#define DCHG_HV_TRD_ALARM_DLY_DEF       3000            /*����ŵ��ѹ����������ʱ*/
#define DCHG_HV_TRD_ALARM_REL_DLY_DEF   4000            /*����ŵ��ѹ���������ͷ���ʱ*/

#define CHG_HV_FST_ALARM_DEF            3600            /*�������ѹһ��������ֵ*/
#define CHG_HV_FST_ALARM_REL_DEF        3400            /*�������ѹһ�������ͷŷ�ֵ*/
#define CHG_HV_SND_ALARM_DEF            3700            /*�������ѹ����������ֵ*/
#define CHG_HV_SND_ALARM_REL_DEF        3600            /*�������ѹ���������ͷŷ�ֵ*/
#define CHG_HV_TRD_ALARM_DEF            3900            /*�������ѹ����������ֵ*/
#define CHG_HV_TRD_ALARM_REL_DEF        3700            /*�������ѹ���������ͷŷ�ֵ*/

#define CHG_HV_FST_ALARM_DLY_DEF        5000            /*�������ѹһ��������ʱ*/
#define CHG_HV_FST_ALARM_REL_DLY_DEF    6000            /*�������ѹһ�������ͷ���ʱ*/
#define CHG_HV_SND_ALARM_DLY_DEF        3000            /*�������ѹ����������ʱ*/
#define CHG_HV_SND_ALARM_REL_DLY_DEF    4000            /*�������ѹ���������ͷ���ʱ*/
#define CHG_HV_TRD_ALARM_DLY_DEF        3000            /*�������ѹ����������ʱ*/
#define CHG_HV_TRD_ALARM_REL_DLY_DEF    4000            /*�������ѹ���������ͷ���ʱ*/

#define DCHG_LV_FST_ALARM_DEF           2900            /*����ŵ��ѹһ��������ֵ*/
#define DCHG_LV_FST_ALARM_REL_DEF       3000            /*����ŵ��ѹһ�������ͷŷ�ֵ*/
#define DCHG_LV_SND_ALARM_DEF           2600            /*����ŵ��ѹ����������ֵ*/
#define DCHG_LV_SND_ALARM_REL_DEF       2800            /*����ŵ��ѹ���������ͷŷ�ֵ*/
#define DCHG_LV_TRD_ALARM_DEF           2500            /*����ŵ��ѹ����������ֵ*/
#define DCHG_LV_TRD_ALARM_REL_DEF       2600            /*����ŵ��ѹ���������ͷŷ�ֵ*/

#define DCHG_LV_FST_ALARM_DLY_DEF       5000            /*����ŵ��ѹһ��������ʱ*/
#define DCHG_LV_FST_ALARM_REL_DLY_DEF   6000            /*����ŵ��ѹһ�������ͷ���ʱ*/
#define DCHG_LV_SND_ALARM_DLY_DEF       5000            /*����ŵ��ѹ����������ʱ*/
#define DCHG_LV_SND_ALARM_REL_DLY_DEF   6000            /*����ŵ��ѹ���������ͷ���ʱ*/
#define DCHG_LV_TRD_ALARM_DLY_DEF       5000            /*����ŵ��ѹ����������ʱ*/
#define DCHG_LV_TRD_ALARM_REL_DLY_DEF   6000            /*����ŵ��ѹ���������ͷ���ʱ*/

#define CHG_LV_FST_ALARM_DEF            2900            /*�������ѹһ��������ֵ*/
#define CHG_LV_FST_ALARM_REL_DEF        3000            /*�������ѹһ�������ͷŷ�ֵ*/
#define CHG_LV_SND_ALARM_DEF            2600            /*�������ѹ����������ֵ*/
#define CHG_LV_SND_ALARM_REL_DEF        2800            /*�������ѹ���������ͷŷ�ֵ*/
#define CHG_LV_TRD_ALARM_DEF            2500            /*�������ѹ����������ֵ*/
#define CHG_LV_TRD_ALARM_REL_DEF        2600            /*�������ѹ���������ͷŷ�ֵ*/

#define CHG_LV_FST_ALARM_DLY_DEF        5000            /*�������ѹһ��������ʱ*/
#define CHG_LV_FST_ALARM_REL_DLY_DEF    6000            /*�������ѹһ�������ͷ���ʱ*/
#define CHG_LV_SND_ALARM_DLY_DEF        5000            /*�������ѹ����������ʱ*/
#define CHG_LV_SND_ALARM_REL_DLY_DEF    6000            /*�������ѹ���������ͷ���ʱ*/
#define CHG_LV_TRD_ALARM_DLY_DEF        5000            /*�������ѹ����������ʱ*/
#define CHG_LV_TRD_ALARM_REL_DLY_DEF    6000            /*�������ѹ���������ͷ���ʱ*/

#define DCHG_OC_FST_ALARM_DEF           0x7FFF            /*����ŵ����һ��������ֵ 0.1A/bit*/
#define DCHG_OC_FST_ALARM_REL_DEF       0x7FFF            /*����ŵ����һ�������ͷŷ�ֵ 0.1A/bit*/
#define DCHG_OC_SND_ALARM_DEF           0x7FFF            /*����ŵ��������������ֵ 0.1A/bit*/
#define DCHG_OC_SND_ALARM_REL_DEF       0x7FFF            /*����ŵ�������������ͷŷ�ֵ 0.1A/bit*/
#define DCHG_OC_TRD_ALARM_DEF           0x7FFF            /*����ŵ��������������ֵ 0.1A/bit*/
#define DCHG_OC_TRD_ALARM_REL_DEF       0x7FFF            /*����ŵ�������������ͷŷ�ֵ 0.1A/bit*/

#define DCHG_OC_FST_ALARM_DLY_DEF       5000            /*����ŵ����һ��������ʱ*/
#define DCHG_OC_FST_ALARM_REL_DLY_DEF   6000            /*����ŵ����һ�������ͷ���ʱ*/
#define DCHG_OC_SND_ALARM_DLY_DEF       3000            /*����ŵ��������������ʱ*/
#define DCHG_OC_SND_ALARM_REL_DLY_DEF   4000            /*����ŵ�������������ͷ���ʱ*/
#define DCHG_OC_TRD_ALARM_DLY_DEF       3000            /*����ŵ��������������ʱ*/
#define DCHG_OC_TRD_ALARM_REL_DLY_DEF   4000            /*����ŵ�������������ͷ���ʱ*/

#define CHG_OC_FST_ALARM_DEF            0x7FFF            /*���������һ��������ֵ 0.1A/bit*/
#define CHG_OC_FST_ALARM_REL_DEF        0x7FFF            /*���������һ�������ͷŷ�ֵ 0.1A/bit*/
#define CHG_OC_SND_ALARM_DEF            0x7FFF            /*�������������������ֵ 0.1A/bit*/
#define CHG_OC_SND_ALARM_REL_DEF        0x7FFF            /*������������������ͷŷ�ֵ 0.1A/bit*/
#define CHG_OC_TRD_ALARM_DEF            0x7FFF            /*�������������������ֵ 0.1A/bit*/
#define CHG_OC_TRD_ALARM_REL_DEF        0x7FFF            /*������������������ͷŷ�ֵ 0.1A/bit*/

#define CHG_OC_FST_ALARM_DLY_DEF        5000            /*���������һ��������ʱ*/
#define CHG_OC_FST_ALARM_REL_DLY_DEF    6000            /*���������һ�������ͷ���ʱ*/
#define CHG_OC_SND_ALARM_DLY_DEF        3000            /*�������������������ʱ*/
#define CHG_OC_SND_ALARM_REL_DLY_DEF    4000            /*������������������ͷ���ʱ*/
#define CHG_OC_TRD_ALARM_DLY_DEF        3000            /*�������������������ʱ*/
#define CHG_OC_TRD_ALARM_REL_DLY_DEF    4000            /*������������������ͷ���ʱ*/

#define DCHG_HT_FST_ALARM_DEF           100             /*����ŵ����һ��������ֵ��ƫ����Ϊ-50��*/
#define DCHG_HT_FST_ALARM_REL_DEF       90              /*����ŵ����һ�������ͷŷ�ֵ��ƫ����Ϊ-50��*/
#define DCHG_HT_SND_ALARM_DEF           110             /*����ŵ���¶���������ֵ��ƫ����Ϊ-50��*/
#define DCHG_HT_SND_ALARM_REL_DEF       100             /*����ŵ���¶��������ͷŷ�ֵ��ƫ����Ϊ-50��*/
#define DCHG_HT_TRD_ALARM_DEF           115             /*����ŵ��������������ֵ��ƫ����Ϊ-50��*/
#define DCHG_HT_TRD_ALARM_REL_DEF       110             /*����ŵ�������������ͷŷ�ֵ��ƫ����Ϊ-50��*/

#define DCHG_HT_FST_ALARM_DLY_DEF       5000            /*����ŵ����һ��������ʱ*/
#define DCHG_HT_FST_ALARM_REL_DLY_DEF   6000            /*����ŵ����һ�������ͷ���ʱ*/
#define DCHG_HT_SND_ALARM_DLY_DEF       3000            /*����ŵ���¶���������ʱ*/
#define DCHG_HT_SND_ALARM_REL_DLY_DEF   4000            /*����ŵ���¶��������ͷ���ʱ*/
#define DCHG_HT_TRD_ALARM_DLY_DEF       1000            /*����ŵ��������������ʱ*/
#define DCHG_HT_TRD_ALARM_REL_DLY_DEF   2000            /*����ŵ�������������ͷ���ʱ*/

#define CHG_HT_FST_ALARM_DEF            95              /*���������һ��������ֵ��ƫ����Ϊ-50��*/
#define CHG_HT_FST_ALARM_REL_DEF        90              /*���������һ�������ͷŷ�ֵ��ƫ����Ϊ-50��*/
#define CHG_HT_SND_ALARM_DEF            105             /*��������¶���������ֵ��ƫ����Ϊ-50��*/
#define CHG_HT_SND_ALARM_REL_DEF        100             /*��������¶��������ͷŷ�ֵ��ƫ����Ϊ-50��*/
#define CHG_HT_TRD_ALARM_DEF            115             /*�������������������ֵ��ƫ����Ϊ-50��*/
#define CHG_HT_TRD_ALARM_REL_DEF        110             /*������������������ͷŷ�ֵ��ƫ����Ϊ-50��*/

#define CHG_HT_FST_ALARM_DLY_DEF        5000            /*���������һ��������ʱ*/
#define CHG_HT_FST_ALARM_REL_DLY_DEF    6000            /*���������һ�������ͷ���ʱ*/
#define CHG_HT_SND_ALARM_DLY_DEF        3000            /*��������¶���������ʱ*/
#define CHG_HT_SND_ALARM_REL_DLY_DEF    4000            /*��������¶��������ͷ���ʱ*/
#define CHG_HT_TRD_ALARM_DLY_DEF        1000            /*�������������������ʱ*/
#define CHG_HT_TRD_ALARM_REL_DLY_DEF    2000            /*������������������ͷ���ʱ*/

#define DCHG_LT_FST_ALARM_DEF           50              /*����ŵ����һ��������ֵ��ƫ����Ϊ-50��*/
#define DCHG_LT_FST_ALARM_REL_DEF       55              /*����ŵ����һ�������ͷŷ�ֵ��ƫ����Ϊ-50��*/
#define DCHG_LT_SND_ALARM_DEF           40              /*����ŵ���¶���������ֵ��ƫ����Ϊ-50��*/
#define DCHG_LT_SND_ALARM_REL_DEF       50              /*����ŵ���¶��������ͷŷ�ֵ��ƫ����Ϊ-50��*/
#define DCHG_LT_TRD_ALARM_DEF           30              /*����ŵ��������������ֵ��ƫ����Ϊ-50��*/
#define DCHG_LT_TRD_ALARM_REL_DEF       45              /*����ŵ�������������ͷŷ�ֵ��ƫ����Ϊ-50��*/

#define DCHG_LT_FST_ALARM_DLY_DEF       5000            /*����ŵ����һ��������ʱ*/
#define DCHG_LT_FST_ALARM_REL_DLY_DEF   6000            /*����ŵ����һ�������ͷ���ʱ*/
#define DCHG_LT_SND_ALARM_DLY_DEF       5000            /*����ŵ���¶���������ʱ*/
#define DCHG_LT_SND_ALARM_REL_DLY_DEF   6000            /*����ŵ���¶��������ͷ���ʱ*/
#define DCHG_LT_TRD_ALARM_DLY_DEF       5000            /*����ŵ��������������ʱ*/
#define DCHG_LT_TRD_ALARM_REL_DLY_DEF   6000            /*����ŵ�������������ͷ���ʱ*/

#define CHG_LT_FST_ALARM_DEF            50              /*���������һ��������ֵ��ƫ����Ϊ-50��*/
#define CHG_LT_FST_ALARM_REL_DEF        55              /*���������һ�������ͷŷ�ֵ��ƫ����Ϊ-50��*/
#define CHG_LT_SND_ALARM_DEF            45              /*��������¶���������ֵ��ƫ����Ϊ-50��*/
#define CHG_LT_SND_ALARM_REL_DEF        50              /*��������¶��������ͷŷ�ֵ��ƫ����Ϊ-50��*/
#define CHG_LT_TRD_ALARM_DEF            35              /*�������������������ֵ��ƫ����Ϊ-50��*/
#define CHG_LT_TRD_ALARM_REL_DEF        45              /*������������������ͷŷ�ֵ��ƫ����Ϊ-50��*/

#define CHG_LT_FST_ALARM_DLY_DEF        5000            /*���������һ��������ʱ*/
#define CHG_LT_FST_ALARM_REL_DLY_DEF    6000            /*���������һ�������ͷ���ʱ*/
#define CHG_LT_SND_ALARM_DLY_DEF        5000            /*��������¶���������ʱ*/
#define CHG_LT_SND_ALARM_REL_DLY_DEF    6000            /*��������¶��������ͷ���ʱ*/
#define CHG_LT_TRD_ALARM_DLY_DEF        5000            /*�������������������ʱ*/
#define CHG_LT_TRD_ALARM_REL_DLY_DEF    6000            /*������������������ͷ���ʱ*/

#define DCHG_DT_FST_ALARM_DEF           10              /*����ŵ��²�һ��������ֵ*/
#define DCHG_DT_FST_ALARM_REL_DEF       5               /*����ŵ��²�һ�������ͷŷ�ֵ*/
#define DCHG_DT_SND_ALARM_DEF           15              /*����ŵ��²����������ֵ*/
#define DCHG_DT_SND_ALARM_REL_DEF       12              /*����ŵ��²���������ͷŷ�ֵ*/
#define DCHG_DT_TRD_ALARM_DEF           20              /*����ŵ��²�����������ֵ*/
#define DCHG_DT_TRD_ALARM_REL_DEF       15              /*����ŵ��²����������ͷŷ�ֵ*/

#define DCHG_DT_FST_ALARM_DLY_DEF       5000            /*����ŵ��²�һ��������ʱ*/
#define DCHG_DT_FST_ALARM_REL_DLY_DEF   6000            /*����ŵ��²�һ�������ͷ���ʱ*/
#define DCHG_DT_SND_ALARM_DLY_DEF       5000            /*����ŵ��²����������ʱ*/
#define DCHG_DT_SND_ALARM_REL_DLY_DEF   6000            /*����ŵ��²���������ͷ���ʱ*/
#define DCHG_DT_TRD_ALARM_DLY_DEF       5000            /*����ŵ��²�����������ʱ*/
#define DCHG_DT_TRD_ALARM_REL_DLY_DEF   6000            /*����ŵ��²����������ͷ���ʱ*/

#define CHG_DT_FST_ALARM_DEF            10              /*�������²�һ��������ֵ*/
#define CHG_DT_FST_ALARM_REL_DEF        5               /*�������²�һ�������ͷŷ�ֵ*/
#define CHG_DT_SND_ALARM_DEF            15              /*�������²����������ֵ*/
#define CHG_DT_SND_ALARM_REL_DEF        12              /*�������²���������ͷŷ�ֵ*/
#define CHG_DT_TRD_ALARM_DEF            20              /*�������²�����������ֵ*/
#define CHG_DT_TRD_ALARM_REL_DEF        15              /*�������²����������ͷŷ�ֵ*/

#define CHG_DT_FST_ALARM_DLY_DEF        5000            /*�������²�һ��������ʱ*/
#define CHG_DT_FST_ALARM_REL_DLY_DEF    6000            /*�������²�һ�������ͷ���ʱ*/
#define CHG_DT_SND_ALARM_DLY_DEF        5000            /*�������²����������ʱ*/
#define CHG_DT_SND_ALARM_REL_DLY_DEF    6000            /*�������²���������ͷ���ʱ*/
#define CHG_DT_TRD_ALARM_DLY_DEF        5000            /*�������²�����������ʱ*/
#define CHG_DT_TRD_ALARM_REL_DLY_DEF    6000            /*�������²����������ͷ���ʱ*/

#define COMM_FST_ALARM_DLY_DEF          15              /*ͨ���ж���ʱʱ��Ĭ��ֵ,��λ��S */
#define COMM_FST_ALARM_REL_DLY_DEF        5                /*ͨ���ж��ͷ���ʱʱ��Ĭ��ֵ*/
#define COMM_SND_ALARM_DLY_DEF          25              /*ͨ���ж϶���������ʱʱ��Ĭ��ֵ,��λ��S */
#define COMM_SND_ALARM_REL_DLY_DEF        5                /*ͨ���ж϶��������ͷ���ʱʱ��Ĭ��ֵ*/
#define COMM_TRD_ALARM_DLY_DEF          35              /*ͨ���ж�����������ʱʱ��Ĭ��ֵ,��λ��S */
#define COMM_TRD_ALARM_REL_DLY_DEF        5                /*ͨ���ж����������ͷ���ʱʱ��Ĭ��ֵ*/
#define COMM_ABORT_ALARM_DLY_MIN        5               /*ͨ���ж���ʱʱ����Сֵ,��λ��S */
#define COMM_ABORT_ALARM_DLY_MAX        65              /*ͨ���ж���ʱʱ�����ֵ,��λ��S */
#define COMM_ABORT_ALARM_REL_DLY_MIN    1               /*ͨ���ж��ͷ���ʱʱ����Сֵ,��λ��S*/
#define COMM_ABORT_ALARM_REL_DLY_MAX    65              /*ͨ���ж��ͷ���ʱʱ�����ֵ,��λ��S*/

#define TEMP_LINE_FST_ALARM_DLY_DEF        10000            /*�����¸�������һ��������ʱĬ��ֵ*/
#define TEMP_LINE_FST_ALARM_REL_DLY_DEF    3000            /*�����¸�����һ�������ͷ���ʱĬ��ֵ*/
#define TEMP_LINE_SND_ALARM_DLY_DEF        20000            /*�����¸����������������ʱĬ��ֵ*/
#define TEMP_LINE_SND_ALARM_REL_DLY_DEF    3000            /*�����¸�������������ͷ���ʱĬ��ֵ*/
#define TEMP_LINE_TRD_ALARM_DLY_DEF        30000            /*�����¸�����������������ʱĬ��ֵ*/
#define TEMP_LINE_TRD_ALARM_REL_DLY_DEF    3000            /*�����¸��������������ͷ���ʱĬ��ֵ*/

#define VOLT_LINE_FST_ALARM_DLY_DEF        10000            /*�����ѹ�����������һ��������ʱĬ��ֵ*/
#define VOLT_LINE_FST_ALARM_REL_DLY_DEF    3000            /*�����ѹ�����������һ�������ͷ���ʱĬ��ֵ*/
#define VOLT_LINE_SND_ALARM_DLY_DEF        20000            /*�����ѹ��������������������ʱĬ��ֵ*/
#define VOLT_LINE_SND_ALARM_REL_DLY_DEF    3000            /*�����ѹ�������������������ͷ���ʱĬ��ֵ*/
#define VOLT_LINE_TRD_ALARM_DLY_DEF        30000            /*�����ѹ���������������������ʱĬ��ֵ*/
#define VOLT_LINE_TRD_ALARM_REL_DLY_DEF    3000            /*�����ѹ��������������������ͷ���ʱĬ��ֵ*/

#define DCHG_DV_FST_ALARM_DEF           500             /*����ŵ�ѹ��һ��������ֵ*/
#define DCHG_DV_FST_ALARM_REL_DEF       300             /*����ŵ�ѹ��һ�������ͷŷ�ֵ*/
#define DCHG_DV_SND_ALARM_DEF           800             /*����ŵ�ѹ�����������ֵ*/
#define DCHG_DV_SND_ALARM_REL_DEF       600             /*����ŵ�ѹ����������ͷŷ�ֵ*/
#define DCHG_DV_TRD_ALARM_DEF           1000            /*����ŵ�ѹ������������ֵ*/
#define DCHG_DV_TRD_ALARM_REL_DEF       700             /*����ŵ�ѹ�����������ͷŷ�ֵ*/

#define DCHG_DV_FST_ALARM_DLY_DEF       5000            /*����ŵ�ѹ��һ��������ʱ*/
#define DCHG_DV_FST_ALARM_REL_DLY_DEF   6000            /*����ŵ�ѹ��һ�������ͷ���ʱ*/
#define DCHG_DV_SND_ALARM_DLY_DEF       5000            /*����ŵ�ѹ�����������ʱ*/
#define DCHG_DV_SND_ALARM_REL_DLY_DEF   6000            /*����ŵ�ѹ����������ͷ���ʱ*/
#define DCHG_DV_TRD_ALARM_DLY_DEF       5000            /*����ŵ�ѹ������������ʱ*/
#define DCHG_DV_TRD_ALARM_REL_DLY_DEF   6000            /*����ŵ�ѹ�����������ͷ���ʱ*/

#define CHG_DV_FST_ALARM_DEF            300             /*������ѹ��һ��������ֵ*/
#define CHG_DV_FST_ALARM_REL_DEF        200             /*������ѹ��һ�������ͷŷ�ֵ*/
#define CHG_DV_SND_ALARM_DEF            600             /*������ѹ�����������ֵ*/
#define CHG_DV_SND_ALARM_REL_DEF        500             /*������ѹ����������ͷŷ�ֵ*/
#define CHG_DV_TRD_ALARM_DEF            900             /*������ѹ������������ֵ*/
#define CHG_DV_TRD_ALARM_REL_DEF        800             /*������ѹ�����������ͷŷ�ֵ*/

#define CHG_DV_FST_ALARM_DLY_DEF        5000            /*������ѹ��һ��������ʱ*/
#define CHG_DV_FST_ALARM_REL_DLY_DEF    6000            /*������ѹ��һ�������ͷ���ʱ*/
#define CHG_DV_SND_ALARM_DLY_DEF        5000            /*������ѹ�����������ʱ*/
#define CHG_DV_SND_ALARM_REL_DLY_DEF    6000            /*������ѹ����������ͷ���ʱ*/
#define CHG_DV_TRD_ALARM_DLY_DEF        5000            /*������ѹ������������ʱ*/
#define CHG_DV_TRD_ALARM_REL_DLY_DEF    6000            /*������ѹ�����������ͷ���ʱ*/

//TODO
#define TEMP_RISE_FAST_DEF                0xFF            /*���������챨��Ĭ��ֵ*/
#define TEMP_RISE_FAST_MIN_DEF            0                /*���������챨����Сֵ*/
#define TEMP_RISE_FAST_MAX_DEF            0xFF            /*���������챨�����ֵ*/

#define TEMP_RISE_FAST_FREE_DEF            0xFF            /*���������챨���ͷ�Ĭ��ֵ*/
#define TEMP_RISE_FAST_FREE_MIN_DEF        0                /*���������챨���ͷ���Сֵ*/
#define TEMP_RISE_FAST_FREE_MAX_DEF        0xFF            /*���������챨���ͷ����ֵ*/

#define TEMP_RISE_OVER_DEF                0xFF            /*�����������챨���ͷ�Ĭ��ֵ*/
#define TEMP_RISE_OVER_MIN_DEF            0                /*�����������챨����Сֵ*/
#define TEMP_RISE_OVER_MAX_DEF            0xFF            /*�����������챨�����ֵ*/

#define TEMP_RISE_OVER_FREE_DEF            0xFF            /*�����������챨���ͷ�Ĭ��ֵ*/
#define TEMP_RISE_OVER_FREE_MIN_DEF        0                /*�����������챨���ͷ���Сֵ*/
#define TEMP_RISE_OVER_FREE_MAX_DEF        0xFF            /*�����������챨���ͷ����ֵ*/

#define TEMP_RISE_FAST_TIME_DEF            5000            /*���������챨�����ʱ�� ��λ��1ms/bit*/
#define TEMP_RISE_FAST_FREE_TIME_DEF    6000            /*���������챨���ͷż��ʱ�� ��λ��1ms/bit*/
#define TEMP_RISE_OVER_TIME_DEF            5000            /*�����������챨�����ʱ�� ��λ��1ms/bit*/
#define TEMP_RISE_OVER_FREE_TIME_DEF    6000            /*�����������챨���ͷż��ʱ�� ��λ��1ms/bit*/

#define TEMP_FALL_FAST_DEF                0xFF            /*�����½��챨��Ĭ��ֵ*/
#define TEMP_FALL_FAST_MIN_DEF            0                /*�����½��챨����Сֵ*/
#define TEMP_FALL_FAST_MAX_DEF            0xFF            /*�����½��챨�����ֵ*/

#define TEMP_FALL_FAST_FREE_DEF            0xFF            /*�����½��챨���ͷ�Ĭ��ֵ*/
#define TEMP_FALL_FAST_FREE_MIN_DEF        0                /*�����½��챨���ͷ���Сֵ*/
#define TEMP_FALL_FAST_FREE_MAX_DEF        0xFF            /*�����½��챨���ͷ����ֵ*/

#define TEMP_FALL_OVER_DEF                0xFF            /*�����½����챨���ͷ�Ĭ��ֵ*/
#define TEMP_FALL_OVER_MIN_DEF            0                /*�����½����챨����Сֵ*/
#define TEMP_FALL_OVER_MAX_DEF            0xFF            /*�����½����챨�����ֵ*/

#define TEMP_FALL_OVER_FREE_DEF            0xFF            /*�����½����챨���ͷ�Ĭ��ֵ*/
#define TEMP_FALL_OVER_FREE_MIN_DEF        0                /*�����½����챨���ͷ���Сֵ*/
#define TEMP_FALL_OVER_FREE_MAX_DEF        0xFF            /*�����½����챨���ͷ����ֵ*/

#define TEMP_FALL_FAST_TIME_DEF            5000            /*�����½��챨�����ʱ�� ��λ��1ms/bit*/
#define TEMP_FALL_FAST_FREE_TIME_DEF    6000            /*�����½��챨���ͷż��ʱ�� ��λ��1ms/bit*/
#define TEMP_FALL_OVER_TIME_DEF            5000            /*�����½����챨�����ʱ�� ��λ��1ms/bit*/
#define TEMP_FALL_OVER_FREE_TIME_DEF    6000            /*�����½����챨���ͷż��ʱ�� ��λ��1ms/bit*/

#define SOC_LOW_FST_ALARM_DEF            PERCENT_TO_SOC(20)    /*SOC��һ��������ֵĬ��ֵ*/
#define SOC_LOW_FST_ALARM_REL_DEF        PERCENT_TO_SOC(25)  /*SOC��һ�������ͷ���ֵĬ��ֵ*/
#define SOC_LOW_SND_ALARM_DEF            PERCENT_TO_SOC(10)  /*SOC�Ͷ���������ֵĬ��ֵ*/
#define SOC_LOW_SND_ALARM_REL_DEF        PERCENT_TO_SOC(15)  /*SOC�Ͷ��������ͷ���ֵĬ��ֵ*/
#define SOC_LOW_TRD_ALARM_DEF            PERCENT_TO_SOC(5)   /*SOC������������ֵĬ��ֵ*/
#define SOC_LOW_TRD_ALARM_REL_DEF        PERCENT_TO_SOC(10)  /*SOC�����������ͷ���ֵĬ��ֵ*/

#define SOC_LOW_FST_ALARM_DLY_DEF        5000            /*SOC��һ��������ʱĬ��ֵ*/
#define SOC_LOW_FST_ALARM_REL_DLY_DEF    6000            /*SOC��һ�������ͷ���ʱĬ��ֵ*/
#define SOC_LOW_SND_ALARM_DLY_DEF        5000            /*SOC�Ͷ���������ʱĬ��ֵ*/
#define SOC_LOW_SND_ALARM_REL_DLY_DEF    6000            /*SOC�Ͷ��������ͷ���ʱĬ��ֵ*/
#define SOC_LOW_TRD_ALARM_DLY_DEF        5000            /*SOC������������ʱĬ��ֵ*/
#define SOC_LOW_TRD_ALARM_REL_DLY_DEF    6000            /*SOC�����������ͷ���ʱĬ��ֵ*/

#define SOC_HIGH_FST_ALARM_DEF            PERCENT_TO_SOC(101) /*SOC��һ��������ֵĬ��ֵ*/
#define SOC_HIGH_FST_ALARM_REL_DEF        PERCENT_TO_SOC(100)    /*SOC��һ�������ͷ���ֵĬ��ֵ*/
#define SOC_HIGH_SND_ALARM_DEF            PERCENT_TO_SOC(102)    /*SOC�߶���������ֵĬ��ֵ*/
#define SOC_HIGH_SND_ALARM_REL_DEF        PERCENT_TO_SOC(101)    /*SOC�߶��������ͷ���ֵĬ��ֵ*/
#define SOC_HIGH_TRD_ALARM_DEF            PERCENT_TO_SOC(105) /*SOC������������ֵĬ��ֵ*/
#define SOC_HIGH_TRD_ALARM_REL_DEF        PERCENT_TO_SOC(102) /*SOC�����������ͷ���ֵĬ��ֵ*/

#define SOC_HIGH_FST_ALARM_DLY_DEF        5000            /*SOC��һ��������ʱĬ��ֵ*/
#define SOC_HIGH_FST_ALARM_REL_DLY_DEF    6000            /*SOC��һ�������ͷ���ʱĬ��ֵ*/
#define SOC_HIGH_SND_ALARM_DLY_DEF        5000            /*SOC�߶���������ʱĬ��ֵ*/
#define SOC_HIGH_SND_ALARM_REL_DLY_DEF    6000            /*SOC�߶��������ͷ���ʱĬ��ֵ*/
#define SOC_HIGH_TRD_ALARM_DLY_DEF        5000            /*SOC������������ʱĬ��ֵ*/
#define SOC_HIGH_TRD_ALARM_REL_DLY_DEF    6000            /*SOC�����������ͷ���ʱĬ��ֵ*/

#define DCHG_HTV_FST_ALARM_DEF          3700            /*����ŵ���ѹ��һ��������ֵ*/
#define DCHG_HTV_FST_ALARM_REL_DEF      3600            /*����ŵ���ѹ��һ�������ͷŷ�ֵ*/
#define DCHG_HTV_SND_ALARM_DEF          3750            /*����ŵ���ѹ�߶���������ֵ*/
#define DCHG_HTV_SND_ALARM_REL_DEF      3700            /*����ŵ���ѹ�߶��������ͷŷ�ֵ*/
#define DCHG_HTV_TRD_ALARM_DEF          3800            /*����ŵ���ѹ������������ֵ*/
#define DCHG_HTV_TRD_ALARM_REL_DEF      3750            /*����ŵ���ѹ�����������ͷŷ�ֵ*/

#define DCHG_HTV_FST_ALARM_DLY_DEF      5000            /*����ŵ���ѹ��һ��������ʱ*/
#define DCHG_HTV_FST_ALARM_REL_DLY_DEF  6000            /*����ŵ���ѹ��һ�������ͷ���ʱ*/
#define DCHG_HTV_SND_ALARM_DLY_DEF      5000            /*����ŵ���ѹ�߶���������ʱ*/
#define DCHG_HTV_SND_ALARM_REL_DLY_DEF  6000            /*����ŵ���ѹ�߶��������ͷ���ʱ*/
#define DCHG_HTV_TRD_ALARM_DLY_DEF      5000            /*����ŵ���ѹ������������ʱ*/
#define DCHG_HTV_TRD_ALARM_REL_DLY_DEF  6000            /*����ŵ���ѹ�����������ͷ���ʱ*/

#define CHG_HTV_FST_ALARM_DEF           3700            /*��������ѹ��һ��������ֵ*/
#define CHG_HTV_FST_ALARM_REL_DEF       3600            /*��������ѹ��һ�������ͷŷ�ֵ*/
#define CHG_HTV_SND_ALARM_DEF           3750            /*��������ѹ�߶���������ֵ*/
#define CHG_HTV_SND_ALARM_REL_DEF       3700            /*��������ѹ�߶��������ͷŷ�ֵ*/
#define CHG_HTV_TRD_ALARM_DEF           3800            /*��������ѹ������������ֵ*/
#define CHG_HTV_TRD_ALARM_REL_DEF       3750            /*��������ѹ�����������ͷŷ�ֵ*/

#define CHG_HTV_FST_ALARM_DLY_DEF       5000            /*��������ѹ��һ��������ʱ*/
#define CHG_HTV_FST_ALARM_REL_DLY_DEF   6000            /*��������ѹ��һ�������ͷ���ʱ*/
#define CHG_HTV_SND_ALARM_DLY_DEF       5000            /*��������ѹ�߶���������ʱ*/
#define CHG_HTV_SND_ALARM_REL_DLY_DEF   6000            /*��������ѹ�߶��������ͷ���ʱ*/
#define CHG_HTV_TRD_ALARM_DLY_DEF       5000            /*��������ѹ������������ʱ*/
#define CHG_HTV_TRD_ALARM_REL_DLY_DEF   6000            /*��������ѹ�����������ͷ���ʱ*/

#define DCHG_LTV_FST_ALARM_DEF          2800            /*����ŵ���ѹ��һ��������ֵ*/
#define DCHG_LTV_FST_ALARM_REL_DEF      2900            /*����ŵ���ѹ��һ�������ͷŷ�ֵ*/
#define DCHG_LTV_SND_ALARM_DEF          2700            /*����ŵ���ѹ�Ͷ���������ֵ*/
#define DCHG_LTV_SND_ALARM_REL_DEF      2800            /*����ŵ���ѹ�Ͷ��������ͷŷ�ֵ*/
#define DCHG_LTV_TRD_ALARM_DEF          2500            /*����ŵ���ѹ������������ֵ*/
#define DCHG_LTV_TRD_ALARM_REL_DEF      2600            /*����ŵ���ѹ�����������ͷŷ�ֵ*/

#define DCHG_LTV_FST_ALARM_DLY_DEF      5000            /*����ŵ���ѹ��һ��������ʱ*/
#define DCHG_LTV_FST_ALARM_REL_DLY_DEF  6000            /*����ŵ���ѹ��һ�������ͷ���ʱ*/
#define DCHG_LTV_SND_ALARM_DLY_DEF      5000            /*����ŵ���ѹ�Ͷ���������ʱ*/
#define DCHG_LTV_SND_ALARM_REL_DLY_DEF  6000            /*����ŵ���ѹ�Ͷ��������ͷ���ʱ*/
#define DCHG_LTV_TRD_ALARM_DLY_DEF      5000            /*����ŵ���ѹ������������ʱ*/
#define DCHG_LTV_TRD_ALARM_REL_DLY_DEF  6000            /*����ŵ���ѹ�����������ͷ���ʱ*/

#define CHG_LTV_FST_ALARM_DEF           3000               /*��������ѹ��һ��������ֵ*/
#define CHG_LTV_FST_ALARM_REL_DEF       3100               /*��������ѹ��һ�������ͷŷ�ֵ*/
#define CHG_LTV_SND_ALARM_DEF           2800               /*��������ѹ�Ͷ���������ֵ*/
#define CHG_LTV_SND_ALARM_REL_DEF       3000               /*��������ѹ�Ͷ��������ͷŷ�ֵ*/
#define CHG_LTV_TRD_ALARM_DEF           2500               /*��������ѹ������������ֵ*/
#define CHG_LTV_TRD_ALARM_REL_DEF       2600               /*��������ѹ�����������ͷŷ�ֵ*/

#define CHG_LTV_FST_ALARM_DLY_DEF       5000            /*��������ѹ��һ��������ʱ*/
#define CHG_LTV_FST_ALARM_REL_DLY_DEF   6000            /*��������ѹ��һ�������ͷ���ʱ*/
#define CHG_LTV_SND_ALARM_DLY_DEF       5000            /*��������ѹ�Ͷ���������ʱ*/
#define CHG_LTV_SND_ALARM_REL_DLY_DEF   6000            /*��������ѹ�Ͷ��������ͷ���ʱ*/
#define CHG_LTV_TRD_ALARM_DLY_DEF       5000            /*��������ѹ������������ʱ*/
#define CHG_LTV_TRD_ALARM_REL_DLY_DEF   6000            /*��������ѹ�����������ͷ���ʱ*/

#define CHG_OUTLET_HT_FST_ALARM_DEF     TEMPERATURE_FROM_C(60)            /*����������¶ȸ�һ��������ֵ*/
#define CHG_OUTLET_HT_FST_ALARM_REL_DEF TEMPERATURE_FROM_C(50)            /*����������¶ȸ�һ�������ͷŷ�ֵ*/
#define CHG_OUTLET_HT_SND_ALARM_DEF     TEMPERATURE_FROM_C(90)            /*����������¶ȸ߶���������ֵ*/
#define CHG_OUTLET_HT_SND_ALARM_REL_DEF TEMPERATURE_FROM_C(80)            /*����������¶ȸ߶��������ͷŷ�ֵ*/
#define CHG_OUTLET_HT_TRD_ALARM_DEF     TEMPERATURE_FROM_C(120)            /*����������¶ȸ�����������ֵ*/
#define CHG_OUTLET_HT_TRD_ALARM_REL_DEF TEMPERATURE_FROM_C(110)            /*����������¶ȸ����������ͷŷ�ֵ*/

#define CHG_OUTLET_ALARM_MAX_DEF     TEMPERATURE_FROM_C(190)  /////////////////////////////////////



#define BMS_CHGR_AC_OUTLET_NUM_MAX      2
#define BMS_CHGR_AC_OUTLET_NUM_MIN      0
#define BMS_CHGR_AC_OUTLET_NUM_DEF      0
#define BMS_CHGR_DC_OUTLET_NUM_MAX      2
#define BMS_CHGR_DC_OUTLET_NUM_MIN      0
#define BMS_CHGR_DC_OUTLET_NUM_DEF      0

#define CHG_OUTLET_HT_FST_ALARM_DLY_DEF        5000            /*����������¶ȸ�һ��������ʱ*/
#define CHG_OUTLET_HT_FST_ALARM_REL_DLY_DEF    6000            /*����������¶ȸ�һ�������ͷ���ʱ*/
#define CHG_OUTLET_HT_SND_ALARM_DLY_DEF        5000            /*����������¶ȸ߶���������ʱ*/
#define CHG_OUTLET_HT_SND_ALARM_REL_DLY_DEF    6000            /*����������¶ȸ߶��������ͷ���ʱ*/
#define CHG_OUTLET_HT_TRD_ALARM_DLY_DEF        5000            /*����������¶ȸ�����������ʱ*/
#define CHG_OUTLET_HT_TRD_ALARM_REL_DLY_DEF    6000            /*����������¶ȸ����������ͷ���ʱ*/



#define PINSU_FST_ALARM_DEF                500                /*������΢©�籨����ֵĬ��ֵ 1ohm/v/bit*/
#define PINSU_FST_ALARM_REL_DEF            550                /*������΢©�籨���ͷ���ֵĬ��ֵ 1ohm/v/bit*/
#define PINSU_SND_ALARM_DEF                200                /*��������©����ֵĬ��ֵ 1ohm/v/bit*/
#define PINSU_SND_ALARM_REL_DEF            250                /*��������©���ͷ���ֵĬ��ֵ 1ohm/v/bit*/
#define PINSU_TRD_ALARM_DEF             100             /*�������©�籨��Ĭ��ֵ 1ohm/v/bit*/
#define PINSU_TRD_ALARM_REL_DEF         150             /*�������©�籨���ͷ�Ĭ��ֵ 1ohm/v/bit*/

#define PINSU_FST_ALARM_DLY_DEF            5000            /*������΢©�籨����ʱĬ��ֵ*/
#define PINSU_FST_ALARM_REL_DLY_DEF        6000            /*������΢©�籨���ͷ���ʱĬ��ֵ*/
#define PINSU_SND_ALARM_DLY_DEF            1000            /*��������©����ʱĬ��ֵ*/
#define PINSU_SND_ALARM_REL_DLY_DEF        2000            /*��������©���ͷ���ʱĬ��ֵ*/
#define PINSU_TRD_ALARM_DLY_DEF            1000            /*�������©����ʱĬ��ֵ*/
#define PINSU_TRD_ALARM_REL_DLY_DEF        2000            /*�������©���ͷ���ʱĬ��ֵ*/

#define CHG_RELAY_ON_DLY_DEF               1000               /*���̵����պ�Ĭ����ʱʱ��*/
#define CHG_RELAY_OFF_DLY_DEF            5000            /*���̵����Ͽ�Ĭ����ʱʱ��*/
#define DISCHG_RELAY_ON_DLY_DEF            0                /*�ŵ�̵����պ�Ĭ����ʱʱ�� */
#define DISCHG_RELAY_OFF_DLY_DEF        5000            /*�ŵ�̵����Ͽ�Ĭ����ʱʱ�� */
#define PRECHG_RELAY_ON_DLY_DEF            0                /*Ԥ��̵����պ�Ĭ����ʱʱ�� */
#define PRECHG_RELAY_OFF_DLY_DEF        3000            /*Ԥ��̵����Ͽ�Ĭ����ʱʱ�� */
#define RESERVE_RELAY_ON_DLY_DEF        5000             /*Ԥ���̵����պ�Ĭ����ʱʱ�� */
#define RESERVE_RELAY_OFF_DLY_DEF        5000            /*Ԥ���̵����Ͽ�Ĭ����ʱʱ�� */

#define CYCLE_DEF                       0               /*ѭ������Ĭ��ֵ*/
#define CYCLE_FLAG_DEF                  0               /*ѭ����д��ʶλĬ��ֵ*/

#define CHARGER_CUR                     160             /*����������0.1A/bit*/
#define CHARGER_CUR_MIN_DEF                0                /*������������Сֵ0.1A/bit*/
#define CHARGER_CUR_MAX_DEF                10000            /*�������������ֵ0.1A/bit*/


#define CAP_CALIB_HIGH_AVER_VOLT_DEF    3300            /*��������������λƽ����ѹ��ֵĬ��ֵ*/
#define CAP_CALIB_HIGH_AVER_VOLT_MIN_DEF 0                /*��������������λƽ����ѹ��Сֵ*/
#define CAP_CALIB_HIGH_AVER_VOLT_MAX_DEF 5000            /*��������������λƽ����ѹ���ֵ*/

#define CAP_CALIB_LOW_AVER_VOLT_DEF        2800            /*��������������λƽ����ѹ��ֵĬ��ֵ*/
#define CAP_CALIB_LOW_AVER_VOLT_MIN_DEF 0                /*��������������λƽ����ѹ��Сֵ*/
#define CAP_CALIB_LOW_AVER_VOLT_MAX_DEF 5000            /*��������������λƽ����ѹ���ֵ*/

#define CAP_CALIB_LOW_SOC_DEF            PERCENT_TO_SOC(10)/*��������������SOCĬ��ֵ*/
#define CAP_CALIB_LOW_SOC_MIN_DEF        PERCENT_TO_SOC(0)/*��������������SOC��Сֵ*/
#define CAP_CALIB_LOW_SOC_MAX_DEF        PERCENT_TO_SOC(100)/*��������������SOC���ֵ*/

#define LOW_CALIBRATE_SOC_DLY_DEF       60              /*��������������SOC��ʱֵ,��λΪs*/
#define LOW_CALIBRATE_SOC_DLY_MIN       0               /*��������������SOC��ʱֵ,��λΪs*/
#define LOW_CALIBRATE_SOC_DLY_MAX       0xFFFF          /*��������������SOC��ʱֵ,��λΪs*/

#define VICHLE_TYRE_DIAMETER            250             /*������ֱ̥���ߴ� 1cm/bit*/
#define SPEED_RATE                      200             /*�ٱ�Ĭ��ֵ*/
#define GEAR_RATE                       367             /*��������ֱ�*/
#define TYRE_RATE                       100             /*���ű�*/
#define BMS_CHARGER_PROTOCOL_DEFAULT    1               /*�������ͨ��Э�飬Ĭ��Ϊ����Э��*/
#define BMS_CHARGER_PROTOCOL_MIN_DEF    0                /*�������ͨ��Э����Сֵ*/
#define BMS_CHARGER_PROTOCOL_MAX_DEF    3//PROTOCOL_MAX-1    /*�������ͨ��Э�����ֵ*/
#define CCHK_DEF                        0               /*�������У׼Ĭ��ֵ*/
#define CCHK_MIN_DEF                    0x8000          /*�������У׼��Сֵ*/
#define CCHK_MAX_DEF                    0x7FFF          /*�������У׼���ֵ*/
#define CCHK_CUR_OUT_VOLT_MAX_DEF       200             /*�����Զ�У׼����������*/
#define MAX_OVER_CUR_DEF                4500            /*����ŵ��������ֵ0.1A/bit*/
#define SLAVE_NUM_DEF                   1               /*����Ĭ�ϴӻ���Ŀ 1/bit*/
#define SLAVE_NUM_MIN_DEF                0                /*����ӻ���Ŀ��Сֵ*/
#define SLAVE_NUM_MAX_DEF                BMU_MAX_SLAVE_COUNT                /*����ӻ���Ŀ���ֵ*/
#define SLAVE_TEMPERA_NUM_DEF           3               /*����Ĭ�ϴӻ��¸���Ŀ 1/bit*/
#define SLAVE_TEMPERA_NUM_MIN_DEF        0                /*����ӻ��¸���Ŀ��Сֵ 1/bit*/
#define SLAVE_TEMPERA_NUM_MAX_DEF        BMS_BMU_TEMPERATURE_MAX_NUM    /*����ӻ��¸���Ŀ���ֵ 1/bit*/

#define HMI_FRAME_COMM_ABORT_TIMEOUT    HMI_FRAME_TIMEOUT/*HMIͨ�ų�ʱʱ�䶨��*/
#define HMI_FRAME_COMM_ABORT_TIMEOUT_MIN 0                /*HMIͨ�ų�ʱ��Сֵ1ms/bit*/
#define HMI_FRAME_COMM_ABORT_TIMEOUT_MAX 5000            /*HMIͨ�ų�ʱ���ֵ 1ms/bit*/
#define HMI_COMM_BPS_DEF                HMI_DEV_BAND    /*HMIͨ�Ų����ʶ���*/
#define RS485_BPS_MIN_DEF                1            /*HMIͨ�Ų�������Сֵ*/
#define RS485_BPS_MAX_DEF                4            /*HMIͨ�Ų��������ֵ*/
#define RS485_DTU_BPS_DEF               DTU_DEV_BAND    /*DTUͨ�Ų����ʶ���*/
#define RS485_SLAVE_BPS_DEF             BMS_DCM_RS485_DEV_BAND
#define DTU_SEND_INTERVAL_DEF            30                /*DTU�������ڶ���, ��λΪsecond*/
#define DTU_SEND_INTERVAL_MIN_DEF        5                /*DTU����������Сֵ*/
#define DTU_SEND_INTERVAL_MAX_DEF        60000            /*DTU�����������ֵ*/
#define UPPER_COMPUTER_CAN_ADDR_DEF        161                /*��λ��CANͨ�ŵ�ַ����*/
#define UPPER_COMPUTER_CAN_ADDR_MIN        0                /*��λ��CANͨ�ŵ�ַ��Сֵ*/
#define UPPER_COMPUTER_CAN_ADDR_MAX        255                /*��λ��CANͨ�ŵ�ַ���ֵ*/
#define BCU_CAN_ADDR_DEF                244                /*BCU_CANͨ�ŵ�ַ����*/
#define BCU_CAN_ADDR_MIN_DEF            0                /*BCU_CANͨ�ŵ�ַ��Сֵ*/
#define BCU_CAN_ADDR_MAX_DEF            255                /*BCU_CANͨ�ŵ�ַ���ֵ*/
#define SYS_STATUS_SAVE_INTERVAL_DEF    60000            /*ϵͳ״̬��Ϣ��������*/
#define SYS_STATUS_SAVE_INTERVAL_MIN_DEF    10            /*ϵͳ״̬��Ϣ����������Сֵ*/
#define SYS_STATUS_SAVE_INTERVAL_MAX_DEF    60000        /*ϵͳ״̬��Ϣ�����������ֵ*/
#define DISCHG_RELAY_DLY_DEF            5000            /*�ŵ�̵���Ĭ����ʱʱ��*/
#define CHG_RELAY_DLY_DEF               5000            /*���̵���Ĭ����ʱʱ�� */
#define OHT_STATE_DLY_DEF               5000            /*������ʱʱ��Ĭ��ֵ*/
#define ODHT_STATE_DLY_DEF              5000            /*�²���ʱʱ��Ĭ��ֵ  ms*/
#define SOC_OLV_STATE_DLY_DEF           5000            /*SOC������ʱʱ��Ĭ��ֵ */
#define OLV_STATE_DLY_DEF               5000            /*������ʱʱ��Ĭ��ֵ*/
#define OCV_STATE_DLY_DEF               5000            /*������ʱʱ��Ĭ��ֵ */
#define PINSU_LEAK_STATE_DLY_DEF        5000            /*��Ե©����ʱʱ��Ĭ��ֵ*/
#define TOTAL_VOLT_MAX_DEF              5000            /*�����ѹĬ��ֵ 1V/bit*/
#define TOTAL_VOLT_MIN_DEF              3000            /*��С��ѹĬ��ֵ 1V/bit*/
#define DIFF_VOLT_DEF                   4000            /*ѹ��Ĭ��ֵ 1mV/bit*/

//�ŵ����ϵ�� 0.001��/bit

#define SOC0_DISCHG_FACTOR_DEFAULT_VAULE   1000//1��
#define SOC0_DISCHG_FACTOR_MIN_VAULE   900//0.9��
#define SOC0_DISCHG_FACTOR_MAX_VAULE   1100//1.1��

#define EEPROM_TV_CHECK_DEF             0
#define EEPROM_TV_RANGE_DEF             0
#define EEPROM_NORMAL_CHECK_CYCLE_DEF   10000            /*����״̬�¾�Ե��Ϣ��������*/
#define EEPROM_INSU_RES_CALIB_FLAG_DEF  1
#define EEPROM_MOS_ON_DEF               600     /*0��ʾMOS����*/
#define EEPROM_INSU_RES_SWITCH_IN_CHG_DEF  0

#define EEPROM_NORMAL_CHECK_CYCLE_MIN_DEF   1000   
#define EEPROM_NORMAL_CHECK_CYCLE_MAX_DEF   60000  
#define EEPROM_MOS_ON_MIN_DEF    0            /*���屨���ͱ�����ʱ��Сֵ*/
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

#define CFG_HEAT_ON_DLY_DEF                5000            /*������ȿ�����ʱʱ��*/
#define CFG_HEAT_OFF_DLY_DEF            5000            /*������ȹر���ʱʱ��*/
#define CFG_COOL_ON_DLY_DEF                5000            /*�������俪����ʱʱ��*/
#define CFG_COOL_OFF_DLY_DEF            5000            /*��������ر���ʱʱ��*/

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


#define SAVE_FILE_A_FLAG_DEF            0               /*A���ļ��洢��־Ĭ��ֵ*/

#define BMU_BALANCE_ENABLE_DEF            1                /*�ӻ�����ʹ��״̬*/
#define BMU_BALANCE_ENABLE_MIN_DEF        0                /*�ӻ�����ʹ��״̬��Сֵ*/
#define BMU_BALANCE_ENABLE_MAX_DEF        1                /*�ӻ�����ʹ��״̬���ֵ*/
#define MCU_WORK_STATE_DELAY_DEF        5000            /*BCU����״̬��ʱʱ�䣬��λ��ms*/
#define BCU_WORK_STATE_DELAY_MIN_DEF    0                /*BCU����״̬��ʱʱ�䣬��λ��ms*/
#define BCU_WORK_STATE_DELAY_MAX_DEF    0xFFFF            /*BCU����״̬��ʱʱ�䣬��λ��ms*/

#define ALARM_BEEP_INTERVAL_DEF         3000            /*����������ʱ����Ĭ��ֵ*/
#define ALARM_BEEP_INTERVAL_MIN_DEF     0                /*����������ʱ������Сֵ*/
#define ALARM_BEEP_INTERVAL_MAX_DEF     60000           /*����������ʱ�������ֵ*/

#define ALARM_BEEP_MASK_HIGH_DEF        0xFFFF          /* �������������� ����*/
#define ALARM_BEEP_MASK_HIGH_MIN_DEF    0                /*���������������������Сֵ*/
#define ALARM_BEEP_MASK_HIGH_MAX_DEF    0xFFFF            /*����������������������ֵ*/
#define ALARM_BEEP_MASK_LOW_DEF            0xFFFF          /* �������������� ����*/
#define ALARM_BEEP_MASK_LOW_MIN_DEF        0                /* �������������������Сֵ*/
#define ALARM_BEEP_MASK_LOW_MAX_DEF        0xFFFF            /* �������������������Сֵ*/

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
#define RELAY_RELAY9_TYPE_DEF           kRelayTypeHeating   //��Ӧ�ľ��� DO1
#define RELAY_RELAY10_TYPE_DEF          kRelayTypeUnknown   //��Ӧ�ľ��� DO2


#define RELAY_CONTROL_ENABLE_FLAG_DEF   0xFF

#define INSULATION_TYPE_DEF             kInsulationTypeBoard
#define INSULATION_TYPE_MIN_DEF         kInsulationNone
#define INSULATION_TYPE_MAX_DEF         kInsulationTypeMax

#define BATTERY_TOTAL_NUM_DEF           100
#define BATTERY_TOTAL_NUM_MIN_DEF       0
#define BATTERY_TOTAL_NUM_MAX_DEF       5000

#define NOMINAL_TOTAL_VOLTAGE_DEF       3400    //�ֱ��� 0.1v/bit
#define NOMINAL_TOTAL_VOLTAGE_MIN_DEF   0
#define NOMINAL_TOTAL_VOLTAGE_MAX_DEF   65535

#define CHARGE_EFFICIENCY_FACTOR_DEF            1000
#define CHARGE_EFFICIENCY_FACTOR_MIN_DEF        0
#define CHARGE_EFFICIENCY_FACTOR_MAX_DEF        2000

#define DISCHARGE_EFFICIENCY_FACTOR_DEF         1000
#define DISCHARGE_EFFICIENCY_FACTOR_MIN_DEF     0
#define DISCHARGE_EFFICIENCY_FACTOR_MAX_DEF     2000

#define SYSTEM_STATE_AVAILABLE_DELAY_DEF        5000 // ϵͳ״̬����ϵ����ʧ�ܺ��ӳ�ʱ��

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
#define MAX_LTC6803_NUM                 10         //���֧�ֲɼ�ģ����
#define LTC_CELLV_NUM                   12        //ÿ���ɼ�ģ��ɼ��������
#define LTC_CELLT_NUM                   8         //ÿ���ɼ�ģ��ɼ�����¶���
#define LTC_CELLT_EXTEND_CNT            4         /*1����ÿ���ɼ�ģ��ɼ�2���¸�
                                                    2����ÿ���ɼ�ģ��ɼ�4���¸�,��������ֱ��6���¸�*/
#define BYU_BALANCE_EN_DEF              1
 //��8λ����� ��8λ�¶���
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
#define BMS_PRODUCT_HOST_CODE_NUM   "110702C001"    /*����Ψһ��*/
#define BMS_PRODUCT_DATE            "2012-03-12"    /*������������*/

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
#define WORK_STATE_STOP_MODE_DELAY   5000 // ����ֹͣ״̬��ʱ��, ms
#define WORK_STATE_WORK_MODE_DELAY   2000 // ���빤��״̬��ʱ��, ms

/**********************************************
 *
 * System State
 *
 ***********************************************/
#define SYSTEM_STATE_SCAN_PERIODIC      500

#define SYSTEM_STARTUP_BEEP_TIME        50   // ϵͳ��������ʱ��

/**********************************************
 *
 * Capacity Setting
 *
 ***********************************************/
#define   MAX_CAP_OFFSET                            20  //0.1/AH  2 // AH, ���������������Ĳ�ֵ��һ�㳧���ṩ�ȱ���Դ������

/**********************************************
 *
 * System Voltage
 *
 ***********************************************/
#define SYSTEM_VOLTAGE_SCAN_PERIOD          500  // ms, ϵͳ��ѹ��������
#define SYSTEM_VOLTAGE_AD_CNT               50 // ϵͳ��ѹ������
#define SYSTEM_VOLTAGE_AD_CONVERT_FACTOR    20

/**********************************************
 *
 * BMU Setting
 *
 ***********************************************/
#define BMU_MAX_SLAVE_COUNT           25 //���֧�ֵĴӻ���Ŀ

#define BMU_MAX_TOTAL_VOLTAGE_NUM     1000
#define BMU_MAX_TOTAL_TEMP_NUM        800
#define BMU_MAX_TOTAL_BLANCE_STATE    ((BMU_MAX_TOTAL_VOLTAGE_NUM + 7) / 8)

#define BMS_BMU_BALANCE_NUM               2  //�ӻ�����·��

#define BMS_BMU_BATTERY_MAX_NUM           120  // �ӻ��������,���Ӵ�ֵҪע��J1939ͨ�ų�ʱ���ʵ�����kBmuResponseTimeoutIndex��Ŀǰ��ֵ����֧��70�����ҡ�
#define BMS_BMU_TEMPERATURE_MAX_NUM       60  //�ӻ�����¶���
#define BMS_BMU_BALANCE_STATE_BYTE_NUM    ((BMS_BMU_BATTERY_MAX_NUM + 7) / 8)

#define BMS_BMU_DEFAULT_TEMPERATURE_NUM       22
#define BMS_BMU_DEFAULT_VOLTAGE_NUM           60
#define BMS_BMU_BALANCE_STATE_NUM(VOLT_NUM)   ((VOLT_NUM + 7) / 8)
#define BMS_BMU_DEFAULT_BALANCE_STATE_NUM     BMS_BMU_BALANCE_STATE_NUM(BMS_BMU_DEFAULT_VOLTAGE_NUM)

#define BMU_STAT_HIGH_TEMPERATURE_NUM  1     // ����¶�ͳ�Ƹ���
#define BMU_STAT_LOW_TEMPERATURE_NUM   1     // ����¶�ͳ�Ƹ���
#define BMU_STAT_HIGH_VOLTAGE_NUM      1     // ��ߵ����ѹͳ�Ƹ���
#define BMU_STAT_LOW_VOLTAGE_NUM       1     // ��͵����ѹͳ�Ƹ���

/**********************************************
 *
 * HMI Setting
 *
 ***********************************************/
#define HMI_DEV_NAME                    kRs485Dev1   // RS485�豸��
#define HMI_DEV_BAND                    3
#define HMI_MAX_RECEIVE_BUFFER_SIZE     100          // ������ݰ�����
#define HMI_MAX_RECEIVE_BUFFER_COUNT    2            // ���ܻ������
#define HMI_FRAME_TIMEOUT               40           // ֡���ʱ��
#define HMI_CUSTOM_INFORMATION_NUM      3           // ֧����ʾ�Ŀͻ���Ϣ�ĸ���

/**********************************************
 *
 * DTU Setting
 *
 ***********************************************/
#define DTU_DEV_NAME                    kRs485Dev0
#define DTU_DEV_BAND                    3
#define DTU_DEV_FRAME_SIZE              110
#define DTU_SEND_PERIODIC               10000 // ms, DTU��������
#define DTU_CURRENT_OFFSET              32000 // ����ƫ��ֵ�� ��������Ϊ��, TODO:��������з���ֵ��ȥ����ƫ��

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

#define  MOTO_VICHLE_TYRE_DIAMETER  250      /*��̥�ߴ� 0.1Ӣ��/bit*/
#define  MOTO_SPEED_RATE            350      /*�ٱ� 0.01 /bit*/
#define  MOTO_GEAR_RATE             367      /*��������ֱ�0.01 /bit*/
#define  MOTO_TYRE_RATE             100      /*���ű�0.01 /bit*/
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
 * DCM����RS485ģʽ�µ�����ͨѶ�;�Ե���ͨѶ
 *
 ***********************************************/
#define BMS_DCM_RS485_DEV_NAME        kRs485Dev0
#define BMS_DCM_RS485_DEV_BAND        3
#define BMS_DCM_RS485_FRAME_SIZE      48

#define BMS_DCM_ISOLATION_ID            0x42
#define BMS_DCM_REQUEST_SLAVE_INTERVAL  50

#define DCM_MAX_RECEIVE_BUFFER_COUNT    2
#define DCM_MAX_RECEIVE_BUFFER_SIZE     74

#define DCM_TIMEOUT                     200         /*DCM��ʱ�ȴ�ʱ��*/


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

#define EMS_ECU_TX_INTERVAL       24  // ECU��֡����ʱ����, ��λΪms

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
#define  BMS_CHARGER_CAN_DEV            kCanDev1 //ע��ʵ����kCanDev4��Ϊ����λ����ʾ����ΪkCanDev3��Ӧ�ò��Զ�����ΪkCanDev4
#define  BMS_CHARGER_MAX_VOLTAGE        5900 // config_get(kChargerVoltIndex)
#define  BMS_CHARGER_MAX_VOLTAGE_MIN    0
#define  BMS_CHARGER_MAX_VOLTAGE_MAX    0xFFFF

#define  BMS_FULL_CHG_RELEASE_VOLT      5700
#define  BMS_FULL_CHG_RELEASE_VOLT_MIN  0
#define  BMS_FULL_CHG_RELEASE_VOLT_MAX  0xFFFF

#define  BMS_CHARGER_ENABLE_DEFAULT     1
#define  BMS_CHARGER_WIRE_REG           PORTK_PK3

#define  BMS_CHG_CUT_OFF_CUR_CAP_RATE   0.05
#define  BMS_CHARGER_CUT_OFF_CURRENT    TOTALCAP_DEF*BMS_CHG_CUT_OFF_CUR_CAP_RATE*10/*��ֹ���� 0.1A/bit*/
#define  BMS_CHARGER_CUT_OFF_CURRENT_MIN    30 // 0.1A/bit
#define  BMS_CHARGER_CUT_OFF_CURRENT_MAX    1000 // 0.1A/bit

#define  BMS_CHARGER_CUT_OFF_CURRENT_DELAY  10000 // 1ms/bit

#define  BMS_CHARGER_TOTAL_TIME_DEF     15 // 1hour/bit
#define  BMS_CHARGER_TOTAL_TIME_MIN     1 // 1hour/bit
#define  BMS_CHARGER_TOTAL_TIME_MAX     30// 1hour/bit

#define  BMS_VALID_CHARGE_CURRENT_MIN   15

#define  BMS_FULL_CHG_RELEASE_DELAY_DEF 5000 //�����ͷ���ʱ�� 1ms/bit

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

#define SOC_CURRENT_SAMPLE_FILTER_NUM 1  // ���������С���Թ�����
#define SOC_CURRENT_FAILURE_DELAY   5000 // ʧ����ʱ

#define SOC_CALIB_MIN_SOH  70       // ����У�����������ٷ���
#define SOC_CALIB_MAX_RATE  1       // ����У���������������������ٷ���
#define SOC_CALIB_MIN_AH    1       // ����У����С����İ�ʱ������Ҫ��������1%����1AH���龰

#define CAP_CALIB_RESOLUTION_MAX    1000 
#define CAP_CALIB_CNT_RESOLUTION    100      //����ֵΪ1��10��100��1000

#define TOTAL_CAP_DECIMAL_MAX       0xFFFF
#define TOTAL_CAP_DECIMAL_MIN       0

#define SOC_UPWARD_CHECK_SOC_RELEASE    PERCENT_TO_SOC(70) 

/** ������¼SOCѭ������ͨ��������伴��Ϊ��ؾ�����һ��ѭ����ŵ� */
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
#define INSULATION_MODULE_UNIT              100  // ��

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
#define ALERT_DISABLE_TIMEOUT           86400000     /*����������ʱ��ʱ��, Ĭ��24Сʱ��ʱ�䵽�������������������*/
#define ALERT_BEEP_ON_PERIODIC          50          /*����������ʱ�䳤��*/

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
#define WAKEUP_INTERVAL_MINUTES (60UL * 24) //���߻�������24hour
#define WORK_MAX_TIME_AFTER_WAKEUP_SECTONDS (60UL * 2) //�ϵ�����2min


/**********************************************
 *
 * �豸��Ϣ
 *
***********************************************/

#define DEVICE_VENDOR_NAME          "Ligoo"
//Ӳ���汾��
#if BMS_SUPPORT_BY5248D 
  #define DEVICE_COMMON_NAME          "BY5XD"
#else
  #define DEVICE_COMMON_NAME          "BC5XB"
#endif
//�豸����
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

//����Ӳ���汾��
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
#define BMS_BUILD_YEAR      2018  //����ʱ��
#define BMS_BUILD_MONTH     1
#define BMS_BUILD_DAY       2  
//ģ��汾                                      
#define DEVICE_SW_MAJOR_VER 2
#define DEVICE_SW_MINOR_VER 1
#define DEVICE_SW_REV_VER   101 
#define DEVICE_SW_BUILD_VER 105                         
//�����汾
#define ORDER_SW_MAJOR_VER  1  
#define ORDER_SW_MINOR_VER  0                  
#define ORDER_SW_REV_VER    0
#define ORDER_SW_BUILD_VER  0




#define DEVICE_SW_SUPPORT_HW_VER    3 //���֧�ֵ�Ӳ���汾 ��SW_A0_MCU~SW_A2_MCUȷ��

/***********************************************************************************
 *
 * �ڲ����ã��벻Ҫ�޸�
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
