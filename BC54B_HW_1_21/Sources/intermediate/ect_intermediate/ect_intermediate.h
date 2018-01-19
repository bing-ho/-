/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    etc_intermediate.h                                       

** @brief       1.完成etc模块接口函数处理
                
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 


#ifndef __ECT_INTERMEDIATE_H_
#define __ECT_INTERMEDIATE_H_

#include "ect_hardware.h"

extern const  EctContext count_ect_context;



#pragma push
#pragma DATA_SEG __RPAGE_SEG SHARED_DATA
extern CaptureSignal cp_pwm_xgate_data;
extern CaptureSignal st1_pwm_xgate_data;
extern CaptureSignal st3_pwm_xgate_data;
extern CaptureSignal st5_pwm_xgate_data;
extern CaptureSignal st7_pwm_xgate_data;
#pragma DATA_SEG DEFAULT
#pragma pop

/***********************************************************************
  * @brief           初始化继电器粘连输入捕捉通道
  * @param[in]       无
  * @return          1: ok   0:err
***********************************************************************/
char st_pwm_start(void);

/***********************************************************************
  * @brief           初始化交流充电桩连接检测输入捕捉
  * @param[in]       无
  * @return          1: ok   0:err
***********************************************************************/
char cp_pwm_start(void);

/***********************************************************************
  * @brief           获取充电连接信号的占空比
  * @param[in]       无
  * @return          占空比大小
***********************************************************************/
unsigned short cp_pwm_get_duty(void);

/***********************************************************************
  * @brief           获取充电连接信号频率
  * @param[in]       无
  * @return          频率大小
***********************************************************************/
unsigned short cp_pwm_get_frequence(void);

/***********************************************************************
  * @brief           判断充电连接信号是否为高
  * @param[in]       无
  * @return          1 高
***********************************************************************/
unsigned char cp_input_is_high(void);

/***********************************************************************
  * @brief           获取继电器粘连故障占空比
  * @param[in]       pin  通道号
  * @return          占空比
***********************************************************************/
unsigned short st_pwm_get_duty(unsigned char pin) ;



#endif
