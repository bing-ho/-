#ifndef __ADC_XGATE_H__
#define __ADC_XGATE_H__

#include "adc_xgate.hxgate"
#include "stdint.h"

/***********************************************************************
  * @brief           adc1 �жϳ�ʼ��
  * @param[in]       ��
  * @return          1: ok
***********************************************************************/
char soc_adc_init(void);

/***********************************************************************
  * @brief           soc  adcת����ʼ���ṹ����
  * @param[in]       param           soc_adc_param�ṹ�����ͱ��� 
  * @return          1: ok
***********************************************************************/
char soc_adc_start(const struct soc_adc_param *__FAR param);

/***********************************************************************
  * @brief           soc  adcת����ʼ���ṹ����������ʽ
  * @param[in]       param           soc_adc_param�ṹ�����ͱ��� 
  * @return          1: ok
***********************************************************************/
char soc_adc_start_independence(const struct soc_adc_param *__FAR param);

/***********************************************************************
  * @brief           ��ȡadc1 ת��ֵ ��ͨ����ʱ��ȡ��ʽ
  * @param[in]       channelת��ͨ��
  * @param[in]       value  ת�����
  * @param[in]       t      �ȴ�ʱ��
  * @return          1��ok  0��err
***********************************************************************/
char soc_wait_channel_timeout(uint8_t channel, uint16_t *near value, unsigned short t);

/***********************************************************************
  * @brief           ֹͣadcת��
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void soc_adc_stop(void);

/***********************************************************************
  * @brief           ��ȡadc1 ת��ֵ�ӿں��� ��ȡ��ʽ�ȴ������ź�
  * @param[in]       high�� low��  singleת�����
  * @return          1��ok  0��err
***********************************************************************/
char soc_wait(uint16_t *near c1, uint16_t *near c2, uint16_t *near single);

/***********************************************************************
  * @brief           ��ȡadc1 ת��ֵ ��ȡ��ʽ��ʱ��ȡ
  * @param[in]       high   ת�����
  * @param[in]       low    ת�����
  * @param[in]       single ת�����
  * @param[in]       t      �ȴ�ʱ��
  * @return          1��ok  0��err
***********************************************************************/
char soc_wait_timeout(uint16_t *near c1, uint16_t *near c2, uint16_t *near c3,unsigned short t);

/***********************************************************************
  * @brief           soc �ź���ֵ����
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void soc_clear_result(void);

char xadc0_init(void);
char xadc0_start(const struct xadc0_param *__FAR param);
char xadc0_wait(uint16_t *near result, unsigned short timeout_tick);

#endif
