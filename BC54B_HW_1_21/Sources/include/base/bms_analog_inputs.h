
#ifndef BMS_SYSTEM_VOLTAGE_H_
#define BMS_SYSTEM_VOLTAGE_H_

#include "bms_defs.h"
#include "app_cfg.h"
#include "bms_bcu.h"
#include "bms_job.h"
#include "adc.h"


void bcu_analog_inputs_init(void);
INT16U bcu_analog_inputs_get(void);
INT8U bcu_calc_analog_inputs(INT16U *volt);
void bcu_analog_inputs_detect(void* data);//系统供电电压检测

#endif 
