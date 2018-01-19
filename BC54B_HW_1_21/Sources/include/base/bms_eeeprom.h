/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file bms_eeeprom.h
* @brief 
* @note  
* @author
* @date 2012-5-16  
*  
*/

#ifndef BMS_EEEPROM_H_
#define BMS_EEEPROM_H_

#include "includes.h"
#include "bms_defs.h"
#include "bms_event.h"

#define BMS_EEEPROM_DFPART          36
#define BMS_EEEPROM_PARAM_MAX_NUM   32

typedef enum
{
    kEEEPROMLogTimesFlag = 0x01
}EEEPRomFlag;

typedef struct
{
    INT8U error_code;
}EEEPROMErrorEvent;

//初始化
Result eeeprom_init(INT8U format);
void eeeprom_uninit(void);

/** 按特定特定保存数据到eeeprom */
Result eeeprom_save_int16u(INT16U index, INT16U value, INT8U flag);
Result eeeprom_load_int16u(INT16U index, INT16U* _PAGED value);

/**
 * Check the item, and repair it if the data area is bad.
 * @param index
 * @param default_value
 * @return
 */
Result eeeprom_repair_int16u_item(INT16U index, INT16U default_value);

INT8U eeeprom_wait_ftm_to_idle(void);

#endif /* BMS_EEEPROM_H_ */
