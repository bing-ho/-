#ifndef BMS_DEFS_H__
#define BMS_DEFS_H__

#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include  <math.h>
#include  "os_cpu.h"
#include  "bms_error.h"

#define MAXINT16U      0xFFFF

typedef INT16U Result;
typedef INT16U result_t;
typedef void*  handle_t;

typedef INT32U CapAms_t;

//#define UNUSED(PARAM)

//#define SUPPORT_FAR

typedef void* __far FAR_PVOID;
typedef INT8U* __far FAR_PINT8U;
typedef const INT8U* __far FAR_PCINT8U;
typedef INT16U* __far FAR_PINT16U;
typedef INT32U* __far FAR_PINT32U;
typedef float* __far FAR_PFLOAT;
typedef INT8U* __far * __far FAR_PPINT8U;
typedef const char* __far FAR_PCSTR;
typedef char* __far       FAR_PSTR;

typedef void*   NEAR_PVOID;
typedef INT8U*  NEAR_PINT8U;
typedef const INT8U*  NEAR_PCINT8U;
typedef INT16U* NEAR_PINT16U;
typedef INT32U* NEAR_PINT32U;
typedef float*  NEAR_PFLOAT;
typedef INT8U** NEAR_PPINT8U;
typedef const char* NEAR_PCSTR;
typedef char*   NEAR_PSTR;

#ifdef SUPPORT_FAR
#define PVOID FAR_PVOID
#define PINT8U FAR_PINT8U
#define PCINT8U FAR_PCINT8U
#define PINT16U FAR_PINT16U
#define PINT32U FAR_PINT32U
#define PFLOAT  FAR_PFLOAT
#define PPINT8U FAR_PPINT8U
#define PCSTR   FAR_PCSTR
#define PSTR    FAR_PSTR
#define PMEM    FAR_PMEM
#define PCMEM   FAR_PCMEM
#define _PAGED  __far
#else
#define PVOID NEAR_PVOID
#define PINT8U NEAR_PINT8U
#define PCINT8U NEAR_PCINT8U
#define PINT16U NEAR_PINT16U
#define PINT32U NEAR_PINT32U
#define PFLOAT  NEAR_PFLOAT
#define PPINT8U NEAR_PPINT8U
#define PCSTR   NEAR_PCSTR
#define PSTR    NEAR_PSTR
#define _PAGED
#endif

/*************************************
 *
 * Macros
 *
*************************************/
#define BMS_ASSERT_RETURN(CONDITION, RES) do{if (!(CONDITION)) return RES;}while(0)
#define BMS_ASSERT(CONDITION)

#define INT16U_IS_NEGATIVE(VALUE) (VALUE >= 0x8000)

#define UNUSED(VALUE)   (0 && VALUE)

/*************************************
 *
 * error code
 *
*************************************/
#define OK          0
#define RES_OK      0
#define RES_TRUE    1
#define RES_FALSE   0
#define RES_ERR     1
#define RES_ERROR   1
#define ERR_ERROR   1

/** common error */
#define RES_INVALID_ARG     2
#define RES_INVALID_HANLDE  3
#define ERR_INVALID_ARG     RES_INVALID_ARG
#define ERR_INVALID_HANLDE  RES_INVALID_HANLDE
#define ERR_TIMEOUT         4
#define ERR_NOT_IMPL        5
#define ERR_NOT_SUPPORT     6
#define RES_INVALID_PARAM   RES_INVALID_ARG


#define RES_NO_MEM			4
//#define RES_EXCEED_LIMITED  5
#define RES_NO_DATA			6
#define RES_NOT_FOUND		7
//#define RES_NOT_FOUND_FUNC  8
//#define RES_NOT_FOUND_VAR	9
//#define RES_NOT_FOUND_CONST 10
#define RES_NOT_SUPPORT     ERR_NOT_SUPPORT
#define ERR_OUT_LIMITED     11
//#define ERR_CRC             12

#define RES_WRITER_NOT_OPEN   100
#define RES_READER_NOT_OPEN   101

#define RES_INVALID_CRC     400

//#define RES_NOT_FOUND       900
#define RES_INVALID_FORMAT  900
#define RES_NOT_FOUND_FUNC  901
#define RES_NOT_FOUND_VAR   902
#define RES_NOT_FOUND_CONST 903
#define RES_EXCEED_LIMITED  904
#define RES_OUT_LIMITED     905
#define RES_ERROR_FUNC_PARAM 906

/* frame error */
#define RES_FRAME_INVALID_FORMAT    50
#define RES_FRAME_WRONG_CRC         51

/** eeprom error */
#define ERR_EEPROM_BASE               300
#define ERR_EEPROM_DATA_FAIL          (ERR_EEPROM_BASE + 1) //eeprom数据失效
#define ERR_EEPROM_DATA_ILL           (ERR_EEPROM_BASE + 2)
#define ERR_EEPROM_BUSY               (ERR_EEPROM_BASE + 3) //eeprom上次操作未结束
#define ERR_EEPROM_INVALID_ADDR      (ERR_EEPROM_BASE + 4)

/** J1939 error */
#define ERR_J1939_BASE                  400
#define ERR_J1939_NOT_ENGOUHT_HANDLER  (ERR_J1939_BASE + 3)
/*************************************
 *
 * log
 *
*************************************/
/** log level */
#define DEBUG_LEVEL         0
#define INFO_LEVEL          1
#define WARNING_LEVEL       2
#define ERROR_LEVEL         3

#define DISABLE_LEVEL       99

/** log type */
#define BMS_LOG_NONE        0
#define BMS_LOG_RS485       1
#define BMS_LOG_FILE        2

/*************************************
 *
 * Event
 *
*************************************/
typedef enum
{
    kEEEPROMErrorEvent = 1,
    kInfoEventStart = 10,
    kEventMaxCount
}EventType;

/**********************************************
 *
 * Version
 *
 ***********************************************/
#define VERSION_MAX_LEN 20
typedef struct
{
    char hardware[VERSION_MAX_LEN];
    char name[VERSION_MAX_LEN];
    char time[VERSION_MAX_LEN];
    INT8U major_number;
    INT8U minor_number;
    INT8U revision_number;
    INT32U build_number;
}Version;

#define setReg8(RegName, val)                                    (RegName = (byte)(val))
#define getReg8(RegName)                                         (RegName)

#define setReg8Bit(RegName, BitName)                             (RegName |= RegName##_##BitName##_##MASK)
#define clrReg8Bit(RegName, BitName)                             (RegName &= ~RegName##_##BitName##_##MASK)

#define setReg8Bits(RegName, SetMask)                            (RegName |= (byte)(SetMask))
#define clrSetReg8Bits(RegName, ClrMask, SetMask)                (RegName = (RegName & (~(byte)(ClrMask))) | (byte)(SetMask))
#define clrReg8Bits(RegName, ClrMask)                            (RegName &= ~(byte)(ClrMask))

#define RPAGE_INIT() INT8U _old_rpage
#define RPAGE_SAVE() (_old_rpage = RPAGE)
#define RPAGE_RESTORE() (RPAGE = _old_rpage)


#endif
