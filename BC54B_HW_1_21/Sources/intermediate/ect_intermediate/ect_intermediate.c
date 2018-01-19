/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    etc_intermediate.c                                       

** @brief       1.完成etc模块接口函数处理
                
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 
 
#include "derivative.h"
#include "ect_intermediate.h"
#include "ect_xgate.h"


#pragma push
#pragma DATA_SEG __RPAGE_SEG SHARED_DATA
 CaptureSignal cp_pwm_xgate_data = {
    0, 0, 0, 0,  (1 << 2), &ECT_TC2
};

 CaptureSignal st1_pwm_xgate_data = {
    0, 0, 0, 0,  (1 << 4), &ECT_TC4
};

 CaptureSignal st3_pwm_xgate_data = {
    0, 0, 0, 0,  (1 << 5), &ECT_TC5
};

 CaptureSignal st5_pwm_xgate_data = {
    0, 0, 0, 0,  (1 << 6), &ECT_TC6
};

 CaptureSignal st7_pwm_xgate_data = {
    0, 0, 0, 0,  (1 << 7), &ECT_TC7
};
#pragma DATA_SEG DEFAULT
#pragma pop

const  EctContext st1_ect_context = {
   kEctChannel4,          //通道
   kEctCapture,           //工作模式
   kEctDiv4,              //自由定时器分频
   kCapBoth,              //捕捉的方式
   kOutCaptureDisable,  
   0,
   0,
                  
};

const  EctContext st3_ect_context = {
   kEctChannel5,          //通道
   kEctCapture,           //工作模式
   kEctDiv4,              //自由定时器分频
   kCapBoth,              //捕捉的方式
   kOutCaptureDisable,  
   0,
   0,
                  
};

const  EctContext st5_ect_context = {
   kEctChannel6,          //通道
   kEctCapture,           //工作模式
   kEctDiv4,              //自由定时器分频
   kCapBoth,              //捕捉的方式
   kOutCaptureDisable,  
   0,
   0,
                  
};

const  EctContext st7_ect_context = {
   kEctChannel7,          //通道
   kEctCapture,           //工作模式
   kEctDiv4,              //自由定时器分频
   kCapBoth,              //捕捉的方式
   kOutCaptureDisable,  
   0,
   0,
                  
};

const  EctContext cp_ect_context = {
   kEctChannel2,          //通道
   kEctCapture,           //工作模式
   kEctDiv4,              //自由定时器分频
   kCapBoth,              //捕捉的方式
   kOutCaptureDisable,  
   0,
   0,
                  
};

const  EctContext count_ect_context = {
   kEctChannelCount,          //通道
   0,                         //工作模式
   kEctDiv4,                  //自由定时器分频
   0,                         //捕捉的方式
   0,  
   0,
   0,
                  
};





/***********************************************************************
  * @brief           初始化继电器粘连输入捕捉通道
  * @param[in]       无
  * @return          1: ok   0:err
***********************************************************************/
char st_pwm_start(void) {
 
  
  if( Ect_Capture_OutCompare_Init(&st1_ect_context,1,1) != 1)return 0;
  if( Ect_Capture_OutCompare_Init(&st3_ect_context,1,1) != 1)return 0;
  if( Ect_Capture_OutCompare_Init(&st5_ect_context,1,1) != 1)return 0;
  if( Ect_Capture_OutCompare_Init(&st7_ect_context,1,1) != 1)return 0;
  
  return 1;


}

/***********************************************************************
  * @brief           初始化交流充电桩连接检测输入捕捉
  * @param[in]       无
  * @return          1: ok   0:err
***********************************************************************/
char cp_pwm_start(void) {

   if( Ect_Capture_OutCompare_Init(&cp_ect_context,1,1) != 1)return 0;
  
  return 1;
}

/***********************************************************************
  * @brief           获取继电器粘连故障占空比
  * @param[in]       pin  通道号
  * @return          占空比
***********************************************************************/
unsigned short st_pwm_get_duty(unsigned char pin) {
    if (pin > 12 || pin < 9) return 0;

    switch (pin) {
    case 9:
        return pwm_get_duty(&st1_pwm_xgate_data);
    case 10:
        return pwm_get_duty(&st3_pwm_xgate_data);
    case 11:
        return pwm_get_duty(&st5_pwm_xgate_data);
    case 12:
        return pwm_get_duty(&st7_pwm_xgate_data);
    default:
        return pwm_get_duty(&st1_pwm_xgate_data);
    }
}

/***********************************************************************
  * @brief           获取充电连接信号的占空比
  * @param[in]       无
  * @return          占空比大小
***********************************************************************/
unsigned short cp_pwm_get_duty(void) {
    // 返回低电平的占空比, 硬件上有反向
    return 1000 - pwm_get_duty(&cp_pwm_xgate_data);
}

/***********************************************************************
  * @brief           获取充电连接信号频率
  * @param[in]       无
  * @return          频率大小
***********************************************************************/
unsigned short cp_pwm_get_frequence(void) {
    unsigned long f = pwm_get_frequence(&cp_pwm_xgate_data);
    if (f > 65535) return 65535;
    return (unsigned short)f;
}

/***********************************************************************
  * @brief           判断充电连接信号是否为高
  * @param[in]       无
  * @return          1 高
***********************************************************************/
unsigned char cp_input_is_high(void) {
    return PTT_PTT2 == 0;
}

/***********************************************************************
  * @brief          ect中断处理
  * @param[in]       无
  * @return          无
***********************************************************************/
void PwmEctISR_Handler(void)
{
    //ECT_TFLG1_C2F = 1; // Clear interrupt
    
   // pwm_ect_isr_hook();
}

