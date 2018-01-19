/**
*
* Copyright (c) 2016 Ligoo Inc.
*
* @file bms_mlock_impl.h
* @brief
* @note
* @author
* @date 2016-9-26
*
*/

#ifndef BMS_MLOCK_IMPL_H_
#define BMS_MLOCK_IMPL_H_
#include "bms_defs.h"
#include "includes.h"
#include "bms_timer.h"

#define  FORCE_CHGLOCK 2    //强制锁止
#define  FORCE_CHGUNLOCK 1  //强制解锁

typedef enum
{  
  kChgNoLock,      //无电子锁
  kLock_Type1,     //巴斯巴
  kLock_Type2,     //安费诺
  kLock_Type3      //海拉
}kChgLock_Type;   // 电子锁类型

typedef enum
{
    kChgLockUnInit=0, // 未初始化
	kChgLockInit,	  // 初始化	
	kChgNoLockErr,    //电子锁型号配置为无
	kChgLockErr,      //锁止错误
	kChgUnLockErr,    //解锁错误
	kChgErr,          // 故障状态
	kChgLock,         // 锁止状态
	kChgLocking,      // 锁止中状态
	kChgUnLock,       // 解锁状态
	kChgUnLocking,     // 解锁中状态
	kChgPrmMax
}ChgLock_StateType;   // 电子锁状态

typedef struct
{   
	INT16U lock_time;
	INT16U unlock_time;
	//INT8U feedback_state;
	INT8U mlocktype;
	Result (*lock_func)(void);
	Result (*unlock_func)(void);
	ChgLock_StateType (*lock_state_feedback)(void);
	void (*lock_uninit)(void);
}ChgLock_ConfigType;

INT32U chglock_get_end_timer(INT16U endtimer);//得到解锁或锁止最终时间
void chglock_set_setstate(INT8U state); //上位机强制设置电子锁动作(解锁还是锁止)
void chglock_set_logic_state(ChgLock_StateType state); //设置电子锁的 逻辑状态 在drv8802.c文件中使用
void chargerlock_stateupdate(void);

void mlock_init(void);
void mlock_lock(void);
void mlock_unlock(void);
void mlock_uninit(void);

Result mlock_drv8802_lock(void);
Result mlock_drv8802_unlock(void);
ChgLock_StateType mlock_drv8802_get_status(void);
void mlock_drv8802_uninit(void);

Result mlock_amphenol_lock(void);
Result mlock_amphenol_unlock(void);
ChgLock_StateType mlock_amphenol_get_status(void);
void mlock_amphenol_uninit(void);

#endif /* BMS_MLOCK_IMPL_H_ */
