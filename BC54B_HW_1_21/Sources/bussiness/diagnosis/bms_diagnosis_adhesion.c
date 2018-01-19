/**
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_adhesion.c
* @brief
* @note
* @author
* @date 2017-10-16
*
*/

#include "bms_diagnosis_adhesion.h"

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
static INT32U g_negative_relay_check_tick = 0;
static RelayTroubleStatus g_state_negative_relay = kRelayTroubleNoCheck;
#pragma DATA_SEG DEFAULT


void bms_adhesion_diagnose(void)
{
//B2MosOn();
#if BMS_SUPPORT_NEGATIVE_ADHESION
    bms_diagnosis_negative_relay();   //总负粘连检测  可在此处添加其它继电器粘连检测
#endif
    
}

/************************************************************************
  * @brief           总负粘连检测函数 
  * @param[in]       无
  * @return          无
***********************************************************************/
void bms_diagnosis_negative_relay(void)
{
    
    if(g_negative_relay_check_tick == 0) 
        g_negative_relay_check_tick = get_tick_count();
    
	  if(relay_control_is_on(kRelayTypeNegative) == 0 && 
	      g_state_negative_relay == kRelayTroubleNoCheck &&
	      MV_TO_V(bcu_get_insulation_total_voltage()) > INSULATION_TOTAL_VOLTAGE_MIN) //充电继电器前端(B+)电压>30V
	      {
	          B2MosOn();          //光耦闭合
	          if(get_interval_by_tick(g_negative_relay_check_tick, get_tick_count()) >= 2000)
	    	    {
			          g_negative_relay_check_tick = get_tick_count();
			          if(bcu_get_insulation_hv3_total_voltage() > bcu_get_insulation_total_voltage()*9/10)    // HV3 > 0.9*B+
			          {
			    	        g_state_negative_relay = kRelayAdhesionTrouble;
				            //bcu_charger_disable(SELFCHECK_CHG_RELAY_ADHESION);
			          }
			          else
		          	{
			            	g_state_negative_relay = kRelayNormal;
			          }
	    		          
		            B2MosOff();       //光耦断开
		        }
	      }
	  else
    {
	      g_negative_relay_check_tick = get_tick_count();
	  }
}


/************************************************************************
  * @brief           获取继电器状态函数 
  * @param[in]       继电器类型
  * @return          继电器状态
***********************************************************************/
RelayTroubleStatus bms_get_relay_state(RelayControlType type)
{    
    RelayTroubleStatus temp = kRelayTroubleNoCheck;
   
    switch(type)
    {
        case kRelayTypeNegative:     //总负继电器       
        		temp = g_state_negative_relay;		
            break;
    
        case kRelayTypeCharging:     //充电继电器
            
            break;
    
        case kRelayTypeHeating:      //加热继电器
            
            break;
    }
    
    return temp;    
}  