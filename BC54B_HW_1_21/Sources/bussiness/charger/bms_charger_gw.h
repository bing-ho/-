/*
 * bms_charger_gw.h
 *
 *  Created on: 2012-10-11
 *      Author: Administrator
 */

#ifndef BMS_CHARGER_GW_H_
#define BMS_CHARGER_GW_H_

#define GUOWANG_CHARGER_CAN_MODE kCanExtendMode

#define GUOWANG_CHARGER_ENABLE_CONTROL_BYTE 1
#define GUOWANG_CHARGER_DISABLE_CONTROL_BYTE 0

#define GUOWANG_CHARGER_BMS_RECEIVE_ID       	0x13D016B7
#define GUOWANG_CHARGER_BMS_RECEIVE_ID_MASK     0x00

#define GUOWANG_TEMP_OFFSET                     40
#define GUOWANG_MESSAGE_SEND_CYCLE              1400

typedef struct
{
    can_t can_context;
    INT16U max_voltage;
    INT16U current;
    INT8U  control_byte;

    INT16U output_voltage;
    INT16S output_current;
    INT16U status;
}GuoWangCharger;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER

extern GuoWangCharger g_guowang_charger;

#pragma DATA_SEG DEFAULT

void guowang_charger_init(void);


#endif /* BMS_CHARGER_GW_H_ */
