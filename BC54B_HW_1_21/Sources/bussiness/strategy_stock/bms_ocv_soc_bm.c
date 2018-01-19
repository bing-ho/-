/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_ocv_soc_bm.c
 * @brief
 * @note
 * @date 2017-11-17
 *
 */

#pragma MESSAGE DISABLE C5919
 
#include "bms_ocv_soc_bm.h"
#include "bms_ocv_soc_impl.h"
#include "bms_config.h"

#define MAX_AH_ADD_FACTOR 1.5
#define CHARGE_CALIB_END_POINT  0.95
#define DISCHARGE_CALIB_END_POINT 0.25
#define SOC_CALIB_DIFF_THD 0.3

/*  OCV校准时，最高最低单体电压查表得到对应的SOC，范围[0,1]   */
float soc_bm_high, soc_bm_low;

/*  安时积分计算出的SOC，范围[0,1]   */
float soc_ah_calib;

/*  用于显示的SOC，范围[0,1]   */
float soc_display;

/*  OCV校准后，安时积分得到的电量累计变化量，单位Ams   */
INT32U delta_battery_cap_ams = 0;

/*  0 为负，1为正 */
INT8U  delta_battery_cap_sign = 0;

/*  显示SOC对应的剩余电量，单位Ams   */
INT32U left_cap_ams_display = 0;


INT8U get_delta_battery_cap_sign(void)
{
    return delta_battery_cap_sign;
}
                                  
void set_delta_battery_cap_sign(INT8U value)
{
    delta_battery_cap_sign = value;
}

/**
* @brief   电量累计变化量外部访问接口
*/
INT32U get_delta_battery_cap_ams(void) 
{ 
  return delta_battery_cap_ams; 
}


/**
* @brief   显示SOC对应剩余电量外部访问接口
*/  
INT32U get_left_cap_ams_display(void) 
{ 
  return left_cap_ams_display; 
}


/**
* @brief   soc_bm_high外部写接口
*/ 
void set_soc_bm_high(float value)
{
	soc_bm_high = value;
	config_save(kSOCBMHigh, (INT16U)(soc_bm_high*10000+0.5));        
}


/**
* @brief   soc_bm_low外部写接口
*/ 
void set_soc_bm_low(float value)
{ 
	soc_bm_low = value;
	config_save(kSOCBMLow,  (INT16U)(soc_bm_low*10000+0.5));
}


/**
* @brief   OCV校准， 查表更新soc_bm_high, soc_bm_low, delta_battery_cap_ams
*/ 
void update_soc_bm_by_ocv_soc_table(void) 
{
  float ocv_soc_high, ocv_soc_low;
  
  ocv_soc_high = bms_ocv_soc_get_soc_float(bcu_get_average_temperature(),
                                          bcu_get_high_voltage()) / 100.0;
  ocv_soc_low = bms_ocv_soc_get_soc_float(bcu_get_average_temperature(),
                                         bcu_get_low_voltage()) / 100.0;
  set_delta_battery_cap_ams(0);
  
  save_delta_battery_cap_ams_to_config();

  if (ocv_soc_high < 0) 
  { 
    ocv_soc_high = bcu_get_SOC() / 10000.0;
  }
  if (ocv_soc_low < 0) 
  { 
    ocv_soc_low = bcu_get_SOC() / 10000.0;
  }
  
  config_save(kSOCBMAvailableFlag, 1);
  
  set_soc_bm_high(ocv_soc_high);
  set_soc_bm_low(ocv_soc_low);
  
  update_left_cap_ams_display_with_factor(0, 1);
}


/**
* @brief   根据电流方向， 充电取soc_bm_high,放电取soc_bm_low作为基准SOC
*/ 
float get_soc_bm(void) 
{
  if(bcu_get_current() > 0)
  {
    return soc_bm_high;
  }
  else
  {
    return soc_bm_low;
  }
}


/**
* @brief   用基准SOC安时积分和电量累计变化量, 更新安时积分计算结果soc_ah_calib
*/ 
void update_soc_ah_calib(void) 
{
    if(get_delta_battery_cap_sign() > 0)
    {
        soc_ah_calib = get_soc_bm() + delta_battery_cap_ams / 3600.0 / 100 / config_get(kTotalCapIndex);
    }
    else
    {
        soc_ah_calib = get_soc_bm() - delta_battery_cap_ams / 3600.0 / 100 / config_get(kTotalCapIndex);
    }
    soc_ah_calib = soc_ah_calib > 1 ? 1 : soc_ah_calib;
    soc_ah_calib = soc_ah_calib < 0 ? 0 : soc_ah_calib;
}


/**
* @brief   计算显示SOC对应的剩余容量，相对安时积分SOC对应的剩余容量变化倍率
*/ 
float get_ah_calib_factor(float current_soc) 
{
  float factor;
  float chg_end, dis_end;
  float soc_ah = get_soc_ah_calib();
  
  chg_end = CHARGE_CALIB_END_POINT;
  dis_end = DISCHARGE_CALIB_END_POINT;
  
  if( (current_soc > CHARGE_CALIB_END_POINT) || (soc_ah > CHARGE_CALIB_END_POINT) ) 
  {
      chg_end = 1;
  }
  if( (current_soc < DISCHARGE_CALIB_END_POINT) || (soc_ah < DISCHARGE_CALIB_END_POINT)  ) 
  {
      dis_end = 0;
  }
  
  if(bcu_get_current() > 0) 
  {
    factor = (chg_end - current_soc) / (chg_end - soc_ah);
  }
  else 
  {
    factor = (current_soc - dis_end) / (soc_ah - dis_end);
  }
  
  if(factor > MAX_AH_ADD_FACTOR) 
  {
    factor = MAX_AH_ADD_FACTOR;
  }  
  if(factor < 1 / MAX_AH_ADD_FACTOR) 
  {
    factor = 1 / MAX_AH_ADD_FACTOR;
  }
  
  return factor;
}


/**
* @brief   更新显示SOC
*/ 
void update_soc_display(void) 
{
  soc_display = left_cap_ams_display / 3600.0 / 100 / config_get(kTotalCapIndex);
  soc_display = soc_display > 1 ? 1 : soc_display;
}


/**
* @brief   left_cap_ams_display外部写接口
*/ 
void set_left_cap_ams_display(INT32U value) 
{ 
  left_cap_ams_display = value; 
}


/**
* @brief   显示SOC外部访问接口， 惰性更新
*/
float get_soc_display(void) 
{ 
  update_soc_display();
  return soc_display; 
}


/**
* @brief   soc_ah_calib外部访问接口， 惰性更新
*/
float get_soc_ah_calib(void) 
{
  update_soc_ah_calib();
  return soc_ah_calib;
}


/**
* @brief   更新bmsinfo中的剩余容量
*/
void update_left_cap_by_soc_display(void) 
{
  bcu_set_left_cap_interm(left_cap_ams_display);
}


/**
* @brief   更新显示SOC对应的剩余容量
*/
INT32U update_left_cap_ams_display_with_factor(INT32U value, INT8S sign) 
{
   INT32U left_cap_ams = 0;
   INT32U temp;
   float factor;
   float soc_current = get_soc_display();
   
   factor = get_ah_calib_factor(soc_current);
   soc_current = fabs(soc_current - get_soc_ah_calib());
   if(  soc_current > SOC_CALIB_DIFF_THD) 
   { 
      left_cap_ams = (INT32U)(soc_ah_calib*config_get(kTotalCapIndex)*3600*100);
   }
   else 
   {  
      temp = (INT32U)(value*factor);
      if(sign > 0) 
      { 
        if(0xFFFFFFFF - left_cap_ams_display < temp) 
        {  
          left_cap_ams = 0xFFFFFFFF;
        }
        else 
        {
          left_cap_ams = left_cap_ams_display + temp;
        }
      } 
      else if(sign < 0) 
      {
        if (left_cap_ams_display > temp) 
        {  
            left_cap_ams = left_cap_ams_display - temp;
        }
        else 
        {  
            left_cap_ams = 0;
        }
      }
   }
   
   set_left_cap_ams_display(left_cap_ams);
   return temp;
}



/**
* @brief   参数初始化
*/
void ocv_soc_calib_params_init(void) 
{
   float soc_bm_power_on;
   
   if(config_get(kSOCBMAvailableFlag) == 1) 
   {
      soc_bm_high = config_get(kSOCBMHigh) / 10000.0;
      soc_bm_low =  config_get(kSOCBMLow) / 10000.0;
      delta_battery_cap_ams = (INT32U)(((INT32U)config_get(kDeltaCapAMSAfterCalibHighIndex))<<16) | (config_get(kDeltaCapAMSAfterCalibLowIndex));
      delta_battery_cap_sign = (INT8U)(config_get(kDeltaCapSign));
      left_cap_ams_display =  (INT32U)((INT32U)(config_get(kLeftCapAmsHighIndex))<<16) | ( config_get(kLeftCapAmsLowIndex) ); 
      update_soc_display();  
   } 
   else
   {
      soc_bm_power_on = (float)(config_get(kLeftCapIndex) ) / config_get(kTotalCapIndex);
      set_soc_bm_high(soc_bm_power_on);
      set_soc_bm_low(soc_bm_power_on);
      
      set_delta_battery_cap_ams(0);
      save_delta_battery_cap_ams_to_config();


      left_cap_ams_display = (INT32U)(soc_bm_power_on * config_get(kTotalCapIndex) * 3600 * 100);
      set_left_cap_ams_display(left_cap_ams_display);
      
      update_soc_display();
   }
}


/**
* @brief   delta_battery_cap_ams外部写接口
*/
void set_delta_battery_cap_ams(INT32U value) 
{ 
  delta_battery_cap_ams = value;
}


/**
* @brief   delta_battery_cap_ams存储到Eeeprom中
*/
void save_delta_battery_cap_ams_to_config(void) 
{
  config_save(kDeltaCapAMSAfterCalibHighIndex, (INT16U)(delta_battery_cap_ams>>16));
  config_save(kDeltaCapAMSAfterCalibLowIndex, (INT16U)(delta_battery_cap_ams)); 
  config_save(kDeltaCapSign, (INT16U)delta_battery_cap_sign);
}


/**
* @brief   left_cap_ams_display存储到Eeeprom中
*/
void save_left_cap_ams_display_to_config(void) 
{
  config_save(kLeftCapAmsHighIndex, (INT16U)(left_cap_ams_display>>16));
  config_save(kLeftCapAmsLowIndex, (INT16U)(left_cap_ams_display));
}