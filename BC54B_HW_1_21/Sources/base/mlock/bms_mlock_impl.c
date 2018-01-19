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
static ChgLock_StateType g_ChgLock_LogicState = kChgLockUnInit; // �߼�״̬�ϵ�Ϊδ��ʼ��
static INT32U g_chglock_end_timer;
//���õ���������(ȥ��������ȥ��ֹ)
void chglock_set_setstate(INT8U state)
{   
  g_ChgLock_SetState=state;
}
//���õ��������߼�״ֵ̬
void chglock_set_logic_state(ChgLock_StateType state)
{ 
  if(state>=kChgPrmMax) return;
  g_ChgLock_LogicState=state;
}
//�õ����������߼�״ֵ̬
ChgLock_StateType chglock_get_logic_state(void)
{
  return  g_ChgLock_LogicState;
}
//�õ���������ֹ����ʱ��
INT32U chglock_get_end_timer(INT16U chglocktimer)
{
  g_chglock_end_timer=OSTimeGet()+(INT32U)chglocktimer;
  return g_chglock_end_timer;
}
//���õ�������������
Result chglock_config_set(ChgLock_ConfigType config)
{     
   if((g_ChgLock_LogicState==kChgLocking)||(g_ChgLock_LogicState==kChgUnLocking))  return 0;
   g_ChgLock_LogicState=kChgLockUnInit; 
   g_ChgLock_Config=config; 
   g_ChgLock_LogicState=kChgLockInit;                               
   return 1;
} 
//�õ����������ò���
ChgLock_ConfigType chglock_config_get(void)
{
 return g_ChgLock_Config;
}
//���õ�������ֹ���ǽ���
Result ChargerLock_Set(INT8U set_state)
{
  if((g_ChgLock_LogicState==kChgLocking)||(g_ChgLock_LogicState==kChgUnLocking))  return 0;
  g_ChgLock_SetState=set_state;
  return 1; 
}
//��λ����������ʱ���ʼ��
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
//��ֹ����
void mlock_lock(void)
{  
  if(g_ChgLock_LogicState==kChgLockUnInit)  return;
  g_ChgLock_Config.lock_func();
  
}
//��������
void mlock_unlock(void)
{  
   if(g_ChgLock_LogicState==kChgLockUnInit)  return; 
   g_ChgLock_Config.unlock_func();
}
//ж�ص�������ֹ���߽����Ķ���
void mlock_uninit(void)
{   
   g_ChgLock_Config.lock_uninit(); 
}
//���ص���������״̬
static ChgLock_StateType mlock_get_status(void)
{   
   if(g_ChgLock_LogicState==kChgLockUnInit)  return kChgNoLockErr;  
   return g_ChgLock_Config.lock_state_feedback(); 
}
//��������������                                                       
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
		case kChgLockErr:    // ��ֹ����
			break;
		case kChgUnLockErr:  // ��������
			break;
		case kChgErr:     // ����״̬
		     mlock_uninit();g_ChgLock_LogicState=state;
			break;
		case kChgLock:       // ��ֹ״̬
		     mlock_uninit();  
			if(g_ChgLock_SetState == FORCE_CHGUNLOCK)
			{
				mlock_unlock();	//ִ�н�������
				g_ChgLock_LogicState = kChgUnLocking;
			}
			break;
		case kChgLocking:    // ��ֹ��״̬
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
		case kChgUnLock:     // ����״̬
		     mlock_uninit();  
		    if(g_ChgLock_SetState== FORCE_CHGLOCK)
			{
				mlock_lock();	//ִ�н�������
				g_ChgLock_LogicState = kChgLocking;
			}
			break;
		case kChgUnLocking:  // ������״̬
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
