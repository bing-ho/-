#ifndef	_LOGIC_CTRL_LIB_FUNC_H_
#define	_LOGIC_CTRL_LIB_FUNC_H_

#include "includes.h"



#define BMS_PRECHARGE_RETRY_CNT                 2 //Ԥ��ʧ�����Դ���
#define BMS_PRECHARGE_RETRY_INTERVAL            3000 //Ԥ��ʧ�����Լ��

#define BMS_PRECHARGE_VALID_TV_MIN_ON_MCU       30 // V/bit MCU����С��Ч��ѹ
#define BMS_PRECHARGE_TV_ON_MCU                 (MV_TO_V(bcu_get_insulation_pre_total_voltage())) //V/bit MCU����ѹֵ
#define BMS_PRECHARGE_TIMEOUT                   1000 //ms/bit Ԥ��糬ʱʱ��
#define BMS_PRECHARGE_TIME_MIN                  500 //ms/bit Ԥ�����СԤ��ʱ��
#define BMS_PRECHARGE_FINISH_DELTA_VOLT         15 // V/bit Ԥ�����ѹ��


#endif