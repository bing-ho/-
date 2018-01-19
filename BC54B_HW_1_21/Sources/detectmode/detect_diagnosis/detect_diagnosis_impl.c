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
    bms_communication_diagnose();//ͨ���жϱ���
    
    bms_volt_line_diagnose();//��ѹ�쳣����
    
    bms_temp_line_diagnose();//�¸��쳣����
    
    bms_high_voltage_diagnose();//�����ѹ����
    
    bms_low_voltage_diagnose(); //�����ѹ����
    
    bms_high_current_diagnose(); //��������
    
    bms_high_temperature_diagnose(); //���±���
    
    bms_low_temperature_diagnose();//���±���
    
    bms_delta_temperature_diagnose(); //�²��
    
    bms_insulation_diagnose(); //��Ե����
    
    bms_high_soc_diagnose(); //SOC�߱���
    
    bms_low_soc_diagnose(); //SOC�ͱ���
    
    bms_delta_voltage_diagnose(); //ѹ���
    
    bms_high_total_volt_diagnose();//��ѹ�߱���
    
    bms_low_total_volt_diagnose(); //��ѹ�ͱ���
    
    bms_chgr_ac_outlet_temp_line_diagnose();
    
    bms_chgr_dc_outlet_temp_line_diagnose();
    
    bms_relay_diagnose(); //�̵�������
}