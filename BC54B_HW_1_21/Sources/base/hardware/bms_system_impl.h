#ifndef _BMS_SYSTEM_IMPL_H__
#define _BMS_SYSTEM_IMPL_H__
#include "bms_system.h"

#define	__TimerInit(ch)	do{\
			ECT_TIOS_IOS##ch = 1;\
		}while(0)
#define	__TimerReload(ch,reload)	do{\
			ECT_TFLG1_C##ch##F = 1;\
			ECT_TC##ch = ECT_TCNT + reload;\
		}while(0)
#define	__TimerOn(ch)	do{\
			ECT_TIE_C##ch##I = 1;\
		}while(0)
		
#define	__TimerOff(ch)	do{\
			ECT_TIE_C##ch##I = 0;\
		}while(0)
#define __TimerIsOn(ch)	ECT_TIE_C##ch##I
		

typedef struct _LockContext
{
    OS_EVENT* event;
} LockContext;

#endif

