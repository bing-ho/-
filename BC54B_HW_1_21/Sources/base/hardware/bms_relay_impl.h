/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_relay_impl.h
* @brief
* @note
* @author
* @date 2012-5-24
*
*/

#ifndef BMS_RELAY_IMPL_H_
#define BMS_RELAY_IMPL_H_
#include "bms_relay.h"

#define DIS_CHG_CTL_BIT 1
#define CHG_CTL_BIT     2
#define PRE_CHG_CTL_BIT 4

#define BMS_RELAY_1_SEL     DDRB_DDRB0  //制热继电器
#define BMS_RELAY_1_DAT     PORTB_PB0

#define BMS_RELAY_2_SEL     DDRB_DDRB1 //制冷继电器
#define BMS_RELAY_2_DAT     PORTB_PB1

#define BMS_RELAY_3_SEL     DDRB_DDRB2  //预充电继电器
#define BMS_RELAY_3_DAT     PORTB_PB2

#define BMS_RELAY_4_SEL     DDRB_DDRB3  //放电继电器
#define BMS_RELAY_4_DAT     PORTB_PB3

#define BMS_RELAY_5_SEL     DDRB_DDRB4  //充电继电器
#define BMS_RELAY_5_DAT     PORTB_PB4

#define BMS_RELAY_6_SEL     DDRB_DDRB5  //预留继电器
#define BMS_RELAY_6_DAT     PORTB_PB5

#define BMS_RELAY_7_SEL     DDRB_DDRB6
#define BMS_RELAY_7_DAT     PORTB_PB6

#define BMS_RELAY_8_SEL     DDRB_DDRB7
#define BMS_RELAY_8_DAT     PORTB_PB7


#define BMS_LOAD_RELAY_ON_TIME  1500 //负载继电器闭合耗时 1ms/bit

/*
#define BMS_REG_CHARGE_RELAY            PTP_PTP3
#define BMS_REG_CHARGE_RELAY_SEL        DDRP_DDRP3

#define BMS_REG_PRECHARGE_RELAY         PTP_PTP2
#define BMS_REG_PRECHARGE_RELAY_SEL     DDRP_DDRP2

#define BMS_REG_DISCHARGE_RELAY         PORTB_PB3
#define BMS_REG_DISCHARGE_RELAY_SEL     DDRB_DDRB3

#define BMS_REG_CHARGE_S_RELAY          PORTK_PK6
#define BMS_REG_CHARGE_S_RELAY_SEL      DDRK_DDRK6

#define BMS_REG_DISCHARGE_S_RELAY       PTJ_PTJ2
#define BMS_REG_DISCHARGE_S_RELAY_SEL   DDRJ_DDRJ2
*/
#define RELAY_PENDING_CHECK_CYCLE       50 //ms 继电器等待闭合检查周期

typedef struct
{
    LIST_ITEM* next;
    byte id;
    byte value;
}RelayPendingItem;

void relay_pending_check(void* pdata);
Result relay_pending_list_add(INT8U id);
Result relay_pending_list_remove(INT8U id);

#endif /* BMS_RELAY_IMPL_H_ */
