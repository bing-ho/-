/*******************************************************************************
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2005 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
********************************************************************************
Services performed by FREESCALE in this matter are performed AS IS and without
any warranty. CUSTOMER retains the final decision relative to the total design
and functionality of the end product. FREESCALE neither guarantees nor will be
held liable by CUSTOMER for the success of this project.
FREESCALE DISCLAIMS ALL WARRANTIES, EXPRESSED, IMPLIED OR STATUTORY INCLUDING,
BUT NOT LIMITED TO, IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
A PARTICULAR PURPOSE ON ANY HARDWARE, SOFTWARE OR ADVISE SUPPLIED TO THE PROJECT
BY FREESCALE, AND OR NAY PRODUCT RESULTING FROM FREESCALE SERVICES. IN NO EVENT
SHALL FREESCALE BE LIABLE FOR INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF
THIS AGREEMENT.

CUSTOMER agrees to hold FREESCALE harmless against any and all claims demands or
actions by anyone on account of any damage, or injury, whether commercial,
contractual, or tortuous, rising directly or indirectly as a result of an advise
or assistance supplied CUSTOMER in connection with product, services or goods
supplied under this Agreement.
********************************************************************************
* File      PFlash.c
* Owner     b01802
* Version   1.0
* Date      Dec-02-2010
* Classification   General Business Information
* Brief     Program and Erase functions
********************************************************************************
Revision History:
Version   Date          Author    Description of Changes
1.0       Dec-02-2010   b01802    Initial version
*******************************************************************************/
#include  "derivative.h"
#include "Types.h"
#include "Regdef.h"
#include "PFlash.h"
#include "Errors.h"
//#include "MSCAN.h"

#define FLASH_SECTOR_SIZE   0x400

#define __SEG_START_REF(a)  __SEG_START_ ## a
#define __SEG_END_REF(a)    __SEG_END_ ## a
#define __SEG_SIZE_REF(a)   __SEG_SIZE_ ## a
#define __SEG_START_DEF(a)  extern INT8 __SEG_START_REF (a) []
#define __SEG_END_DEF(a)    extern INT8 __SEG_END_REF   (a) []
#define __SEG_SIZE_DEF(a)   extern INT8 __SEG_SIZE_REF  (a) []

__SEG_START_DEF (RAM_CODE);
__SEG_END_DEF   (RAM_CODE);
__SEG_SIZE_DEF  (RAM_CODE);

/******************************************************************************/
#pragma push
#pragma CODE_SEG RAM_CODE
UINT8 PFlash_Program(UINT32 address, UINT16 *ptr,UINT8 length)
{                               //length 为新加参数,指定要编程的数据长度,因为记录长度不一样

  UINT8 i;
  UINT8 j = length/2;

  while((FSTAT & FSTAT_CCIF_MASK) == 0);  //wait if command in progress
  FSTAT = 0x30;       //clear ACCERR and PVIOL

  FCCOBIX = 0x00;
  FCCOB = 0x0600 | ((address & 0x007F0000)>>16);

  FCCOBIX = 0x01;
  FCCOB = (address & 0x0000FFFF);

  for(i=2; i<2+j; i++)  //fill data (4 words) to FCCOB register
  {
    FCCOBIX = i;
    FCCOB = *ptr;
    ptr++;
  }

  FSTAT = 0x80;       //launch command
  while((FSTAT & FSTAT_CCIF_MASK) == 0);  //wait for done

  if((FSTAT & (FSTAT_ACCERR_MASK | FSTAT_FPVIOL_MASK)) != 0)
  {

    return FlashProgramError;
  }
  else
   {

     return noErr;
   }
}

/******************************************************************************/
UINT8 PFlash_EraseSector(UINT32 address)
{
  while((FSTAT & FSTAT_CCIF_MASK) == 0);  //wait if command in progress
  FSTAT = 0x30;       //clear ACCERR and PVIOL

  FCCOBIX = 0x00;
  FCCOB = 0x0A00 | ((address & 0x007F0000)>>16);

  FCCOBIX = 0x01;
  FCCOB = (address & 0x0000FFF8);

  FSTAT = 0x80;       //launch command
  while((FSTAT & FSTAT_CCIF_MASK) == 0);  //wait for done

  if((FSTAT & (FSTAT_ACCERR_MASK | FSTAT_FPVIOL_MASK)) != 0)
    return FlashEraseError;
  else
    return noErr;
}

/******************************************************************************/
UINT8 PFlash_EraseSectorBySector(UINT32 addr_l, UINT32 addr_h)
{
  UINT32 Address;
  UINT8 Error;

  for(Address = addr_l; Address < addr_h; Address += FLASH_SECTOR_SIZE)
  {
    Error = PFlash_EraseSector(Address);
    if(Error != noErr)
      return(Error);
  }
  return(noErr);
}

/******************************************************************************/
UINT8 PFlash_EraseBlock(UINT32 address)
{
  volatile int fprot = FPROT;
  while((FSTAT & FSTAT_CCIF_MASK) == 0);  //wait if command in progress
  FSTAT = 0x30;       //clear ACCERR and PVIOL

  FCCOBIX = 0x00;
  FCCOB = 0x0900 | ((address & 0x007F0000)>>16);

  FCCOBIX = 0x01;
  FCCOB = (address & 0x0000FFF8);

  FSTAT = 0x80;       //launch command
  while((FSTAT & FSTAT_CCIF_MASK) == 0);  //wait for done

  if((FSTAT & (FSTAT_ACCERR_MASK | FSTAT_FPVIOL_MASK)) != 0)
    return FlashEraseError;
  else
    return noErr;
}
#pragma pop
/******************************************************************************/
