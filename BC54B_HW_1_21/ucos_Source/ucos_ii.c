/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
* File : uCOS_II.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#define  OS_GLOBALS                           /* Declare GLOBAL variables                              */
#include "..\ucos_source\cpu\includes.h"

#define  OS_MASTER_FILE                       /* Prevent the following files from including includes.h */
 
#include "..\ucos_source\os_core.c"
#include "..\ucos_source\os_flag.c"
#include "..\ucos_source\os_mbox.c"
#include "..\ucos_source\os_mem.c"
#include "..\ucos_source\os_mutex.c"
#include "..\ucos_source\os_q.c"
#include "..\ucos_source\os_sem.c"
#include "..\ucos_source\os_task.c"
#include "..\ucos_source\os_time.c"
#include "..\ucos_source\os_tmr.c"	

