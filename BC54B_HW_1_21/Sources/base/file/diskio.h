/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file   (C)ChaN, 2013
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#define _USE_WRITE	1	/* 1: Enable disk_write function */
#define _USE_IOCTL	1	/* 1: Enable disk_ioctl fucntion */

#include "integer.h"


/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	DRES_OK = 0,		/* 0: Successful */
	DRES_ERROR,		/* 1: R/W Error */
	DRES_WRPRT,		/* 2: Write Protected */
	DRES_NOTRDY,		/* 3: Not Ready */
	DRES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


/*---------------------------------------*/
/* Prototypes for disk control functions */


DSTATUS disk_initialize (BYTE pdrv);
DSTATUS disk_status (BYTE pdrv);
DRESULT disk_read (BYTE pdrv, BYTE*buff, DWORD sector, UINT count);
DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);


/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl fucntion */

/* Generic command (used by FatFs) */
#define CTRL_SYNC			0	/* Flush disk cache (for write functions) */
#define GET_SECTOR_COUNT	1	/* Get media size (for only f_mkfs()) */
#define GET_SECTOR_SIZE		2	/* Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (for only f_mkfs()) */
#define CTRL_ERASE_SECTOR	4	/* Force erased a block of sectors (for only _USE_ERASE) */

/* Generic command (not used by FatFs) */
#define CTRL_POWER			5	/* Get/Set power status */
#define CTRL_LOCK			6	/* Lock/Unlock media removal */
#define CTRL_EJECT			7	/* Eject media */
#define CTRL_FORMAT			8	/* Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */
#define MMC_GET_SCR         15 

/* ATA/CF specific ioctl command */
#define ATA_GET_REV			20	/* Get F/W revision */
#define ATA_GET_MODEL		21	/* Get model name */
#define ATA_GET_SN			22	/* Get serial number */


/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK	0x08		/* Block addressing */



//#if BMS_HARDWARE_PLATFORM == BMS_HARDWARE_C32
//    #define SDWPDetect()     PORTC_PC7 // ((PORTC&0x04)==0x04)  // PORTC_PC7            //C32
//    #define SDLINKDetect()   PORTC_PC6 // ((PORTC&0x02)==0x02) // PORTC_PC6
//#else
    #define SDWPDetect()    0//((PORTA&0x04)==0x04)
    #define SDLINKDetect()  0//((PORTA&0x02)==0x02)
//#endif
#define TF_POWER_ON()           //(DDR1AD1_DDR1AD14 = 1, PT1AD1_PT1AD14 = 0)
#define TF_POWER_OFF()          //(DDR1AD1_DDR1AD14 = 1, PT1AD1_PT1AD14 = 1)
#define TF_CHIP_CONNECT()       //(DDR1AD1_DDR1AD15 = 1, PT1AD1_PT1AD15 = 1)
#define TF_CHIP_DISCONNECT()    //(DDR1AD1_DDR1AD15 = 1, PT1AD1_PT1AD15 = 0)

#define USB_CONNECTED_IO_INIT() //(ATD1DIENL_IEN6 = 1, DDR1AD1_DDR1AD16 = 0)
#define USB_IS_CONNECTED()      0//PT1AD1_PT1AD16

#define SDIsAvailable() ((SDWPDetect()|SDLINKDetect())==0)

/*---------------------------------------------*/
/* Prototypes for each physical disk functions */

void sdcard_init(void);
void sdcard_heart_beat(void);
int sdcard_is_available(void);

#ifdef __cplusplus
}
#endif

#endif
