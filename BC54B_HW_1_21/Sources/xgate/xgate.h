#ifndef __XGATE_H_
#define __XGATE_H_

#include "derivative.h"

#define inline

#define XGATE_VECTOR_OFFSET 9

void SetupXGATE(void);
void interrupt_route_request(unsigned char index, char to_xgate, unsigned char prio);


#define CPU_SET_SEM(x)      (XGSEM = 0x0101 << (x))
#define CPU_SEM_IS_SET(x)   (XGSEM & 0x0001 << (x))
#define CPU_RELEASE_SEM(x)  (XGSEM = 0x0100 << (x))

#define IRQ_ISR_XGATE_NUM     0x79
#define RTI_ISR_XGATE_NUM     0x78
#define ECT0_ISR_XGATE_NUM    0x77
#define ECT1_ISR_XGATE_NUM    0x76
#define ECT2_ISR_XGATE_NUM    0x75
#define ECT3_ISR_XGATE_NUM    0x74
#define ECT4_ISR_XGATE_NUM    0x73
#define ECT5_ISR_XGATE_NUM    0x72
#define ECT6_ISR_XGATE_NUM    0x71
#define ECT7_ISR_XGATE_NUM    0x70
#define ECT_OVERFLOW_ISR_XGATE_NUM         0x6F
#define PC_A_OVERFLOW_ISR_XGATE_NUM        0x6E
#define PC_EDGE_ISR_XGATE_NUM              0x6D
#define SPI0_ISR_XGATE_NUM                 0x6C
#define SCI0_ISR_XGATE_NUM                 0x6B
#define SCI1_ISR_XGATE_NUM                 0x6A
#define ATD0_ISR_XGATE_NUM                 0x69
#define ATD1_ISR_XGATE_NUM                 0x68
#define PORTJ_ISR_XGATE_NUM                0x67
#define PORTH_ISR_XGATE_NUM                0x66
#define MDC_UNDERFLOW_ISR_XGATE_NUM        0x65
#define PC_B_OVERFLOW_ISR_XGATE_NUM        0x64
#define PLL_LOCK_ISR_XGATE_NUM             0x63
#define SELF_CLOCK_MODE_ISR_XGATE_NUM      0x62
#define IIC0_ISR_XGATE_NUM                 0x60
#define SPI1_ISR_XGATE_NUM                 0x5F
#define SPI2_ISR_XGATE_NUM                 0x5E
#define EEPROM_ISR_XGATE_NUM               0x5D
#define FLASH_ISR_XGATE_NUM                0x5C
#define CAN0_WAKEUP_ISR_XGATE_NUM          0x5B
#define CAN0_ERROR_ISR_XGATE_NUM           0x5A
#define CAN0_RECEIVE_ISR_XGATE_NUM         0x59
#define CAN0_TRANSMIT_ISR_XGATE_NUM        0x58
#define CAN1_WAKEUP_ISR_XGATE_NUM          0x57
#define CAN1_ERROR_ISR_XGATE_NUM           0x56
#define CAN1_RECEIVE_ISR_XGATE_NUM         0x55
#define CAN1_TRANSMIT_ISR_XGATE_NUM        0x54
#define CAN2_WAKEUP_ISR_XGATE_NUM          0x53
#define CAN2_ERROR_ISR_XGATE_NUM           0x52
#define CAN2_RECEIVE_ISR_XGATE_NUM         0x51
#define CAN2_TRANSMIT_ISR_XGATE_NUM        0x50
#define CAN3_WAKEUP_ISR_XGATE_NUM          0x4F
#define CAN3_ERROR_ISR_XGATE_NUM           0x4E
#define CAN3_RECEIVE_ISR_XGATE_NUM         0x4D
#define CAN3_TRANSMIT_ISR_XGATE_NUM        0x4C
#define CAN4_WAKEUP_ISR_XGATE_NUM          0x4B
#define CAN4_ERROR_ISR_XGATE_NUM           0x4A
#define CAN4_RECEIVE_ISR_XGATE_NUM         0x49
#define CAN4_TRANSMIT_ISR_XGATE_NUM        0x48
#define PORTP_ISR_XGATE_NUM                0x47
#define PWM_SGUTDOWN_ISR_XGATE_NUM         0x46
#define SCI2_ISR_XGATE_NUM            0x45
#define SCI3_ISR_XGATE_NUM            0x44
#define SCI4_ISR_XGATE_NUM            0x43
#define SCI5_ISR_XGATE_NUM            0x42
#define IIC1_ISR_XGATE_NUM            0x41
#define LVI_ISR_XGATE_NUM             0x40
#define API_ISR_XGATE_NUM             0x3F
#define PIT0_ISR_XGATE_NUM            0x3D
#define PIT1_ISR_XGATE_NUM            0x3C
#define PIT2_ISR_XGATE_NUM            0x3B
#define PIT3_ISR_XGATE_NUM            0x3A
#define XST0_ISR_XGATE_NUM            0x39
#define XST1_ISR_XGATE_NUM            0x38
#define XST2_ISR_XGATE_NUM            0x37
#define XST3_ISR_XGATE_NUM            0x36
#define XST4_ISR_XGATE_NUM            0x35
#define XST5_ISR_XGATE_NUM            0x34
#define XST6_ISR_XGATE_NUM            0x33
#define XST7_ISR_XGATE_NUM            0x32
#define XST_ERROR_ISR_XGATE_NUM       0x31

#define TO_XGATE       1 

#define  XST0   0x00 
#define  XST1   0x01 
#define  XST2   0x02 
#define  XST3   0x03 
#define  XST4   0x04 
#define  XST5   0x05 
#define  XST6   0x06 
#define  XST7   0x07  


inline void xgate_software_set_int(unsigned char channel);

inline void xgate_software_clear_flag(unsigned char channel);




#endif /* __XGATE_H_ */
