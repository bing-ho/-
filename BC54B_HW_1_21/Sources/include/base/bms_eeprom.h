#ifndef _BMS_EEPROM_H__
#define _BMS_EEPROM_H__
//#include "os.h"
#include "bms_eeeprom.h"

#ifndef BMS_SUPPORT_EEPROM
#define BMS_SUPPORT_EEPROM      1
#endif


#define EEPROM_INT16_SAVE_SIZE          4
#define BMS_EEPROM_MAX_ADDRESS_OFFSET   2048

#if 0
#define BMS_BOOTLOADER_USE_EEPROM_SIZE  64  //��eeprom�����ǰ��ʹ�õ��ֽ���
#define BMS_BOOTLOADER_ADDRESS_OFFSET   (BMS_EEPROM_MAX_ADDRESS_OFFSET - BMS_BOOTLOADER_USE_EEPROM_SIZE)
#define BMS_BOOTLOADER_START_INDEX      (BMS_BOOTLOADER_ADDRESS_OFFSET / EEPROM_INT16_SAVE_SIZE)

#define BMS_DTU_CONFIG_USE_EEPROM_SIZE  128 // ��EEPROM��BOOTLOADER����Ϣ��ǰʹ��128���ֽ�
#define BMS_DTU_CONFIG_ADDRESS_OFFSET   (BMS_BOOTLOADER_ADDRESS_OFFSET - BMS_DTU_CONFIG_USE_EEPROM_SIZE)
#define BMS_DTU_CONFIG_START_INDEX      (BMS_DTU_CONFIG_ADDRESS_OFFSET / EEPROM_INT16_SAVE_SIZE)
#endif
  
#define EEPROM_SUCCESS	            0x01		    /*eeprom���ʳɹ�*/
#define EEPROM_DATA_ILL	            0x02		    /*eeprom����ʧЧ*/
#define EEPROM_BUSY		            0x03	      /*eeprom�ϴβ���δ����*/
#define EEPROM_ADDR_ILL             0x04		  	/*���ʵ�EEPROM��ַ�Ƿ�*/

#pragma CODE_SEG IEE1_CODE

//��ʼ��
void eeprom_init(void);
void eeprom_uninit(void);

/* */
Result eeprom_wait_ready(void);

/** ֱ�Ӷ�eeprom��д */
Result eeprom_raw_write_int16(INT16U dst, INT16U value);
Result eeprom_raw_read_int16(INT16U dst,INT16U* _PAGED data);


/** ���ض��ض��������ݵ�eeprom */
Result eeprom_save_int16u(INT16U index, INT16U value);
Result eeprom_uds_save_int16u(INT16U index, INT16U value);
Result eeprom_load_int16u(INT16U index, INT16U* _PAGED value);
Result eeprom_uds_load_int16u(INT16U index, INT16U* _PAGED value);
/**
 * Check the item, and repair it if the data area is bad.
 * @param index
 * @param default_value
 * @return
 */
Result eeprom_repair_int16u_item(INT16U index, INT16U default_value);
Result eeprom_uds_repair_int16u_item(INT16U index, INT16U default_value);
//Result eeprom_write_block(INT16U dst, void* data, INT16U size);

//��ȡһ������              

//Result eeprom_read_block(INT16U dst, void* data, INT16U size);

#pragma CODE_SEG DEFAULT


#endif
