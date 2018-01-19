/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   ect_hardware.c                                       

** @brief       1.���ECTģ��ײ���������͵Ķ��� 
                2.���ECTģ��ײ�ĳ�ʼ��
                3.���ECTģ��ײ㲶׽�ȵĹ��ܺ���
                4.ECT�жϴ���ʽΪxgateЭ����������ʽ 
** @copyright		Ligoo Inc.         
** @date		    2017-03-30.
** @author            
*******************************************************************************/ 


#include "ect_hardware.h"
//#include "MC9S12XEP100.h"
//#include "os_cpu.h"
#include "xgate.h"
#include "derivative.h"

//#define ECT_TIOS_

/***********************************************************************
  * @brief           ���ڶ�ECTģ�����ɼ��������г�ʼ�� 
  * @param[in]       ECTContext  �ṹ��Ĳ���
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
unsigned char Ect_Tcnt_Init(const EctContext *__FAR ect_context,char to_xgate, unsigned char prio)
{
 (void)ect_context;
 interrupt_route_request(0x6F,to_xgate,prio);  //�����ж�Э�������ж�����  
 //ECT_TSCR2 = (1<<7)|ect_context->div ; //���� ��Ƶֵ ʹ��TCNT����ж�
 ECT_TSCR2 |= (1<<7); //���� ��Ƶֵ ʹ��Ĭ��ֵ ʹ��TCNT����ж�
 ECT_TSCR1 = 0xC0;                   //ʹ���ж�
 //ECT_TIE |= ect_context->channel; 
 return 1;
} 


/***********************************************************************
  * @brief           ���ڶ�ECTģ�鲶�������Ƚ��ж����Գ�ʼ�� 
  * @param[in]       ECTContext  �ṹ��Ĳ���
  * @param[in]       to_xgate    �Ƿ�ʹ��Э�������ж�
  * @param[in]       prio        �ж����ȼ�    
  * @return          1���ɹ�  0��ʧ��
***********************************************************************/
unsigned char Ect_Capture_OutCompare_Init(const EctContext *__FAR ect_context,char to_xgate, unsigned char prio) 
{
  unsigned char temp=0;

  if(ect_context->ect_fun == kEctCapture)       //���ò���
  {
    ECT_TIOS &= (~(1<<ect_context->channel));   //��Ӧλ����
    if(ect_context->channel > kEctChannel3) 
    {
      ECT_TCTL3 &= (~(3<<((ect_context->channel-kEctChannel3)<<1)));   //�Ƚ�λ����
      ECT_TCTL3 |= ect_context->edge<<((ect_context->channel-kEctChannel3)<<1);
    }
    else
    {
      ECT_TCTL4 &= (~(3<<((ect_context->channel)<<1))); 
      ECT_TCTL4 |= ect_context->edge<<(ect_context->channel<<1);
    }
  }
  else if(ect_context->ect_fun == kEctOutCompare) //��������Ƚ�
  {
    ECT_TIOS |= (1<<ect_context->channel);         //��Ӧλ��1
    if(ect_context->channel > kEctChannel3) 
    {
      ECT_TCTL1 &= (~(3<<((ect_context->channel-kEctChannel3)<<1)));   //�Ƚ�λ����
      ECT_TCTL1 |= ect_context->out_mode<<((ect_context->channel-kEctChannel3)<<1);
    }
    else
    {
      ECT_TCTL2 &= (~(3<<((ect_context->channel)<<1))); 
      ECT_TCTL2 |= ect_context->out_mode<<(ect_context->channel<<1);
    }
  }
  // else if(ect_context->ect_fun == kEctOutCompareTimeFlip) 
 //{
  
// } 
// else if(ect_context->ect_fun == kEctOutCompareOc7Contrul) 
 //{
  
 //}
// else if(ect_context->ect_fun == kEctOutPwm) 
 //{
  
 //}  
  else 
  {
    return 0;  
  }
  // ECT_TSCR2 |= ect_context->div ; //���� ��Ƶֵ
   
   temp = 0x77 -  ect_context->channel;
   if(temp>0x77||temp<0x6F)return 0;             //�жϺŵ�ַ����
   interrupt_route_request(temp,to_xgate,prio);  //�����ж�Э�������ж�����  
  
   ECT_TIE |= (1<<ect_context->channel);     //ʹ���ж�λ 
  
   return 1;

}


/***********************************************************************
  * @brief           ���ڶ�ECTģ���ݼ�������ģ����г�ʼ�� 
  * @param[in]       ECTContext  �ṹ��Ĳ���
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
void Ect_Mccnt_Init(const MccntConfig *__FAR ect_mccnt,char to_xgate, unsigned char prio) 
{
    interrupt_route_request(0x65,to_xgate,prio);  //�����ж�Э�������ж����� 
    ECT_MCCNT = ect_mccnt->initial_value;         //��ֵ
    ECT_MCCTL = (1<<7)|(ect_mccnt->count<<6)|(1<<2)|(ect_mccnt->div); 
    
    ECT_TSCR1 |= 0x80;
} 

#if 0  //�жϴ����ַ���xgate �ļ����� �� etc_xgate ��
/***********************************************************************
  * @brief           ���ڶ�ECTģ�鲶׽�ź��жϴ���
  * @param[in]       CaptureSignal�ṹ��Ĳ���
  * @return          ��
***********************************************************************/
interrupt void Capture_Hardware_ISR(CaptureSignal *__restrict pwm)
{
   //�жϽ����ж�Э����������
    unsigned long tmp;
    unsigned char edge = PTT & pwm->port_t_bit;
    unsigned short this_capture = *(pwm->ect2_tc);


    ECT_TFLG1 = (unsigned char)(pwm->port_t_bit); // clear flag

    tmp = pwm->count_l32 + (this_capture - pwm->last_capture);
    if (tmp < pwm->count_l32) {
        tmp = 0xFFFFFFFF;
    }

    asm {
        LOOP1: SSEM #PWM_SEM_NUM
        BCC LOOP1
    }

    if (edge) { // ������
        pwm->low_l32 = tmp;
    } else {
        pwm->high_l32 = tmp;
    }
    asm {
        CSEM #PWM_SEM_NUM
    }

    pwm->count_l32 = 0;
    pwm->last_capture = this_capture;
}

/***********************************************************************
  * @brief           ���ڶ�ECTģ������ȽϹ��ܵ��жϴ��� 
  * @param[in]       OutCompareSignal�ṹ��Ĳ���
  * @return          ��
***********************************************************************/
interrupt void OutCompare_Hardware_ISR(OutCompareSignal *__restrict ect_compare)
{
    //�жϽ����ж�Э����������
     ECT_TFLG1 = (unsigned char)(ect_compare->port_t_bit); // clear flag 
     /*
       ������Ҫ����û��жϴ�����  
     */
}


/***********************************************************************
  * @brief           ���ڶ�ECTģ�����������жϴ���
  * @param[in]       ��
  * @return          ��
***********************************************************************/
interrupt void Counter_Overflow_Hardware_ISR(int unused)
{
     unsigned long tmp;
    unsigned char level;
    unsigned char i;
    struct pwm_xgate_data *pwm;

    (void)unused;
     //�жϽ����ж�Э����������

    ECT_TFLG2 = ECT_TFLG2_TOF_MASK;

    for (i = 0; i < 5; ++i) {
        pwm = pwm_list[i];
        if (pwm == 0) break;

        level = PTT & pwm->port_t_bit;

        tmp = pwm->count_l32 + (65536 - pwm->last_capture);
        pwm->last_capture = 0;
        if (tmp < pwm->count_l32) {
            tmp = 0xFFFFFFFF;
        }
        pwm->count_l32 = tmp;

        asm {
            LOOP1: SSEM #PWM_SEM_NUM
            BCC LOOP1
        }

        if (level) {
            if (pwm->count_l32 > pwm->high_l32) {
                pwm->high_l32 = pwm->count_l32;
            }
        } else {
            if (pwm->count_l32 > pwm->low_l32) {
                pwm->low_l32 = pwm->count_l32;
            }
        }

        asm {
            CSEM #PWM_SEM_NUM
        }
    }
}

/***********************************************************************
  * @brief           ���ڶ�ECTģ��ģ���ݼ�����������жϴ���
  * @param[in]       ��
  * @return          ��
***********************************************************************/
interrupt void Modulus_Counter_Underflow_Hardware_ISR(int unused)
{
    //�жϽ����ж�Э����������
     (void)unused;
     ECT_MCFLG = ECT_MCFLG_MCZF_MASK;   //���flag��־
     /*
       ������Ҫ����û��жϴ�����  
     */
}

#endif 

/***********************************************************************
  * @brief           ���ڶ�ECTģ��������Ƚ�ʱ���ֵ
  * @param[in]       OutCompareSignal�ṹ��Ĳ���
                     UINT16 Timer  ��ֵֵ
  * @return          ��
***********************************************************************/
void Compare_SetTc(OutCompareSignal *__restrict pwm,INT16U Timer)
{
  *(pwm->ect2_tc) = Timer;
}
  

/***********************************************************************
  * @brief           ���ڶ�ȡECTģ�������벶׽ʱ�Ĳ�׽ֵ 
  * @param[in]       OutCompareSignal�ṹ��Ĳ���
  * @return          ��
***********************************************************************/
INT16U  Compare_GetTc (OutCompareSignal *__restrict pwm)
{
    INT16U Timer;
    Timer = *(pwm->ect2_tc);
    return Timer;
}














