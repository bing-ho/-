/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file iee2.c
* @brief
* @note
* @author
* @date 2012-5-15
*
*/
#include "iee1.h"

Result iee1_init(void)
{
    FCLKDIV = 7;
    return 0;
}

Result iee1_wait_ready(void)
{
    INT16U timeout = 5000;
    
    while(((FSTAT_MGBUSY != 0) || (FSTAT_CCIF == 0)) && timeout--);
    if(timeout == 0xFFFF) return ERR_BUSY;
    return RES_OK;
}

/*
 ** ===================================================================
 **     Method      :  iee1_backup_sector (bean IntEEPROM)
 **
 **     Description :
 **         The method backups the content of a sector being erased.
 **         This method is internal. It is used by Processor Expert only.
 ** ===================================================================
 */
void iee1_backup_sector(IEE1TAddress addr, word from, word to)
{
    word i;

    for (i = from; i < to; i = i + 2)
    {
        g_backup_array[i / 2] = *(word * far)(addr + i / 2); /* save one sector to RAM */
    }
}

/*
 ** ===================================================================
 **     Method      :  iee1_write_block (bean IntEEPROM)
 **
 **     Description :
 **         The method writes the block of data to EEPROM memory.
 **         This method is internal. It is used by Processor Expert only.
 ** ===================================================================
 */
byte iee1_write_block(IEE1TAddress addr, word from, word to, word* _PAGED data)
{
    byte err = 0;
    word i;
    byte j;
    dword PhraseAddr;

    if (from == to)
    {
        return ERR_OK;
    }
    i = from;
    PhraseAddr = (dword) addr;
    while (i < to)
    {
        /* FSTAT: ACCERR=1,FPVIOL=1 */
        FSTAT = 48; /* Clear error flags */
        FCCOBIX = 0; /* Clear index register */
        FCCOBHI = 17; /* Program D-Flash command */
        FCCOBLO = (byte) (PhraseAddr >> 16); /* High address word */
        FCCOBIX++; /* Shift index register */
        FCCOB = (word) PhraseAddr; /* Low address word */
        for (j = 0; j < 8; j += 2)
        {
            FCCOBIX++; /* Shift index register */
            FCCOB = *(word* _PAGED) (data + i / 2); /* Load new data */
            i += 2;
            if (i >= to)
            {
                break;
            }
        }
        FSTAT = 128; /* Clear flag command buffer empty */
        while (FSTAT_CCIF == 0)
            ; /* Wait to command complete */
        if ((FSTAT_FPVIOL == 1) || (FSTAT_ACCERR == 1))
        { /* Is protection violation or acces error detected ? */
            return ERR_NOTAVAIL; /* If yes then error */
        }
        if (FSTAT_MGSTAT)
        { /* Was attempt to write data to the given address errorneous? */
            err = 1; /* If yes then mark an error */
        }
        PhraseAddr += 8;
    }
    if (err)
    {
        return ERR_VALUE; /* If yes then error */
    }
    return ERR_OK; /* OK */
}

/*
 ** ===================================================================
 **     Method      :  iee1_erase_sector_internal (bean IntEEPROM)
 **
 **     Description :
 **         The method erase a specific sector.
 **         This method is internal. It is used by Processor Expert only.
 ** ===================================================================
 */
byte iee1_erase_sector_internal(IEE1TAddress addr)
{
    if (FSTAT_CCIF == 0) /* Is command complete ? */
    {
        return ERR_BUSY; /* If yes then error */
    }
    /* FSTAT: ACCERR=1,FPVIOL=1 */
    FSTAT = 48; /* Clear error flags */
    FCCOBIX = 0; /* Clear index register */
    FCCOBHI = 18; /* Erase D-Flash sector command */
    FCCOBLO = (byte) (((dword) addr) >> 16); /* High address word */
    FCCOBIX++; /* Shift index register */
    FCCOB = (word) (((dword) addr) & 4294967294L); /* Low address word aligned to word*/
    FSTAT = 128; /* Clear flag command buffer empty */
    while (FSTAT_CCIF == 0)
        ; /* Wait to command complete */
    if (FSTAT_ACCERR || FSTAT_MGSTAT)
    { /* Is access error or other error detected ? */
        return ERR_NOTAVAIL; /* If yes then error */
    }
    return ERR_OK; /* OK */
}
/*
 ** ===================================================================
 **     Method      :  iee1_write_word (bean IntEEPROM)
 **
 **     Description :
 **         The method writes the word data to EEPROM memory.
 **         This method is internal. It is used by Processor Expert only.
 ** ===================================================================
 */
static byte iee1_write_word(IEE1TAddress addr_row, word data16)
{
    if (FSTAT_CCIF == 0)
    { /* Is previous command in process ? */
        return ERR_BUSY; /* If yes then error */
    }
    /* FSTAT: ACCERR=1,FPVIOL=1 */
    FSTAT = 48; /* Clear error flags */
    FCCOBIX = 0; /* Clear index register */
    FCCOBHI = 17; /* Program D-Flash command */
    FCCOBLO = (byte) (((dword) addr_row) >> 16); /* High address word */
    FCCOBIX++; /* Shift index register */
    FCCOB = (word) ((dword) addr_row); /* Low address word */
    FCCOBIX++; /* Shift index register */
    FCCOB = data16; /* Load new data */
    FSTAT = 128; /* Clear flag command complete */
    if ((FSTAT_FPVIOL == 1) || (FSTAT_ACCERR == 1))
    { /* Is protection violation or acces error detected ? */
        return ERR_NOTAVAIL; /* If yes then error */
    }
    while (!FSTAT_CCIF)
    {
    } /* Wait for command completition */
    if (FSTAT_MGSTAT)
    { /* Was attempt to write data to the given address errorneous? */
        return ERR_VALUE; /* If yes then error */
    }
    return ERR_OK;
}


/*
** ===================================================================
**     Method      :  IEE1_SetByte (bean IntEEPROM)
**
**     Description :
**         This method writes a given byte to a specified address in
**         EEPROM. The method also sets address pointer for <SetActByte>
**         and <GetActByte> methods (applicable only if these methods
**         are enabled). The pointer is set to address passed as the
**         parameter.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Addr            - Address to EEPROM
**         Data            - Data to write
**     Returns     :
**         ---             - Error code, possible codes:
**                           - ERR_OK - OK
**                           - ERR_SPEED - the bean does not work in the
**                           active speed mode
**                           - ERR_BUSY - device is busy
**                           - ERR_VALUE - verification of written data
**                           failed (read value does not match with
**                           written value)
**                           - ERR_NOTAVAIL - other device-specific
**                           error
**                           - ERR_RANGE - parameter Addr is out of range
** ===================================================================
*/
Result iee1_set_word(IEE1TAddress addr, INT16U data)
{
    INT8U err;

    IEE1TAddress Secaddr; /* EEPROM Sector address */

    if((addr < (IEE1TAddress)IEE1_AREA_ADRESS_START) || (addr > (IEE1TAddress)IEE1_AREA_ADRESS_END))
    { /* Is given address out of EEPROM area array ? */
        return ERR_RANGE; /* If yes then error */
    }
    if(!FSTAT_CCIF)
    { /* Is reading from EEPROM possible? */
        return ERR_BUSY; /* If no then error */
    }
    if (*(addr) == 65535)
    { /* Is the word erased? */
        Secaddr = (IEE1TAddress)((INT32U)addr & 16777214); /* Aligned word address */
        return(iee1_write_word(Secaddr, data));
    }
    else
    { /* Is given address non-erased ? */
        Secaddr = (IEE1TAddress)((INT32U)addr & 16776960); /* Sector Aligned address */
        iee1_backup_sector(Secaddr, 0, 256); /* Backup sector */
        g_backup_array[(((INT32U)addr) % 256) / 2] = data; /* Write new data to saved sector */
        err = iee1_erase_sector_internal(addr); /* Erase sector */
        if(err)
        {
            return(err); /* Return error code if previous operation finished not correctly */
        }
        err = iee1_write_block(Secaddr, 0, 256, g_backup_array); /* Restore sector */
        return(err);
    }
}

/*
** ===================================================================
**     Method      :  IEE1_GetWord (bean IntEEPROM)
**
**     Description :
**         This method reads a word from a specified EEPROM address.
**         The method also sets address pointer for <SetActByte> and
**         <GetActByte> methods (applicable only if these methods are
**         enabled). The pointer is set to address passed as the
**         parameter.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Addr            - EEPROM Address
**       * Data            - A pointer to the returned 16-bit data
**     Returns     :
**         ---             - Error code, possible codes:
**                           - ERR_OK - OK
**                           - ERR_BUSY - device is busy
**                           - ERR_RANGE - parameter Addr is out of range
** ===================================================================
*/
Result iee1_get_word(IEE1TAddress addr, INT16U* _PAGED data)
{
    if((addr < (IEE1TAddress)IEE1_AREA_ADRESS_START) || (addr > (IEE1TAddress)IEE1_AREA_ADRESS_END))
    { /* Is given address out of EEPROM area array ? */
        return ERR_RANGE;                   /* If yes then error */
    }
    if(!FSTAT_CCIF)
    {                                       /* Is reading from EEPROM possible? */
        return ERR_BUSY;                    /* If no then error */
    }
    *data = *addr;
    return ERR_OK;                          /* OK */
}


#if( SWITCH_ON == UDS_SWITCH_CONFIG)

/*
** ===================================================================
**     Method      :  IEE1_Setblock (bean IntEEPROM)
**
**     Description :
**         This method writes 256 bytes to a specified address in
**         EEPROM. The method also sets address pointer for <SetActByte>
**         and <GetActByte> methods (applicable only if these methods
**         are enabled). The pointer is set to address passed as the
**         parameter.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Addr            - Address to EEPROM
**       * Data            - A pointer to the 256 bytes data
**     Returns     :
**         ---             - Error code, possible codes:
**                           - ERR_OK - OK
**                           - ERR_SPEED - the bean does not work in the
**                           active speed mode
**                           - ERR_BUSY - device is busy
**                           - ERR_VALUE - verification of written data
**                           failed (read value does not match with
**                           written value)
**                           - ERR_NOTAVAIL - other device-specific
**                           error
**                           - ERR_RANGE - parameter Addr is out of range
** ===================================================================
*/

Result iee1_set_block(IEE1TAddress addr, INT16U* _PAGED data)
{
    INT8U err;

    IEE1TAddress Secaddr; 

    if((addr < (IEE1TAddress)EEPROM_UDS_START_ADDRESS) || (addr > (IEE1TAddress)EEPROM_UDS_END_ADDRESS))
    { 
        return ERR_RANGE; 
    }
    if(!FSTAT_CCIF)
    { 
        return ERR_BUSY; 
    }

    
      Secaddr = (IEE1TAddress)((INT32U)addr & 16776960); 

      err = iee1_erase_sector_internal(addr); 
      if(err)
      {
          return(err);
      }
      err = iee1_write_block(Secaddr, 0, 256, data); 
      return(err);
    
}  

/*
** ===================================================================
**     Method      :  IEE1_Getblock (bean IntEEPROM)
**
**     Description :
**         This method reads 256 bytes from a specified EEPROM address.
**         The method also sets address pointer for <SetActByte> and
**         <GetActByte> methods (applicable only if these methods are
**         enabled). The pointer is set to address passed as the
**         parameter.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Addr            - EEPROM Address
**       * Data            - A pointer to the returned 256 bytes data
**     Returns     :
**         ---             - Error code, possible codes:
**                           - ERR_OK - OK
**                           - ERR_BUSY - device is busy
**                           - ERR_RANGE - parameter Addr is out of range
** ===================================================================
*/


Result iee1_get_block(IEE1TAddress addr, INT16U* _PAGED data)
{
    INT8U  i;
    if((addr < (IEE1TAddress)EEPROM_UDS_START_ADDRESS) || (addr > (IEE1TAddress)EEPROM_UDS_END_ADDRESS))
    { 
        return ERR_RANGE;                 
    }
    if(!FSTAT_CCIF)
    {                                      
        return ERR_BUSY;                   
    }
    for(i=0;i<128;i++)
    {
     *data = *addr; 
     data++;
     addr++;   
    }
    
    return ERR_OK;                         
}
 
/*
** ===================================================================
**     Method      :  IEE1_GetWord (bean IntEEPROM)
**
**     Description :
**         This method reads a word from a specified EEPROM address.
**         The method also sets address pointer for <SetActByte> and
**         <GetActByte> methods (applicable only if these methods are
**         enabled). The pointer is set to address passed as the
**         parameter.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Addr            - EEPROM Address
**       * Data            - A pointer to the returned 16-bit data
**     Returns     :
**         ---             - Error code, possible codes:
**                           - ERR_OK - OK
**                           - ERR_BUSY - device is busy
**                           - ERR_RANGE - parameter Addr is out of range
** ===================================================================
*/
Result iee1_get_word_uds(IEE1TAddress addr, INT16U* _PAGED data)
{
    if((addr < (IEE1TAddress)EEPROM_UDS_START_ADDRESS) || (addr > (IEE1TAddress)EEPROM_UDS_END_ADDRESS))
    { /* Is given address out of EEPROM area array ? */
        return ERR_RANGE;                   /* If yes then error */
    }
    if(!FSTAT_CCIF)
    {                                       /* Is reading from EEPROM possible? */
        return ERR_BUSY;                    /* If no then error */
    }
    *data = *addr;
    return ERR_OK;                          /* OK */
}
#endif