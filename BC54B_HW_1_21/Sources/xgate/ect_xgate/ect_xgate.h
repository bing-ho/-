#ifndef __ECT_XGATE_H__
#define __ECT_XGATE_H__

// PWM
/***********************************************************************
  * @brief           ��ʼ��PWM��������
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void pwm_init(void);

/***********************************************************************
  * @brief           ��ȡ����PWM��ռ�ձ�
  * @param[in]       pwm_data  CaptureSignal�ṹ���Ͳ���
  * @return          ռ�ձȴ�С
***********************************************************************/
unsigned long pwm_get_cycle(CaptureSignal *__FAR pwm_data);

/***********************************************************************
  * @brief           ��ȡ����PWM������
  * @param[in]       pwm_data  CaptureSignal�ṹ���Ͳ���
  * @return          ���ڴ�С
***********************************************************************/
unsigned long pwm_get_frequence(CaptureSignal *__FAR pwm_data);

/***********************************************************************
  * @brief           ��ȡ����PWM��Ƶ��
  * @param[in]       pwm_data  CaptureSignal�ṹ���Ͳ���
  * @return          Ƶ�ʴ�С
***********************************************************************/
unsigned short pwm_get_duty( CaptureSignal *__FAR pwm_data);

#endif
