/*
 * j1939_poll.h
 *
 *  Created on: 2012-9-19
 *      Author: Administrator
 */

#ifndef J1939_POLL_H_
#define J1939_POLL_H_

#include "j1939_cfg.h"
#include "bms_upper_computer_j1939.h"
#include "bms_charger_common.h"
#include "bms_dtu.h"

#ifndef BMS_SUPPORT_J1939_COMM
#define BMS_SUPPORT_J1939_COMM      1
#endif

void j1939_poll_task_create(void);

#endif /* J1939_POLL_H_ */
