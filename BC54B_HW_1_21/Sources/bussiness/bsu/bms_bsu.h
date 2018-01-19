/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:bms_bsu.h
 **创建日期:2015.10.15
 **文件说明:电压温度更新到BCU中
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef   _BMS_BSU_H_ 
#define   _BMS_BSU_H_

#include "includes.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

#define BMU_SUPPORT_BSU_MAX_NUM    5


void bsu_init(void);  //初始化

void main_bsu_pos_set(void);
INT8U get_main_bsu_pos(INT8U slave_index);
INT8U bmu_get_max_temp_from_bsu(INT8U slave_index);
INT8U bmu_get_min_temp_from_bsu(INT8U slave_index);
BOOLEAN is_bmu_comm_error_use_bsu(INT8U slave_index);
void bsu_update_bcu_volt_info(void);
void bsu_update_bcu_temp_info(void);
#endif
#endif