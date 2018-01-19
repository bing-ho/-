#include <stdio.h>
#include "bms_base_cfg.h"
#include "ring_buffer.h"
#include "ucos_ii.h"
#include "stdio_impl.h"

#if 0
#if (!defined(STDIO_USE_CAN)) || (STDIO_USE_CAN == 0)

RINGBUFFER_DEFINE_STATIC(recv_buffer, 10);
static OS_EVENT *new_char_event;
void TERMIO_PutChar(char c) {
    while (SCI1SR1_TC == 0);
    SCI1DRL = c;
    if (c == '\n') {
        while (SCI1SR1_TC == 0);
        SCI1DRL = '\r';
    }
}

int TERMIO_GetChar(void) {
    unsigned char tmp;
    OS_CPU_SR cpu_sr = 0;

    for (;;) {
        OS_ENTER_CRITICAL();
        if (ringbuffer_try_read(&recv_buffer, &tmp, 1)) {
            OS_EXIT_CRITICAL();
            return tmp;
        }
        OS_EXIT_CRITICAL();

        OSSemPend(new_char_event, 0, &tmp);
        if (tmp != OS_ERR_NONE) {
            return -1;
        }
    }
}


void stdio_impl_init(void) {
    if (new_char_event == 0) {
        unsigned long br;
        new_char_event = OSSemCreate(0);
        br = (unsigned long)((BMS_BUS_CLOCK + (unsigned long)115200 * 8UL) / ((unsigned long)9600 * 16UL));

        SCI1BDH = (unsigned char)(br >> 8);
        SCI1BDL = (unsigned char)br;
        SCI1CR1 = 0x00;
        SCI1CR2 = 0x00;
        SCI1CR2 |= SCI1CR2_TE_MASK | SCI1CR2_RE_MASK | SCI1CR2_RIE_MASK ;
    }
}

void SCI1_ISR_Handler(void) {
    unsigned char state = SCI1SR1;
    if (state & (SCI1SR1_OR_MASK | SCI1SR1_FE_MASK | SCI1SR1_NF_MASK)) {
        (void)SCI1DRL;
    }

    if (state & SCI1SR1_RDRF_MASK) {
        (void)ringbuffer_put(&recv_buffer, SCI1DRL, 1);
        (void)OSSemPost(new_char_event);
    }
}


#else

void SCI1_ISR_Handler(void) {
    unsigned char state = SCI1SR1;
    if (state & (SCI1SR1_OR_MASK | SCI1SR1_FE_MASK | SCI1SR1_NF_MASK)) {
        (void)SCI1DRL;
    }
}
#endif

#endif
