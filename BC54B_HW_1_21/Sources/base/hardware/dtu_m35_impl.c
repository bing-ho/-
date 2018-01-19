#include "dtu_m35_impl.h"
#include "bms_base_cfg.h"
#include "bms_config.h"
#include "ring_buffer.h"
#include "ucos_ii.h"
#include "gpio_interface.h"
#include "bms_input_signal_impl.h"

#pragma MESSAGE DISABLE C4001 // Condition always FALSE


#define NOT_USED(p) ((void)(p))

enum gpio_name {
    GPIO_DTU_POWER_EN,      //GPIO_MODE_OUTPUT_PUSHPULL
    GPIO_DTU_POWER_KEY,     //GPIO_MODE_OUTPUT_PUSHPULL
    GPIO_DTU_STATUS,        //GPIO_MODE_INPUT
    GPIO_GSM_ID             //GPIO_MODE_INPUT
};

#define STRING_DTU_POWER_EN         "DTU_POWER_EN"
#define STRING_DTU_POWER_KEY        "DTU_POWER_KEY"
#define STRING_DTU_STATUS           "DTU_STATUS"

#pragma push
#pragma CONST_SEG DEFAULT
const struct hcs12_gpio_detail gsm_gpio_out_v114[] = {
    { STRING_DTU_POWER_EN,                   { &PORTA,   &PORTA,     &DDRA,      1 << 1}, 0 , 0},//PA1 ==> PA0
    { STRING_DTU_POWER_KEY,                  { &PORTD,   &PORTD,     &DDRD,      1 << 3}, 0 , 0},
    { STRING_DTU_STATUS,                     { &PORTD,   &PORTD,     &DDRD,      1 << 2}, 1 , 0},
    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0}
};

const struct hcs12_gpio_detail gsm_gpio_out_v116[] = {
    { STRING_DTU_POWER_EN,                   { &PORTA,   &PORTA,     &DDRA,      1 << 0}, 0 , 0},
    { STRING_DTU_POWER_KEY,                  { &PORTD,   &PORTD,     &DDRD,      1 << 3}, 0 , 0},    
    { STRING_DTU_STATUS,                     { &PORTD,   &PORTD,     &DDRD,      1 << 2}, 1 , 0},
    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0}
};

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
static const struct hcs12_gpio_detail*  gsm_gpio_out = NULL;
static const struct hcs12_gpio_detail*  all_ver_gsm_gpio_out_list[] = 
{
#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    gsm_gpio_out_v114,
    gsm_gpio_out_v114,
    gsm_gpio_out_v116,
    gsm_gpio_out_v116
#else
    gsm_gpio_out_v116
#endif
};

#pragma pop

uint8_t rs485_send_over = 0;

static void m35_serial_init(int baud) {
    unsigned long br = (unsigned long)((BMS_BUS_CLOCK + (unsigned long)baud * 8UL) / ((unsigned long)baud * 16UL));
    SCI2BDH = (unsigned char)(br >> 8);
    SCI2BDL = (unsigned char)br;
    SCI2CR1 = 0x00;
    SCI2CR2 = 0x00;
    SCI2CR2 |= SCI2CR2_TE_MASK | SCI2CR2_RE_MASK | SCI2CR2_RIE_MASK;
}

extern void DTUM35_RecvByte(unsigned char b);

RINGBUFFER_DEFINE_STATIC(tx_buffer, 40);

void SCI2_ISR_Handler(void) {
    unsigned char state = SCI2SR1;
    if (state & (SCI2SR1_OR_MASK | SCI2SR1_FE_MASK | SCI2SR1_NF_MASK)) {
        unsigned char b;
        (void)SCI2DRL;
        if (RingBuffer_TryRead(&tx_buffer, &b, 1)) {
            SCI2DRL = b;
        } else {
            SCI2CR2_TIE = 0;
            rs485_send_over = 1;
        }
        return;
    }

    if (state & SCI2SR1_RDRF_MASK) {
        DTUM35_RecvByte(SCI2DRL);
    }

    if (state & SCI2SR1_TDRE_MASK) {
        unsigned char b;
        if (RingBuffer_TryRead(&tx_buffer, &b, 1)) {
            SCI2DRL = b;
        } else {
            SCI2CR2_TIE = 0;
            rs485_send_over = 1;
        }
    }
}

static void m35_serial_send(const unsigned char *__FAR dat, int len) {
    // 如果没有数据发送, 把第一个字节直接发送, 并打开发送中断.
    // 接着把剩余的数据放入发送缓冲.
    int sent;
    int this_buffered_len;
    unsigned char full_try_times = 0;

    OS_CPU_SR cpu_sr = 0;

    OS_ENTER_CRITICAL();
    // 触发发送
    if (!SCI2CR2_TIE) {
        SCI2CR2_TIE = 1;
        SCI2DRL = *dat++;
        --len;
    }

    if (len <= 0) {
        OS_EXIT_CRITICAL();
        return;
    }

    sent = 0;
    for (;;) {
        this_buffered_len = RingBuffer_TryWrite(&tx_buffer, &dat[sent], len - sent);
        if (this_buffered_len == 0) { // 缓冲区满
            if (++full_try_times >= 2) {
                unsigned char b;
                full_try_times = 0;
                // 多次都是缓冲区满, 可能是中断错过, 重新触发发送
                (void)RingBuffer_TryRead(&tx_buffer, &b, 1);
                SCI2CR2_TIE = 1;
                SCI2DRL = b;
            }
        } else {
            full_try_times = 0;
            sent += this_buffered_len;
            if (sent >= len) {
                OS_EXIT_CRITICAL();
                while(rs485_send_over == 0)
                {
                    OSTimeDly(1);    
                }
                rs485_send_over = 0;
                return;
            }
        }

        OS_EXIT_CRITICAL();
        OSTimeDly(5);
        OS_ENTER_CRITICAL();
    }
}

static char m35_is_poweron(void) {
    //return !hcs12_gpio_input_is_high(&pt_signal_input_pins[kInputPin_DTU_STATUS].gpio.io);
    return !hcs12_gpio_input_is_high(&gsm_gpio_out[GPIO_DTU_STATUS].io);
}

static void m35_set_pwrkey(char is_assert) {
    hcs12_gpio_set_output(&gsm_gpio_out[GPIO_DTU_POWER_KEY].io, is_assert);
}

static void m35_power_enable(char is_on) {
    hcs12_gpio_set_output(&gsm_gpio_out[GPIO_DTU_POWER_EN].io, is_on);
}

static void m35_control_init(int baud) 
{
    uint8_t i;
    
    PUCR_PUPAE = 1;         /* Pull-up Port A Enable */
    
#if BMS_SUPPORT_BY5248D == 0   
#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    gsm_gpio_out = all_ver_gsm_gpio_out_list[hardware_io_revision_get()];
#else
    gsm_gpio_out = all_ver_gsm_gpio_out_list[0];
#endif
#endif
    for(i=0; i<4; i++)
    {
        if(i < GPIO_DTU_STATUS)
        {
            hcs12_gpio_init(&gsm_gpio_out[i].io,       GPIO_MODE_OUTPUT_PUSHPULL);
        }
        else
        {
            hcs12_gpio_init(&gsm_gpio_out[i].io,       GPIO_MODE_INPUT);
        }
        hcs12_gpio_set_output(&gsm_gpio_out[i].io, gsm_gpio_out[i].def_val);
    }

    m35_serial_init(baud);
}


static int nvm_store_m35_data(const simcard_data_t *__FAR dat) {
    if (RES_OK != config_save_dtu_cimi((unsigned char *_PAGED)dat->cimi)) {
        return FALSE;
    }

    if (RES_OK != config_save_dtu_phone((unsigned char *_PAGED)dat->phone)) {
        return FALSE;
    }
    return TRUE;
}

static int nvm_restore_m35_data(simcard_data_t *__FAR dat) {
    if (RES_OK != config_get_dtu_cimi((unsigned char *_PAGED)dat->cimi, sizeof(dat->cimi))) {
        return FALSE;
    }
    if (RES_OK != config_get_dtu_phone((unsigned char *_PAGED)dat->phone, sizeof(dat->phone))) {
        return FALSE;
    }
    return TRUE;
}

const DTUM35_BspInterface g_DTUM35_BspInterface = {
#if DTUM35_CHECK != 0
    DTUM35_CHECK_MAGIC,
#endif
    nvm_restore_m35_data,
    nvm_store_m35_data,
    m35_control_init,
    m35_power_enable,
    m35_set_pwrkey,
    m35_is_poweron,
    m35_serial_send,
};

