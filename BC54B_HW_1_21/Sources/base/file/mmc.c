/*------------------------------------------------------------------------/
/  MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2010, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/
#ifndef  MMC_SOURCE
#define  MMC_SOURCE

#include "includes.h"
//#include "bms_platform.h"
#include "diskio.h"
#include "SPI2.H"
#include "SPI0.H"
#include "bms_bcu.h"

#pragma MESSAGE DISABLE C1020
#pragma MESSAGE DISABLE C1140
#pragma MESSAGE DISABLE C1420
#pragma MESSAGE DISABLE C1440
#pragma MESSAGE DISABLE C1825
#pragma MESSAGE DISABLE C1855
#pragma MESSAGE DISABLE C4000
#pragma MESSAGE DISABLE C4001
#pragma MESSAGE DISABLE C4003
#pragma MESSAGE DISABLE C4200
#pragma MESSAGE DISABLE C2705
#pragma MESSAGE DISABLE C12056

#define  _READONLY 0
/* Definitions for MMC/SDC command */
#define CMD0   (0)			/* GO_IDLE_STATE */
#define CMD1   (1)			/* SEND_OP_COND */
#define ACMD41 (41|0x80)	/* SEND_OP_COND (SDC) */
#define CMD8   (8)			/* SEND_IF_COND */
#define CMD9   (9)			/* SEND_CSD */
#define CMD10  (10)			/* SEND_CID */
#define CMD12  (12)			/* STOP_TRANSMISSION */
#define ACMD13 (13|0x80)	/* SD_STATUS (SDC) */
#define CMD16  (16)			/* SET_BLOCKLEN */
#define CMD17  (17)			/* READ_SINGLE_BLOCK */
#define CMD18  (18)			/* READ_MULTIPLE_BLOCK */
#define CMD23  (23)			/* SET_BLOCK_COUNT */
#define ACMD23 (23|0x80)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)			/* WRITE_BLOCK */
#define CMD25  (25)			/* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)			/* SEND_OP_COND (ACMD) */
#define ACMD51 (51|0x80)
#define CMD55  (55)			/* APP_CMD */
#define CMD58  (58)			/* READ_OCR */


/* Port Controls  (Platform dependent) */
//#if BMS_HARDWARE_PLATFORM == BMS_HARDWARE_C32
    #define CS_LOW()  clrReg8Bits(PTH, 128);             /* PTS7=0 */
    #define CS_HIGH() setReg8Bits(PTH, 128);             /* PTS7=1 */
//#else
//    #define CS_LOW()  clrReg8Bits(PTS, 128);             /* PTS7=0 */
//    #define CS_HIGH() setReg8Bits(PTS, 128);             /* PTS7=1 */
//#endif


#define SOCKPORT	PORTB		/* Socket contact port */
#define SOCKWP	(1<<10)		/* Write protect switch (RB10) */
#define SOCKINS	(1<<11)		/* Card detect switch (RB11) */

#define	FCLK_SLOW()			/* Set slow clock (100k-400k) */
#define	FCLK_FAST()			/* Set fast clock (depends on the CSD) */



/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
INT16U Timer1, Timer2;		/* 100Hz decrement timer */

static
INT16U CardType;



/*-----------------------------------------------------------------------*/
/* Exchange a INT8U between PIC and MMC via SPI  (Platform dependent)     */
/*-----------------------------------------------------------------------*/
//#if BMS_HARDWARE_PLATFORM == BMS_HARDWARE_C32
    #define xmit_spi(dat)   SPI0_SendByte(dat)
    #define rcvr_spi()        SPI0_SendByte(0xFF)
    #define rcvr_spi_m(p)   *(p) = SPI0_SendByte(0xFF)
//#else
//    #define xmit_spi(dat)     SPI0_SendByte(dat)
//    #define rcvr_spi()          SPI0_SendByte(0xFF)
//    #define rcvr_spi_m(p) *(p) = SPI0_SendByte(0xFF)
//#endif




//void SDDetectTask (void* pdata)	;					//SD卡检测任务
//OS_STK SDDetectTaskStack[SD_DETECT_STK_SIZE] = {0};	//SD卡检测任务堆栈

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
INT8U wait_ready (void)
{
	INT8U res;


	Timer2 = 50;	/* Wait for ready in timeout of 500ms */
	rcvr_spi();
	do
	{
		res = rcvr_spi();
	}
	while ((res != 0xFF) && Timer2);

	return res;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void deselect (void)
{
	CS_HIGH();
	rcvr_spi();

}



/*-----------------------------------------------------------------------*/
/* Select the card and wait ready                                        */
/*-----------------------------------------------------------------------*/

static
int select (void)	/* 1:Successful, 0:Timeout */
{

	CS_HIGH();

	CS_LOW();
	if (wait_ready() != 0xFF) {
		deselect();
		return 0;
	}
	return 1;
}



/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static
void power_on (void)
{
	//TF_POWER_ON();
}

static void power_off (void)
{
	//TF_POWER_OFF();
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
int rcvr_datablock (	/* 1:OK, 0:Failed */
	INT8U *buff,			/* Data buffer to store received data */
	INT16U btr			/* INT8U count (must be multiple of 4) */
)
{
	INT8U token;


	Timer1 = 10;
	do {							/* Wait for data packet in timeout of 100ms */
		token = rcvr_spi();
	} while ((token == 0xFF) && Timer1);

	if(token != 0xFE) return 0;		/* If not valid data token, retutn with error */

	do {							/* Receive the data block into buffer */
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} while (btr -= 4);
	rcvr_spi();						/* Discard CRC */
	rcvr_spi();

	return 1;						/* Return with success */
}


/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static
int xmit_datablock (	/* 1:OK, 0:Failed */
	const INT8U *buff,	/* 512 INT8U data block to be transmitted */
	INT8U token			/* Data token */
)
{
	INT8U resp;
	INT16U bc = 512;


	if (wait_ready() != 0xFF) return 0;

	xmit_spi(token);		/* Xmit a token */
	if (token != 0xFD) {	/* Not StopTran token */
		do {						/* Xmit the 512 INT8U data block to the MMC */
			xmit_spi(*buff++);
			xmit_spi(*buff++);
		} while (bc -= 2);
		xmit_spi(0xFF);				/* CRC (Dummy) */
		xmit_spi(0xFF);
		resp = rcvr_spi();			/* Receive a data response */
		if ((resp & 0x1F) != 0x05)	/* If not accepted, return with error */
			return 0;
	}

	return 1;
}
#endif	/* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
INT8U send_cmd (
	INT8U cmd,		/* Command INT8U */
	INT32U arg		/* Argument */
)
{
	INT8U n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	deselect();
	if (!select()) return 0xFF;

	/* Send command packet */
	xmit_spi(0x40 | cmd);			/* Start + Command index */
	xmit_spi((INT8U)(arg >> 24));	/* Argument[31..24] */
	xmit_spi((INT8U)(arg >> 16));	/* Argument[23..16] */
	xmit_spi((INT8U)(arg >> 8));		/* Argument[15..8] */
	xmit_spi((INT8U)arg);			/* Argument[7..0] */
	n = 0x01;						/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;		/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;		/* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive command response */
	if (cmd == CMD12) rcvr_spi();	/* Skip a stuff INT8U when stop reading */
	n = 10;							/* Wait for a valid response in timeout of 10 attempts */
	do
		res = rcvr_spi();
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	INT8U drv		/* Physical drive nmuber (0) */
)
{
	INT8U n, cmd, ty, ocr[4] , rc;


	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	power_on();							/* Force socket power on */
	FCLK_SLOW();
	for (n = 10; n; n--) rcvr_spi();	/* 80 dummy clocks */

	ty = 0;
	rc = send_cmd(CMD0, 0);
	if (rc == 1) {			/* Enter Idle state */
		Timer1 = 100;						/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2? */
			for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();			/* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 0x40000000));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (Timer1 && send_cmd(CMD58, 0) == 0) {			/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;	/* SDv2 */
				}
			}
		} else {							/* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
			while (Timer1 && send_cmd(cmd, 0));		/* Wait for leaving idle state */
			if (!Timer1 || send_cmd(CMD16, 512) != 0)	/* Set read/write block length to 512 */
				ty = 0;
		}
	}
	CardType = ty;
	deselect();

	if (ty) {			/* Initialization succeded */
		Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT */
		FCLK_FAST();
	} else {			/* Initialization failed */
		power_off();
	}

//	return Stat;
  return 0;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	INT8U drv		/* Physical drive nmuber (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	INT8U drv,		/* Physical drive nmuber (0) */
	INT8U *buff,		/* Pointer to the data buffer to store read data */
	INT32U sector,	/* Start sector number (LBA) */
	INT8U count		/* Sector count (1..255) */
)
{
	if (drv || !count) return DRES_PARERR;
	if (Stat & STA_NOINIT) return DRES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to INT8U address if needed */

	if (count == 1) {		/* Single block read */
		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
			count = 0;
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	deselect();

	return count ? DRES_ERROR : DRES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
	INT8U drv,				/* Physical drive nmuber (0) */
	const INT8U *buff,		/* Pointer to the data to be written */
	INT32U sector,			/* Start sector number (LBA) */
	INT8U count				/* Sector count (1..255) */
)
{

	if (drv || !count) return DRES_PARERR;
	if (Stat & STA_NOINIT) return DRES_NOTRDY;
	if (Stat & STA_PROTECT) return DRES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to INT8U address if needed */

	if (count == 1) {		/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	deselect();

	return count ? DRES_ERROR : DRES_OK;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	INT8U drv,		/* Physical drive nmuber (0) */
	INT8U ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;
	INT8U n, csd[16], *ptr = buff;
	INT16U csize;


	if (drv) return DRES_PARERR;
	if (Stat & STA_NOINIT) return DRES_NOTRDY;

	res = DRES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC :	/* Flush dirty buffer if present */
			if (select()) {
				deselect();
				res = DRES_OK;
			}
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (INT16U) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDv2? */
					csize = csd[9] + ((INT32U)csd[8] << 8) + 1;
					*(INT32U*)buff = (INT32U)csize << 10;
				} else {					/* SDv1 or MMCv2 */
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((INT16U)csd[7] << 2) + ((INT16U)(csd[6] & 3) << 10) + 1;
					*(INT32U*)buff = (INT32U)csize << (n - 9);
				}
				res = DRES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get sectors on the disk (INT16U) */
			*(INT16U*)buff = 512;
			res = DRES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (DWORD) */
			if (CardType & CT_SD2) {	/* SDv2? */
				if (send_cmd(ACMD13, 0) == 0) {		/* Read SD status */
					rcvr_spi();
					if (rcvr_datablock(csd, 16)) {				/* Read partial block */
						for (n = 64 - 16; n; n--) rcvr_spi();	/* Purge trailing data */
						*(INT32U*)buff = 16UL << (csd[10] >> 4);
						res = DRES_OK;
					}
				}
			} else {					/* SDv1 or MMCv3 */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
					if (CardType & CT_SD1) {	/* SDv1 */
						*(INT32U*)buff = (((csd[10] & 63) << 1) + ((INT16U)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					/* MMCv3 */
						*(INT32U*)buff = ((INT16U)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = DRES_OK;
				}
			}
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 INT8U) */
			*ptr = CardType;
			res = DRES_OK;
			break;

		case MMC_GET_CSD :	/* Receive CSD as a data block (16 bytes) */
			if ((send_cmd(CMD9, 0) == 0)	/* READ_CSD */
				&& rcvr_datablock(buff, 16))
				res = DRES_OK;
			break;

		case MMC_GET_CID :	/* Receive CID as a data block (16 bytes) */
			if ((send_cmd(CMD10, 0) == 0)	/* READ_CID */
				&& rcvr_datablock(buff, 16))
				res = DRES_OK;
			break;

		case MMC_GET_OCR :	/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 0; n < 4; n++)
					*((INT8U*)buff+n) = rcvr_spi();
				res = DRES_OK;
			}
			break;

		case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
			if (send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
				rcvr_spi();
				if (rcvr_datablock(buff, 64))
					res = DRES_OK;
			}
			break;
	    case MMC_GET_SCR:
			if (send_cmd(ACMD51, 0) == 0) {
				rcvr_spi();
				if (rcvr_datablock(buff, 8))
					res = DRES_OK;
			}
	        break;
		default:
			res = DRES_PARERR;
			break;
	}

	deselect();

	return res;
}

int sdcard_is_available(void)
{
    //return 0;
    if(system_voltage_is_valid())
    {
        if(system_voltage_is_usful() == FALSE) return 0;//检查供电电压
    }
    return ((SDWPDetect() | SDLINKDetect() | USB_IS_CONNECTED()) == 0);
}


void sdcard_init(void)		   //SD卡初始化函数
{
    /*USB_CONNECTED_IO_INIT();
    
    TF_POWER_ON();

    if(USB_IS_CONNECTED()) //USB连接中
    {
        TF_CHIP_DISCONNECT();
    }
    else //正常启动SD卡硬件
    {
        TF_CHIP_CONNECT();
    }
    */
    SPI0_Init();
}

/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 1ms                         */

void sdcard_heart_beat()
{
    static INT16U n;
    static INT8U s, p, pv = 0;

    //OSSemPend(FatFsSem,0,&err);	    //锁存SD卡资源

    n = Timer1; /* 1000Hz decrement timer */
    if (n) Timer1 = --n;
    n = Timer2;
    if (n) Timer2 = --n;

    p = pv;
    pv = (SDWPDetect() | (SDLINKDetect() << 1));

    if (p == pv)
    { /* Have contacts stabled? */
        s = Stat;

        if (p & SDWPDetect()) /* WP is H (write protected) */
            s |= STA_PROTECT;
        else
            /* WP is L (write enabled) */
            s &= ~STA_PROTECT;

        if (p & (SDLINKDetect() << 1)) /* INS = H (Socket empty) */
        {
            s |= (STA_NODISK | STA_NOINIT);
        }
        else
            /* INS = L (Card inserted) */
            s &= ~STA_NODISK;

        Stat = s;
    }
    //Stat &= ~STA_NODISK;
    //Stat &= ~STA_PROTECT;

    //OSSemPost(FatFsSem);   //释放SD卡资源
    //OSTimeDly(10);
}



INT8U bcd2dec(INT8U bcd,INT8U m)
{
	INT8U dec = 0;
	dec = bcd &	0x0F;
	dec += ((bcd >> 4) & ((1 << m) - 1)) * 10 ;
	return dec;
}
INT32U get_fattime (void)
{
	DateTime date;
	INT32U time = 0;
	
	bcu_get_system_time(&date);
	time = date.second >> 1;
	time += ((INT16U)date.minute << 5);
	time += ((INT16U)date.hour << 11);
	time += ((INT32U)date.day << 16);
	time += ((INT32U)date.month << 21);
	time += (((INT32U)date.year + 20)<<25);
	return time;

}

#endif
