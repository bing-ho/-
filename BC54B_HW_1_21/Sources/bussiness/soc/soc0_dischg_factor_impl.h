/**
*
* Copyright (c) 2016 Ligoo Inc.
*
* @file soc0_dischg_factor_impl.h
* @brief SOC0放电积分因子计算
* @note
* @author
* @date 2012-5-9
*
*/


#ifndef _SOC0_DISCHG_FACTOR_IMPL_H_
#define _SOC0_DISCHG_FACTOR_IMPL_H_

#include "bms_defs.h"
#include "bms_charger_gb.h"
#include "bms_bcu.h"


typedef enum
{
	kSOC0Init = 0, 
	kSOC0Integraling = 1, 
} SOC0IntegralStatus;

INT16U get_soc_from_volt_table(INT16U volt);
void SOC0_IntegralAlgorithm(INT32U integral_time, INT16S current);
void full_chg_set_dischg_factor(void);
INT16U get_dischg_factor(void);

#endif