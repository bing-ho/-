/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file iee1.h
* @brief 
* @note  
* @author 
* @date 2012-5-15  
*  
*/

#ifndef IEE1_H_
#define IEE1_H_
#include "bms_defs.h"
#include "includes.h"

#pragma DATA_SEG __RPAGE_SEG IEE1_DATA

extern word g_backup_array[128];

#pragma DATA_SEG DEFAULT


//typedef INT16U* IEE1TAddress; // User type for address to the EEPROM */
typedef far word * far IEE1TAddress;
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
typedef far byte * far UDSAddress;
#endif

/* EEPROM area begin address (in the format used by bean methods). */
#define IEE1_AREA_ADRESS_START                 1048576UL
/* EEPROM area last address (in the format used by bean methods). */
#define IEE1_AREA_ADRESS_END                   1056575UL

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
/**UDS相关定义**/
#define EEPROM_UDS_START_ADDRESS   1056768UL         /*UDS写入数据的首地址,以0x10_2000开头*/
#define EEPROM_UDS_END_ADDRESS     1057791UL        /*UDS写入数据的末地址，0x10_23FF*/
#endif
Result iee1_init(void);
Result iee1_set_word(IEE1TAddress Addr, INT16U Data);
Result iee1_get_word(IEE1TAddress addr, INT16U* _PAGED data);
Result iee1_wait_ready(void);
void iee1_backup_sector(IEE1TAddress addr, word from, word to);

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
Result iee1_set_block(IEE1TAddress addr, INT16U* _PAGED data);
Result iee1_get_block(IEE1TAddress addr, INT16U* _PAGED data);
Result iee1_get_word_uds(IEE1TAddress addr, INT16U* _PAGED data);
#endif
#endif /* IEE1_H_ */
