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
    bms_diagnosis_negative_relay();   //�ܸ�ճ�����  ���ڴ˴���������̵���ճ�����
#endif
    
}

/************************************************************************
  * @brief           �ܸ�ճ����⺯�� 
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void bms_diagnosis_negative_relay(void)
{
    
    if(g_negative_relay_check_tick == 0) 
        g_negative_relay_check_tick = get_tick_count();
    
	  if(relay_control_is_on(kRelayTypeNegative) == 0 && 
	      g_state_negative_relay == kRelayTroubleNoCheck &&
	      MV_TO_V(bcu_get_insulation_total_voltage()) > INSULATION_TOTAL_VOLTAGE_MIN) //���̵���ǰ��(B+)��ѹ>30V
	      {
	          B2MosOn();          //����պ�
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
	    		          
		            B2MosOff();       //����Ͽ�
		        }
	      }
	  else
    {
	      g_negative_relay_check_tick = get_tick_count();
	  }
}


/************************************************************************
  * @brief           ��ȡ�̵���״̬���� 
  * @param[in]       �̵�������
  * @return          �̵���״̬
***********************************************************************/
RelayTroubleStatus bms_get_relay_state(RelayControlType type)
{    
    RelayTroubleStatus temp = kRelayTroubleNoCheck;
   
    switch(type)
    {
        case kRelayTypeNegative:     //�ܸ��̵���       
        		temp = g_state_negative_relay;		
            break;
    
        case kRelayTypeCharging:     //���̵���
            
            break;
    
        case kRelayTypeHeating:      //���ȼ̵���
            
            break;
    }
    
    return temp;    
}  