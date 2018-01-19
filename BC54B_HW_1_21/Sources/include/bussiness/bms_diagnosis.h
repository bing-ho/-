    /**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_impl.h
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-17
*
*/

#ifndef BMS_DIAGNOSIS_H_
#define BMS_DIAGNOSIS_H_

#include "includes.h"
#include "bms_bcu.h"
#include "bms_rule_stock.h"

#define BMS_DIAGNOSIS_CHARGE_CONDITION()            (charger_is_connected())  //认为是充电状态的条件
#define BMS_DIAGNOSIS_AC_CHARGE_CONDITION()         (guobiao_charger_cc_is_connected() || guobiao_charger_pwm_is_connected())
#define BMS_DIAGNOSIS_DC_CHARGE_CONDITION()         (guobiao_charger_cc2_is_connected())

#define BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN    1 //充放电独立单体高压报警使能控制
#define BMS_DIAGNOSIS_LV_INDEPENDENT_WITH_CUR_EN    1 //充放电独立单体低压报警使能控制
#define BMS_DIAGNOSIS_OC_INDEPENDENT_WITH_CUR_EN    1 //充放电独立过流报警全能控制
#define BMS_DIAGNOSIS_HT_INDEPENDENT_WITH_CUR_EN    1 //充放电独立单体高温报警使能控制
#define BMS_DIAGNOSIS_LT_INDEPENDENT_WITH_CUR_EN    1 //充放电独立单体低温报警使能控制
#define BMS_DIAGNOSIS_HDT_INDEPENDENT_WITH_CUR_EN   1 //充放电独立单体温差大报警使能控制
#define BMS_DIAGNOSIS_HDV_INDEPENDENT_WITH_CUR_EN   1 //充放电独立单体压差大报警使能控制
#define BMS_DIAGNOSIS_HTV_INDEPENDENT_WITH_CUR_EN   1 //充放电独立总压高报警使能控制
#define BMS_DIAGNOSIS_LTV_INDEPENDENT_WITH_CUR_EN   1 //充放电独立总压低报警使能控制


void bms_diagnosis_init(void);
void detect_diagnosis_init(void);//hzx 20171228

#endif /* BMS_DIAGNOSIS_IMPL_H_ */
