/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   ads1015_hardware.c                                       

** @brief       1.���ads1015ģ����������͵Ķ��� 
                2.ads1015ģ��Ӳ����ʼ������ 
                2.ads1015ģ��Ӳ����д���� 
** @copyright		Ligoo Inc.         
** @date		    2017-03-30.
** @author           
*******************************************************************************/ 
#include "ads1015_hardware.h"
#include "ucos_ii.h"

#include "iic_interface.h"



/************************************************************************
  * @brief           Ads1015ģ��adת����ʱ���� 
  * @param[in]       ADS1015_BspInterface Ads1015ģ����������
  * @param[in]       howmany����ʱ��ϵͳʱ�ӽ�����
  * @return          ��
***********************************************************************/
 void ads_delay_ms(const ADS1015_BspInterface *iface, unsigned short howmany) {
    (void)iface;
    OSTimeDly(howmany);
}


/************************************************************************
  * @brief           Ads1015ģ��д�Ĵ��� 
  * @param[in]       i2c_bus Ads1015ģ��IIC������������
  * @param[in]       reg   �Ĵ�����ַ
  * @param[in]       value д��ֵ
  * @return          0��write_err   1:write_ok
***********************************************************************/
  char ads_write_reg(const struct i2c_bus *__FAR bus, unsigned char reg, unsigned short value) { 
    unsigned char dat[3];

    dat[0] = reg;
    dat[1] = (value >> 8);
    dat[2] = (value & 0xFF);

    if (sizeof(dat) != bus_i2c_transmit(bus, ADS1015_ADDR_ADDRRESS, dat, sizeof(dat), 0, 0)) {
        return 0;
    }
    return 1;
}


/************************************************************************
  * @brief           Ads1015ģ����Ĵ��� 
  * @param[in]       i2c_bus Ads1015ģ��IIC������������
  * @param[in]       reg    �Ĵ�����ַ
  * @param[in]       pvalue ��ȡ��ֵ
  * @return          0��write_err   1:write_ok
***********************************************************************/
 char ads_read_reg(const struct i2c_bus *__FAR bus, unsigned char reg, unsigned short *pvalue) {
  
    unsigned char dat[2];

    if (sizeof(dat) + 1 != bus_i2c_transmit(bus, ADS1015_ADDR_ADDRRESS, &reg, 1, dat, sizeof(dat))) {
        return 0;
    }

    *pvalue = ((dat[0] << 8) | dat[1]);
    return 1;
}


 
 /************************************************************************
  * @brief           Ads1015ģ���ʼ��
  * @param[in]       ADS1015_BspInterface Ads1015ģ����������  
  * @return          ��
***********************************************************************/
void ADS1015_Init(const ADS1015_BspInterface *iface) {
    (void)bus_i2c_init(iface->bus);
}



// ads1015ģ���������Գ�ʼ�� 
const ADS1015_BspInterface g_ADS1015_BspInterface = 
{
#if ADS1015_CHECK != 0
    ADS1015_CHECK_MAGIC,
#endif
    &hcs12_i2c_bus_0,
    ADS1015_1600SPS,
    ADS1015_SINGLE_MODE,
    ADS1015_Init,
    ads_write_reg,
    ads_read_reg,
    ads_delay_ms,
};






