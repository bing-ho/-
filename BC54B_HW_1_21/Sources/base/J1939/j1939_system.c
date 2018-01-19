/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
**�� �� ���Sj1939_system.c                                       
**��    �ߣ�����ΰ
**�������ڣ�2013.09.14
**�޸ļ�¼��
**�ļ�˵��:CANͨ�ű�׼J1939ϵͳ����Դ�ļ�
**��    ��:V1.0
**��    ע��
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