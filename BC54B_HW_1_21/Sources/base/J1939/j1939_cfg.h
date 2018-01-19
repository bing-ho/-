/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
**文 件 名j1939_cfg.h                                       
**作    者：董丽伟
**创建日期：2012.05.25
**修改记录：
**文件说明:CAN通信标准J1939配置头文件
**版    本:V1.0
**备    注：
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

#define J1939_OS_EVENT_EN                   1           //J1939信号量开关

#define J1939_CAN_FRAME_MAX                 8           //一帧CAN报文的数据最大长度
#define J1939_TP_DT_CNT_MAX                 5           //一次请求DT的个数，此值要小于CAN的最大帧接收缓存J1939_REC_FRAME_BUFF

#define J1939_SEND_RETRY_MAX                2           //数据发送错误重试次数

#define TP_CTS_REC_DLY                      1250        //接收CTS的超时时间值
#define TP_DT_SEND_DLY                      2           //数据帧发送延时
#define TP_DT_REC_DLY                       750         //数据帧接收最大间隔
#define TP_EOM_REC_DLY                      1250        //接收EOM的超时时间值
#define TP_BROADCAST_DT_SEND_DLY            50          //广播数据帧间隔

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
