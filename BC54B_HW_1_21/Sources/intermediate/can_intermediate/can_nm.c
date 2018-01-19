/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    can_intermediate.c                                                       
** @copyright   	Ligoo Inc.         
** @date		    2017-04-25.
** @author            
*******************************************************************************/ 

#include "can_nm.h"
#include "app_cfg.h"
#include "ems_message.h"

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

unsigned char can_nm_state;

#pragma DATA_SEG DEFAULT



 	#define CAN_EN_HIGH()    DDRK_DDRK6 = 1;PORTK_PK6 = 1; 
 	#define CAN_STB_HIGH()   DDRK_DDRK3 = 1;PORTK_PK3 = 1; 
	#define CAN_EN_LOW()    DDRK_DDRK6 = 1;PORTK_PK6 = 0; 
 	#define CAN_STB_LOW()   DDRK_DDRK3 = 1;PORTK_PK3 = 0;
 	

#define LOCAL_WKUP_EVENT()   \
    (cp_input_is_high() || \
    dc_24V_voltage_is_usful() == TRUE || \
    guobiao_charger_pwm_is_connected() || \
    input_signal_control_is_high(kInputTypePowerOn) == 0 ||\
    input_signal_control_is_high(kInputTypeChargerConnect) == 0 )


static void Set_Can2_Transceiver_Normal(void);
static void Set_Can2_Transceiver_Standby(void);
/************************************************************************
  * @brief           初始化函数
  * @param[in]       void   
  * @return          void 
***********************************************************************/ 
void Can_nm_init(void)
  {	
     can_nm_state = CAN_NM_NORMAL;       
     Set_Can2_Transceiver_Normal();
  }
  
/************************************************************************
  * @brief           设置整车CAN收发器进入NORMAL状态
  * @param[in]       无  
  * @return          无
***********************************************************************/

void Set_Can2_Transceiver_Normal(void)
{ 
     CAN_STB_HIGH()
     CAN_EN_HIGH()
}

/************************************************************************
  * @brief           设置整车CAN收发器进入standby状态
  * @param[in]       无  
  * @return          无
***********************************************************************/

void Set_Can2_Transceiver_Standby(void)
{ 
     CAN_STB_LOW()
     CAN_EN_HIGH()
}



/************************************************************************
  * @brief           CAN_NM主任务
  * @param[in]         
  * @return          
***********************************************************************/ 
void Can_nm_mainfunction(void) 
{
   static unsigned long tick = 0; 
   switch(can_nm_state) 
   {
    case CAN_NM_NORMAL: 
    {
        //Set_Can2_Transceiver_Normal();

        
        
        
        
        
        if((can_rx_timeout_check() >= CAN_COMM_TIMEOUT)&&(LOCAL_WKUP_EVENT() == 0))
        {
            Set_Can2_Transceiver_Standby();
            can_nm_state = CAN_NM_SLEEP; 
            tick = get_tick_count();   
        }
      break;
    }
    case CAN_NM_SLEEP: 
    {
        //Set_Can2_Transceiver_Standby();
        if(((get_interval_by_tick(tick, get_tick_count()) >= 20) && \
            (input_signal_is_high_by_name("WKUP_INH") == 0)) ||(LOCAL_WKUP_EVENT() == 1))
        {
           Set_Can2_Transceiver_Normal();  
           can_nm_state = CAN_NM_NORMAL;
           clr_can_rx_timeout();
        }
      break; 
    }
    default:
      break;
   }

}

/************************************************************************
  * @brief           获取CAN休眠状态
  * @param[in]         
  * @return          0 ：休眠  1 ：正常工作
***********************************************************************/ 

unsigned char Get_can_nm_state(void) 
{
    if(config_get(kCanWkupEnableIndex) == CAN_WKUP_DISABLE) 
    {
        return 0;   //非CAN唤醒模式
    }
    return can_nm_state; 
}


