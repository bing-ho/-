/**
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_adhesion.h
* @brief
* @note
* @date 2017-10-16
*
*/
#ifndef BMS_DIAGNOSIS_ADHESION_H_
#define BMS_DIAGNOSIS_ADHESION_H_

#include "bms_diagnosis_impl.h"
#include "bms_charger_gb.h"
#include "bms_relay.h"
#include "gpio_hardware.h"

extern const struct hcs12_gpio_detail insu_mos_ctrl[];

/*1.23�ܸ�ճ������MOS���ؿ���*/
#define B2MosOn()  hcs12_gpio_set_output(&insu_mos_ctrl[2].io, 1)   //ע������Ǹ߱��������ǵͱ�����
#define B2MosOff() hcs12_gpio_set_output(&insu_mos_ctrl[2].io, 0)

#define INSULATION_TOTAL_VOLTAGE_MIN    30

void bms_adhesion_diagnose(void);
void bms_diagnosis_negative_relay(void);
void bms_update_negative_relay_state(void);
RelayTroubleStatus bms_get_relay_state(RelayControlType type);

#endif