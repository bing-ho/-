#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <string.h>
#include "xgate.hxgate"
#include "xgate.h"

#pragma push
#pragma DATA_SEG __RPAGE_SEG XGATE_STK_L
word XGATE_STACK_L[1];
#pragma DATA_SEG __RPAGE_SEG XGATE_STK_H
word XGATE_STACK_H[1];
#pragma pop

void interrupt_route_request(unsigned char index, char to_xgate, unsigned char prio) {
    unsigned char dat = to_xgate ? 0x80 : 0x00;
    INT_CFADDR = (index << 1) & 0xF0;     //保证一组8个
    dat |= (prio & 0x07);
    INT_CFDATA_ARR[index & 0x07] = dat;
}


void SetupXGATE(void) {
    /* initialize the XGATE vector block and
     set the XGVBR register to its start address */
    XGVBR = (unsigned int)(void *__far)(XGATE_VectorTable - XGATE_VECTOR_OFFSET);
    /* switch software trigger 0 interrupt to XGATE */

    /* when changing your derivative to non-core3 one please remove next five lines */
    XGISPSEL = 1;
    XGISP31 = (unsigned int)(void *__far)(XGATE_STACK_L + 1);
    XGISPSEL = 2;
    XGISP74 = (unsigned int)(void *__far)(XGATE_STACK_H + 1);
    XGISPSEL = 0;

    /* enable XGATE mode and interrupts */
    XGMCTL = 0xFBC1; /* XGE | XGFRZ | XGIE */
}

/*
  设置xgate 软件中断
*/
inline void xgate_software_set_int(unsigned char channel) 
{
  XGSWT = (channel<<8)|channel; 
}
/*
  清除xgate 软件中断标志
*/
inline void xgate_software_clear_flag(unsigned char channel) 
{
 XGSWT = (channel<<8)|((~channel)&0xff);  
}
