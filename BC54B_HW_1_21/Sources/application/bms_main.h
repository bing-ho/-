/*******************************************************************************
 **                       ������������Դ���޹�˾ Copyright (c)
 **                            http://www.ligoo.cn/
 **�� �� ����bms_main.h
 **��    �ߣ�����ΰ
 **�������ڣ�2011.07.01
 **�޸ļ�¼��
 **�ļ�˵��:ϵͳ�������ļ�
 **��    ��: V4.0
 **��    ע��
 *******************************************************************************/
#ifndef  _BMSMAIN_HEADER
#define  _BMSMAIN_HEADER
//#include  "..\cpu_source\includes.h"
#include "bms_business_defs.h"


BMS_EXT void system_init(void);

#if BMS_SUPPORT_BATTERY == 0
#   error you must define the Battery Pack Tpye(LiFePO4 OR LiMnO2) in "app_cfg.h" file 
#endif

#if 0
#if BMS_24S_SLAVE_EN > 0
#   if BMS_16S_SLAVE_EN > 0
#       error The BMS can not use for the Both of 24S_SLAVE and 16S_SLAVE in "app_cfg.h" file
#   endif
#elif BMS_16S_SLAVE_EN == 0
#   error you must define the battery num(16S or 24S) in "app_cfg.h" file
#endif

#if BMS_485_SLAVE_EN > 0
#	if BMS_CAN_SLAVE_EN > 0
#		error The��BMS CCM Can not communicate with  DCM by RS485 and CAN at the same time in "app_cfg.h" file
#	endif
#elif BMS_CAN_SLAVE_EN == 0
# 	error you must define the type of the communication between CCM and DCM in"app_cfg.h" file
#endif
#endif

#endif
