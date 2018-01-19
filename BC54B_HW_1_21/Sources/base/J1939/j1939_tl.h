/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
**�� �� ���Sj1939_tl.h                                       
**��    �ߣ�����ΰ
**�������ڣ�2012.05.25
**�޸ļ�¼��
**�ļ�˵��:CANͨ�ű�׼J1939���������ͷ�ļ�
**��    ��:V1.0
**��    ע��
*******************************************************************************/
#ifndef J1939_TL_H_
#define J1939_TL_H_

#ifdef   J1939_TL_GLOBALS
    #define  J1939_TL_EXT
#else
    #define  J1939_TL_EXT  extern
#endif

#define PF_TP                   236
#define PF_DT                   235

#define TP_PRI                  7
#define TP_CMD_RTS              16
#define TP_CMD_CTS              17
#define TP_CMD_EOM              19
#define TP_CMD_ABORT            255
#define TP_CMD_BAM              32

#define STATUS_TP_IDLE          0
#define STATUS_TP_RTS           1
#define STATUS_TP_CTS           2
#define STATUS_TP_DT            3
#define STATUS_TP_EOM           4
#define STATUS_TP_ABORT         5
#define STATUS_TP_FINISH        6
#define STATUS_TP_BAM           7

#define TP_CM_ID_BASE          0x1CEC0000
#define TP_DT_ID_BASE          0x1CEB0000

J1939_TL_EXT J1939Result J1939_tl_with_buff_init(J1939CanContext* _PAGED context);
J1939_TL_EXT void J1939_tl_clear_tp_index(J1939CanTranPro* _PAGED tp_index);
J1939_TL_EXT J1939Result J1939_tl_send_poll(J1939CanContext* _PAGED context, unsigned int elap);
J1939_TL_EXT J1939Result J1939_tl_rec_poll(J1939CanContext* _PAGED context, unsigned int elap);
J1939_TL_EXT void J1939_tl_config_eom(J1939CanFrame* _PAGED frame, J1939CanTranPro* _PAGED tp_index);
J1939_TL_EXT void J1939_tl_config_cts(J1939CanFrame* _PAGED frame, J1939CanTranPro* _PAGED tp_index);
J1939_TL_EXT void J1939_tl_config_abort(J1939CanFrame* _PAGED frame, J1939CanTranPro* _PAGED tp_index);
J1939_TL_EXT void J1939_tl_config_rts(J1939CanFrame* _PAGED frame, J1939CanTranPro* _PAGED tp_index);
void J1939_tl_config_bam(J1939CanFrame* _PAGED frame, J1939CanTranPro* _PAGED tp_index);

#endif
