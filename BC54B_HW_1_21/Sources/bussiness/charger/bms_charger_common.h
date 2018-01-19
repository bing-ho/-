#ifndef __BMS_CHARGER_COMMON_H_
#define __BMS_CHARGER_COMMON_H_

#include "app_cfg.h"
#include "bms_can.h"
#include "j1939_cfg.h"

#define CHARGER_EXCEPT_USE_BIT_NUM          2
#define CHARGER_EXCEPT_MASK_NUM             3
#define CHARGER_EXCEPT_NUM_IN_BYTE          (8/CHARGER_EXCEPT_USE_BIT_NUM)

#define CHARGER_CAN_RE_INIT_TIME_WITH_COMM_ABORT    5000//10min 充电机CAN通信中断后重启延时时间 单位：ms

typedef enum
{
    CHARGER_HARDWARE_FAULT_NUM,                 //充电机硬件故障编号
    CHARGER_OVER_TEMPERATURE_EXCEPT_NUM,        //充电机温度异常编号
    CHARGER_INPUT_VOLT_EXCEPT_NUM,              //充电机输入电压异常编号
    CHARGER_RUN_STATUS_NUM,                     //充电机运行（启动）状态编号
    CHARGER_COMM_ABORT_WITH_BMS_NUM,            //BMS通信故障编号
    CHARGER_OUTPUT_VOLT_EXCEPT_NUM,             //充电机输出电压异常编号
    CHARGER_BATTERY_REVERSE_CONNECT_NUM,        //电池反接异常编号
    CHARGER_STOP_CHG_WITH_FINISH,               //充电机充电达到完成条件停止充电状态编号
    CHARGER_INNER_OVER_TEMPERATURE_EXCEPT_NUM,  //充电机内部过温异常编号
    CHARGER_CHAGER_CONNECTOR_EXCEPT_NUM,        //充电连接器异常编号
    CHARGER_ENERGY_TRANSIMIT_EXCEPT_NUM,        //充电电量不能传输异常编号
    CHARGER_EMERGENCY_STOP_NUM,                 //充电机急停状态编号
    CHARGER_CURRENT_NOT_MATCH_NUM,              //电流不匹配编号
    CHARGER_PHASE_LACK_EXCEPT_NUM,              //缺相异常编号
    CHARGER_HIGH_VOLT_ALARM_NUM,                //充电机过压报警编号
    CHARGER_HIGH_CURRENT_ALARM_NUM,             //充电机过流报警编号
    CHARGER_OVER_PROTECT_VOLT_NUM,              //电池总压越充电机保护上限编号
    CHARGER_OVER_PROTECT_CURR_NUM,              //总电流越充电机保护上限编号
    CHARGER_OVER_NOMINAL_VOLT_NUM,              //电池总压越充电机额定总压上限编号
    CHARGER_OVER_NOMINAL_CURR_NUM,              //总电流越充电机额定电流上限编号
    CHARGER_BMS_STOP_CHARGE_EXCEPT_NUM,         //BMS停止充电状态编号
    CHARGER_FAULT_EXCEPT_NUM,                   //充电机其他故障编号
    CHARGER_COMM_ABORT_WITH_CHARGE_SPOT_NUM,    //充电机与充电桩通信中断编号
    CHARGER_CHARGE_SPOT_ENERGENCY_STOP_NUM,     //充电桩急停状态编号
    CHARGER_MANUAL_STOP_NUM,                    //人工停止充电状态编号
    CHARGER_EXCEPT_NUM_MAX
}CHARGER_EXCEPT_STATUS;

#define CHARGER_EXCEPT_STATUS_BUFF_NUM  (CHARGER_EXCEPT_NUM_MAX+CHARGER_EXCEPT_NUM_IN_BYTE-1)/CHARGER_EXCEPT_NUM_IN_BYTE
#define CHARGER_EXCEPT_STATUS_DEFAULT   0xFF

typedef struct
{
    INT16U ChargeCur;           //当前充电电流
    INT16U ChargeVolt;          //当前充电电压
    INT16U ChargeOnOff;         //当前充电开关
    INT16U PulseChargeTime;     //脉冲充电时间
    INT16U PulseChargeCurMax;   //最大脉冲充电电流
}CHARGER_SET_CONFIG;

typedef struct
{
    INT8U IsCommunication;      //当前通信状态
    INT8U IsCharging;           //当前充电状态
    INT16U OutputChgVolt;       //当前输出电压
    INT16U OutputChgCur;        //当前输出电流
    INT16U ChgrOutVoltMax;      //充电机最大输出电压
    INT16U ChgrOutVoltMin;      //充电机最小输出电压
    INT16U ChgrOutCurMax;       //充电机最大输出电流
    INT16U ChgrOutCurMin;       //充电机最小输出电流
    INT8U  ChgrExceptStatus[CHARGER_EXCEPT_STATUS_BUFF_NUM];    //充电机异常状态
    INT16U ChgTimeEclip;        //已充电时间
    INT16U ChgTimeRemain;       //充电剩余时间
    INT16U ChgEnergyEclip;      //已充电能量0.1kW.h
    INT8U IsChargeReady;        //充电机准备就绪状态
}CHARGER_STATUS;

typedef struct
{
    CHARGER_SET_CONFIG ChgrSetCfg;
    CHARGER_STATUS ChargerSts;
}CHARGER_PARA_INDEX;

typedef void (*ChargerInitFuc)(void);//充电机初始化函数
typedef void (*ChargerUninitFuc)(void);//充电机反初始化函数
typedef Result (*ChargeOnFuc)(void);//充电开启函数
typedef Result (*ChargeOffFuc)(void);//充电关闭函数
typedef INT8U (*ChargeIsOnFuc)(void);//充电状态查询
typedef INT8U (*ChargerIsConnectedFuc)(void);//充电机是否连接状态查询
typedef Result (*GetChargerStatusFuc)(INT8U *status);//获取充电机状态信息
typedef Result (*GetChargerTypeFuc)(void* info);//获取充电机类型
typedef Result (*SetChargeVoltFuc)(INT16U volt);//设置充电电压
typedef Result (*SetChargeCurFuc)(INT16U cur);//设置充电电流
typedef Result (*GetChargeVoltFuc)(INT16U* volt);//获取充电电压
typedef Result (*GetChargeCurFuc)(INT16U* cur);//获取充电电流
typedef Result (*ChargerGetOutputVoltFuc)(INT16U* volt);//获取充电机输出电压
typedef Result (*ChargerGetOutputCurFuc)(INT16U* cur);//获取充电机输出电流
typedef Result (*ChargerSetPulseCurFuc)(INT16U cur);//设置脉冲充电电流
typedef Result (*ChargerGetPulseCurFuc)(INT16U* cur);//获取脉冲充电电流
typedef Result (*ChargerSetPulseChgTimeFuc)(INT16U time);//设置脉冲充电时间
typedef Result (*ChargerGetPulseChgTimeFuc)(INT16U* time);//获取脉冲充电时间
typedef Result (*ChargerConstantVoltCurCtlFuc)(void);//恒流恒压充电控制
typedef Result (*ChargerGetChargeVoltMaxFuc)(INT16U* volt);//获取充电机最大充电电压
typedef Result (*ChargerGetChargeVoltMinFuc)(INT16U* volt);//获取充电机最小充电电压
typedef Result (*ChargerGetChargeCurMaxFuc)(INT16U* cur);//获取充电机最大充电电流
typedef Result (*ChargerGetChargeCurMinFuc)(INT16U* cur);//获取充电机最小充电电流
typedef Result (*ChargerGetPulseCurMaxFuc)(INT16U* cur);//获取充电机最大脉冲充电电流
typedef Result (*ChargerGetChargedEnergyFuc)(INT16U* energy);//获取充电机已充电电量
typedef Result (*ChargerGetChgTimeEclipseFuc)(INT16U* time);//获取充电机已充电时间
typedef Result (*ChargerGetChgTimeRequireFuc)(INT16U* time);//获取充电机剩余充电时间
typedef Result (*ChargerGetChargeCycleFuc)(INT16U* cycle);//获取充电次数
typedef Result (*ChargerGetChargeReadyStatusFuc)(void);//获取充电机充电准备就绪状态
typedef INT8U  (*ChargerChargingIsEnableFuc)(void);//获取充电机充电使能状态
typedef Result (*ChargerIsCommunicationFuc)(void);//获取充电机通信状态

typedef struct
{
    ChargerInitFuc ChargerInit;//充电机初始化函数
    ChargerUninitFuc ChargerUninit;//充电机反初始化函数
    ChargeOnFuc ChargeOn;//充电开启函数
    ChargeOffFuc ChargeOff;//充电关闭函数
    ChargeIsOnFuc ChargeIsOn;//充电状态查询
    ChargerIsConnectedFuc ChargerIsConnected;//充电机是否连接状态查询
    GetChargerStatusFuc GetChargerStatus;//获取充电机状态信息
    GetChargerTypeFuc GetChargerType;//获取充电机类型
    SetChargeVoltFuc SetChargeVolt;//设置充电电压
    SetChargeCurFuc SetChargeCur;//设置充电电流
    GetChargeVoltFuc GetChargeVolt;//获取充电电压
    GetChargeCurFuc GetChargeCur;//获取充电电流
    ChargerGetOutputVoltFuc ChargerGetOutputVolt;//获取充电机输出电压
    ChargerGetOutputCurFuc ChargerGetOutputCur;//获取充电机输出电流
    ChargerSetPulseCurFuc ChargerSetPulseCur;//设置脉冲充电电流
    ChargerGetPulseCurFuc ChargerGetPulseCur;//获取脉冲充电电流
    ChargerSetPulseChgTimeFuc ChargerSetPulseChgTime;//设置脉冲充电时间
    ChargerGetPulseChgTimeFuc ChargerGetPulseChgTime;//获取脉冲充电时间
    ChargerConstantVoltCurCtlFuc ChargerConstantVoltCurCtl;//恒流恒压充电控制
    ChargerGetChargeVoltMaxFuc ChargerGetChargeVoltMax;//获取充电机最大充电电压
    ChargerGetChargeVoltMinFuc ChargerGetChargeVoltMin;//获取充电机最小充电电压
    ChargerGetChargeCurMaxFuc ChargerGetChargeCurMax;//获取充电机最大充电电流
    ChargerGetChargeCurMinFuc ChargerGetChargeCurMin;//获取充电机最小充电电流
    ChargerGetPulseCurMaxFuc ChargerGetPulseCurMax;//获取充电机最大脉冲充电电流
    ChargerGetChargedEnergyFuc ChargerGetChargedEnergy;//获取充电机已充电电量
    ChargerGetChgTimeEclipseFuc ChargerGetChgTimeEclipse;//获取充电机已充电时间
    ChargerGetChgTimeRequireFuc ChargerGetChgTimeRequire;//获取充电机剩余充电时间
    ChargerGetChargeCycleFuc ChargerGetChargeCycle;//获取充电次数
    ChargerGetChargeReadyStatusFuc ChargerGetChargeReadyStatus;//获取充电机充电准备就绪状态
    ChargerChargingIsEnableFuc ChargerChargingIsEnable;//获取充电机充电使能状态
    ChargerIsCommunicationFuc ChargerIsCommunication;//获取充电机通信状态
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

#define CHARGER_REC_PDU_BUFF                  	3           //接收PDU单帧数据缓存大小
#define CHARGER_SENT_FRAME_BUFF					3			//发送帧缓存个数
#define CHARGER_REC_FRAME_BUFF					3			//接收帧缓存个数
#define CHARGER_SENT_MESSAGE_LENGTH_MAX			512			//待发送消息的最大长度
#define CHARGER_REC_MESSAGE_LENGTH_MAX        	10          //接收消息的最大长度
#define CHARGER_REC_MESSAGE_BUFF_MAX          	1           //接收消息的最大缓存个数
#define CHARGER_TP_REC_MAX                    	1           //同时可以接收的TP的最大个数

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
