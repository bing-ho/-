/*******************************************************************************
**         安徽力高                                     
**
**文 件 名SPI2.c                                       
**作    者：
**创建日期：
**修改记录：

**文件说明:SD卡底层SPI驱动源文件
**版    本:
**备    注：
*******************************************************************************/

/* MODULE SPI0. */
#ifndef _SPI2_SOURCE
#define _SPI2_SOURCE

#include "..\ucos_source\cpu\includes.h"





#pragma CODE_SEG DEFAULT

/*
********************************************************************************
**函数名称：SPI0_SendByte
**功    能：SPI0发送与接收函数
**参    数：void
**返 回 值：void
**备    注：
********************************************************************************
*/

unsigned char SPI2_SendByte(unsigned char ubData) 
{
	unsigned int timeOut;  
	timeOut = 0;      	
	SPI2DRL = ubData;
	while((!(SPI2SR_SPIF))  && (timeOut++ < 1000) );//等待数据接收完成 	    
	return SPI2DRL;  
}


/*
** ===================================================================
**     Method      :  SPI0_Init (bean Init_SPI)
**
**     Description :
**         This method initializes registers of the SPI module
**         according to this Peripheral Initialization Bean settings.
**         Call this method in user code to initialize the module.
**         By default, the method is called by PE automatically; see
**         "Call Init method" property of the bean for more details.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void SPI2_Init(void)
{
  
  
 	// MODRR_MODRR4 =0;   //使用S口
 	// DDRS    = 0xE0;    //SCK0=1,MOSI=1,pm0为cs 0b00110001
 	 MODRR   = 0x40;  
 	         
 	
 	// SPI0CR1: SPIE=0,SPE=0,SPTIE=0,MSTR=0,CPOL=0,CPHA=0,SSOE=0,LSBFE=0 */
 	setReg8(SPI2CR1, 0);                 /* Disable the SPI0 module and clearing flags in SPISR register */ 
 	/* SPI0CR2: ??=0,XFRW=0,??=0,MODFEN=0,BIDIROE=0,??=0,SPISWAI=0,SPC0=0 */
 	setReg8(SPI2CR2, 0);                  
 	/* SPI0BR: ??=0,SPPR2=0,SPPR1=1,SPPR0=0,??=0,SPR2=1,SPR1=1,SPR0=1 */
 	setReg8(SPI2BR, 0x02);            //1MHz       
 	//* SPI0CR1: SPIE=0,SPE=1,SPTIE=0,MSTR=1,CPOL=1,CPHA=1,SSOE=1,LSBFE=0 */
 	setReg8(SPI2CR1, 0x5E);    
 	/* SPI0CR2: ??=0,XFRW=0,??=0,MODFEN=1,BIDIROE=0,??=0,SPISWAI=0,SPC0=0 */
 	setReg8(SPI2CR2, 0x10);
 	
 	/* PPSS: PPSS6=0,PPSS5=0,PPSS4=0 */
 	clrReg8Bits(PPSH, 112);               
 	/* PERS: PERS6=1,PERS5=1,PERS4=1 */
 	setReg8Bits(PERH, 112);               
 	/* RDRS: RDRS7=0,RDRS6=0,RDRS5=0,RDRS4=0,RDRS3=0,RDRS2=0,RDRS1=0,RDRS0=0 */
 	setReg8(RDRH, 0);  
 	SPI2SR = 0xB0 ; //清除所有标志                   
}

/* END SPI0. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 3.00 [04.12]
**     for the Freescale HCS12X series of microcontrollers.
**
** ###################################################################
*/
#endif