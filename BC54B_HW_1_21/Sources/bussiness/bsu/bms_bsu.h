/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:bms_bsu.h
 **��������:2015.10.15
 **�ļ�˵��:��ѹ�¶ȸ��µ�BCU��
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef   _BMS_BSU_H_ 
#define   _BMS_BSU_H_

#include "includes.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

#define BMU_SUPPORT_BSU_MAX_NUM    5


void bsu_init(void);  //��ʼ��

void main_bsu_pos_set(void);
INT8U get_main_bsu_pos(INT8U slave_index);
INT8U bmu_get_max_temp_from_bsu(INT8U slave_index);
INT8U bmu_get_min_temp_from_bsu(INT8U slave_index);
BOOLEAN is_bmu_comm_error_use_bsu(INT8U slave_index);
void bsu_update_bcu_volt_info(void);
void bsu_update_bcu_temp_info(void);
#endif
#endif