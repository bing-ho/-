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

#define  FORCE_CHGLOCK 2    //ǿ����ֹ
#define  FORCE_CHGUNLOCK 1  //ǿ�ƽ���

typedef enum
{  
  kChgNoLock,      //�޵�����
  kLock_Type1,     //��˹��
  kLock_Type2,     //����ŵ
  kLock_Type3      //����
}kChgLock_Type;   // ����������

typedef enum
{
    kChgLockUnInit=0, // δ��ʼ��
	kChgLockInit,	  // ��ʼ��	
	kChgNoLockErr,    //�������ͺ�����Ϊ��
	kChgLockErr,      //��ֹ����
	kChgUnLockErr,    //��������
	kChgErr,          // ����״̬
	kChgLock,         // ��ֹ״̬
	kChgLocking,      // ��ֹ��״̬
	kChgUnLock,       // ����״̬
	kChgUnLocking,     // ������״̬
	kChgPrmMax
}ChgLock_StateType;   // ������״̬

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

INT32U chglock_get_end_timer(INT16U endtimer);//�õ���������ֹ����ʱ��
void chglock_set_setstate(INT8U state); //��λ��ǿ�����õ���������(����������ֹ)
void chglock_set_logic_state(ChgLock_StateType state); //���õ������� �߼�״̬ ��drv8802.c�ļ���ʹ��
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
