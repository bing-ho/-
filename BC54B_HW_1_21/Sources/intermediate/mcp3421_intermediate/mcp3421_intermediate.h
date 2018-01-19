

#ifndef __MCP3421_INTERMEDIATE_H__
#define __MCP3421_INTERMEDIATE_H__

#include "mcp3421_hardware.h"
#include "bms_bcu.h"

#define MCP3421_RESULT_ERROR   0xFFFF0000


//#define SAMPLE_BASE_RESISTANCE  1871 // 0.1΢ŷ
#define VALID_CURRENT             0
#define INVALID_CURRENT           1

extern unsigned char Mcp3421_AvrgCurrent_Get(INT16S *avrg_current);
extern void Mcp3421_Intermediate_Init(const Mcp3421_BspInterface *__FAR iface);

#endif
