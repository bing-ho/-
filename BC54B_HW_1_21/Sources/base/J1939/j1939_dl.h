/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
**文 件 名j1939_dl.h                                       
**作    者：董丽伟
**创建日期：2012.05.25
**修改记录：
**文件说明:CAN通信标准J1939数据链路层驱动头文件
**版    本:V1.0
**备    注：
*******************************************************************************/
#ifndef J1939_DL_H_
#define J1939_DL_H_

#ifdef   J1939_DL_GLOBALS
    #define  J1939_DL_EXT
#else
    #define  J1939_DL_EXT  extern
#endif

J1939_DL_EXT J1939Result J1939_dl_with_buff_Init(J1939CanContext* _PAGED context);
J1939_DL_EXT J1939Result J1939_dl_write_buff(J1939CanFrameBuff* _PAGED can_buff, J1939CanFrame* _PAGED msg);
J1939_DL_EXT J1939Result J1939_dl_read_buff(J1939CanFrameBuff* _PAGED can_buff, J1939CanFrame* _PAGED msg);
J1939_DL_EXT J1939Result J1939_dl_read_message(J1939RecMessageBuff* _PAGED source, J1939RecMessage* _PAGED dest);
J1939_DL_EXT J1939Result J1939_dl_write_message(J1939RecMessageBuff* _PAGED dest, J1939RecMessage* _PAGED source);
J1939_DL_EXT void J1939_dl_send_poll(J1939CanContext* _PAGED context);
J1939_DL_EXT unsigned char J1939_dl_get_tp_chain(J1939CanContext* _PAGED context);
J1939_DL_EXT void J1939_dl_put_tp_chain(J1939CanContext* _PAGED context, unsigned char tp);
J1939_DL_EXT unsigned char J1939_dl_search_tp_with_source(J1939CanContext* _PAGED context, unsigned char addr);


#endif
