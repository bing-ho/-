/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   iic_hardware.c                                       

** @brief       1.���IICģ����������͵Ķ��� 
                2.���IICģ��ĳ�ʼ��
                3.���IICģ������ݶ�д
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
  * @brief           iicģ��Ӳ����ʼ������ 
  * @param[in]       hcs12_i2c iicģ���������
  * @return          1��Init_OK   
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
  * @brief           ���iicģ��Ӳ������ 
  * @param[in]       hcs12_i2c iicģ���������
  * @return          ��  
***********************************************************************/
void hcs12_i2c_deinit(hcs12_i2c_t *__FAR i2c) 
{
    (void)i2c;
}

/************************************************************************
  * @brief           iicģ��Ӳ���Ĵ�����ʼ�� 
  * @param[in]       hcs12_i2c_platform iicģ��Ĵ�����������
  * @return          ��  
***********************************************************************/
static void init_regs(const struct hcs12_i2c_platform *plat) 
{
    // clear status reg
    plat->regs->status.Byte = (1 << 7) | (1 << 4) | (1 << 1);   //�������λ�ٲ�λ���ж�λ
    plat->regs->control.Byte = 0xc4;   //�������� �����ж� �������¿�ʼ����
}

/************************************************************************
  * @brief           ���iicģ��Ӳ���Ĵ���
  * @param[in]       hcs12_i2c_platform iicģ��Ĵ�����������
  * @return          ��  
***********************************************************************/
static void deinit_regs(const struct hcs12_i2c_platform *plat) 
{
    plat->regs->control.Byte = 0;
}

/************************************************************************
  * @brief           ����iic����
  * @param[in]       hcs12_i2c iicģ����������
  * @return          1��start_ok  0:start_err  
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
  * @brief           ֹͣiic����
  * @param[in]       hcs12_i2c iicģ����������
  * @return          ��  
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
  * @brief           �ȴ�д���
  * @param[in]       hcs12_i2c iicģ����������
  * @return          len��д�ɹ������ݳ���  
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
  * @brief           �ȴ������
  * @param[in]       hcs12_i2c iicģ����������
  * @return          len�����������ݳ���  
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
  * @brief           �ȴ������
  * @param[in]       hcs12_i2c iicģ����������
  * @return          len�����������ݳ���  
***********************************************************************/
uint8_t hcs12_i2c_start_and_tx_addr(struct hcs12_i2c *__FAR i2c) 
{
    uint8_t addr;
    OS_CPU_SR cpu_sr = 0;
    unsigned int i = 0;
    const struct hcs12_i2c_platform *plat = i2c->platform;

    OS_ENTER_CRITICAL();
    for (;;) {
        if ((plat->regs->status.Byte & (1 << 5)) == 0) break;  //���߿���

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
    plat->regs->control.Byte |= 0x30;       // ��Ϊ��������

    for (i = 0; i < 1000; ++i) {
        if (plat->regs->status.Byte & (1 << 5)) { // ���������ɹ�
            addr = i2c->slave_addr << 1;          //�����ӻ���ַ
            if (i2c->tx_length <= 0)
             {
                addr = addr | 0x01;         //����д����
            }
            plat->regs->dat.Byte = addr;   // д������ַ
            return 1;
        }
    }    
    return 0;
}
/************************************************************************
  * @brief           iicд����
  * @param[in]       hcs12_i2c iicģ����������
  * @param[in]       addr ��������ַ��
  * @param[in]       dat ��Ҫ���͵�����ָ��
  * @param[in]       len �������ݵĳ���     
  * @return          len�����ͳɹ������ݳ���  
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
  * @brief           iic������ǰ�Ĵ�����ʼ�ĵ�ַ����
  * @param[in]       hcs12_i2c iicģ����������
  * @param[in]       addr ��������ַ��
  * @param[in]       dat ���ض�ȡ���ݵĵ�ַָ��
  * @param[in]       len ��ȡ���ݵĳ���     
  * @return          len����ȡ�������ݳ���  
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
  * @brief           iic��д����
  * @param[in]       hcs12_i2c iicģ����������
  * @param[in]       addr ��������ַ��
  * @param[in]       w ��Ҫ���͵�����ָ��
  * @param[in]       wlen �������ݵĳ���  
  * @param[in]       r ���ض�ȡ���ݵĵ�ַָ��
  * @param[in]       rlen ��ȡ���ݵĳ���     
  * @return          len���շ��ɹ������ݳ���  
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

    if (wlen > 0) {  //�ڶ�����ʱ���ȷ��ͼĴ�����ַ �ڷ��Ͷ����������ź�
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
  * @brief           iic��ʼ��������
  * @param[in]       hcs12_i2c iicģ����������  
  * @return          ��  
***********************************************************************/
static void start_rx_data(struct hcs12_i2c *__FAR i2c)
 {
    const struct hcs12_i2c_platform *plat = i2c->platform;
    if (i2c->rx_length == 1) { // ֻ����һ������ʱNAK
        plat->regs->control.Bits.TXAK = 1;
    } else {
        plat->regs->control.Bits.TXAK = 0;//���ջظ�ack
    }
    plat->regs->control.Bits.TX_RX = 0;      //���ݽ���ģʽ
    i2c->rx_data[0] = plat->regs->dat.Byte;
    i2c->rx_index = 0;
    i2c->flags.bits.rxing = 1;
}

/************************************************************************
  * @brief           iic��ʼ��������
  * @param[in]       hcs12_i2c iicģ����������  
  * @return          ��  
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
  * @brief           iic�жϴ�����
  * @param[in]       hcs12_i2c iicģ����������  
  * @return          ��  
***********************************************************************/
void hcs12_i2c_isr(struct hcs12_i2c *__FAR i2c) {
    const struct hcs12_i2c_platform *plat = i2c->platform;
    plat->regs->status.Bits.IBIF = 1;
    if (i2c->flags.bits.txing_addr) {      //��ַ�������
        i2c->flags.bits.txing_addr = 0;    //�����ַ��ַ��־λ
        if (plat->regs->status.Bits.RXAK) { // û���յ�ACK
            plat->regs->control.Byte &= ~0x20; // ֹͣ����
            goto __exit;
        }

        // ACK
        if (i2c->tx_length > i2c->tx_index) { // ��ʼ����
            start_tx_data(i2c);
        } else if (i2c->rx_length > i2c->rx_index) {//��ʼ����
            start_rx_data(i2c);
        }else;

        goto __exit;
    }

    if (i2c->flags.bits.txing) { // ���ݷ��ͳɹ���־.
        if (plat->regs->status.Bits.RXAK) { // û���յ�ack
            i2c->flags.bits.txing = 0;
            plat->regs->control.Byte &= ~0x20; // STOP
            goto __exit;
        }

        ++i2c->tx_index;

        //ACK
        if (i2c->tx_index >= i2c->tx_length) { // ���ݷ������.
            i2c->flags.bits.txing = 0;
            if (i2c->rx_length > 0)           //��Ҫ��ȡ���ݣ�ǰ�淢�͵�Ϊ���Ĵ�����ַ
            {
                plat->regs->control.Byte |= 1 << 2; // ��������
                plat->regs->dat.Byte = (i2c->slave_addr << 1) | 0x01; // ���Ͷ�������ַ addr
                i2c->flags.bits.txing_addr = 1;       //���ö���ַ��־
            } else {                                  //д��� ֹͣ����
                plat->regs->control.Byte &= ~0x20; // STOP
            }
            goto __exit;
        }

        // ������һ������.
        {
            uint8_t c = i2c->tx_data[i2c->tx_index];
            plat->regs->dat.Byte = c;
        }
        goto __exit;
    }

    if (i2c->flags.bits.rxing) // ���յ�һ������.
    { 
        if (i2c->rx_length == i2c->rx_index + 2) { // ��һ����������NACK
            plat->regs->control.Bits.TXAK = 1;  //������2�����ݶ���ɺ� �ر�ack��Ӧ
        } 
        else if (i2c->rx_length == i2c->rx_index + 1) { //������
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
  * @brief           iic0�ж���ں���
  * @param[in]       ��  
  * @return          ��  
***********************************************************************/
void i2c0_isr_handler(void) 
{
    hcs12_i2c_isr(&hcs12_impl_i2c0);
}

/************************************************************************
  * @brief           iic1�ж���ں���
  * @param[in]       ��  
  * @return          ��  
***********************************************************************/
void i2c1_isr_handler(void) 
{
    hcs12_i2c_isr(&hcs12_impl_i2c1);
}



