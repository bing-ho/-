/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
**文 件 名j1939_dtc.h                                       
**作    者：董丽伟
**创建日期：2013.09.14
**修改记录：
**文件说明:CAN通信标准J1939应用层故障诊断驱动头文件
**版    本:V1.0
**备    注：
*******************************************************************************/
#ifndef J1939_DTC_H_
#define J1939_DTC_H_

#include "os_cpu.h"
#include "bms_defs.h"
#include "bms_list.h"

#define J1939_DTC_SEND_BUFF_MAX 50
#define J1939_DTC_MAX           20
#define J1939_OC_MAX            126

#define J1939_DTC_DM_BROADCAST_INTERVAL 1000

#define J1939_DM1_PRI           6
#define J1939_DM1_PGN           0xFECA

typedef enum
{
    kDTCNone = 0,
    kDTCOn,
    kDTCOff
}DTC_STATUS;

typedef struct
{
    INT32U spn;
    INT8U fmi;
    INT8U cm;
    INT8U oc;
}J1939_DTC_INFO;

typedef struct
{
    LIST_ITEM* _PAGED next;
    DTC_STATUS status;
    INT8U send_flag;
    INT8U oc_last_second;
    INT32U last_tick;
    J1939_DTC_INFO info;
}J1939_DTC_ITEM;

typedef struct _J1939_DTC_CONTEXT
{
    J1939_DTC_ITEM* _PAGED dtc_item_start;
    INT8U is_start;
    INT32U last_tick; //用于1秒周期计时
    INT8U occure_cnt_one_second;
    INT8U release_cnt_one_second;
    INT8U send_failure_flag;
    INT8U dtc_item_cnt;
    INT8U dtc_item_max;
    INT8U send_byte_cnt;
    INT16U send_byte_max;
    J1939_DTC_ITEM* _PAGED dtc_item_buff;
    INT8U* _PAGED send_buff;
}J1939_DTC_CONTEXT;

#define UPDATE_DTC(DTC_INFO, SPN, FMI, OC, CM)  {DTC_INFO.spn = SPN; DTC_INFO.fmi = FMI; DTC_INFO.oc = OC; DTC_INFO.cm = CM;}

#define WRITE_DTC(BUFF, INDEX, MAX, SPN, FMI, OC, CM)    {\
    WRITE_LT_INT8U_WITH_BUFF_SIZE(send_buff.data, index, (INT8U)SPN, send_buff.byte_max);\
    WRITE_LT_INT8U_WITH_BUFF_SIZE(send_buff.data, index, (INT8U)(SPN>>8), send_buff.byte_max);\
    WRITE_LT_INT8U_WITH_BUFF_SIZE(send_buff.data, index, (FMI&0x1F)|(SPN>>16&0x07)<<5, send_buff.byte_max);\
    WRITE_LT_INT8U_WITH_BUFF_SIZE(send_buff.data, index, (OC&0x7F)|CM<<7, send_buff.byte_max);}    

#define WRITE_DTC_NONE(BUFF, INDEX, MAX)        WRITE_DTC(BUFF, INDEX, MAX, 0, 0, 0, 0)

J1939Result j1939_dtc_init(J1939CanContext* _PAGED context);
J1939Result j1939_dtc_trouble_on(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info);
J1939Result j1939_dtc_trouble_off(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info);
J1939Result j1939_dtc_reset_all_dtc(J1939CanContext* _PAGED context);
void j1939_dtc_poll(J1939CanContext* _PAGED context, INT32U tick);

#endif