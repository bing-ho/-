/**
*
* Copyright (c) 2017 Ligoo Inc.
*
* @file detect_diagnosis_impl.c
* @brief
* @note
* @author Han Zhaoxia
* @date 2017-11-1
*
*/
#include "detect_diagnosis_impl.h"
#include "bms_diagnosis_relay.h"
#include "bms_diagnosis_high_voltage.h"
#include "bms_diagnosis_low_voltage.h"
#include "bms_diagnosis_high_temperature.h"
#include "bms_diagnosis_high_total_volt.h"
#include "bms_diagnosis_low_total_volt.h"
#include "bms_diagnosis_communication.h"
#include "bms_diagnosis_volt_line.h"
#include "bms_diagnosis_temp_line.h"
#include "bms_diagnosis_high_current.h"
#include "bms_diagnosis_low_temperature.h"
#include "bms_diagnosis_delta_temperature.h"
#include "bms_diagnosis_insulation.h"
#include "bms_diagnosis_high_soc.h"
#include "bms_diagnosis_low_soc.h"
#include "bms_diagnosis_delta_voltage.h"


#pragma MESSAGE DISABLE C5703 // Parameter 'data' declared in function 'detect_diagnosis_update' but not referenced

void detect_diagnosis_update(void* data);
void detect_relay_diagnose_init(void);
void bms_high_soc_diagnose(void);
void bms_low_soc_diagnose(void);
void bms_chgr_ac_outlet_temp_line_diagnose(void);
void bms_chgr_dc_outlet_temp_line_diagnose(void);


void detect_diagnosis_init(void)
{
    detect_relay_diagnose_init();
    
    job_schedule(MAIN_JOB_GROUP, MAIN_JOB_COMMON_PERIODIC, detect_diagnosis_update, NULL);
}

void detect_diagnosis_update(void* data)
{
    bms_communication_diagnose();//通信中断报警
    
    bms_volt_line_diagnose();//电压异常报警
    
    bms_temp_line_diagnose();//温感异常报警
    
    bms_high_voltage_diagnose();//单体高压报警
    
    bms_low_voltage_diagnose(); //单体低压报警
    
    bms_high_current_diagnose(); //过流报警
    
    bms_high_temperature_diagnose(); //高温报警
    
    bms_low_temperature_diagnose();//低温报警
    
    bms_delta_temperature_diagnose(); //温差报警
    
    bms_insulation_diagnose(); //绝缘报警
    
    bms_high_soc_diagnose(); //SOC高报警
    
    bms_low_soc_diagnose(); //SOC低报警
    
    bms_delta_voltage_diagnose(); //压差报警
    
    bms_high_total_volt_diagnose();//总压高报警
    
    bms_low_total_volt_diagnose(); //总压低报警
    
    bms_chgr_ac_outlet_temp_line_diagnose();
    
    bms_chgr_dc_outlet_temp_line_diagnose();
    
    bms_relay_diagnose(); //继电器控制
}