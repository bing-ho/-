/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_base_cfg.h
* @brief
* @note
* @author
* @date 2012-5-7
*
*/

#ifndef BMS_BASE_CFG_H_
#define BMS_BASE_CFG_H_
#include "bms_defs.h"
//#include "bms_platform.h"

#define BMS_CLOCK_IIC   1
#define BMS_CLOCK_I2C   2

/**********************************************
 *
 * Base Module Support
 *
 ***********************************************/
#define BMS_SUPPORT_LIST        1
#define BMS_SUPPORT_EVENT       1
#define BMS_SUPPORT_TIMER       1
#define BMS_SUPPORT_CAN         1
#define BMS_SUPPORT_CLOCK       BMS_CLOCK_IIC
#define BMS_SUPPORT_CURRENT     1
#define BMS_SUPPORT_EEPROM      1
#define BMS_SUPPORT_FRAMER      1
#define BMS_SUPPORT_LOG         1
#define BMS_SUPPORT_MODBUS      1
#define BMS_SUPPORT_RS485       1
#define BMS_SUPPORT_SLAVE_NET   1
#define BMS_SUPPORT_SYSTEM      1
#define BMS_SUPPORT_TIMER       1
#define BMS_SUPPORT_UNITTEST    1
#define BMS_SUPPORT_UTIL        1
#define BMS_SUPPORT_FIX_MEMORY  1 // if enabled, the memory called by malloc() is allocated by the fix memory,
                                  // it cannot be free again.

/**********************************************
 *
 * Memory Setting
 *
 ***********************************************/
#define BMS_MALLOC_FIX_MEMORY_SIZE   508

/**********************************************
 *
 * EVENT Module Setting
 *
 ***********************************************/
/** 可以支持最大多少事件*/
#define BMS_EVENT_HANDLER_MAX_COUNT     100
#define BMS_EVENT_TYPE_MAX_COUNT        100


/**********************************************
 *
 * Log Setting
 *
 ***********************************************/
#define BMS_LOG_LEVEL DISABLE_LEVEL // DEBUG_LEVEL
#define BMS_LOG_TYPE  BMS_LOG_NONE // BMS_LOG_RS485

/** RS485 Log设置 */
#define BMS_LOG_RS485_DEV   kRs485Dev1
#define BMS_LOG_RS485_BAND  9600

/**********************************************
 *
 * EEPROM Setting
 *
 ***********************************************/
#define EEPROM_INT16_SAVE_SIZE          4
#define BMS_BOOTLOADER_USE_EEPROM_SIZE  64  //从eeprom最后往前算使用的字节数
#define BMS_EEPROM_MAX_ADDRESS_OFFSET   2048

/**********************************************
 *
 * EEEPROM Setting
 *
 ***********************************************/
#define BMS_EEEPROM_DFPART 36
#define BMS_EEEPROM_PARAM_MAX_NUM   32

/**********************************************
 *
 * LED Setting
 *
 ***********************************************/
/** LED灯的个数 */
#define BMS_LED_NUM     3


/**********************************************
 *
 * Relay Setting
 *
 ***********************************************/
#define BMS_RELAY_LOAD_RELAY_ON_MIN_INTERVAL 150

/**********************************************
 *
 * Job Setting
 *
 ***********************************************/
#define BMS_MAX_JOB_NUM         23
#define BMS_MAX_JOB_GROUP_NUM   4


/**********************************************
 *
 * MCU Setting
 *
 ***********************************************/
#define BMS_BUS_CLOCK 48000000UL


#endif /* BMS_BASE_CFG_H_ */
