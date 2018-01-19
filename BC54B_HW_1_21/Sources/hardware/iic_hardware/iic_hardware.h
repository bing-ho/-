/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    iic_hardware.h                                       

** @brief       1.完成IIC模块的数据类型的定义 
                2.完成IIC模块的初始化
                3.完成IIC模块的数据读写
** @copyright		Ligoo Inc.         
** @date		    2017-03-28.
** @author           
*******************************************************************************/ 
#ifndef __IIC_HARDWARE_H__
#define __IIC_HARDWARE_H__

#include "stdint.h"
#include "MC9S12XEP100.h"
#include "includes.h"
#ifndef __FAR
#define __FAR
#endif



/************************************************
  * @struct   hcs12_i2c_regs
  * @brief    iic模块硬件寄存器
 ***********************************************/
struct hcs12_i2c_regs {
    volatile IIC1_IBADSTR address;                //从地址
    volatile IIC1_IBFDSTR freq_div;               //频率分频寄存器
    
    /* bit7      bit6      bit5    bit4    bit3          bit2              bit1     bit0
    0  iic关闭  中断关闭   从模式  接收    第9位发送ack  无                保留     无
    1  开启     使能       主模式  发送    不发          产生重新开始周期  保留     进入等待模式
    */
    volatile IIC1_IBCRSTR control;                // 控制寄存器
    
    /* bit7       bit6        bit5    bit4    bit3    bit2        bit1     bit0
    0  正在传输   不访问      空闲    仲裁    保留    从接收      无       收到ack
    1  传输完成   从地址访问  忙      仲裁    保留    从发送      中断     没有收到
    */
    volatile IIC1_IBSRSTR status;                //状态寄存器
    volatile IIC1_IBDRSTR dat;                    //数据寄存器
    
    /* bit7       bit6        bit5    bit4    bit3    bit[2 : 0]   
    0  全乎关闭   7位地址     保留    保留    保留    
    1  全乎使能   10位地址    保留    保留    保留    标示10位地址的高三位
    */
    volatile IIC1_IBCR2STR control2;             //控制寄存器
};

/************************************************
  * @struct   hcs12_i2c_platform
  * @brief    iic模块硬件设备结构体
 ***********************************************/
struct hcs12_i2c_platform {
    struct hcs12_i2c_regs *regs;
    uint8_t freq_div;
};

/************************************************
  * @struct   hcs12_i2c
  * @brief    iic模块硬件配置属性
 ***********************************************/
struct hcs12_i2c {   
   const struct hcs12_i2c_platform *platform;  //指向iic寄存器的结构体指针
    union {
        struct {
            uint8_t aquired: 1;
            uint8_t txing: 1;
            uint8_t rxing: 1;
            uint8_t txing_addr: 1;
            uint8_t txed_addr: 1;
        } bits;
        uint8_t byte;
    } flags;

    const uint8_t *__FAR tx_data;
    uint8_t *__FAR rx_data;
    uint8_t tx_length;
    uint8_t tx_index;
    uint8_t rx_length;
    uint8_t rx_index;
    uint8_t slave_addr;
    OS_EVENT * muxtex_sem;

};

/************************************************
  * @typedef   hcs12_i2c_t
  * @brief     定义 hcs12_i2c 类型
 ***********************************************/
typedef struct hcs12_i2c *__FAR hcs12_i2c_t;

/************************************************************************
  * @brief           iic模块硬件初始化函数 
  * @param[in]       hcs12_i2c iic模块控制属性
  * @return          1：Init_OK   
***********************************************************************/
uint8_t hcs12_i2c_init(struct hcs12_i2c *__FAR i2c); 

/************************************************************************
  * @brief           清空iic模块硬件属性 
  * @param[in]       hcs12_i2c iic模块控制属性
  * @return          无  
***********************************************************************/
void hcs12_i2c_deinit(hcs12_i2c_t *__FAR i2c) ;


/************************************************************************
  * @brief           iic读写数据
  * @param[in]       hcs12_i2c iic模块配置属性
  * @param[in]       addr 子器件地址号
  * @param[in]       w 需要发送的数据指针
  * @param[in]       wlen 发送数据的长度  
  * @param[in]       r 返回读取数据的地址指针
  * @param[in]       rlen 读取数据的长度     
  * @return          len：收发成功的数据长度  
***********************************************************************/
uint8_t hcs12_i2c_write_then_read(hcs12_i2c_t i2c,
                                  uint8_t addr,
                                  const uint8_t *__FAR w,
                                  uint8_t wlen,
                                  uint8_t *__FAR r,
                                  uint8_t rlen);





#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

extern struct hcs12_i2c hcs12_impl_i2c0;

extern struct hcs12_i2c hcs12_impl_i2c1;

#pragma pop


#endif

