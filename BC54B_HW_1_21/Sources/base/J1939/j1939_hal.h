/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
**文 件 名j1939_hal.h                                       
**作    者：董丽伟
**创建日期：2012.05.25
**修改记录：
**文件说明:CAN通信标准J1939硬件抽象层驱动头文件
**版    本:V1.0
**备    注：
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
