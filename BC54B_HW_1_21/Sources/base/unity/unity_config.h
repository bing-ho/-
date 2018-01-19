#ifndef __UNITY_CONFIG_H__
#define __UNITY_CONFIG_H__

//#undef UNITY_EXCLUDE_STDINT_H
#define UNITY_EXCLUDE_STDINT_H

#undef UNITY_EXCLUDE_FLOAT
//#define UNITY_EXCLUDE_FLOAT


#define UNITY_LONG_WIDTH 16

extern void TERMIO_PutChar(char c);

#include "bms_util.h"
#define strlen safe_strlen
#define strstr safe_strstr
#define strcpy safe_strcpy
#define strcmp safe_strcmp

#define UNITY_OUTPUT_CHAR(a) (void)TERMIO_PutChar(a)

#endif
