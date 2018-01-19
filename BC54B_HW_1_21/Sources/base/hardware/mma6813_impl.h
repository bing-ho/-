/**
*
* Copyright (c) 2015 Ligoo Inc.
*
* @file mma6813_impl.h
* @brief
* @note
* @author 
* @date 2015-8-18
*
*/
#include "mma6813.h"


#define MMA6813_CS_HIGH()   (DDRH_DDRH3 = 1, PTH_PTH3 = 1)
#define MMA6813_CS_LOW()    (DDRH_DDRH3 = 1, PTH_PTH3 = 0)

#define MMA6813_REG_DEVCTL_ADDR   0x0A
#define MMA6813_REG_DEVCFG_ADDR   0x0B

#define MMA6813_REG_DEVCFG_X_ADDR  0x0C
#define MMA6813_REG_DEVCFG_Y_ADDR  0x0D

#define MMA6813_REG_ARMCFGX_ADDR  0x0E
#define MMA6813_REG_ARMCFGY_ADDR  0x0F

#define MMA6813_REG_ARMT_XP_ADDR  0x10
#define MMA6813_REG_ARMT_YP_ADDR  0x11
#define MMA6813_REG_ARMT_XN_ADDR  0x12
#define MMA6813_REG_ARMT_YN_ADDR  0x13

#define MMA6813_REG_DEVSTAT_ADDR  0x14
#define MMA6813_REG_COUNT_ADDR    0x15
