/**
 *
 * Copyright (c) 2014 Ligoo Inc.
 *
 * @file  bms_i2c.h
 * @brief ads8317��SPI����ͷ�ļ�
 * @note 
 * @version 1.00
 * @author
 * @date 2014/9/17
 *
 */
#ifndef _ADS8317_H
#define _ADS8317_H

#include "includes.h"
//#include "SPI1.h"
#include "bms_base_cfg.h"

void ads8317_init(void);
INT16U ads8317_read(void);

#endif