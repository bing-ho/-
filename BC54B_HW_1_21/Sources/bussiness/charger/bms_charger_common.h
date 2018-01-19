#ifndef __BMS_CHARGER_COMMON_H_
#define __BMS_CHARGER_COMMON_H_

#include "app_cfg.h"
#include "bms_can.h"
#include "j1939_cfg.h"

#define CHARGER_EXCEPT_USE_BIT_NUM          2
#define CHARGER_EXCEPT_MASK_NUM             3
#define CHARGER_EXCEPT_NUM_IN_BYTE          (8/CHARGER_EXCEPT_USE_BIT_NUM)

#define CHARGER_CAN_RE_INIT_TIME_WITH_COMM_ABORT    5000//10min ����CANͨ���жϺ�������ʱʱ�� ��λ��ms

typedef enum
{
    CHARGER_HARDWARE_FAULT_NUM,                 //����Ӳ�����ϱ��
    CHARGER_OVER_TEMPERATURE_EXCEPT_NUM,        //�����¶��쳣���
    CHARGER_INPUT_VOLT_EXCEPT_NUM,              //���������ѹ�쳣���
    CHARGER_RUN_STATUS_NUM,                     //�������У�������״̬���
    CHARGER_COMM_ABORT_WITH_BMS_NUM,            //BMSͨ�Ź��ϱ��
    CHARGER_OUTPUT_VOLT_EXCEPT_NUM,             //���������ѹ�쳣���
    CHARGER_BATTERY_REVERSE_CONNECT_NUM,        //��ط����쳣���
    CHARGER_STOP_CHG_WITH_FINISH,               //�������ﵽ�������ֹͣ���״̬���
    CHARGER_INNER_OVER_TEMPERATURE_EXCEPT_NUM,  //�����ڲ������쳣���
    CHARGER_CHAGER_CONNECTOR_EXCEPT_NUM,        //����������쳣���
    CHARGER_ENERGY_TRANSIMIT_EXCEPT_NUM,        //���������ܴ����쳣���
    CHARGER_EMERGENCY_STOP_NUM,                 //������ͣ״̬���
    CHARGER_CURRENT_NOT_MATCH_NUM,              //������ƥ����
    CHARGER_PHASE_LACK_EXCEPT_NUM,              //ȱ���쳣���
    CHARGER_HIGH_VOLT_ALARM_NUM,                //������ѹ�������
    CHARGER_HIGH_CURRENT_ALARM_NUM,             //���������������
    CHARGER_OVER_PROTECT_VOLT_NUM,              //�����ѹԽ�����������ޱ��
    CHARGER_OVER_PROTECT_CURR_NUM,              //�ܵ���Խ�����������ޱ��
    CHARGER_OVER_NOMINAL_VOLT_NUM,              //�����ѹԽ�������ѹ���ޱ��
    CHARGER_OVER_NOMINAL_CURR_NUM,              //�ܵ���Խ������������ޱ��
    CHARGER_BMS_STOP_CHARGE_EXCEPT_NUM,         //BMSֹͣ���״̬���
    CHARGER_FAULT_EXCEPT_NUM,                   //�����������ϱ��
    CHARGER_COMM_ABORT_WITH_CHARGE_SPOT_NUM,    //��������׮ͨ���жϱ��
    CHARGER_CHARGE_SPOT_ENERGENCY_STOP_NUM,     //���׮��ͣ״̬���
    CHARGER_MANUAL_STOP_NUM,                    //�˹�ֹͣ���״̬���
    CHARGER_EXCEPT_NUM_MAX
}CHARGER_EXCEPT_STATUS;

#define CHARGER_EXCEPT_STATUS_BUFF_NUM  (CHARGER_EXCEPT_NUM_MAX+CHARGER_EXCEPT_NUM_IN_BYTE-1)/CHARGER_EXCEPT_NUM_IN_BYTE
#define CHARGER_EXCEPT_STATUS_DEFAULT   0xFF

typedef struct
{
    INT16U ChargeCur;           //��ǰ������
    INT16U ChargeVolt;          //��ǰ����ѹ
    INT16U ChargeOnOff;         //��ǰ��翪��
    INT16U PulseChargeTime;     //������ʱ��
    INT16U PulseChargeCurMax;   //������������
}CHARGER_SET_CONFIG;

typedef struct
{
    INT8U IsCommunication;      //��ǰͨ��״̬
    INT8U IsCharging;           //��ǰ���״̬
    INT16U OutputChgVolt;       //��ǰ�����ѹ
    INT16U OutputChgCur;        //��ǰ�������
    INT16U ChgrOutVoltMax;      //������������ѹ
    INT16U ChgrOutVoltMin;      //������С�����ѹ
    INT16U ChgrOutCurMax;       //��������������
    INT16U ChgrOutCurMin;       //������С�������
    INT8U  ChgrExceptStatus[CHARGER_EXCEPT_STATUS_BUFF_NUM];    //�����쳣״̬
    INT16U ChgTimeEclip;        //�ѳ��ʱ��
    INT16U ChgTimeRemain;       //���ʣ��ʱ��
    INT16U ChgEnergyEclip;      //�ѳ������0.1kW.h
    INT8U IsChargeReady;        //����׼������״̬
}CHARGER_STATUS;

typedef struct
{
    CHARGER_SET_CONFIG ChgrSetCfg;
    CHARGER_STATUS ChargerSts;
}CHARGER_PARA_INDEX;

typedef void (*ChargerInitFuc)(void);//������ʼ������
typedef void (*ChargerUninitFuc)(void);//��������ʼ������
typedef Result (*ChargeOnFuc)(void);//��翪������
typedef Result (*ChargeOffFuc)(void);//���رպ���
typedef INT8U (*ChargeIsOnFuc)(void);//���״̬��ѯ
typedef INT8U (*ChargerIsConnectedFuc)(void);//�����Ƿ�����״̬��ѯ
typedef Result (*GetChargerStatusFuc)(INT8U *status);//��ȡ����״̬��Ϣ
typedef Result (*GetChargerTypeFuc)(void* info);//��ȡ��������
typedef Result (*SetChargeVoltFuc)(INT16U volt);//���ó���ѹ
typedef Result (*SetChargeCurFuc)(INT16U cur);//���ó�����
typedef Result (*GetChargeVoltFuc)(INT16U* volt);//��ȡ����ѹ
typedef Result (*GetChargeCurFuc)(INT16U* cur);//��ȡ������
typedef Result (*ChargerGetOutputVoltFuc)(INT16U* volt);//��ȡ���������ѹ
typedef Result (*ChargerGetOutputCurFuc)(INT16U* cur);//��ȡ�����������
typedef Result (*ChargerSetPulseCurFuc)(INT16U cur);//�������������
typedef Result (*ChargerGetPulseCurFuc)(INT16U* cur);//��ȡ���������
typedef Result (*ChargerSetPulseChgTimeFuc)(INT16U time);//����������ʱ��
typedef Result (*ChargerGetPulseChgTimeFuc)(INT16U* time);//��ȡ������ʱ��
typedef Result (*ChargerConstantVoltCurCtlFuc)(void);//������ѹ������
typedef Result (*ChargerGetChargeVoltMaxFuc)(INT16U* volt);//��ȡ����������ѹ
typedef Result (*ChargerGetChargeVoltMinFuc)(INT16U* volt);//��ȡ������С����ѹ
typedef Result (*ChargerGetChargeCurMaxFuc)(INT16U* cur);//��ȡ������������
typedef Result (*ChargerGetChargeCurMinFuc)(INT16U* cur);//��ȡ������С������
typedef Result (*ChargerGetPulseCurMaxFuc)(INT16U* cur);//��ȡ����������������
typedef Result (*ChargerGetChargedEnergyFuc)(INT16U* energy);//��ȡ�����ѳ�����
typedef Result (*ChargerGetChgTimeEclipseFuc)(INT16U* time);//��ȡ�����ѳ��ʱ��
typedef Result (*ChargerGetChgTimeRequireFuc)(INT16U* time);//��ȡ����ʣ����ʱ��
typedef Result (*ChargerGetChargeCycleFuc)(INT16U* cycle);//��ȡ������
typedef Result (*ChargerGetChargeReadyStatusFuc)(void);//��ȡ�������׼������״̬
typedef INT8U  (*ChargerChargingIsEnableFuc)(void);//��ȡ�������ʹ��״̬
typedef Result (*ChargerIsCommunicationFuc)(void);//��ȡ����ͨ��״̬

typedef struct
{
    ChargerInitFuc ChargerInit;//������ʼ������
    ChargerUninitFuc ChargerUninit;//��������ʼ������
    ChargeOnFuc ChargeOn;//��翪������
    ChargeOffFuc ChargeOff;//���رպ���
    ChargeIsOnFuc ChargeIsOn;//���״̬��ѯ
    ChargerIsConnectedFuc ChargerIsConnected;//�����Ƿ�����״̬��ѯ
    GetChargerStatusFuc GetChargerStatus;//��ȡ����״̬��Ϣ
    GetChargerTypeFuc GetChargerType;//��ȡ��������
    SetChargeVoltFuc SetChargeVolt;//���ó���ѹ
    SetChargeCurFuc SetChargeCur;//���ó�����
    GetChargeVoltFuc GetChargeVolt;//��ȡ����ѹ
    GetChargeCurFuc GetChargeCur;//��ȡ������
    ChargerGetOutputVoltFuc ChargerGetOutputVolt;//��ȡ���������ѹ
    ChargerGetOutputCurFuc ChargerGetOutputCur;//��ȡ�����������
    ChargerSetPulseCurFuc ChargerSetPulseCur;//�������������
    ChargerGetPulseCurFuc ChargerGetPulseCur;//��ȡ���������
    ChargerSetPulseChgTimeFuc ChargerSetPulseChgTime;//����������ʱ��
    ChargerGetPulseChgTimeFuc ChargerGetPulseChgTime;//��ȡ������ʱ��
    ChargerConstantVoltCurCtlFuc ChargerConstantVoltCurCtl;//������ѹ������
    ChargerGetChargeVoltMaxFuc ChargerGetChargeVoltMax;//��ȡ����������ѹ
    ChargerGetChargeVoltMinFuc ChargerGetChargeVoltMin;//��ȡ������С����ѹ
    ChargerGetChargeCurMaxFuc ChargerGetChargeCurMax;//��ȡ������������
    ChargerGetChargeCurMinFuc ChargerGetChargeCurMin;//��ȡ������С������
    ChargerGetPulseCurMaxFuc ChargerGetPulseCurMax;//��ȡ����������������
    ChargerGetChargedEnergyFuc ChargerGetChargedEnergy;//��ȡ�����ѳ�����
    ChargerGetChgTimeEclipseFuc ChargerGetChgTimeEclipse;//��ȡ�����ѳ��ʱ��
    ChargerGetChgTimeRequireFuc ChargerGetChgTimeRequire;//��ȡ����ʣ����ʱ��
    ChargerGetChargeCycleFuc ChargerGetChargeCycle;//��ȡ������
    ChargerGetChargeReadyStatusFuc ChargerGetChargeReadyStatus;//��ȡ�������׼������״̬
    ChargerChargingIsEnableFuc ChargerChargingIsEnable;//��ȡ�������ʹ��״̬
    ChargerIsCommunicationFuc ChargerIsCommunication;//��ȡ����ͨ��״̬
}CHARGER_FUCTION;

typedef enum
{
    NO_PROTOCOL=0,
    LIGOO_PROTOCOL = 1,
    GUO_BIAO_PROTOCOL = 2,
    GUO_WANG_PROTOCOL = 3,
    PROTOCOL_MAX
}CHARGER_PROTOCOL_INDEX;

#define CHARGER_CAN_BUFFER_COUNT                5

#define CHARGER_REC_PDU_BUFF                  	3           //����PDU��֡���ݻ����С
#define CHARGER_SENT_FRAME_BUFF					3			//����֡�������
#define CHARGER_REC_FRAME_BUFF					3			//����֡�������
#define CHARGER_SENT_MESSAGE_LENGTH_MAX			512			//��������Ϣ����󳤶�
#define CHARGER_REC_MESSAGE_LENGTH_MAX        	10          //������Ϣ����󳤶�
#define CHARGER_REC_MESSAGE_BUFF_MAX          	1           //������Ϣ����󻺴����
#define CHARGER_TP_REC_MAX                    	1           //ͬʱ���Խ��յ�TP��������

#define CHARGER_FRAME_TIMEOUT_DEFINE            100


extern OS_STK g_charger_tx_task_stack[CHARGER_TX_STK_SIZE];
extern OS_STK g_charger_rx_task_stack[CHARGER_RX_STK_SIZE];


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER
extern CHARGER_PARA_INDEX ChargerParaIndex;
extern CanMessage g_charger_can_buffers[CHARGER_CAN_BUFFER_COUNT];

extern CHARGER_FUCTION ChargerFuction[PROTOCOL_MAX];

extern J1939CanContext* _PAGED g_chargerCanContextPtr;
extern J1939SendMessageBuff g_chargerSendMessage;
extern unsigned char g_chargerSendMsgDataBuff[CHARGER_SENT_MESSAGE_LENGTH_MAX];
extern J1939RecMessage g_chargerRecMessage;
extern unsigned char g_chargerRecMsgDataBuff[CHARGER_REC_MESSAGE_LENGTH_MAX];

extern J1939CanContext g_j1939ChgCanContext;
extern J1939Event g_j1939ChgSendEventSem;
extern J1939Event g_j1939ChgRecEventSem;
extern J1939CanInfo g_j1939ChgCanInfo;
extern J1939CanTranPro g_j1939ChgSendTpIndex;
extern J1939CanTPBuff g_j1939ChgRecTpBuffIndex;
extern J1939TPChain g_j1939ChgRecTpBuffs[CHARGER_TP_REC_MAX];
extern J1939CanFrameBuff g_j1939ChgSendFrameBuffIndex;
extern J1939CanFrame g_j1939ChgSendFrameBuffs[CHARGER_SENT_FRAME_BUFF];
extern J1939CanFrameBuff g_j1939ChgRecPduBuffIndex;
extern J1939CanFrame g_j1939ChgRecPduBuffs[CHARGER_REC_PDU_BUFF];
extern J1939CanFrameBuff g_j1939ChgRecFrameBuffIndex;
extern J1939CanFrame g_j1939ChgRecFrameBuffs[CHARGER_REC_FRAME_BUFF];
extern J1939SendMessageBuff g_j1939ChgSendMsgBuff;
extern unsigned char g_j1939ChgSendMsgDataBuff[CHARGER_SENT_MESSAGE_LENGTH_MAX];
extern J1939RecTempMessageBuff g_j1939ChgRecTempMsgBuffIndex;
extern J1939RecMessage g_j1939ChgRecTempMsgBuffs[CHARGER_TP_REC_MAX];
extern unsigned char g_j1939ChgRecTempMsgDataBuff[CHARGER_TP_REC_MAX][CHARGER_REC_MESSAGE_LENGTH_MAX];
extern J1939RecMessageBuff g_j1939ChgRecMsgDataBuffIndex;
extern J1939RecMessage g_j1939ChgRecMsgDataBuffs[CHARGER_REC_MESSAGE_BUFF_MAX];
extern unsigned char g_j1939ChgRecMsgDataBuff[CHARGER_REC_MESSAGE_BUFF_MAX][CHARGER_REC_MESSAGE_LENGTH_MAX];
#pragma DATA_SEG DEFAULT

extern INT16U charger_callback(can_t context, CanMessage* msg, void* userdata);
extern void j1939_comm_para_init(void);
extern INT16U charger_get_send_message_timeout(J1939SendMessageBuff* _PAGED msg);
extern Result charger_set_charge_except_status_with_num(INT8U num, INT8U flag);
extern Result charger_clear_charge_except_status_with_num(INT8U num);
extern INT8U charger_get_charge_except_status_with_num(INT8U num);
extern void charger_clear_all_except_status(void);
extern INT8U charger_get_except_status_buff_num(void);
extern INT8U charger_get_except_status_with_buff_num(INT8U num);
extern INT32U charger_get_control_bits(void);
extern INT16U guobiao_get_pwm_allow_charge_current_max(void);
extern INT16U guobiao_get_cc_allow_charge_current_max(void);
extern INT16U guobiao_get_allow_charge_current_max(void);

#endif
