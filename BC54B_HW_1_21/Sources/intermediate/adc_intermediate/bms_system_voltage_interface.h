         /**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_system_voltage_impl.h
* @brief
* @note
* @author Liwei Dong
* @date 2014-9-23
*
*/

#ifndef BMS_SYSTEM_VOLTAGE_INTERFACE_H_
#define BMS_SYSTEM_VOLTAGE_INTERFACE_H_

#include "bms_defs.h"
#include "app_cfg.h"
#include "ads1013.h"
#include "bms_bcu.h"
#include "bms_job.h"
//#include "adc.h"

#define inline

#define BMS_SYSTEM_VOLTAGE_MIN              9000
#define BMS_SYSTEM_VOLTAGE_MAX              36000
#define BMS_SYSTEM_VOLTAGE_LOW_THRESHOLD    5000

#define BMS_24V_DC_VOLTAGE_MIN              9000
#define BMS_24V_DC_VOLTAGE_MAX              36000
#define BMS_24V_DC_VOLTAGE_LOW_THRESHOLD    5000

#define BMS_BAT_DET_VOLTAGE_MIN             9000
#define BMS_BAT_DET_VOLTAGE_MAX             36000
#define BMS_BAT_DET_VOLTAGE_LOW_THRESHOLD   5000

#define BCU_SYSTEM_VOLTAGE_JOB_PERIODIC     1000

#define BCU_SYSTEM_VOLT_BUFF_MAX            3
#define BCU_BATTERY_V_VOLT_MAGNIFICATION    11//12.8
#define BCU_24V_IN_VOLT_MAGNIFICATION       11

#if BMS_SUPPORT_BY5248D   ==  0 
#define BCU_BATTERY_V_VOLT_OFFSET           300 //mv ����������ѹ��
#else
#define BCU_BATTERY_V_VOLT_OFFSET           400 //mv ����������ѹ��
#endif
#define BCU_SYSTEM_VOLT_AD_PORT             15       
#define BCU_DC_VOLT_AD_PORT_V114            13
#define BCU_DC_VOLT_AD_PORT_V116            12

/***********************************************************************
  * @brief           bms������ѹ����ʼ��
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
void system_voltage_init(void);


/***********************************************************************
  * @brief           ����ϵͳ��ѹֵ
  * @param[in]       ��  
  * @return          ��ѹֵ
***********************************************************************/ 
inline INT16U system_voltage_get(void);

/***********************************************************************
  * @brief           �ж�ϵͳ�����ѹ�Ƿ���Ч
  * @param[in]       ��  
  * @return          1��ok  0��err
***********************************************************************/
INT8U system_voltage_is_usful(void); 

/***********************************************************************
  * @brief           �ж�ϵͳ��ѹ�Ƿ���Ч
  * @param[in]       ��  
  * @return          1��ok  0��err
***********************************************************************/
INT8U system_voltage_is_valid(void);

/***********************************************************************
  * @brief           ��ȡϵͳ�����ѹֵ
  * @param[in]       *__far volt  ϵͳ�����ѹ��ַ
  * @return          1��ok  0��err
***********************************************************************/
INT8U bcu_calc_system_voltage(INT16U *__far volt);

/***********************************************************************
  * @brief           ��ȡϵͳ�����ѹֵ
  * @param[in]       *data  δʹ��
  * @return          ��
***********************************************************************/
void bcu_system_voltage_detect(void* data);//ϵͳ�����ѹ���

/***********************************************************************
  * @brief           ��ȡ24V�����ѹֵ��С
  * @param[in]       ��
  * @return          ���ص�ѹֵ
***********************************************************************/
INT16U dc_24V_voltage_get(void);

/***********************************************************************
  * @brief           �ж�24v��ѹֵ�Ƿ���Ч
  * @param[in]       ��
  * @return          1��ok  0��err
***********************************************************************/
INT8U dc_24V_voltage_is_usful(void);

/***********************************************************************
  * @brief           �ж�24v��ѹֵ�Ƿ���Ч
  * @param[in]       ��
  * @return          1��ok  0��err
***********************************************************************/
INT8U dc_24V_voltage_is_valid(void);

/***********************************************************************
  * @brief           ���ֱ����ѹ���
  * @param[in]       *volt  ���ص�ѹֵ��ַ
  * @return          1��ok  0��err
***********************************************************************/
INT8U bcu_calc_24V_dc_voltage(INT16U *volt);

/***********************************************************************
  * @brief           �������صĵ�ѹֵ
  * @param[in]       ��
  * @return          ��ѹֵ
***********************************************************************/
INT16U bcu_get_lead_acid_volt(void);

/***********************************************************************
  * @brief           ����2.5V��׼��ѹֵ
  * @param[in]       ��
  * @return          ��ѹֵ
***********************************************************************/
INT16U bat_det_voltage_get(void);






#endif /* BMS_SYSTEM_VOLTAGE_IMPL_H_ */
