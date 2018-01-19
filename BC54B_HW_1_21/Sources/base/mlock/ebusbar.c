/**
 *
 * Copyright (c) 2016 Ligoo Inc.
 *
 * @file bms_mlock_impl.c
 * @brief
 * @note
 * @author
 * @date 2016-9-26
 *
 */

#include "bms_mlock.h"
#include "adc0_intermediate.h"
#include "gpio_interface.h"
#include "mc9s12xep100.h"
#include "bms_system.h"
#include "bms_config.h"
#include "drv8802_hardware.h"

#pragma MESSAGE DISABLE C5703
#pragma MESSAGE DISABLE C4001 //Condition always FALSE //do...while(0)

#define     MLOCK_SET_UNLOCK    0
#define     MLOCK_SET_LOCK      1

#define MIN_LOCK_UNLOCK_TIME         200 //ms
#define MAX_LOCK_UNLOCK_TIME         400 //ms

typedef enum _MLOCK_CTRL_PORT
{
    MLOCK_PORT_PHASE,
    MLOCK_PORT_ENABLE,
    MLOCK_PORT_DECAY,
    MLOCK_PORT_SLEEP,
    MLOCK_PORT_RESET
}MLOCK_CTRL_PORT;

const struct hcs12_gpio_detail mlock_gpio_out_v120[] = {
    { "MPHASE",                 { &PORTA,   &PORTA,     &DDRA,      1 << 3}, 0,  0},
    { "MENBL",                  { &PORTA,   &PORTA,     &DDRA,      1 << 4}, 0,  0},
    { "MDECAY",                 { &PORTA,   &PORTA,     &DDRA,      1 << 5}, 0,  0},
    { "MSLEEP",                 { &PORTA,   &PORTA,     &DDRA,      1 << 7}, 0,  0},
    { "MRESET",                 { &PORTD,   &PORTD,     &DDRD,      1 << 4}, 0,  0},
    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0}
};

extern const struct adc_onchip_channel adc_onchip_motor_det_adc_v120;

ChgLock_StateType mlock_drv8802_get_status(void);

const Mlock8802_Platform *Mlock8802= &Mlock8802_v121;

static void mlock_drv8802_reset_io(void)
{
    Mlock_Drv8802_Phase_Set(Mlock8802,0);
    Mlock_Drv8802_Enable_Set(Mlock8802,0);
    Mlock_Drv8802_Reset_Set(Mlock8802,0);
    Mlock_Drv8802_Decay_Set(Mlock8802,0); 
}

static void mlock_drv8802_force_ctrl(uint8_t type)
{
    PUCR_PUPAE = 1;         /* Pull-up Port A Enable */
    Mlock_Drv8802_Phase_Set(Mlock8802,type);
    Mlock_Drv8802_Sleep_Set(Mlock8802,1);
    Mlock_Drv8802_Enable_Set(Mlock8802,1);
    Mlock_Drv8802_Reset_Set(Mlock8802,1);
    Mlock_Drv8802_Decay_Set(Mlock8802,1);
}
void mlock_drv8802_uninit(void)
{
  mlock_drv8802_reset_io();
}


/*void mlock_drv8802_init(void* pvdata)
{
}
*/

/*
    ------ K --------  Motor ---
    |              |           |
   PIN2           PIN1       PIN3
 Motor_Det        OUT1       OUT2
 
  PIN1==OUT1, PIN3==OUT2, PIN2==Motor_Det
  Lock: PIN1=0 (12V),  PIN3=1 (0V),  PIN1 disconnect PIN2, PIN2 Ðü¿Õ;
Unlock: PIN1=1 (0V),   PIN3=0 (12V), PIN1 connect to PIN2£¬Vpin2 =Vpin1=0V
*/
Result mlock_drv8802_lock(void)
{
   ChgLock_ConfigType config=chglock_config_get();
   INT16U  lock_time = config.lock_time;//g_ChgLock_Config.lock_time;//lock_time = config_get(kMlockParam1);
    if(lock_time > MAX_LOCK_UNLOCK_TIME)
    {
        lock_time = MAX_LOCK_UNLOCK_TIME;
    }
    else if(lock_time < MIN_LOCK_UNLOCK_TIME)
    {
        lock_time = MIN_LOCK_UNLOCK_TIME;
    }
    //mlock_timer_end_tick = OSTimeGet() + lock_time;
    chglock_get_end_timer(lock_time); 
    mlock_drv8802_force_ctrl(MLOCK_SET_LOCK);
        return 1;
}

Result mlock_drv8802_unlock(void)
{
    ChgLock_ConfigType config=chglock_config_get();
    INT16U  unlock_time = config.unlock_time;//g_ChgLock_Config.unlock_time;//unlock_time = config_get(kMlockParam2);
    if(unlock_time > MAX_LOCK_UNLOCK_TIME)
    {
        unlock_time = MAX_LOCK_UNLOCK_TIME;
    }
    else if(unlock_time < MIN_LOCK_UNLOCK_TIME)
    {
        unlock_time = MIN_LOCK_UNLOCK_TIME;
    }
    chglock_get_end_timer(unlock_time); //mlock_timer_end_tick = OSTimeGet() + unlock_time;
    mlock_drv8802_force_ctrl(MLOCK_SET_UNLOCK);   
    return 1;  
}

ChgLock_StateType mlock_drv8802_get_status(void)
{/*
  Lock: PIN1=1 (12V), PIN3(0V), PIN2(suspend) ==> mlock_pin2_v = 2200 mV = (5000-600)/2
UnLock: PIN1=1 (0V) , PIN3(12V),PIN2 connected to PIN1 ==> mlock_pin2_v = 1430 mV = (5000-700)/3
UnLocking: PIN2 12V --> 0V,  ==> mlock_pin2_v = 5000 mV --> 1430mV
  Locking: PIN2 0V --> 12V,  ==> mlock_pin2_v = 1430 mV --> 2150mV
//Motor_Det: High(12V), low(0V), Suspend(? V)
*/                                                            
    ChgLock_ConfigType config=chglock_config_get();                                                          
    INT16U mlock_pin2_v = 0;
    ChgLock_StateType ret = chglock_get_logic_state();
    
    if(!Mlock_Drv8802_Fault_Get(Mlock8802))
    {
       chglock_set_logic_state(kChgErr); return kChgErr;
    } 
    if (!adc_onchip_read(&adc_onchip_motor_det_adc_v120, &mlock_pin2_v)) 
    {
       return ret;
    }
    if((mlock_pin2_v >= 2150) && (mlock_pin2_v <= 2300))//if((mlock_pin2_v >= 1990) && (mlock_pin2_v <= 2430))
    {
        if(config.mlocktype==kLock_Type1) ret=kChgLock;
        else if(config.mlocktype==kLock_Type3) ret=kChgUnLock;
    }
    else if((mlock_pin2_v >= 1350) && (mlock_pin2_v <= 1700))  
    {
        if(config.mlocktype==kLock_Type1) ret=kChgUnLock;
        else if(config.mlocktype==kLock_Type3)  ret=kChgLock;
    }
    else if((mlock_pin2_v >= 1705) && (mlock_pin2_v <= 1980))
    {
      if(chglock_get_logic_state()==kChgLockInit)  
        {
          if(config.mlocktype==kLock_Type1) ret=kChgUnLock;
          else if(config.mlocktype==kLock_Type3) ret=kChgLock; 
        }
    }
    return ret;
}