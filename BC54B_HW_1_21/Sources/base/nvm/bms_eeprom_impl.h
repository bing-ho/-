#ifndef _BMS_EEPROM_IMPL_H__
#include "bms_defs.h"
#include "includes.h"
#include "bms_eeprom.h"
#include "derivative.h" /* include peripheral declarations */
#include "iee1.h"
#include "bms_system.h"
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#include "Dcm.h"
#endif


#if 0
/* Include inherited beans */  

/* Public constants */
//#define IEE1_AREA_START                0x1400U          // Start address of the selected EEPROM address range */
//#define IEE1_AREA_SIZE                 0x0400U          // Size of the selected EEPROM address range */
//#define IEE1_AREA_END                  0x17FFU          // Last address of the selected EEPROM address range */
//#define IEE1_AREA_SECTOR_SIZE          0x08U            // EEPROM area sector size: minimal erasable unit (in bytes). */
/* Deprecated constants */
#define EEPROM                         IEE1_AREA_START
#define IEE1_EEPROMSize                0x0400U          // Size of on-chip EEPROM */
/* TRUE if the bean setting allows clear bits of already programmed flash memory location without destruction of the value in surrounding addresses by sector erase. */
#define IEE1_ALLOW_CLEAR               (TRUE)
/* Size of programming phrase, i.e. number of bytes that must be programmed at once */
#define IEE1_PROGRAMMING_PHRASE        (0x01U)

//Result iee1_init(void);
//Result iee1_set_byte(IEE1TAddress Addr, INT8U Data);
//esult iee1_get_byte(IEE1TAddress addr, INT8U *data);
#endif

#pragma CODE_SEG IEE1_CODE

/** 存储数据并带有验证 */
Result eeprom_save_int16u_with_validation(INT16U addr_base, INT16U index, INT16U value);
Result eeprom_load_int16u_with_validation(INT16U addr_base, INT16U index, INT16U* _PAGED value);

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
/*读取数据操作*/
Result eeprom_load_block_uds(INT16U dst, INT16U* _PAGED data);
Result eeprom_save_block_uds(INT16U dst, INT16U* _PAGED data);
Result eeprom_raw_read_int16_uds(INT16U dst, INT16U* _PAGED data);
#endif
#pragma CODE_SEG DEFAULT_CODE
#endif
