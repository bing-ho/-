/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    ect_hardware.c                                       

** @brief       1.完成ECT模块底层的数据类型的定义 
                2.完成ECT模块底层的初始化
                3.完成ECT模块底层捕捉等的功能函数
                4.ECT中断处理方式为xgate协处理器处理方式 
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
  * @brief           用于对ECT模块自由计数器进行初始化 
  * @param[in]       ECTContext  结构体的参数
  * @return          0：成功  1：失败
***********************************************************************/
unsigned char Ect_Tcnt_Init(const EctContext *__FAR ect_context,char to_xgate, unsigned char prio)
{
 (void)ect_context;
 interrupt_route_request(0x6F,to_xgate,prio);  //设置中断协处理器中断属性  
 //ECT_TSCR2 = (1<<7)|ect_context->div ; //设置 分频值 使能TCNT溢出中断
 ECT_TSCR2 |= (1<<7); //设置 分频值 使用默认值 使能TCNT溢出中断
 ECT_TSCR1 = 0xC0;                   //使能中断
 //ECT_TIE |= ect_context->channel; 
 return 1;
} 


/***********************************************************************
  * @brief           用于对ECT模块捕获和输出比较中断属性初始化 
  * @param[in]       ECTContext  结构体的参数
  * @param[in]       to_xgate    是否使用协处理器中断
  * @param[in]       prio        中断优先级    
  * @return          1：成功  0：失败
***********************************************************************/
unsigned char Ect_Capture_OutCompare_Init(const EctContext *__FAR ect_context,char to_xgate, unsigned char prio) 
{
  unsigned char temp=0;

  if(ect_context->ect_fun == kEctCapture)       //设置捕获
  {
    ECT_TIOS &= (~(1<<ect_context->channel));   //对应位置零
    if(ect_context->channel > kEctChannel3) 
    {
      ECT_TCTL3 &= (~(3<<((ect_context->channel-kEctChannel3)<<1)));   //先将位清零
      ECT_TCTL3 |= ect_context->edge<<((ect_context->channel-kEctChannel3)<<1);
    }
    else
    {
      ECT_TCTL4 &= (~(3<<((ect_context->channel)<<1))); 
      ECT_TCTL4 |= ect_context->edge<<(ect_context->channel<<1);
    }
  }
  else if(ect_context->ect_fun == kEctOutCompare) //设置输出比较
  {
    ECT_TIOS |= (1<<ect_context->channel);         //对应位置1
    if(ect_context->channel > kEctChannel3) 
    {
      ECT_TCTL1 &= (~(3<<((ect_context->channel-kEctChannel3)<<1)));   //先将位清零
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
  // ECT_TSCR2 |= ect_context->div ; //设置 分频值
   
   temp = 0x77 -  ect_context->channel;
   if(temp>0x77||temp<0x6F)return 0;             //中断号地址错误
   interrupt_route_request(temp,to_xgate,prio);  //设置中断协处理器中断属性  
  
   ECT_TIE |= (1<<ect_context->channel);     //使能中断位 
  
   return 1;

}


/***********************************************************************
  * @brief           用于对ECT模数递减计数器模块进行初始化 
  * @param[in]       ECTContext  结构体的参数
  * @return          0：成功  1：失败
***********************************************************************/
void Ect_Mccnt_Init(const MccntConfig *__FAR ect_mccnt,char to_xgate, unsigned char prio) 
{
    interrupt_route_request(0x65,to_xgate,prio);  //设置中断协处理器中断属性 
    ECT_MCCNT = ect_mccnt->initial_value;         //初值
    ECT_MCCTL = (1<<7)|(ect_mccnt->count<<6)|(1<<2)|(ect_mccnt->div); 
    
    ECT_TSCR1 |= 0x80;
} 

#if 0  //中断处理部分放在xgate 文件夹下 的 etc_xgate 中
/***********************************************************************
  * @brief           用于对ECT模块捕捉信号中断处理
  * @param[in]       CaptureSignal结构体的参数
  * @return          无
***********************************************************************/
interrupt void Capture_Hardware_ISR(CaptureSignal *__restrict pwm)
{
   //中断交给中断协处理器处理
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

    if (edge) { // 上升沿
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
  * @brief           用于对ECT模块输出比较功能的中断处理 
  * @param[in]       OutCompareSignal结构体的参数
  * @return          无
***********************************************************************/
interrupt void OutCompare_Hardware_ISR(OutCompareSignal *__restrict ect_compare)
{
    //中断交给中断协处理器处理
     ECT_TFLG1 = (unsigned char)(ect_compare->port_t_bit); // clear flag 
     /*
       根据需要添加用户中断处理函数  
     */
}


/***********************************************************************
  * @brief           用于对ECT模块计数器溢出中断处理
  * @param[in]       无
  * @return          无
***********************************************************************/
interrupt void Counter_Overflow_Hardware_ISR(int unused)
{
     unsigned long tmp;
    unsigned char level;
    unsigned char i;
    struct pwm_xgate_data *pwm;

    (void)unused;
     //中断交给中断协处理器处理

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
  * @brief           用于对ECT模块模数递减计数器溢出中断处理
  * @param[in]       无
  * @return          无
***********************************************************************/
interrupt void Modulus_Counter_Underflow_Hardware_ISR(int unused)
{
    //中断交给中断协处理器处理
     (void)unused;
     ECT_MCFLG = ECT_MCFLG_MCZF_MASK;   //清空flag标志
     /*
       根据需要添加用户中断处理函数  
     */
}

#endif 

/***********************************************************************
  * @brief           用于对ECT模块做输出比较时设初值
  * @param[in]       OutCompareSignal结构体的参数
                     UINT16 Timer  初值值
  * @return          无
***********************************************************************/
void Compare_SetTc(OutCompareSignal *__restrict pwm,INT16U Timer)
{
  *(pwm->ect2_tc) = Timer;
}
  

/***********************************************************************
  * @brief           用于读取ECT模块做输入捕捉时的捕捉值 
  * @param[in]       OutCompareSignal结构体的参数
  * @return          无
***********************************************************************/
INT16U  Compare_GetTc (OutCompareSignal *__restrict pwm)
{
    INT16U Timer;
    Timer = *(pwm->ect2_tc);
    return Timer;
}














