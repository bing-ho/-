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
#include "bms_mlock_impl.h"
#include "bms_config.h"

static ChgLock_ConfigType g_ChgLock_Config={0};
static INT8U g_ChgLock_SetState;
static ChgLock_StateType g_ChgLock_LogicState = kChgLockUnInit; // 逻辑状态上电为未初始化
static INT32U g_chglock_end_timer;
//设置电子锁动作(去解锁还是去锁止)
void chglock_set_setstate(INT8U state)
{   
  g_ChgLock_SetState=state;
}
//设置电子锁的逻辑状态值
void chglock_set_logic_state(ChgLock_StateType state)
{ 
  if(state>=kChgPrmMax) return;
  g_ChgLock_LogicState=state;
}
//得到电子锁的逻辑状态值
ChgLock_StateType chglock_get_logic_state(void)
{
  return  g_ChgLock_LogicState;
}
//得到解锁或锁止最终时间
INT32U chglock_get_end_timer(INT16U chglocktimer)
{
  g_chglock_end_timer=OSTimeGet()+(INT32U)chglocktimer;
  return g_chglock_end_timer;
}
//设置电子锁参数配置
Result chglock_config_set(ChgLock_ConfigType config)
{     
   if((g_ChgLock_LogicState==kChgLocking)||(g_ChgLock_LogicState==kChgUnLocking))  return 0;
   g_ChgLock_LogicState=kChgLockUnInit; 
   g_ChgLock_Config=config; 
   g_ChgLock_LogicState=kChgLockInit;                               
   return 1;
} 
//得到电子锁配置参数
ChgLock_ConfigType chglock_config_get(void)
{
 return g_ChgLock_Config;
}
//设置电子锁锁止还是解锁
Result ChargerLock_Set(INT8U set_state)
{
  if((g_ChgLock_LogicState==kChgLocking)||(g_ChgLock_LogicState==kChgUnLocking))  return 0;
  g_ChgLock_SetState=set_state;
  return 1; 
}
//上位机进行配置时候初始化
void mlock_init(void)
{   g_ChgLock_LogicState = kChgLockUnInit; 
    g_ChgLock_Config.lock_time=config_get(kMlockParam1);
    g_ChgLock_Config.unlock_time=config_get(kMlockParam2);
    g_ChgLock_Config.mlocktype=(INT8U)config_get(kMlockType);
    if((g_ChgLock_Config.mlocktype== kLock_Type1)||(g_ChgLock_Config.mlocktype== kLock_Type3))  
    {
        g_ChgLock_Config.lock_func=mlock_drv8802_lock;
        g_ChgLock_Config.unlock_func=mlock_drv8802_unlock;
        g_ChgLock_Config.lock_state_feedback=mlock_drv8802_get_status; 
        g_ChgLock_Config.lock_uninit=mlock_drv8802_uninit;
    } 
    else if(g_ChgLock_Config.mlocktype== kLock_Type2)
    {
        g_ChgLock_Config.lock_func=mlock_amphenol_lock;
        g_ChgLock_Config.unlock_func=mlock_amphenol_unlock;
        g_ChgLock_Config.lock_state_feedback=mlock_amphenol_get_status; 
        g_ChgLock_Config.lock_uninit=mlock_amphenol_uninit;
    }
    else
    {
        g_ChgLock_LogicState=kChgNoLockErr;return;
    } 
    g_ChgLock_LogicState=kChgLockInit;

}
//锁止动作
void mlock_lock(void)
{  
  if(g_ChgLock_LogicState==kChgLockUnInit)  return;
  g_ChgLock_Config.lock_func();
  
}
//解锁动作
void mlock_unlock(void)
{  
   if(g_ChgLock_LogicState==kChgLockUnInit)  return; 
   g_ChgLock_Config.unlock_func();
}
//卸载电子锁锁止或者解锁的动作
void mlock_uninit(void)
{   
   g_ChgLock_Config.lock_uninit(); 
}
//返回电子锁反馈状态
static ChgLock_StateType mlock_get_status(void)
{   
   if(g_ChgLock_LogicState==kChgLockUnInit)  return kChgNoLockErr;  
   return g_ChgLock_Config.lock_state_feedback(); 
}
//电子锁动作更新                                                       
void chargerlock_stateupdate(void)
{
	ChgLock_StateType state;
	if(g_ChgLock_LogicState==kChgNoLockErr) return;  
    state= mlock_get_status();
    switch(g_ChgLock_LogicState)
    {
        case kChgLockInit:
			 mlock_uninit();g_ChgLock_LogicState=state;
			break;
		case kChgLockErr:    // 锁止出错
			break;
		case kChgUnLockErr:  // 解锁出错
			break;
		case kChgErr:     // 故障状态
		     mlock_uninit();g_ChgLock_LogicState=state;
			break;
		case kChgLock:       // 锁止状态
		     mlock_uninit();  
			if(g_ChgLock_SetState == FORCE_CHGUNLOCK)
			{
				mlock_unlock();	//执行解锁操作
				g_ChgLock_LogicState = kChgUnLocking;
			}
			break;
		case kChgLocking:    // 锁止中状态
			if(g_ChgLock_SetState == FORCE_CHGUNLOCK) return;
			else
			{
			  if(state == kChgLock) g_ChgLock_LogicState=kChgLock;
			  else if((OSTimeGet()>g_chglock_end_timer)&&(state != kChgLock))
				{
					g_ChgLock_LogicState = kChgLockErr;mlock_uninit();
				}
			}
			break;
		case kChgUnLock:     // 解锁状态
		     mlock_uninit();  
		    if(g_ChgLock_SetState== FORCE_CHGLOCK)
			{
				mlock_lock();	//执行解锁操作
				g_ChgLock_LogicState = kChgLocking;
			}
			break;
		case kChgUnLocking:  // 解锁中状态
		    if(g_ChgLock_SetState == FORCE_CHGLOCK) return;
			else
			{
			  if(state == kChgUnLock) g_ChgLock_LogicState=kChgUnLock;
		   	  else if((OSTimeGet()>g_chglock_end_timer)&&(state != kChgUnLock))
				{
					g_ChgLock_LogicState = kChgUnLockErr;mlock_uninit();
				}
			}
			break;
		default:break;                                     
	}
  }
