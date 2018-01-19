/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
**文 件 名j1939_system.c                                       
**作    者：董丽伟
**创建日期：2013.09.14
**修改记录：
**文件说明:CAN通信标准J1939系统驱动源文件
**版    本:V1.0
**备    注：
*******************************************************************************/
#ifndef J1939_SYSTEM_C_
#define J1939_SYSTEM_C_

#include "j1939_cfg.h"

extern INT32U j1939_get_tick_count(void);

extern INT32U j1939_get_elapsed_tick_count(INT32U last_tick);

extern INT32U j1939_get_interval_by_tick(INT32U old_tick, INT32U new_tick);

extern INT32U j1939_get_elapsed_tick_count_with_init(INT32U* _PAGED last_tick);


INT32U j1939_get_tick_count(void)
{
    return get_tick_count();
}

INT32U j1939_get_elapsed_tick_count(INT32U last_tick)
{
    return get_elapsed_tick_count(last_tick);
}

INT32U j1939_get_interval_by_tick(INT32U old_tick, INT32U new_tick)
{
    return get_interval_by_tick(old_tick, new_tick);
}

INT32U j1939_get_elapsed_tick_count_with_init(INT32U* _PAGED last_tick)
{
    return get_elapsed_tick_count_with_init(last_tick);
}

#endif