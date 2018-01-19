/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file soh_impl.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-7-2
*
*/
#include "bms_soh_impl.h"


#if BMS_SUPPORT_SOH == BMS_SOH_DEFAULT

void soh_init(void)
{
    
    soh_fix_as_totalcap();
    soh_refersh(config_get(kTotalCapIndex), config_get(kNominalCapIndex));

    config_register_observer(kTotalCapIndex, kTotalCapIndex, soh_releated_config_changed);
    config_register_observer(kNominalCapIndex, kNominalCapIndex, soh_releated_config_changed);
}

INT16U soh_get(void)
{
    return bcu_get_SOH();
}

Result soh_releated_config_changed(ConfigIndex index, INT16U new_value)
{
    if (index == kTotalCapIndex)
    {
        soh_refersh(new_value, config_get(kNominalCapIndex));
    }
    else if (index == kNominalCapIndex)
    {
        soh_refersh(config_get(kTotalCapIndex), new_value);
    }

    return RES_OK;
}

void soh_refersh(INT16U total_cap, INT16U max_cap)
{
    INT16U soh = 0;

    if (max_cap > total_cap && max_cap > 0)
    {
        soh = (INT16U)(((INT32U)total_cap * SOH_MAX_VALUE) / max_cap);
    }
    else
    {
        soh = SOH_MAX_VALUE;
    }
    if(soh > PERCENT_TO_SOH(97))   soh = SOH_MAX_VALUE;//SOH大于97%显示100%
    bcu_set_SOH(soh);

}

static INT16U soh_70_percent_cycle_valid(INT16U value)
{
  if((value >= SOH_70_PERCENT_CYCLE_MIN) && (value <= SOH_70_PERCENT_CYCLE_MAX))
   {
      value =  SOH_70_PERCENT_CYCLE;
   }
  else
   {
      if(value < SOH_70_PERCENT_CYCLE_MIN) 
       {
         value = SOH_70_PERCENT_CYCLE_MIN; 
       }
      else if(value > SOH_70_PERCENT_CYCLE_MAX) 
       {
         value = SOH_70_PERCENT_CYCLE_MAX; 
       }
   } 
  return value;
}

INT8U soh_is_support_calib(void)
{
    volatile INT32U value1 = (INT32U)config_get(kNominalCapIndex)*soh_70_percent_cycle_valid(SOH_70_PERCENT_CYCLE);
    union 
    {INT16U tcword[2];
     INT32U doubleword;
    }value2={0};
    value2.tcword[1]=config_get(kChgTotalCapLowIndex);   //获取累计充电电量值来进行SOH的计算
    value2.tcword[0]=config_get(kChgTotalCapHighIndex);
    if (value2.doubleword <= value1)
    {
        return 1;
    }
    return 0;
}

void soh_fix_as_totalcap(void)
{
  volatile INT16U reduce_cap;
  INT16U soh_70_percent_cycle_value = soh_70_percent_cycle_valid(SOH_70_PERCENT_CYCLE);
  INT16U total_cap = config_get(kTotalCapIndex),total_cap_diff = 0;
  union 
    {INT16U tcword[2];
     INT32U doubleword;
    }totalcapdouble={0};
  totalcapdouble.tcword[1]=config_get(kChgTotalCapLowIndex);   //获取累计充电电量值来进行SOH的计算
  totalcapdouble.tcword[0]=config_get(kChgTotalCapHighIndex);
  
  if(soh_is_support_calib() == 1) //判断累计充放电量是否小于SOH_70_PERCENT_CYCLE*C
     {    
        reduce_cap = (INT16U)DIVISION((3*totalcapdouble.doubleword),(soh_70_percent_cycle_value*10));
        if(reduce_cap > config_get(kNominalCapIndex)) return;
        total_cap_diff = config_get(kNominalCapIndex) - reduce_cap;
        if(abs(total_cap_diff - total_cap) > DIVISION(config_get(kNominalCapIndex),100))   //容量变化1%才进行校准
        {
           config_save(kTotalCapIndex, total_cap_diff); 
           config_save(kLeftCapIndex,(INT16U)DIVISION(GAIN(bcu_get_SOC(),config_get(kTotalCapIndex)),SOC_MAX_VALUE));
           bcu_set_left_cap_interm(CAP_TENFOLD_AH_TO_MAS(config_get(kLeftCapIndex)));
        }
     }

 }
 
#endif




