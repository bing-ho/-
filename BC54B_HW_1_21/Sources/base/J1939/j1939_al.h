/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
**�� �� ���Sj1939.h                                       
**��    �ߣ�����ΰ
**�������ڣ�2012.05.25
**�޸ļ�¼��
**�ļ�˵��:CANͨ�ű�׼J1939Ӧ�ò�����ͷ�ļ�
**��    ��:V1.0
**��    ע��
*******************************************************************************/
#ifndef J1939_AL_H_
#define J1939_AL_H_

#ifdef J1939_AL_GLOBALS
    #define J1939_AL_EXT
#else
    #define J1939_AL_EXT    extern
#endif

#include "bms_list.h"

typedef unsigned char J1939Result;

#if J1939_OS_EVENT_EN  > 0

typedef OS_EVENT J1939_EVENT;

#define J1939_EVENT_NO_ERR   0

#endif    

#define J1939_BROADCAST_ID  0xFF

#define J1939_GET_PGN(BYTE_HIGH, BYTE_MID, BYTE_LOW) (((INT32U)BYTE_HIGH << 16) + ((INT32U)BYTE_MID << 8) + BYTE_LOW)
#define J1939_GET_PRI(ID)   (((ID) >> 26) & 0x07)
/*
typedef struct
{
    CanDev dev;
    CanInfo info;

    INT32U receive_id;
    INT32U mask_id;

    INT8U read_buffer_count;
    INT8U read_buffer_write_pos;
    INT8U read_buffer_read_pos;
    OS_EVENT* read_buffer_event;

    BOOLEAN     buffers_is_allocated_by_self; // �ڲ��������buffer
} CanContext, *can_t;
*/

#define J1939_OK                    0           //�ɹ�
#define J1939_ERR                   1           //����
#define J1939_ERR_PTR               2           //CAN����ָ����Ч
#define J1939_ERR_FRAME_NUM         3           //TP�д����֡�������
#define J1939_ERR_MSG_PTR           4           //FRAMEָ�����
#define J1939_ERR_MSG_LEN           5           //��Ϣ���ȴ���
#define J1939_ERR_BUFF_FULL         6           //������������
#define J1939_ERR_BUFF_EMPTY        7           //����Ϊ�մ���
#define J1939_ERR_RAM               8           //�ڴ�������
#define J1939_ERR_CTS               9           //TPͨ��CTS���ճ�ʱ����
#define J1939_ERR_EOM               10          //TPͨ��EOM���ճ�ʱ����
#define J1939_ERR_ADDR              11          //���յ����ݵ�ID��PS��ƥ��
#define J1939_ERR_TP_BUSY           12          //����Э�鴫�������ͻ
#define J1939_ERR_SEND_TIMEOUT      13          //��Ϣ���ͳ�ʱ����
#define J1939_ERR_REC_TIMEOUT       14          //��Ϣ���ճ�ʱ����
#define J1939_ERR_ABORT             15          //��Ϣ��ֹ�������

//delay
#define J1939_GET_SEND_TP_TIMEOUT   50          //������Ϣ��ȡ����TP�����ʱʱ��
#define J1939_SEND_MSG_TIMEOUT_DEFAULT  50      //������Ϣ��Ĭ�ϳ�ʱʱ��

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef union
{
    unsigned long value;
    unsigned char data[4];  
}J1939CanId;

typedef struct
{
    J1939CanId id;
    unsigned char byte_cnt;
    unsigned char data[J1939_CAN_FRAME_MAX];   
}J1939CanFrame;

typedef struct
{
    J1939CanFrame* _PAGED buffer;
    unsigned char read_index;
    unsigned char write_index;
    unsigned char buffer_cnt;   
    unsigned char frame_cnt;
}J1939CanFrameBuff;

typedef enum
{
    kNormalType = 0,
    kBroadcastType
}J1939MessageType;

typedef struct
{
    J1939CanId id;
    unsigned long pgn;
    //unsigned char pri;
    unsigned int byte_cnt;
    unsigned int byte_max;
    //unsigned char dest_addr;
    unsigned char* _PAGED data;
    J1939MessageType type;
}J1939SendMessageBuff;

typedef struct
{
    J1939CanId id;
    unsigned long pgn;
    unsigned int byte_cnt;
    unsigned int byte_max;
    unsigned char* _PAGED data;
    J1939MessageType type;
    unsigned char user_data;
}J1939RecMessage;

typedef struct
{
    J1939RecMessage* _PAGED buffer;
    unsigned char buffer_cnt;
}J1939RecTempMessageBuff;

typedef struct
{
    J1939RecMessage* _PAGED buffer;
    unsigned char buffer_cnt;
    unsigned char read_index;
    unsigned char write_index;
    unsigned char message_cnt; 
}J1939RecMessageBuff;

typedef INT16U (*CanOnReceiveFunc)(can_t handle, CanMessage* msg, void* userdata);

typedef struct
{
    LIST_ITEM* _PAGED next;
    INT8U flag;
    INT8U start_addr;
    INT8U stop_addr;
}J1939CanRecMaskGroupItem;

typedef struct
{
    unsigned char dev;
    J1939CanId receive_id;
    J1939CanId mask_id;
    unsigned long bps;      
    unsigned char id_style;                 //CAN֡ģʽ����׼/��չ
    unsigned char addr;
    CanOnReceiveFunc receive_callback; //< the callback handling function
    void* receive_userdata; //< the userdata of the callback function
    J1939CanRecMaskGroupItem rec_mask_group_item;    
}J1939CanInfo;

typedef struct
{
    unsigned long pgn;
    unsigned char frame_cnt;
    unsigned int  byte_cnt;
    unsigned char source_addr;
    unsigned char dest_addr;
    unsigned char status;
    unsigned int delay;
    unsigned char start_frame;
    unsigned char frame_index;
    unsigned char stop_frame;
    unsigned int rec_sts;
}J1939CanTranPro;

typedef struct
{
    J1939CanTranPro tp;
    unsigned char before;
    unsigned char after;   
}J1939TPChain;

typedef struct
{
    J1939TPChain* _PAGED buffer;
    unsigned char buffer_cnt;
    unsigned char tp_cnt;
    unsigned char tp_max;
    unsigned char free_index;   
}J1939CanTPBuff;

typedef struct
{
    J1939_EVENT* _PAGED event;
    unsigned char use_flag;   
}J1939Event;

typedef struct _J1939_DTC_CONTEXT J1939_DTC_CONTEXT;

typedef struct
{
    J1939CanInfo* _PAGED can_info;
    unsigned int heart_beat;
    #if J1939_OS_EVENT_EN > 0
    J1939Event* _PAGED send_event_sem;
    J1939Event* _PAGED receive_event_sem;
    #endif
    J1939CanFrameBuff* _PAGED send_frame_buff;             //����֡����
    J1939CanFrameBuff* _PAGED rec_frame_buff;              //����֡����
    J1939CanTPBuff* _PAGED rec_tp_buff;                    //����Э�鴫�������Ϣ����
    J1939CanTranPro* _PAGED send_tp_index;                 //����Э�鴫�䷢����Ϣ����
    J1939RecTempMessageBuff* _PAGED rec_temp_mess_buff;    //TP������Ϣ��ʱ��Ż���
    J1939CanFrameBuff* _PAGED rec_pdu_buff;                //���յ����ݰ�����
    J1939SendMessageBuff* _PAGED send_message_buff;        //������Ϣ����
    J1939RecMessageBuff* _PAGED rec_message_buff;          //��Ϣ���ջ���
    J1939_DTC_CONTEXT* _PAGED dtc_context;
}J1939CanContext;
/*********************�ֶ�������Ҫ����Ļ��棨�ο���**********************
J1939CanContext g_upperCanContext;
J1939Event g_upperSendEventSem;
J1939Event g_upperRecEventSem;
J1939CanInfo g_upperCanInfo;
J1939CanTranPro g_upperSendTpIndex;
J1939CanTPBuff g_upperRecTpBuffIndex;
J1939TPChain g_upperRecTpBuffs[J1939_TP_REC_MAX];
J1939CanFrameBuff g_upperSendFrameBuffIndex;
J1939CanFrame g_upperSendFrameBuffs[J1939_SENT_FRAME_BUFF];
J1939CanFrameBuff g_upperRecPduBuffIndex;
J1939CanFrame g_upperRecPduBuffs[J1939_REC_PDU_BUFF];
J1939CanFrameBuff g_upperRecFrameBuffIndex;
J1939CanFrame g_upperRecFrameBuffs[J1939_REC_FRAME_BUFF];
J1939SendMessageBuff g_upperSendMessageBuff;
J1939RecTempMessageBuff g_upperRecTempMessageBuffIndex;
J1939RecMessage g_upperRecTempMessageBuffs[J1939_TP_REC_MAX];
J1939RecMessageBuff g_upperRecMessageBuffIndex;
J1939RecMessage g_upperRecMessageBuffs[J1939_REC_MESSAGE_BUFF_MAX];
**************************************************************************/

/*************************�Ի���ָ���ʼ�����ο���*************************
g_upperCanContext.send_event_sem = &g_upperSendEventSem;
g_upperCanContext.send_event_sem->event = J1939CreateSem(1);
g_upperCanContext.receive_event_sem = &g_upperRecEventSem;
g_upperCanContext.receive_event_sem->event = J1939CreateSem(0);
g_upperCanContext.can_info = &g_upperCanInfo;
g_upperCanContext.send_tp_index = &g_upperSendTpIndex;
g_upperCanContext.rec_tp_buff = &g_upperRecTpBuffIndex;
g_upperCanContext.rec_tp_buff->buffer = g_upperRecTpBuffs;
g_upperCanContext.send_frame_buff = &g_upperSendFrameBuffIndex;
g_upperCanContext.send_frame_buff->buffer = g_upperSendFrameBuffs;
g_upperCanContext.rec_pdu_buff = &g_upperRecPduBuffIndex;
g_upperCanContext.rec_pdu_buff->buffer = g_upperRecPduBuffs;
g_upperCanContext.rec_frame_buff = &g_upperRecFrameBuffIndex;
g_upperCanContext.rec_frame_buff->buffer = &g_upperRecFrameBuffs;
g_upperCanContext.send_message_buff = &g_upperSendMessageBuff;
g_upperCanContext.rec_temp_mess_buff = &g_upperRecTempMessageBuffIndex;
g_upperCanContext.rec_temp_mess_buff->buffer = g_upperRecTempMessageBuffs;
g_upperCanContext.rec_message_buff = &g_upperRecMessageBuffIndex;
g_upperCanContext.rec_message_buff->buffer = g_upperRecMessageBuffs;
*****************************************************************************/
J1939_AL_EXT void J1939CanPoll(J1939CanContext* _PAGED context);
J1939_AL_EXT J1939CanContext* J1939Init(J1939CanInfo* _PAGED can_info);
J1939_AL_EXT J1939Result J1939UnInit(J1939CanContext* _PAGED context);
J1939_AL_EXT void J1939CanHeartBeat(J1939CanContext* _PAGED context);
J1939_AL_EXT J1939Result J1939CanSendMessage(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg);
J1939_AL_EXT J1939Result J1939CanSendMessageWithWait(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, unsigned int timeout);
J1939_AL_EXT J1939Result J1939CanSendMessageWithFinish(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED buff, unsigned int timeout);
J1939_AL_EXT J1939Result J1939CanRecMessageWithWait(J1939CanContext* _PAGED context, J1939RecMessage* _PAGED msg, INT16U timeout);
J1939_AL_EXT unsigned int J1939CanGetHeartBeat(J1939CanContext* _PAGED context);
J1939_AL_EXT void J1939CanSetHeartBeat(J1939CanContext* _PAGED context, unsigned int cnt);
J1939_AL_EXT void J1939Sleep(unsigned int time);
J1939_AL_EXT J1939Result J1939MemCopy(INT8U* _PAGED dest, INT8U* _PAGED source, INT16U len);
J1939_AL_EXT J1939Result J1939MemSet(INT8U* _PAGED dest, INT8U val, INT16U len);
J1939_AL_EXT J1939Result J1939CanFrameCopy(J1939CanFrame* _PAGED dest, J1939CanFrame* _PAGED source);
J1939_AL_EXT J1939Result J1939CanSendMessageCopy(J1939SendMessageBuff* _PAGED dest, J1939SendMessageBuff* _PAGED source);
J1939_AL_EXT J1939Result J1939CanRecMessageCopy(J1939RecMessage* _PAGED dest, J1939RecMessage* _PAGED source);
J1939_AL_EXT J1939Result J1939InitWithBuff(J1939CanContext* _PAGED context, J1939CanInfo* _PAGED can_info);

#if J1939_OS_EVENT_EN > 0
J1939_AL_EXT J1939_EVENT* J1939CreateSem(unsigned char data);
J1939_AL_EXT void J1939SendSem(J1939_EVENT* _PAGED sem);
J1939_AL_EXT void J1939PendSem(J1939_EVENT* _PAGED sem, unsigned int timeout, unsigned char* err);
#endif
J1939Result J1939AddReceiveID(J1939CanInfo* _PAGED can_info, INT32U id);
J1939Result J1939RecMessage2CanMessage(J1939RecMessage* _PAGED j1939_msg, CanMessage* _PAGED can_msg);

#endif
