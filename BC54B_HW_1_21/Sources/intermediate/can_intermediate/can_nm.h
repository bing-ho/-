/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    can_intermediate.h                                                       
** @copyright   	Ligoo Inc.         
** @date		    2017-04-25.
** @author            
*******************************************************************************/ 

#ifndef __CAN_NM_H_
#define __CAN_NM_H_


/**********************************************
 *
 * Context
 *
 **********************************************/


#define CAN_NM_NORMAL              1   
#define CAN_NM_SLEEP               0   
#define CAN_COMM_TIMEOUT           8000

extern void Can_nm_init(void);
extern unsigned char Get_can_nm_state(void);
extern void Can_nm_mainfunction(void);


#endif
