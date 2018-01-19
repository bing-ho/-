/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    ect_hardware.h                                       

** @brief       1.完成ECT模块底层的数据类型的定义 
                2.完成ECT模块底层的初始化
                3.完成ECT模块底层捕捉等的功能函数
** @copyright		Ligoo Inc.         
** @date		    2017-03-30.
** @author            
*******************************************************************************/ 

#ifndef __ECT_HARDWARE_H__
#define __ECT_HARDWARE_H__


#include "os_cpu.h"
#include "types.h"

#include "stdint.h"  

#ifndef __FAR
#define __FAR
#endif

/************************************************
  * @enum   EctChannel
  * @brief  用来描述定时器通道号0-7 及最大通道号8
 ***********************************************/
typedef enum
{
    kEctChannel0=0,
    kEctChannel1,
    kEctChannel2,
    kEctChannel3,
    kEctChannel4,
    kEctChannel5,
    kEctChannel6,
    kEctChannel7,
    kEctChannelCount
}EctChannel;

/************************************************
  * @enum   EctDiv
  * @brief  用来描述分频值共8种分频值
 ***********************************************/
typedef enum
{
    kEctDiv1,
    kEctDiv2,
    kEctDiv4,
    kEctDiv8,
    kEctDiv16,
    kEctDiv32,
    kEctDiv64,
    kEctDiv128
}EctDiv;

/************************************************
  * @enum   EctCapMode
  * @brief  用来描述捕捉信号方式
 ***********************************************/
typedef enum
{
    kCapDisable, //禁止捕捉
    kCapUp,      //仅捕捉上升沿
    kCapDown,    //仅捕捉下降沿
    kCapBoth     //上升沿、下降沿均捕捉
}EctCapMode;

/************************************************
  * @enum   EctFunction
  * @brief  用来描述ECT模块工作方式
 ***********************************************/
typedef enum
{
    kEctCapture,       //输入捕捉功能
    kEctOutCompare,     //输出比较功能
    kEctOutCompareTimeFlip,   //输出比较占空比可调 定时器输出翻转模式
    kEctOutCompareOc7Contrul, //输出比较占空比可调 通道7增强控制
    kEctOutPwm         // 占空比可调pwm输出模式
}EctFunction; 

/************************************************
  * @enum   EctOutCompareMode
  * @brief  用来描述ECT模块作为比较输出时的输出动作
 ***********************************************/
typedef enum
{
    kOutCaptureDisable,         //不输出
    kOutCaptureChange,         //翻转输出
    kOutCaptureLow,           //输出低电平
    kOutCaptureHigh          //输出高电平
} EctOutCompareMode;

/************************************************
  * @struct   EctContext
  * @brief    用来描述ECT模块初始化结构体
 ***********************************************/
typedef struct  _ECTContext
{
    EctChannel  channel ;  //定时器通道号   
    EctFunction ect_fun;  //ECT 模块 功能选择 
    EctDiv      div;      //ECT模块分频值 
    EctCapMode  edge;    //ECT模块捕捉信号模式
    EctOutCompareMode  out_mode;    // ECT输出比较时输出操作
    unsigned int      timer;    //ECT输出比较时 比较初值 做pwm输出 占空比
    unsigned int      cycle;   //做pwm输出时 周期  oc7值
} EctContext;

/************************************************
  * @struct   CaptureSignal
  * @brief    用来描述输入捕捉的结构体
 ***********************************************/
typedef struct  _CaptureSignal {
    unsigned long high_cnt;     // 高电平时间
    unsigned long low_cnt;       //低电平时间
    unsigned long count_pre;    //保留时间
    unsigned short last_capture;   //上次捕获信号时间
    unsigned int port_t_bit;       //通道引脚号
    unsigned int *ect2_tc;        //对应通道口计数器
} CaptureSignal;

/************************************************
  * @struct   OutCompareSignal
  * @brief    用来描述比较输出的结构体
 ***********************************************/
typedef struct  _OutCompareSignal {
    unsigned char  last_level;    //上次输出电平信号  0:低 1：高
    unsigned char  now_level;     //当前输出电平信号   0:低 1：高
    unsigned int  last_timer;     //上次比较初值
    unsigned int  next_timer;     //下次比较初值
    unsigned int  port_t_bit;     //通道引脚号
    unsigned int *ect2_tc;        //对应通道口计数器
} OutCompareSignal; 



/************************************************
  * @enum   MccntCount
  * @brief  模数计数器运行方式
 ***********************************************/
typedef enum {
  SingleCount=0,
  CycleCount,  
}MccntCount;

/************************************************
  * @enum   MccntDiv
  * @brief  模数计数器分频值
 ***********************************************/
typedef enum
{
    MccntDiv1,
    MccntDiv2,
    MccntDiv4,
    MccntDiv8,
   
}MccntDiv;


/************************************************
  * @struct   MccntConfig
  * @brief  模数计数器配置属性
 ***********************************************/
typedef struct  _MccntConfig
{
  MccntCount    count;
  MccntDiv      div;
  unsigned int  initial_value;
  
}MccntConfig;


/***********************************************************************
  * @brief           用于对ECT模块进行初始化 
  * @param[in]       ECTContext  结构体的参数
  * @return          0：成功  1：失败
***********************************************************************/
unsigned char Ect_Tcnt_Init(const EctContext *__FAR ect_context,char to_xgate, unsigned char prio);

/***********************************************************************
  * @brief           用于对ECT模块捕获和输出比较中断属性初始化 
  * @param[in]       ECTContext  结构体的参数
  * @param[in]       to_xgate    是否使用协处理器中断
  * @param[in]       prio        中断优先级    
  * @return          1：成功  0：失败
***********************************************************************/
unsigned char Ect_Capture_OutCompare_Init(const EctContext *__FAR ect_context,char to_xgate, unsigned char prio);

/***********************************************************************
  * @brief           用于对ECT模数递减计数器模块进行初始化 
  * @param[in]       ECTContext  结构体的参数
  * @return          0：成功  1：失败
***********************************************************************/
void Ect_Mccnt_Init(const MccntConfig *__FAR ect_mccnt,char to_xgate, unsigned char prio);



/***********************************************************************
  * @brief           用于对ECT模块做输出比较时设初值
  * @param[in]       OutCompareSignal结构体的参数
                     UINT16 Timer  初值值
  * @return          无
***********************************************************************/
void Compare_SetTc(OutCompareSignal *__restrict pwm,INT16U Timer);


/***********************************************************************
  * @brief           用于读取ECT模块做输入捕捉时的捕捉值 
  * @param[in]       OutCompareSignal结构体的参数
  * @return          无
***********************************************************************/
INT16U Compare_GetTc(OutCompareSignal *__restrict pwm);

#endif
