/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_ocv_soc_calibration.h
 * @brief
 * @note
 * @date 2017-11-17
 *
 */

#ifndef BMS_OCV_SOC_BM_H_
#define BMS_OCV_SOC_BM_H_

#include "bms_ocv_soc.h"

INT8U get_delta_battery_cap_sign(void);
void set_delta_battery_cap_sign(INT8U value);


/**
* @brief   �����ۼƱ仯���ⲿ���ʽӿ�
*/
INT32U get_delta_battery_cap_ams(void);

/**
* @brief   ��ʾSOC�ⲿ���ʽӿڣ� ���Ը���
*/
float get_soc_display(void);

/**
* @brief   soc_ah_calib�ⲿ���ʽӿڣ� ���Ը���
*/
float get_soc_ah_calib(void) ;

/**
* @brief   delta_battery_cap_ams�ⲿд�ӿ�
*/
void set_delta_battery_cap_ams(INT32U value);

/**
* @brief   ��ʾSOC��Ӧʣ������ⲿ���ʽӿ�
*/  
INT32U get_left_cap_ams_display(void);

/**
* @brief   left_cap_ams_display�ⲿд�ӿ�
*/ 
void set_left_cap_ams_display(INT32U value);

/**
* @brief   OCVУ׼�� ������soc_bm_high, soc_bm_low, delta_battery_cap_ams
*/ 
void update_soc_bm_by_ocv_soc_table(void);

/**
* @brief   ������ʾSOC��Ӧ��ʣ������
*/
INT32U update_left_cap_ams_display_with_factor(INT32U value, INT8S sign);

/**
* @brief   ������ʼ��
*/
void ocv_soc_calib_params_init(void) ;

/**
* @brief   soc_bm_high�ⲿд�ӿ�
*/ 
void set_soc_bm_high (float value);

/*
* @brief   soc_bm_low�ⲿд�ӿ�
*/ 
void set_soc_bm_low (float value);

/**
* @brief   ���ݵ������� ���ȡsoc_bm_high,�ŵ�ȡsoc_bm_low��Ϊ��׼SOC
*/ 
float get_soc_bm(void);

/**
* @brief   ������ʾSOC��Ӧ��ʣ����������԰�ʱ����SOC��Ӧ��ʣ�������仯����
*/ 
float get_ah_calib_factor(float current_soc);

/**
* @brief   ����bmsinfo�е�ʣ������
*/
void update_left_cap_by_soc_display(void);

/**
* @brief   left_cap_ams_display�洢��Eeeprom��
*/
void save_left_cap_ams_display_to_config(void);

/**
* @brief   delta_battery_cap_ams�洢��Eeeprom��
*/
void save_delta_battery_cap_ams_to_config(void);
#endif /* BMS_OCV_SOC_CALIBRATION_H_ */
