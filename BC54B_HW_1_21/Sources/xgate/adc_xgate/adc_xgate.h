#ifndef __ADC_XGATE_H__
#define __ADC_XGATE_H__

#include "adc_xgate.hxgate"
#include "stdint.h"

/***********************************************************************
  * @brief           adc1 中断初始化
  * @param[in]       无
  * @return          1: ok
***********************************************************************/
char soc_adc_init(void);

/***********************************************************************
  * @brief           soc  adc转换初始化结构函数
  * @param[in]       param           soc_adc_param结构体类型变量 
  * @return          1: ok
***********************************************************************/
char soc_adc_start(const struct soc_adc_param *__FAR param);

/***********************************************************************
  * @brief           soc  adc转换初始化结构函数独立方式
  * @param[in]       param           soc_adc_param结构体类型变量 
  * @return          1: ok
***********************************************************************/
char soc_adc_start_independence(const struct soc_adc_param *__FAR param);

/***********************************************************************
  * @brief           获取adc1 转换值 按通道延时获取方式
  * @param[in]       channel转换通道
  * @param[in]       value  转换结果
  * @param[in]       t      等待时长
  * @return          1：ok  0：err
***********************************************************************/
char soc_wait_channel_timeout(uint8_t channel, uint16_t *near value, unsigned short t);

/***********************************************************************
  * @brief           停止adc转换
  * @param[in]       无
  * @return          无
***********************************************************************/
void soc_adc_stop(void);

/***********************************************************************
  * @brief           获取adc1 转换值接口函数 获取方式等待阻塞信号
  * @param[in]       high、 low、  single转换结果
  * @return          1：ok  0：err
***********************************************************************/
char soc_wait(uint16_t *near c1, uint16_t *near c2, uint16_t *near single);

/***********************************************************************
  * @brief           获取adc1 转换值 获取方式延时获取
  * @param[in]       high   转换结果
  * @param[in]       low    转换结果
  * @param[in]       single 转换结果
  * @param[in]       t      等待时长
  * @return          1：ok  0：err
***********************************************************************/
char soc_wait_timeout(uint16_t *near c1, uint16_t *near c2, uint16_t *near c3,unsigned short t);

/***********************************************************************
  * @brief           soc 信号量值清零
  * @param[in]       无
  * @return          无
***********************************************************************/
void soc_clear_result(void);

char xadc0_init(void);
char xadc0_start(const struct xadc0_param *__FAR param);
char xadc0_wait(uint16_t *near result, unsigned short timeout_tick);

#endif
