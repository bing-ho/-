/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    ads1015_hardware.c                                       

** @brief       1.完成ads1015模块的数据类型的定义 
                2.ads1015模块硬件初始化函数 
                2.ads1015模块硬件读写函数 
** @copyright		Ligoo Inc.         
** @date		    2017-03-30.
** @author           
*******************************************************************************/ 
#include "ads1015_hardware.h"
#include "ucos_ii.h"

#include "iic_interface.h"



/************************************************************************
  * @brief           Ads1015模块ad转换延时函数 
  * @param[in]       ADS1015_BspInterface Ads1015模块配置属性
  * @param[in]       howmany：延时的系统时钟节拍数
  * @return          无
***********************************************************************/
 void ads_delay_ms(const ADS1015_BspInterface *iface, unsigned short howmany) {
    (void)iface;
    OSTimeDly(howmany);
}


/************************************************************************
  * @brief           Ads1015模块写寄存器 
  * @param[in]       i2c_bus Ads1015模块IIC总线配置属性
  * @param[in]       reg   寄存器地址
  * @param[in]       value 写的值
  * @return          0：write_err   1:write_ok
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
  * @brief           Ads1015模块读寄存器 
  * @param[in]       i2c_bus Ads1015模块IIC总线配置属性
  * @param[in]       reg    寄存器地址
  * @param[in]       pvalue 读取的值
  * @return          0：write_err   1:write_ok
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
  * @brief           Ads1015模块初始化
  * @param[in]       ADS1015_BspInterface Ads1015模块配置属性  
  * @return          无
***********************************************************************/
void ADS1015_Init(const ADS1015_BspInterface *iface) {
    (void)bus_i2c_init(iface->bus);
}



// ads1015模块配置属性初始化 
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






