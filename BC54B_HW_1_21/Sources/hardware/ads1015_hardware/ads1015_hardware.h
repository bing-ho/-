/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    ads1015_hardware.h                                       

** @brief       1.完成ads1015模块的数据类型的定义 
                2.ads1015模块硬件初始化函数 
                2.ads1015模块硬件读写函数 
** @copyright		Ligoo Inc.         
** @date		    2017-03-30.
** @author           
*******************************************************************************/
#ifndef __ADS1015_hardware_H__
#define __ADS1015_hardware_H__


#ifndef ADS1015_CHECK
#define ADS1015_CHECK 0
#endif

#define ADS1015_CHECK_MAGIC (0x1234bc6fUL)

#ifndef __FAR
#define __FAR
#endif

///ADS1015芯片的IIC地址主要由ADDR引脚决定：
///ADDR引脚接GND:----IIC地址为:100 1000(0X48)
///ADDR引脚接VDD:----IIC地址为:100 1001(0X49)
///ADDR引脚接SDA:----IIC地址为:100 1010(0X4A)
///ADDR引脚接SCL:----IIC地址为:100 1011(0X4B)
#define ADS1015_ADDR_ADDRRESS  ADS1015_ADDR_VDD  //硬件接ADDR引脚接VDD引脚
#define ADS1015_ADDR_GND (0x48)
#define ADS1015_ADDR_VDD (0x49)
#define ADS1015_ADDR_SDA (0x4A)
#define ADS1015_ADDR_SCL (0x4B)



/************************************************
  * @enum   ADS1015_Rate
  * @brief  ADS1015底层接口  速率设置
  ***********************************************/
typedef enum {
    ADS1015_128SPS=0,
    ADS1015_250SPS,
    ADS1015_490SPS,
    ADS1015_920SPS,
    ADS1015_1600SPS,
    ADS1015_2400SPS,
    ADS1015_3300SPS,
    ADS1015_3300_1SPS,
} ADS1015_Rate;



/************************************************
  * @enum   ADS1015_Rate
  * @brief  ADS1015底层接口  采集模式 
  ***********************************************/
typedef enum {
   ADS1015_CONTINUOUS_MODE=0,    //连续采集
   ADS1015_SINGLE_MODE,  	    //单次采集
   
 } ADS1015_Mode;



/************************************************
  * @struct   ADS1015_BspInterface
  * @brief    ADS1015底层接口  配置属性
  ***********************************************/
typedef struct ADS1015_BspInterface {
#if ADS1015_CHECK != 0
    unsigned long magic;
#endif
    const struct i2c_bus *__FAR bus; 
    ADS1015_Rate  ads1015rate;
    ADS1015_Mode  ad_mode;
    void (*init)(struct ADS1015_BspInterface  *__FAR platform); 
  
    char (*i2c_write)(const struct i2c_bus  *__FAR bus, unsigned char reg, unsigned short len); 

    char (*i2c_read)(const struct i2c_bus  *__FAR bus, unsigned char reg, unsigned short *pvalue);

    void (*delay_ms)(const struct ADS1015_BspInterface *iface, unsigned short howmany);
} ADS1015_BspInterface;



/************************************************
  * @extern   g_ADS1015_BspInterface
  * @brief    外部声明
  ***********************************************/
extern const ADS1015_BspInterface g_ADS1015_BspInterface;


#endif
