/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    iic_hardware.c                                       

** @brief       1.完成IIC模块的数据类型的定义 
                2.完成IIC模块的初始化
                3.完成IIC模块的数据读写
** @copyright		Ligoo Inc.         
** @date		    2017-03-28.
** @author           
*******************************************************************************/ 


#include "iic_hardware.h"
#include "ucos_ii.h"



const struct hcs12_i2c_platform i2c0_platform = 
{
    (struct hcs12_i2c_regs *)0xe0,
#if BMS_SUPPORT_CURRENT_DIVIDER 
    0x67u,
#else
	0x27u,
#endif
	
};

const struct hcs12_i2c_platform i2c1_platform = 
{
    (struct hcs12_i2c_regs *)0xb0,
#if BMS_SUPPORT_CURRENT_DIVIDER
    0x27u, 
#else
    0x67u,
#endif	  
};



#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
struct hcs12_i2c hcs12_impl_i2c0 = 
{
    &i2c0_platform,
};

struct hcs12_i2c hcs12_impl_i2c1 = 
{
    &i2c1_platform,
};   
#pragma DATA_SEG DEFAULT

#pragma pop




/************************************************************************
  * @brief           iic模块硬件初始化函数 
  * @param[in]       hcs12_i2c iic模块控制属性
  * @return          1：Init_OK   
***********************************************************************/
uint8_t hcs12_i2c_init(struct hcs12_i2c *__FAR i2c) 
{
    const struct hcs12_i2c_platform *plat = i2c->platform;
    i2c->tx_length = 0;
    i2c->tx_index = 0;
    i2c->tx_data = 0;
    i2c->rx_length = 0;
    i2c->rx_index = 0;
    i2c->rx_data = 0;
    i2c->flags.byte = 0;
    plat->regs->freq_div.Byte = plat->freq_div;
    // enable, master, tx, txak,
    plat->regs->control.Byte = 0;
    if(i2c->muxtex_sem == NULL) 
    {
       i2c->muxtex_sem =OSSemCreate(1);
    }
    
    return 1;
}

/************************************************************************
  * @brief           清空iic模块硬件属性 
  * @param[in]       hcs12_i2c iic模块控制属性
  * @return          无  
***********************************************************************/
void hcs12_i2c_deinit(hcs12_i2c_t *__FAR i2c) 
{
    (void)i2c;
}

/************************************************************************
  * @brief           iic模块硬件寄存器初始化 
  * @param[in]       hcs12_i2c_platform iic模块寄存器配置属性
  * @return          无  
***********************************************************************/
static void init_regs(const struct hcs12_i2c_platform *plat) 
{
    // clear status reg
    plat->regs->status.Byte = (1 << 7) | (1 << 4) | (1 << 1);   //清除传输位仲裁位和中断位
    plat->regs->control.Byte = 0xc4;   //开启总线 开启中断 产生重新开始周期
}

/************************************************************************
  * @brief           清空iic模块硬件寄存器
  * @param[in]       hcs12_i2c_platform iic模块寄存器配置属性
  * @return          无  
***********************************************************************/
static void deinit_regs(const struct hcs12_i2c_platform *plat) 
{
    plat->regs->control.Byte = 0;
}

/************************************************************************
  * @brief           启动iic总线
  * @param[in]       hcs12_i2c iic模块配置属性
  * @return          1：start_ok  0:start_err  
***********************************************************************/
static uint8_t acquire_i2c(struct hcs12_i2c *__FAR i2c) 
{
    int i = 0;
    OS_CPU_SR cpu_sr = 0;

    for (;;) {
        OS_ENTER_CRITICAL();
        if (i2c->flags.bits.aquired == 0) {
            i2c->flags.bits.aquired = 1;
            OS_EXIT_CRITICAL();
            init_regs(i2c->platform);
            return 1;
        }
        if (i >= 20) {
            OS_EXIT_CRITICAL();
            return 0;
        }
        ++i;
        OSTimeDly(1);
    }
}

/************************************************************************
  * @brief           停止iic总线
  * @param[in]       hcs12_i2c iic模块配置属性
  * @return          无  
***********************************************************************/
static void release_i2c(struct hcs12_i2c *__FAR i2c) 
{
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    deinit_regs(i2c->platform);
    i2c->flags.byte = 0;
    OS_EXIT_CRITICAL();
}

/************************************************************************
  * @brief           等待写完成
  * @param[in]       hcs12_i2c iic模块配置属性
  * @return          len：写成功的数据长度  
***********************************************************************/
static uint8_t wait_txing(struct hcs12_i2c *__FAR i2c) 
{
    uint8_t i = 0;
    OS_CPU_SR cpu_sr = 0;  
    OS_ENTER_CRITICAL();
    for (;;) {
        if (i2c->tx_index >= i2c->tx_length) break;
	#if BMS_SUPPORT_CURRENT_DIVIDER
        if (i > 2*i2c->tx_length) break;   
	#else
        if (i > i2c->tx_length) break;
	#endif
        OS_EXIT_CRITICAL();
        OSTimeDly(1);
        OS_ENTER_CRITICAL();
        ++i;
    }

    i = i2c->tx_index;
    OS_EXIT_CRITICAL();
    return i;
}

/************************************************************************
  * @brief           等待读完成
  * @param[in]       hcs12_i2c iic模块配置属性
  * @return          len：读到的数据长度  
***********************************************************************/
static uint8_t wait_rxing(struct hcs12_i2c *__FAR i2c) 
{
    uint8_t i = 0;
    OS_CPU_SR cpu_sr = 0; 
    
    OS_ENTER_CRITICAL();
    for (;;) {
        if (i2c->rx_index >= i2c->rx_length) break;
        if (i > i2c->rx_length) break;

        OS_EXIT_CRITICAL();
        OSTimeDly(1);
        OS_ENTER_CRITICAL();
        ++i;
    }

    i = i2c->rx_index;
    OS_EXIT_CRITICAL();
    return i;
}

/************************************************************************
  * @brief           等待读完成
  * @param[in]       hcs12_i2c iic模块配置属性
  * @return          len：读到的数据长度  
***********************************************************************/
uint8_t hcs12_i2c_start_and_tx_addr(struct hcs12_i2c *__FAR i2c) 
{
    uint8_t addr;
    OS_CPU_SR cpu_sr = 0;
    unsigned int i = 0;
    const struct hcs12_i2c_platform *plat = i2c->platform;

    OS_ENTER_CRITICAL();
    for (;;) {
        if ((plat->regs->status.Byte & (1 << 5)) == 0) break;  //总线空闲

        if (i > 10) {
            OS_EXIT_CRITICAL();
            return 0;
        }
        OS_EXIT_CRITICAL();
        OSTimeDly(1);
        OS_ENTER_CRITICAL();
        i++;
    }

    i2c->flags.bits.txing_addr = 1;
    OS_EXIT_CRITICAL();
    plat->regs->control.Byte |= 0x30;       // 作为主机发送

    for (i = 0; i < 1000; ++i) {
        if (plat->regs->status.Byte & (1 << 5)) { // 总线启动成功
            addr = i2c->slave_addr << 1;          //拷贝从机地址
            if (i2c->tx_length <= 0)
             {
                addr = addr | 0x01;         //发送写命令
            }
            plat->regs->dat.Byte = addr;   // 写器件地址
            return 1;
        }
    }    
    return 0;
}
/************************************************************************
  * @brief           iic写数据
  * @param[in]       hcs12_i2c iic模块配置属性
  * @param[in]       addr 子器件地址号
  * @param[in]       dat 需要发送的数据指针
  * @param[in]       len 发送数据的长度     
  * @return          len：发送成功的数据长度  
***********************************************************************/
uint8_t hcs12_i2c_write(struct hcs12_i2c *__FAR i2c,
                        uint8_t addr,
                        const uint8_t *__FAR dat,
                        uint8_t len) {
    if (!acquire_i2c(i2c)) return 0;

    i2c->tx_data = dat;
    i2c->tx_length = len;
    i2c->tx_index = 0;
    i2c->rx_length = 0;
    i2c->flags.byte = 0;
    i2c->slave_addr = addr;

    if (!hcs12_i2c_start_and_tx_addr(i2c)) {
        release_i2c(i2c);
        return 0;
    }

    len = wait_txing(i2c);
    release_i2c(i2c);
    return len;
}

/************************************************************************
  * @brief           iic读出当前寄存器开始的地址数据
  * @param[in]       hcs12_i2c iic模块配置属性
  * @param[in]       addr 子器件地址号
  * @param[in]       dat 返回读取数据的地址指针
  * @param[in]       len 读取数据的长度     
  * @return          len：读取到的数据长度  
***********************************************************************/
uint8_t hcs12_i2c_read(struct hcs12_i2c *__FAR i2c,
                       uint8_t addr,
                       uint8_t *__FAR dat,
                       uint8_t len) 
{
    if (!acquire_i2c(i2c)) return 0;

    i2c->tx_length = 0;
    i2c->rx_data = dat;
    i2c->rx_length = len;
    i2c->rx_index = 0;
    i2c->slave_addr = addr;

    if (!hcs12_i2c_start_and_tx_addr(i2c)) {
        release_i2c(i2c);
        return 0;
    }

    len = wait_rxing(i2c);
    release_i2c(i2c);
    return len;

}

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
                                  uint8_t rlen) 
{
    uint8_t len;
    INT8U err = 0;
    (void)OSSemPend (i2c->muxtex_sem , 0, &err);
    
    if (!acquire_i2c(i2c)) 
    {
      (void)OSSemPost (i2c->muxtex_sem);
      return 0;     
    }

    i2c->tx_length = wlen;
    i2c->tx_data = w;
    i2c->tx_index = 0;
    i2c->rx_length = rlen;
    i2c->rx_data = r;
    i2c->rx_index = 0;
    i2c->slave_addr = addr;

    if (!hcs12_i2c_start_and_tx_addr(i2c)) {
        release_i2c(i2c);
        (void)OSSemPost (i2c->muxtex_sem);
        return 0;
    }

    if (wlen > 0) {  //在读数据时，先发送寄存器地址 在发送对子器件读信号
        len = wait_txing(i2c);
        if (len < wlen) {
           
            release_i2c(i2c);
            (void)OSSemPost (i2c->muxtex_sem);
            return len;
        }
    } else {
        len = 0;
    }

    if (rlen > 0) {
        len += wait_rxing(i2c);
    }

    release_i2c(i2c);
    (void)OSSemPost (i2c->muxtex_sem);
    return len;
}

/************************************************************************
  * @brief           iic开始接受数据
  * @param[in]       hcs12_i2c iic模块配置属性  
  * @return          无  
***********************************************************************/
static void start_rx_data(struct hcs12_i2c *__FAR i2c)
 {
    const struct hcs12_i2c_platform *plat = i2c->platform;
    if (i2c->rx_length == 1) { // 只接收一个数据时NAK
        plat->regs->control.Bits.TXAK = 1;
    } else {
        plat->regs->control.Bits.TXAK = 0;//接收回复ack
    }
    plat->regs->control.Bits.TX_RX = 0;      //数据接收模式
    i2c->rx_data[0] = plat->regs->dat.Byte;
    i2c->rx_index = 0;
    i2c->flags.bits.rxing = 1;
}

/************************************************************************
  * @brief           iic开始发送数据
  * @param[in]       hcs12_i2c iic模块配置属性  
  * @return          无  
***********************************************************************/
static void start_tx_data(struct hcs12_i2c *__FAR i2c)
 {
    const struct hcs12_i2c_platform *plat = i2c->platform;
    i2c->flags.bits.txing = 1;
    plat->regs->dat.Byte = i2c->tx_data[0];
    i2c->tx_index = 0;
    i2c->flags.bits.txing = 1;
}

/************************************************************************
  * @brief           iic中断处理函数
  * @param[in]       hcs12_i2c iic模块配置属性  
  * @return          无  
***********************************************************************/
void hcs12_i2c_isr(struct hcs12_i2c *__FAR i2c) {
    const struct hcs12_i2c_platform *plat = i2c->platform;
    plat->regs->status.Bits.IBIF = 1;
    if (i2c->flags.bits.txing_addr) {      //地址发送完成
        i2c->flags.bits.txing_addr = 0;    //清除地址地址标志位
        if (plat->regs->status.Bits.RXAK) { // 没有收到ACK
            plat->regs->control.Byte &= ~0x20; // 停止总线
            goto __exit;
        }

        // ACK
        if (i2c->tx_length > i2c->tx_index) { // 开始发送
            start_tx_data(i2c);
        } else if (i2c->rx_length > i2c->rx_index) {//开始接收
            start_rx_data(i2c);
        }else;

        goto __exit;
    }

    if (i2c->flags.bits.txing) { // 数据发送成功标志.
        if (plat->regs->status.Bits.RXAK) { // 没有收到ack
            i2c->flags.bits.txing = 0;
            plat->regs->control.Byte &= ~0x20; // STOP
            goto __exit;
        }

        ++i2c->tx_index;

        //ACK
        if (i2c->tx_index >= i2c->tx_length) { // 数据发送完成.
            i2c->flags.bits.txing = 0;
            if (i2c->rx_length > 0)           //需要读取数据，前面发送的为读寄存器地址
            {
                plat->regs->control.Byte |= 1 << 2; // 重启总线
                plat->regs->dat.Byte = (i2c->slave_addr << 1) | 0x01; // 发送读器件地址 addr
                i2c->flags.bits.txing_addr = 1;       //设置读地址标志
            } else {                                  //写完成 停止总线
                plat->regs->control.Byte &= ~0x20; // STOP
            }
            goto __exit;
        }

        // 发送下一个数据.
        {
            uint8_t c = i2c->tx_data[i2c->tx_index];
            plat->regs->dat.Byte = c;
        }
        goto __exit;
    }

    if (i2c->flags.bits.rxing) // 接收到一个数据.
    { 
        if (i2c->rx_length == i2c->rx_index + 2) { // 下一个接收数据NACK
            plat->regs->control.Bits.TXAK = 1;  //倒数第2个数据读完成后 关闭ack回应
        } 
        else if (i2c->rx_length == i2c->rx_index + 1) { //发送完
            plat->regs->control.Byte &= ~0x20; // STOP
            i2c->flags.bits.rxing = 0;
        }
        else;
        i2c->rx_data[i2c->rx_index++] = plat->regs->dat.Byte;
    }
__exit:
    plat->regs->status.Bits.IBIF = 1;
    return;
}

/************************************************************************
  * @brief           iic0中断入口函数
  * @param[in]       无  
  * @return          无  
***********************************************************************/
void i2c0_isr_handler(void) 
{
    hcs12_i2c_isr(&hcs12_impl_i2c0);
}

/************************************************************************
  * @brief           iic1中断入口函数
  * @param[in]       无  
  * @return          无  
***********************************************************************/
void i2c1_isr_handler(void) 
{
    hcs12_i2c_isr(&hcs12_impl_i2c1);
}



