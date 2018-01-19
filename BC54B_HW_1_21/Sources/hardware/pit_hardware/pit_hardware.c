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

#include "pit_hardware.h"
#include "MC9S12XEP100.h"
#include "stddef.h"
 
#define 	PIT_ERROR       0x01
#define 	PIT_OK          0x00



/*
********************************************************************************
**函数名称：Pit_MicroTimer_Init
**功    能：PIT微定时器赋初值
**参    数：PitContext结构体指针参数
**返 回 值：无
********************************************************************************
*/ 
void Pit_MicroTimer_Init(PitContext*  pit_context) 
{
  if(pit_context->mt_dev== kPitMtDev0)
        {
            PITCFLMT_PFLMT0 =  0 ;              //关闭微定时器0
            PITMTLD0 = pit_context->TimerBase;  //给微定时器设初值
            PITCFLMT_PFLMT0 =  1 ;
        } 
  else {
            PITCFLMT_PFLMT1 =  0 ;              //关闭微定时器0
            PITMTLD1 = pit_context->TimerBase;  //给微定时器设初值
            PITCFLMT_PFLMT1 =  1 ;
       }
  
}

/*
********************************************************************************
**函数名称：Pit_Hardware_Init
**功    能：PIT硬件初始化
**参    数：PitContext结构体指针参数
**返 回 值：void
********************************************************************************
*/ 
char Pit_Hardware_Init(PitContext*  pit_context)
{
    if(pit_context->mt_dev >= kPitMtDevMaxCount)    return PIT_ERROR;
    if(pit_context->dev >= kPitDevMaxCount)    return PIT_ERROR;
    if(pit_context->mt_dev== kPitMtDev0)
        {
            switch(pit_context->dev)
            {
                case kPitDev0:
                	PITLD0 = pit_context->Timer; //设定时器Timer 0初值
                    break;
                case kPitDev1:
                	PITLD1 = pit_context->Timer; //设定时器Timer 1初值
                    break;
                case kPitDev2:
                	PITLD2 = pit_context->Timer; //设定时器Timer 2初值
                    break;
                case kPitDev3:
                	PITLD3 = pit_context->Timer; //设定时器Timer 3初值
                    break;
                case kPitDev4:
                	PITLD4 = pit_context->Timer; //设定时器Timer 4初值
                    break;
                case kPitDev5:
                	PITLD5 = pit_context->Timer; //设定时器Timer 5初值
                    break;
                case kPitDev6:
                	PITLD6 = pit_context->Timer; //设定时器Timer 6初值
                    break;
                case kPitDev7:
                	PITLD7 = pit_context->Timer; //设定时器Timer 7初值
                    break;
            }
            PITCE &=~(1<< pit_context->dev); //关闭定时器通道0
            PITMUX &=~(1<< pit_context->dev);//定时器使用微时间基准0
            PITINTE &=~(1<< pit_context->dev); //关闭定时器的溢出中断
    	}
    if(pit_context->mt_dev== kPitMtDev1)
        {
            switch(pit_context->dev)
            {
                case  kPitDev0:
                	PITLD0 = pit_context->Timer; //设定时器Timer 0初值
                    break;
                case  kPitDev1:
                	PITLD1 = pit_context->Timer; //设定时器Timer1初值
                    break;
                case  kPitDev2:
                	PITLD2 = pit_context->Timer; //设定时器Timer 2初值
                    break;
                case  kPitDev3:
                  	PITLD3 = pit_context->Timer; //设定时器Timer 3初值
                    break;
                case  kPitDev4:
                	PITLD4 = pit_context->Timer; //设定时器Timer 4初值
                    break;
                case  kPitDev5:
                	PITLD5 = pit_context->Timer; //设定时器Timer 5初值
                    break;
                case  kPitDev6:
                	PITLD6 = pit_context->Timer; //设定时器Timer 6初值
                    break;
                case  kPitDev7:
                	PITLD7 = pit_context->Timer; //设定时器Timer7初值
                    break;
            }
            PITCE &=~(1<< pit_context->dev); //关闭定时器通道0
            PITMUX |= (1<< pit_context->dev); //定时器使用基准1
            PITINTE &=~(1<< pit_context->dev); //关闭定时器溢出中断
    	}
    	return PIT_OK;
}

/*
********************************************************************************
**函数名称：Pit_Hardware_Start
**功    能：启动16位定时器
**参    数：PitDev参数，选择设备号;
**返 回 值：void
********************************************************************************
*/ 
void  Pit_Hardware_Start(PitDev dev)
{
    PITCFLMT_PITE = 1;   //使能PIT
    PITCE |= (1<<dev);//开启定时器
}

/*
********************************************************************************
**函数名称：Pit_Hardware_Stop
**功    能：关闭16位定时器
**参    数：PitDev参数，选择设备号;
**返 回 值：void
********************************************************************************
*/
void  Pit_Hardware_Stop(PitDev dev)
{
    PITCE &= ~(1<<dev);//关闭定时器
}

/*
********************************************************************************
**函数名称：Pit_Hardware_SetInterrupt
**功    能：使能/失能 PIT中断
**参    数：PitDev参数，选择设备号;
            set    1：使能， 0：失能
**返 回 值：void
********************************************************************************
*/
void  Pit_Hardware_SetInterrupt(PitDev dev,INT8U set)
{
    if(set == 1)
        PITINTE |=(1<<dev); //开启定时器中断
    if(set == 0)
        PITINTE &=~(1<<dev); //关闭定时器中断
} 

/*
********************************************************************************
**函数名称：Pit_Hardware_Clear_Time_out_Flag
**功    能：清除溢出标志位
**参    数：PitDev参数，选择设备号;
**返 回 值：void
********************************************************************************
*/
void Pit_Hardware_Clear_Time_out_Flag(PitDev dev)
{
    PITTF = 1<< dev; //清除溢出标志位 默认为0
}

/*
********************************************************************************
**函数名称：Pit_Hardware_down_counter
**功    能：将相应的十六位定时器重载倒计数器模式
**参    数：PitDev参数，选择设备号;
**返 回 值：void
********************************************************************************
*/
void Pit_Hardware_down_counter(PitDev dev,INT8U set)
{
    if(set == 1)
        PITFLT = 1<< dev;//置1为倒计数器模式，默认为0
    if(set == 0)
        PITFLT = 0<< dev;//默认为0
}


char Pit_Hardware_Reload(PitContext*  pit_context)
{
    if(pit_context->mt_dev >= kPitMtDevMaxCount)    return PIT_ERROR;
    if(pit_context->dev >= kPitDevMaxCount)    return PIT_ERROR;
    if(pit_context->mt_dev== kPitMtDev0)
        {
            switch(pit_context->dev)
            {
                case kPitDev0:
                	PITLD0 = pit_context->Timer; //设定时器Timer 0初值
                    break;
                case kPitDev1:
                	PITLD1 = pit_context->Timer; //设定时器Timer 1初值
                    break;
                case kPitDev2:
                	PITLD2 = pit_context->Timer; //设定时器Timer 2初值
                    break;
                case kPitDev3:
                	PITLD3 = pit_context->Timer; //设定时器Timer 3初值
                    break;
                case kPitDev4:
                	PITLD4 = pit_context->Timer; //设定时器Timer 4初值
                    break;
                case kPitDev5:
                	PITLD5 = pit_context->Timer; //设定时器Timer 5初值
                    break;
                case kPitDev6:
                	PITLD6 = pit_context->Timer; //设定时器Timer 6初值
                    break;
                case kPitDev7:
                	PITLD7 = pit_context->Timer; //设定时器Timer 7初值
                    break;
            }
    	}
    if(pit_context->mt_dev== kPitMtDev1)
        {
            switch(pit_context->dev)
            {
                case  kPitDev0:
                	PITLD0 = pit_context->Timer; //设定时器Timer 0初值
                    break;
                case  kPitDev1:
                	PITLD1 = pit_context->Timer; //设定时器Timer1初值
                    break;
                case  kPitDev2:
                	PITLD2 = pit_context->Timer; //设定时器Timer 2初值
                    break;
                case  kPitDev3:
                  	PITLD3 = pit_context->Timer; //设定时器Timer 3初值
                    break;
                case  kPitDev4:
                	PITLD4 = pit_context->Timer; //设定时器Timer 4初值
                    break;
                case  kPitDev5:
                	PITLD5 = pit_context->Timer; //设定时器Timer 5初值
                    break;
                case  kPitDev6:
                	PITLD6 = pit_context->Timer; //设定时器Timer 6初值
                    break;
                case  kPitDev7:
                	PITLD7 = pit_context->Timer; //设定时器Timer7初值
                    break;
            }
    	}
    	return PIT_OK;
}

void Pit_Hardware_isr(PitDev dev) 
{
    PITCE &= ~(1<<dev);//关闭定时器
    PITTF = 1<< dev; //清除溢出标志位 默认为0
    // 开始采集通道channel
   // Pit_Hardware_Reload(&pit_param);
    PITCE |= (1<<dev);//开启定时器   
}