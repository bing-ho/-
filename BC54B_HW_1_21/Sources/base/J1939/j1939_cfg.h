/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
**�� �� ���Sj1939_cfg.h                                       
**��    �ߣ�����ΰ
**�������ڣ�2012.05.25
**�޸ļ�¼��
**�ļ�˵��:CANͨ�ű�׼J1939����ͷ�ļ�
**��    ��:V1.0
**��    ע��
*******************************************************************************/
#ifndef J1939_CFG_H_
#define J1939_CFG_H_

#include  "includes.h"
#include  "bms_can.h"

#ifndef _PAGED
#define _PAGED __far
#endif

#define J1939_BROADCAST_MESSAGE_RECEIVE_SUPPORT     1

#define J1939_PRO_STACK_VERSION             100

#define J1939_OS_EVENT_EN                   1           //J1939�ź�������

#define J1939_CAN_FRAME_MAX                 8           //һ֡CAN���ĵ�������󳤶�
#define J1939_TP_DT_CNT_MAX                 5           //һ������DT�ĸ�������ֵҪС��CAN�����֡���ջ���J1939_REC_FRAME_BUFF

#define J1939_SEND_RETRY_MAX                2           //���ݷ��ʹ������Դ���

#define TP_CTS_REC_DLY                      1250        //����CTS�ĳ�ʱʱ��ֵ
#define TP_DT_SEND_DLY                      2           //����֡������ʱ
#define TP_DT_REC_DLY                       750         //����֡���������
#define TP_EOM_REC_DLY                      1250        //����EOM�ĳ�ʱʱ��ֵ
#define TP_BROADCAST_DT_SEND_DLY            50          //�㲥����֡���

#define J1939_VALUE_DEFAULT                 255

#define J1939_AUTO_MALLOC_EN                0

#include  "j1939_system.h"
#include  "bms_buffer.h"
#include  "j1939_al.h"
#include  "j1939_hal.h"
#include  "j1939_tl.h"
#include  "j1939_dl.h"
#include  "j1939_dtc.h"

#endif
