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

#ifndef __BMS_BOARD_TEMPERATURE_INTERFACE_H__
#define __BMS_BOARD_TEMPERATURE_INTERFACE_H__

#include "bms_defs.h"
#include "app_cfg.h"
//#include "adc.h"
#include "bms_job.h"
#include "NTC.h"
#include "bms_util.h"
#include "bms_bmu.h"

#define inline

#define NCP18XH103_MAX          34
#define NCP18XH103_TEMP_FACTOR  5 //�¶ȷֱ���Ϊ5
#define NCP18XH103_TEMP_OFFSET  -40
#define NCP18XH103_TEMP_MIN     -40
#define NCP18XH103_TEMP_MAX     125

#define BCU_BOARD_TEMPERATURE_JOB_PERIODIC  1000

#define BCU_BOARD_TEMPERATURE_AD_CHANNEL    0 //BCU�����¶ȼ��ADͨ��
#define BCU_BOARD_TEMPERATURE_BUFF_MAX      4

#define BCU_BOARD_TEMPERATURE2_AD_CHANNEL    12 //BCU�����¶ȼ��ADͨ��
#define BCU_BOARD_TEMPERATURE2_BUFF_MAX      4


/***********************************************************************
  * @brief           �����¶ȳ�ʼ������
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
void board_temperature_init(void);

/***********************************************************************
  * @brief           ��ȡ�����¶�1��С
  * @param[in]       ��  
  * @return          �¶�ֵ
***********************************************************************/ 
inline INT8U board_temperature_get(void); 

/***********************************************************************
  * @brief           ��ȡ�����¶�2��С
  * @param[in]       ��  
  * @return          �¶�ֵ
***********************************************************************/ 
inline INT8U board_temperature2_get(void); 

/***********************************************************************
  * @brief           ��ȡ�����¶�ֵ
  * @param[in]       which   �¶����  
  * @param[in]       *temp   �����¶�ֵ 
  * @return          1��ok  0��err
***********************************************************************/ 
INT8U bcu_calc_board_temperature(INT8U which, INT8U *temp);

/***********************************************************************
  * @brief           ��ȡ�����¶�ֵ
  * @param[in]       data   δʹ��  
  * @return          ��
***********************************************************************/ 
void bcu_board_temperature_detect(void* data);

/***********************************************************************
  * @brief           ������Ч�¶ȵ�ƽ��ֵ
  * @param[in]       ��  
  * @return          �¶�ֵ
***********************************************************************/ 
INT8U board_average_temperature_get(void);



#endif /* BMS_SYSTEM_VOLTAGE_IMPL_H_ */
