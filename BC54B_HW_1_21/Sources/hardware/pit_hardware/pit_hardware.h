/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	  pit_hardware.c                                       

** @brief       1.完成PIT模块的数据类型的定义 
                2.完成PIT模块的初始化
                3.完成PIT模块的各种定时器的功能函数
** @copyright		Ligoo Inc.         
** @date		    2017-03-21.
** @author      韩 慧      
*******************************************************************************/
#ifndef __PIT_HARDWARE_H__
#define __PIT_HARDWARE_H__

#include "Types.h"
#include "os_cpu.h"

//typedef unsigned long  INT32U;
//typedef unsigned char  INT8U;

//微定时器的通道号0-1及最大编号2
typedef enum
{
    kPitMtDev0,
    kPitMtDev1,
    kPitMtDevMaxCount
}PitMtDev;


//定时器通道号0-7 及最大通道号8
typedef enum
{
    kPitDev0,
    kPitDev1,
    kPitDev2,
    kPitDev3,
    kPitDev4,
    kPitDev5,
    kPitDev6,
    kPitDev7,
    kPitDevMaxCount
}PitDev;


//PIT模块硬件初始化配置属性
typedef  struct
{
    PitMtDev mt_dev;  //微定时器通道号
    PitDev dev;    //定时器通道号   
    INT8U TimerBase;  //PIT模块中的8位计数器初值 
    INT16U Timer;    //PIT模块中的16位计数器初值
}PitContext;

extern PitContext pit_param;


char Pit_Hardware_Init(PitContext* pit_context);

void Pit_MicroTimer_Init(PitContext*  pit_context) ;

void Pit_Hardware_Start(PitDev dev);
void Pit_Hardware_Stop(PitDev dev);
void Pit_Hardware_SetInterrupt(PitDev dev,INT8U set);
void Pit_Hardware_Clear_Time_out_Flag(PitDev dev);
void Pit_Hardware_down_counter(PitDev dev,INT8U set);
char Pit_Hardware_Reload(PitContext*  pit_context);
void Pit_Hardware_isr(PitDev dev);

#endif
