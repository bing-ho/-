/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
**�� �� ���Sj1939_system.h                                       
**��    �ߣ�����ΰ
**�������ڣ�2013.09.14
**�޸ļ�¼��
**�ļ�˵��:CANͨ�ű�׼J1939ϵͳͷ�ļ�
**��    ��:V1.0
**��    ע��
*******************************************************************************/
#ifndef J1939_SYSTEM_H_
#define J1939_SYSTEM_H_

#include "os_cpu.h"
#include "bms_defs.h"

INT32U j1939_get_tick_count(void);

INT32U j1939_get_elapsed_tick_count(INT32U last_tick);

INT32U j1939_get_interval_by_tick(INT32U old_tick, INT32U new_tick);

INT32U j1939_get_elapsed_tick_count_with_init(INT32U* _PAGED last_tick);


#endif