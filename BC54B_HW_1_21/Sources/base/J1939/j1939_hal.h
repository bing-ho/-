/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
**�� �� ���Sj1939_hal.h                                       
**��    �ߣ�����ΰ
**�������ڣ�2012.05.25
**�޸ļ�¼��
**�ļ�˵��:CANͨ�ű�׼J1939Ӳ�����������ͷ�ļ�
**��    ��:V1.0
**��    ע��
*******************************************************************************/
#ifndef J1939_HAL_H_
#define J1939_HAL_H_

#ifdef   J1939_HAL_GLOBALS
    #define  J1939_HAL_EXT
#else
    #define  J1939_HAL_EXT  extern
#endif

J1939_HAL_EXT can_t g_can_info[kCanDevMaxCount];

J1939_HAL_EXT J1939Result J1939HalWithBuffInit(J1939CanContext* _PAGED context);
J1939_HAL_EXT J1939Result J1939CanSendFrame(J1939CanContext* _PAGED j1939_context, J1939CanFrame* _PAGED msg);
J1939_HAL_EXT J1939Result J1939CanRecFrame(J1939CanContext* _PAGED can_info, J1939CanFrame* _PAGED msg);

#endif
