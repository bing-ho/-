/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_charger_imp.h
* @brief        
* @note
* @author
* @date 2012-5-12
*
*/
#ifndef BMS_CHARGER_TEMPERATURE_H_
#define BMS_CHARGER_TEMPERATURE_H_

#include "bms_charger.h"
#include "bms_can.h"
#include "bms_config.h"

#define CHARGER_OUTLET_TEMP_NUM_MAX     2

typedef struct
{
    INT8U temperature[CHARGER_OUTLET_TEMP_NUM_MAX];
    INT8U high_temperature;
    INT8U exception;
}charger_outlet_type;
INT8U bms_get_chgr_ac_outlet_temperature(INT8U id);     
INT8U bms_get_chgr_dc_outlet_temperature(INT8U id);
INT8U bms_get_chgr_ac_outlet_temperature_num(void);
INT8U bms_get_chgr_dc_outlet_temperature_num(void);
INT8U bms_get_chgr_ac_outlet_high_temperature(void);
INT8U bms_get_chgr_dc_outlet_high_temperature(void);
void bms_set_chgr_ac_outlet_temperature_exception(INT8U state);
void bms_set_chgr_dc_outlet_temperature_exception(INT8U state);
INT8U bms_get_chgr_ac_outlet_temperature_exception(void);
INT8U bms_get_chgr_dc_outlet_temperature_exception(void);
void bms_set_chgr_ac_outlet_high_temperature(INT8U temperature);
void bms_set_chgr_dc_outlet_high_temperature(INT8U temperature);
void charger_outlet_temperature_update(void* data);
#endif /* BMS_CHARGER_TEMPERATURE_H_ */

