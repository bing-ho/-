/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file bms_dtu.h
* @brief 
* @note  
* @author
* @date 2012-5-25  
*  
*/

#ifndef BMS_DTU_H_
#define BMS_DTU_H_

#include "bms_defs.h"
#include "app_cfg.h"
#include "bms_system.h"
#include "bms_buffer.h"
#include "bms_bmu.h"
#include "bms_util.h"
#include "bms_bcu.h"
#include "bms_charger.h"
#include "rs485_hardware.h"
#include "j1939_cfg.h"

/**********************************************
 *
 * Structures
 *
 ***********************************************/

typedef struct _DtuCanContext
{
    J1939CanContext* _PAGED can_cxt;
    INT8U work_start;
    INT8U send_index; //0-主机信息，1~25-从机信息
    INT32U send_interval_tick;
    INT8U start_receive_flag;
    INT32U start_receive_tick;
}DtuCanContext;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU

extern DtuCanContext g_dtuContext;

#pragma DATA_SEG DEFAULT

void dtu_init(void);
void dtu_uninit(void);
void dtu_can_start(void);
void dtu_can_stop(void);
INT8U dtu_can_is_dtu_pgn(INT32U pgn);

BOOLEAN dtu_is_connected(void);



#endif /* BMS_DTU_H_ */
