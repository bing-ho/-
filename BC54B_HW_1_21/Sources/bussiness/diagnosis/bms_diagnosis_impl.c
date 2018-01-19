/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_impl.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-17
*
*/
#include "bms_diagnosis_impl.h"

#pragma MESSAGE DISABLE C5703 // Parameter 'data' declared in function 'bms_diagnosis_update' but not referenced

void bms_communication_diagnose(void);
void bms_diagnosis_update(void* data);
void bms_volt_line_diagnose(void);
void bms_temp_line_diagnose(void);
void bms_high_voltage_diagnose(void);
void bms_low_voltage_diagnose(void);
void bms_high_current_diagnose(void);
void bms_high_temperature_diagnose(void);
void bms_low_temperature_diagnose(void);
void bms_delta_temperature_diagnose(void);
void bms_insulation_diagnose(void);
void bms_high_soc_diagnose(void);
void bms_low_soc_diagnose(void);
void bms_delta_voltage_diagnose(void);
void bms_high_total_volt_diagnose(void);
void bms_low_total_volt_diagnose(void);
void bms_relay_diagnose_init(void);
void bms_relay_diagnose(void);
void bms_chgr_ac_outlet_temp_line_diagnose(void);
void bms_chgr_dc_outlet_temp_line_diagnose(void);
void bms_adhesion_diagnose(void);

void bms_diagnosis_init(void)
{
    bms_relay_diagnose_init();
    
    job_schedule(MAIN_JOB_GROUP, MAIN_JOB_COMMON_PERIODIC, bms_diagnosis_update, NULL);
}

void bms_diagnosis_update(void* data)
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
 
    bms_adhesion_diagnose(); //�̵���ճ�����
}