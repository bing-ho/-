#ifndef __ECT_XGATE_H__
#define __ECT_XGATE_H__

// PWM
/***********************************************************************
  * @brief           初始化PWM捕获设置
  * @param[in]       无
  * @return          无
***********************************************************************/
void pwm_init(void);

/***********************************************************************
  * @brief           获取输入PWM的占空比
  * @param[in]       pwm_data  CaptureSignal结构体型参数
  * @return          占空比大小
***********************************************************************/
unsigned long pwm_get_cycle(CaptureSignal *__FAR pwm_data);

/***********************************************************************
  * @brief           获取输入PWM的周期
  * @param[in]       pwm_data  CaptureSignal结构体型参数
  * @return          周期大小
***********************************************************************/
unsigned long pwm_get_frequence(CaptureSignal *__FAR pwm_data);

/***********************************************************************
  * @brief           获取输入PWM的频率
  * @param[in]       pwm_data  CaptureSignal结构体型参数
  * @return          频率大小
***********************************************************************/
unsigned short pwm_get_duty( CaptureSignal *__FAR pwm_data);

#endif
