/**
*
* Copyright (c) 2011 Ligoo Inc. 
* 
* @file     NTC.h
* @brief    
* @note 
* @version 1.00
* @author 
* @date 2011/10/24 
* 
*/


#ifndef _NTC_H
#define _NTC_H
#define NTC_TABLE_LEN   171
#define INVALID_TEMP_VALUE 0xFF


#define NTCTAB_TEMP_MIN_WITH_OFST_40     0
#define NTCTAB_TEMP_MAX_WITH_OFST_40     165

typedef enum
{
    NTCTAB_HFSJ,
    NTCTAB_NCP18XH103,
    NTCTAB_CWF4B103F3950,
    NTCTAB_NTCG163JF103FTB
}NTCTAB_NAME;


extern INT8U TempQuery(INT16U volt , NTCTAB_NAME tbl_name) ;
#endif

/* END SPI.h */