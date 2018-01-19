/*******************************************************************************
 **                       安徽新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:ltc68041_impl.c
 **作    者:曹志勇
 **创建日期:2016.12.15
 **文件说明:LTC6804采用菊链式
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef _LTC68041_IMPL_C_ 
#define _LTC68041_IMPL_C_

#include "ltc_util.h"
#include "ltc68041_impl.h" 
//#include "SPI1.h"
#include "app_cfg.h"
#include "bms_config.h"
#include "main_bsu_relay_ctrl.h"

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU_LTC6804
//LTC6804均衡控制字
static INT16U ltc6804_balance_ctrl_word[MAX_LTC6804_IC_NUM] = {0};
//6804Gpio引脚控制
static Ltc6804Gpio ltc6804_gpio[MAX_LTC6804_IC_NUM] = {0};
//菊链写寄存器时临时缓存.
static INT8U ltc6804_reg_buffer[MAX_LTC6804_IC_NUM][REG_WITH_PEC_LEN] = {0};
#pragma DATA_SEG DEFAULT 

/*****************************************************************************
 *函数名称:balance_ctrl_word_convert
 *函数功能:因跨线需要将均衡控制字转换
 *参    数:INT8U current_ic,当前的芯片
           INT16U ctrl_word控制字(未经跨箱处理的控制字)
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
INT16U balance_ctrl_word_convert(INT8U current_ic, INT16U ctrl_word)
{
	INT16U new_ctrl_word = 0;//新的控制字(经跨箱处理的)
	INT8U i = 0,index = 0;
    volatile INT16U bits = 0;
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return 0;
    ctrl_word = ctrl_word&0x0FFF;
	//跨箱处理
	bits = config_get(kBSU1VoltSampleNoUseBitIndex+current_ic);
    if((bits!=0)&&(ctrl_word != 0)) 
    {  //允许跨箱 且需要均衡
       new_ctrl_word = 0;
       index = 0;
       
       for(i = 0;i < LTC6804_MAX_CELL_NUM;i++) 
       {
           if(GET(bits,i))
           {//需要跨箱,跳过  
              continue;
           }
           if(GET(ctrl_word,index)) 
           {
             SET(new_ctrl_word,i);
           }
           index++;
       }
    }
    else
    {  //无需跨箱处理
       new_ctrl_word = ctrl_word;
    }
    return new_ctrl_word;
}


/*****************************************************************************
 *函数名称:set_ltc6804_balance_ctrl_word
 *函数功能:设置LTC6804均衡控制字
 *参    数:INT8U current_ic,当前的芯片
           INT16U ctrl_word控制字(未经跨箱处理的控制字)
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void set_ltc6804_balance_ctrl_word(INT8U current_ic, INT16U ctrl_word)
{
	OS_CPU_SR cpu_sr = 0;
	INT16U new_ctrl_word = 0;//新的控制字(经跨箱处理的)
	//参数检查
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return;

     new_ctrl_word = balance_ctrl_word_convert(current_ic,ctrl_word);

     
	OS_ENTER_CRITICAL();
	ltc6804_balance_ctrl_word[current_ic] = (new_ctrl_word & 0x0FFF);
	OS_EXIT_CRITICAL();

}
/*****************************************************************************
 *函数名称:get_ltc6804_balance_ctrl_word
 *函数功能:获取LTC6804均衡控制字
 *参    数:INT8U current_ic,当前的芯片
 *返 回 值:均衡控制字
 *修订信息:
 ******************************************************************************/
static INT16U get_ltc6804_balance_ctrl_word(INT8U current_ic)
{
	OS_CPU_SR cpu_sr = 0;
	INT16U ctrl_word = 0;
	//参数检查
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return 0;

	OS_ENTER_CRITICAL();
	ctrl_word = (ltc6804_balance_ctrl_word[current_ic]& 0x0FFF);
	OS_EXIT_CRITICAL();
	return ctrl_word;
}
/*****************************************************************************
 *函数名称:clr_all_ltc6804_balance_ctrl_word
 *函数功能:清除被动均衡控制字
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void clr_all_ltc6804_balance_ctrl_word(void)
{
	INT8U current_ic = 0;
	OS_CPU_SR cpu_sr = 0;
	for (current_ic = 0; current_ic < MAX_LTC6804_IC_NUM; current_ic++)
	{
        OS_ENTER_CRITICAL();
        ltc6804_balance_ctrl_word[current_ic] = 0;
        OS_EXIT_CRITICAL();
	}
}
/*****************************************************************************
 *函数名称:set_ltc6804_gpio2_pin_high
 *函数功能:设置GPIO2位为高
 *参    数:INT8U current_ic当前6804编号
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_ltc6804_gpio2_pin_high(INT8U current_ic)
{
   		//参数检查
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return ;
   	ltc6804_gpio[current_ic].Bits.GPIO2 = 1;
}
/*****************************************************************************
 *函数名称:SetLTCGpio2BitLow
 *函数功能:设置GPIO2位为低
 *参    数:INT8U current_ic当前6804编号
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_ltc6804_gpio2_pin_low(INT8U current_ic)
{
   	//参数检查
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return ;
   	ltc6804_gpio[current_ic].Bits.GPIO2 = 0;
}
/*****************************************************************************
 *函数名称:GetLTCGpio2Bit
 *函数功能:获取GPIO2位
 *参    数:INT8U current_ic当前6804编号
 *返 回 值:
 *修订信息:
 ******************************************************************************/
INT8U is_ltc6804_gpio2_pin_high(INT8U current_ic)
{
   	//参数检查
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return 0;
   	return ltc6804_gpio[current_ic].Bits.GPIO2;
}
/*****************************************************************************
 *函数名称:set_ltc6804_gpio1_pin_high
 *函数功能:设置GPIO3位为高
 *参    数:INT8U current_ic当前6804编号
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_ltc6804_gpio1_pin_high(INT8U current_ic)
{
   //参数检查
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return ;
   	ltc6804_gpio[current_ic].Bits.GPIO1 = 1;
}
/*****************************************************************************
 *函数名称:set_ltc6804_gpio3_pin_low
 *函数功能:设置GPIO1位为低
 *参    数:INT8U current_ic当前6804编号
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_ltc6804_gpio1_pin_low(INT8U current_ic)
{
   	//参数检查
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return ;
   	ltc6804_gpio[current_ic].Bits.GPIO1 = 0;
}
/*****************************************************************************
 *函数名称:is_ltc6804_gpio1_pin_high
 *函数功能:获取GPIO1位
 *参    数:INT8U current_ic当前6804编号
 *返 回 值:
 *修订信息:
 ******************************************************************************/
INT8U is_ltc6804_gpio1_pin_high(INT8U current_ic)
{
   	//参数检查
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return 0;
   	return ltc6804_gpio[current_ic].Bits.GPIO1;
}
/*****************************************************************************
 *函数名称:bsu_board_gpio_init
 *函数功能:采集板IO初始化
 *参    数:INT8U current_ic当前6804编号
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void bsu_board_gpio_init(INT8U current_ic)
{
    if (current_ic >= MAX_LTC6804_IC_NUM)
		return ;
    //GPIO1 GPIO2做继电器控制使用,需要置0
    ltc6804_gpio[current_ic].Bits.GPIO1 = 1;
    ltc6804_gpio[current_ic].Bits.GPIO2 = 1;
    ltc6804_gpio[current_ic].Bits.GPIO3 = 1;
    ltc6804_gpio[current_ic].Bits.GPIO4 = 1;
    ltc6804_gpio[current_ic].Bits.GPIO5 = 1;
    ltc6804_gpio[current_ic].Bits.NA = 0;
}

/*****************************************************************************
 *函数名称:broad_cast_ltc6804_cmd
 *函数功能:广播命令,内含CS电平操作。
 *参    数:cmd 命令
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void broad_cast_ltc6804_cmd(INT16U cmd)
{
	INT8U tx_data[4] = { 0 }; //发送数组
	INT16U tmp_pec = 0; //pec校验
    
	tx_data[0] = (INT8U) (cmd >> 8);
	tx_data[1] = (INT8U) cmd;
	tmp_pec = pec15_calc(2, tx_data);
	tx_data[2] = (INT8U) (tmp_pec >> 8);
	tx_data[3] = (INT8U) (tmp_pec);

	wakeup_iso_spi_idle();
    OSSchedLock();
	ltc6804_cs_pin_low(); //片选
	(void)spi_write_array(&ltc6804_high_speed_spi_bus,4,tx_data);
	ltc6804_cs_pin_high(); //取消片选
	OSSchedUnlock();
}
/*****************************************************************************
 *函数名称:clr_ltc6804_reg
 *函数功能:清除LTC6804寄存器,包括电池电压、辅助、状态寄存器。10个周期执行一次
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void clr_ltc6804_reg(void)
{
	static INT8U cnt = 0;
	if (cnt % 10 == 0)
	{
		broad_cast_ltc6804_cmd(CLRCELL);
		OSTimeDly(2);
		broad_cast_ltc6804_cmd(CLRAUX);
		OSTimeDly(2);
		broad_cast_ltc6804_cmd(CLRSTAT);
		OSTimeDly(5);
	}
	cnt++;
}
/*****************************************************************************
 *函数名称:start_ltc6804_cells_volt_adc
 *函数功能:启动电池电压采集
 *参    数:INT8U MD 模式 三种模式MD_FAST快速、MD_NORMAL正常、MD_FILTERED滤波
 *         INT8U DCP 采集均衡允许，DCP_ENABLED使能 DCP_DISABLED禁止
 *         INT8U CH 通道  CELL_CH_ALL所有
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void start_ltc6804_cells_volt_adc(INT8U MD, INT8U DCP, INT8U CH)
{
	
	INT16U tmp_16u = 0;
	INT16U cmd = 0;
	MD = MD & 0x03;
	DCP = DCP & 0x01;
	CH = CH & 0x07;

	tmp_16u = ((INT16U)MD<<7);
	cmd = tmp_16u  + (DCP << 4) + CH + 0x260;
	
	broad_cast_ltc6804_cmd(cmd);
}
/*****************************************************************************
 *函数名称:start_ltc6804_volt_cable_open_adc
 *函数功能:启动电压导线开路采集
 *参    数:INT8U MD 模式 三种模式MD_FAST快速、MD_NORMAL正常、MD_FILTERED滤波
 *         INT8U PUP上拉下拉电流
 *         INT8U DCP 采集均衡允许，DCP_ENABLED使能 DCP_DISABLED禁止
 *         INT8U CH 通道  CELL_CH_ALL所有
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void start_ltc6804_volt_cable_open_adc(INT8U MD, INT8U DCP, INT8U PUP, INT8U CH)
{
	INT16U tmp_16u = 0;
	INT16U cmd = 0;
	MD = MD & 0x03;
	DCP = DCP & 0x01;
	PUP = PUP & 0x01;
	CH = CH & 0x07;

	tmp_16u = ((INT16U)MD<<7);
	cmd = tmp_16u + (PUP << 6) + (DCP << 4) + CH + 0x228;

	broad_cast_ltc6804_cmd(cmd);
}
/*****************************************************************************
 *函数名称:start_ltc6804_aux_adc
 *函数功能:启动GPIO、基准辅助AD采集
 *参    数:INT8U MD 模式 三种模式MD_FAST快速、MD_NORMAL正常、MD_FILTERED滤波
 *         INT8U AUX_CH 通道 
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void start_ltc6804_aux_adc(INT8U MD, INT8U AUX_CH)
{
	INT16U tmp_16u = 0;
	INT16U cmd = 0;
	MD = MD & 0x03;
	AUX_CH = AUX_CH & 0x07;

	tmp_16u =  ((INT16U)MD<<7);
	cmd = tmp_16u + 0x460 + AUX_CH;

	broad_cast_ltc6804_cmd(cmd);
}
/*****************************************************************************
 *函数名称:start_ltc6804_stat_adc
 *函数功能:启动状态组采集
 *参    数:INT8U MD 模式 三种模式MD_FAST快速、MD_NORMAL正常、MD_FILTERED滤波
 *         INT8U CHST 通道 
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void start_ltc6804_stat_adc(INT8U MD, INT8U CHST)
{
	INT16U tmp_16u = 0;
	INT16U cmd = 0;
	MD = MD & 0x03;
	CHST = CHST & 0x07;

	tmp_16u =  ((INT16U)MD<<7);
	cmd = tmp_16u + 0x468 + CHST;

	broad_cast_ltc6804_cmd(cmd);
}
/*****************************************************************************
 *函数名称:send_ltc6804_wrcomm_cmd
 *函数功能:写COMM寄存器命令(广播方式)。注不含CS电平操作
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void send_ltc6804_wrcomm_cmd(void)
{
	INT8U tx_data[4] =	{ 0 };
	INT16U tmp_pec = 0;
	INT16U cmd = WRCOMM;
	tx_data[0] = (INT8U) (cmd >> 8);
	tx_data[1] = (INT8U) (cmd);
	//计算PEC
	tmp_pec = pec15_calc(2, tx_data);
	tx_data[2] = (INT8U) (tmp_pec >> 8);
	tx_data[3] = (INT8U) tmp_pec;
	//SPI发送
	(void)spi_write_array(&ltc6804_high_speed_spi_bus,4,tx_data);

}

/*****************************************************************************
 *函数名称:read_ltc6804_register
 *函数功能:读LTC6804寄存器
 *参    数:ltc6804_num 读取LTC6804芯片数目
 *         cmd 命令
 *         read_buffer 存储数据数组
 *返 回 值:LTC_OK,操作成功  其他失败
 *修订信息:
 ******************************************************************************/
static LtcResult read_ltc6804_register(INT8U ltc6804_num, INT16U cmd, INT8U*far read_buffer)
{
	INT8U tx_data[4] = { 0 };//发送数据数组
	INT16U tmp_pec = 0;//PEC校验
	INT16U read_len = 0;

	//参数检查
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
		ltc6804_num = MAX_LTC6804_IC_NUM;
	}

	if ((cmd != RDCFG) && //读配置寄存器
		(cmd != RDCVA) && //读电池电压寄存器组A
		(cmd != RDCVB) && //读电池电压寄存器组B
		(cmd != RDCVC) && //读电池电压寄存器组C
		(cmd != RDCVD) && //读电池电压寄存器组D
		(cmd != RDAUXA) && //读辅助寄存器组A
		(cmd != RDAUXB) && //读辅助寄存器组B
		(cmd != RDSTATA) && //读状态寄存器组A
		(cmd != RDSTATB) && //读状态寄存器组B
		(cmd != RDCOMM)) //读COMM寄存器组
	{
		return LTC_CMD_ERROR;
	}
	if (read_buffer == NULL)
	{
		return LTC_PTR_NULL;
	}

	tx_data[0] = (INT8U) (cmd >> 8);
	tx_data[1] = (INT8U) cmd;
	tmp_pec = pec15_calc(2, tx_data);
	tx_data[2] = (INT8U) (tmp_pec >> 8);
	tx_data[3] = (INT8U) tmp_pec;
        read_len = ((INT16U) ltc6804_num * (REG_WITH_PEC_LEN) * sizeof(INT8U));
	OSSchedLock();
	wakeup_iso_spi_idle(); //唤醒ISOspi总线
	ltc6804_cs_pin_low(); //片选
	(void)spi_write_read_array(&ltc6804_high_speed_spi_bus,tx_data, 4, read_buffer, read_len);
	ltc6804_cs_pin_high(); //取消片选
    OSSchedUnlock();
	return LTC_OK;
}
/*****************************************************************************
 *函数名称:read_ltc6804_cells_volt
 *函数功能:读取电池电压
 *参    数:INT8U ltc6804_num 芯片数目
 *         INT16U  (*_LTC_PAGED cell_volt)[LTC6804_MAX_CELL_NUM] 存储电池电压数组  单位mv
 *         INT8U * _LTC_PAGED comm_err_flag 通信失败标志
 *返 回 值:LTC_OK 操作成功,其他 失败
 *修订信息:                                                    
 ******************************************************************************/
LtcResult read_ltc6804_cells_volt(INT8U ltc6804_num, INT16U  (*far cell_volt)[LTC6804_MAX_CELL_NUM], Ltc6804CommErrorByte * far comm_err,Ltc6804RegReadType type)
{
	INT16U tmp_16u = 0;
    INT8U current_ic = 0;
    INT16U tmp_pec = 0;
    INT16U cmd = 0;
    INT8U pos = 0;
    INT8U i = 0;
    INT16U  bits = 0;
     //参数检测
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
		ltc6804_num = MAX_LTC6804_IC_NUM;
	}

	if (cell_volt == NULL)
	{
		return LTC_PTR_NULL;
	}
	if (comm_err == NULL)
	{
		return LTC_PTR_NULL;
	}
	
	for(i=0;i<4;i++)
	{
        if(i==0) 
        {
            cmd =RDCVA; 
        }
        else if(i==1)
        {
            cmd =RDCVB;  
        }
        else if(i==2)
        {
            cmd =RDCVC; 
        }
        else
        {
            cmd =RDCVD; 
        }
	  pos = i*3;
      ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
      (void)read_ltc6804_register(ltc6804_num, cmd, &ltc6804_reg_buffer[0][0]);
  	    
  	    
  	    for(current_ic = 0; current_ic < ltc6804_num; current_ic++) 
  	    {
            tmp_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic][0]);
            tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][6] << 8) + ltc6804_reg_buffer[current_ic][7];
            if(tmp_pec == tmp_16u) 
            {
                tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][1] << 8) + ltc6804_reg_buffer[current_ic][0]; 
                cell_volt[current_ic][pos+0] = (tmp_16u+5) / 10;
                tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][3] << 8) + ltc6804_reg_buffer[current_ic][2]; 
                cell_volt[current_ic][pos+1] = (tmp_16u+5) / 10;
                tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][5] << 8) + ltc6804_reg_buffer[current_ic][4]; 
                cell_volt[current_ic][pos+2] = (tmp_16u+5) / 10;
            } 
            else 
            {            
                switch(type)
                {
                    case kReadCfg:
                         comm_err[current_ic].Bits.read_cfg_reg = 1;
                    break;
                    case kReadCV:
                         comm_err[current_ic].Bits.read_cv = 1;
                    break;
                    case kReadGpio:
                         comm_err[current_ic].Bits.read_gpio1 = 1;
                    break;
                    case kReadPUCV:
                         comm_err[current_ic].Bits.read_pucv = 1;
                    break;
                    case kReadPUDV:
                         comm_err[current_ic].Bits.read_pudv = 1;
                    break;
                    default:
                    break;
                }
            }
  	    }
  	    //OSTimeDly(2); //从机太多，占用时间较长，稍作休眠
	}
	//跨箱处理
	for(current_ic = 0; current_ic < ltc6804_num; current_ic++) 
	{
	     INT8U index = 0;
	     bits = config_get(kBSU1VoltSampleNoUseBitIndex+current_ic);
	     if(bits!=0) 
	     {
	          for(i=0;i<LTC6804_MAX_CELL_NUM;i++)
	          {
	              if(!GET(bits,i)) 
	              {
	                cell_volt[current_ic][index++] =cell_volt[current_ic][i];      
	              }
	          
	          }
	          for(;index < LTC6804_MAX_CELL_NUM;index++)
	          {
	             cell_volt[current_ic][index] = 0;      
	          }
	     }
	}
    return LTC_OK;
 }

/*****************************************************************************
 *函数名称:read_ltc6804_aux_a_volt
 *函数功能:读取辅助A测量电压(gpio,基准测量电压值)
 *参    数:INT8U ltc6804_num 芯片数目
 *         INT8U AUX_CH 通道
 *         INT16U *volt 电压数组
 *         INT8U *comm_err_flag 通信失败标志
 *返 回 值:LTC_OK 操作成功,其他 失败
 *修订信息:
 ******************************************************************************/
LtcResult read_ltc6804_aux_a_volt(INT8U ltc6804_num, INT16U  (*far volt)[PARAM_BSU_NUM_MAX], Ltc6804CommErrorByte * far comm_err)
{
	INT8U current_ic = 0;//ic编号
	INT16U tmp_16u = 0; //临时16位变量
	INT16U tmp_pec = 0; //PEC校验
	LtcResult result;
	//参数检测
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
	    ltc6804_num = MAX_LTC6804_IC_NUM;
	}

	if (volt == NULL)
	{
		return LTC_PTR_NULL;
	}
	if (comm_err == NULL)
	{
		return LTC_PTR_NULL;
	}
	//清除数组
	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	//读取数据
	result = read_ltc6804_register(ltc6804_num, RDAUXA, &ltc6804_reg_buffer[0][0]);
	if (result != LTC_OK)
	{
		return result;
	}

	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		tmp_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic][0]);
		tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][6] << 8) + ltc6804_reg_buffer[current_ic][7];

		if (tmp_pec == tmp_16u)//校验通过
		{
			comm_err[current_ic].Bits.read_gpio1 = 0;
			tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][1] << 8) + ltc6804_reg_buffer[current_ic][0];
			volt[0][current_ic] = (tmp_16u+5) / 10;
	
			comm_err[current_ic].Bits.read_gpio2 = 0;
			tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][3] << 8) + ltc6804_reg_buffer[current_ic][2];
			volt[1][current_ic] = (tmp_16u+5) / 10;
	
			comm_err[current_ic].Bits.read_gpio3 = 0;
			tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][5] << 8) + ltc6804_reg_buffer[current_ic][4];
			volt[2][current_ic] = (tmp_16u+5) / 10;
		
		}
		else
		{ 
			comm_err[current_ic].Bits.read_gpio1 = 1;
			comm_err[current_ic].Bits.read_gpio2 = 1;
			comm_err[current_ic].Bits.read_gpio3 = 1;
		}
	}
	return LTC_OK;
}
/*****************************************************************************
 *函数名称:read_ltc6804_aux_b_volt
 *函数功能:读取辅助B测量电压(gpio,基准测量电压值)
 *参    数:INT8U ltc6804_num 芯片数目
 *         INT8U AUX_CH 通道
 *         INT16U *volt 电压数组
 *         INT8U *comm_err_flag 通信失败标志
 *返 回 值:LTC_OK 操作成功,其他 失败
 *修订信息:
 ******************************************************************************/
LtcResult read_ltc6804_aux_b_volt(INT8U ltc6804_num, INT16U  (*far volt)[PARAM_BSU_NUM_MAX], Ltc6804CommErrorByte * far comm_err)
{
	INT8U current_ic = 0;//ic编号
	INT16U tmp_16u = 0; //临时16位变量
	INT16U tmp_pec = 0; //PEC校验
	LtcResult result;
	//参数检测
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
	    ltc6804_num = MAX_LTC6804_IC_NUM;
	}

	if (volt == NULL)
	{
		return LTC_PTR_NULL;
	}
	if (comm_err == NULL)
	{
		return LTC_PTR_NULL;
	}
	//清除数组
	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	//读取数据
	result = read_ltc6804_register(ltc6804_num, RDAUXB, &ltc6804_reg_buffer[0][0]);
	if (result != LTC_OK)
	{
		return result;
	}

	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		tmp_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic][0]);
		tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][6] << 8) + ltc6804_reg_buffer[current_ic][7];

		if (tmp_pec == tmp_16u)//校验通过
		{
			comm_err[current_ic].Bits.read_gpio4 = 0;
			tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][1] << 8) + ltc6804_reg_buffer[current_ic][0];
			volt[0][current_ic] = (tmp_16u+5) / 10;
	
			comm_err[current_ic].Bits.read_gpio5 = 0;
			tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][3] << 8) + ltc6804_reg_buffer[current_ic][2];
			volt[1][current_ic] = (tmp_16u+5) / 10;
		}
		else
		{ 
			comm_err[current_ic].Bits.read_gpio4 = 1;
			comm_err[current_ic].Bits.read_gpio5 = 1;
		}
	}
	return LTC_OK;
}

/*****************************************************************************
 *函数名称:write_cfg_reg_to_ltc6804
 *函数功能:写配置寄存器
 *参    数:INT8U ltc6804_num芯片数
 *返 回 值:LTC_OK 操作成功,其他 失败
 *修订信息:
 ******************************************************************************/
LtcResult write_cfg_reg_to_ltc6804(INT8U ltc6804_num,BOOLEAN balance_en)
{
	INT8U current_ic = 0;
	INT8U i = 0;
	INT8U index = 0;
	INT16U cfg_pec = 0;
	INT8U tx_data[4] = { 0 };//发送数据数组
	INT16U tmp_pec = 0;//PEC校验
	INT8U tmp_8u = 0;
	
	//参数检查
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
	    ltc6804_num = MAX_LTC6804_IC_NUM;
	}
	tx_data[0] = (INT8U) (WRCFG >> 8);
	tx_data[1] = (INT8U) WRCFG;
	tmp_pec = pec15_calc(2, tx_data);
	tx_data[2] = (INT8U) (tmp_pec >> 8);
	tx_data[3] = (INT8U) tmp_pec;	
	OSSchedLock();
	wakeup_iso_spi_idle();
	ltc6804_cs_pin_low(); //片选
	(void)spi_write_array(&ltc6804_high_speed_spi_bus,4,tx_data);
	
	//从最后一个开始写
	for (current_ic = ltc6804_num; current_ic > 0; current_ic--)
	{
		index = 0;
		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = (ltc6804_gpio[current_ic-1].value << 3) + (REFON << 2) + ADCOPT;
		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = 0x00;
		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = 0x00;
		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = 0x00;
		if (index < REG_WITH_PEC_LEN)
		{
		    if(balance_en)
		        tmp_8u = (INT8U)get_ltc6804_balance_ctrl_word(current_ic - 1);
		    else
		        tmp_8u = 0;
		    ltc6804_reg_buffer[current_ic - 1][index++] = tmp_8u;
		}
		if (index < REG_WITH_PEC_LEN)
		{
		    if(balance_en)
		        tmp_8u = (INT8U)((get_ltc6804_balance_ctrl_word(current_ic - 1)>>8)&0x0F);
		    else
		        tmp_8u = 0;
		    #if DCTO != 0
		    tmp_8u += (DCTO << 4);
		    #endif
		    ltc6804_reg_buffer[current_ic - 1][index++] = tmp_8u ;
		}
		cfg_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic - 1][0]);

		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = (INT8U) (cfg_pec >> 8);
		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = (INT8U) cfg_pec;
		(void)spi_write_array(&ltc6804_high_speed_spi_bus,REG_WITH_PEC_LEN, &ltc6804_reg_buffer[current_ic - 1][0]);
	}
	ltc6804_cs_pin_high(); //取消片选
        OSSchedUnlock();
	return LTC_OK;
}

/*****************************************************************************
 *函数名称:send_balance_cmd_to_ltc6804
 *函数功能:被动均衡命令
 *参    数:ltc6804_num芯片数
 *返 回 值:LTC_OK 操作成功,其他 失败
 *修订信息:
 ******************************************************************************/
LtcResult send_balance_cmd_to_ltc6804(INT8U ltc6804_num) 
{
    LtcResult result = LTC_OK;
	//参数检查
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
		ltc6804_num = MAX_LTC6804_IC_NUM;
	}
	result = write_cfg_reg_to_ltc6804(ltc6804_num,TRUE);//配置所有芯片
    return result;
}
/*****************************************************************************
 *函数名称:send_ltc6804_stcomm_cmd
 *函数功能:启动I2C/SPI通信命令(广播方式)。注不含CS电平操作
 *参    数:INT8U byte_cnt  发送字节数
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void send_ltc6804_stcomm_cmd(INT8U byte_cnt)
{
	INT8U tx_data[4] = { 0 };
	INT16U tmp_pec = 0;
	INT16U cmd = STCOMM;
	INT8U i = 0;
	INT8U data = 0;

    if(byte_cnt > 3)
       byte_cnt = 3;
    
	tx_data[0] = (INT8U) (cmd >> 8);
	tx_data[1] = (INT8U) (cmd);
	//计算PEC
	tmp_pec = pec15_calc(2, tx_data);
	tx_data[2] = (INT8U) (tmp_pec >> 8);
	tx_data[3] = (INT8U) tmp_pec;
	//要考虑降频处理,PCF8574仅支持100K,那么6804最大通信频率不超过200K
	(void)spi_reinit(&ltc6804_low_speed_spi_bus, 1);
	//SPI发送
	 (void)spi_write_array(&ltc6804_low_speed_spi_bus,4,tx_data);;
	//在SCK上发送N个时钟
	for (i = 0; i < (byte_cnt*3); i++)
	{
		data = 0xFF;
        (void)spi_transmit_byte(&ltc6804_low_speed_spi_bus,&data);
	}
    (void)spi_reinit(&ltc6804_high_speed_spi_bus, 1);;
	//恢复频率
}
/*****************************************************************************
 *函数名称:ltc68041_iic_spi_read_write
 *函数功能:6804IIC和SPI读写
 *参    数:ltc6804_num IC个数 
 *        buffer写数组
 *        byte_cnt 传输字节数最大3个
 *返 回 值:LTC_OK 操作成功,其他 失败
 *修订信息:
 ******************************************************************************/
LtcResult ltc68041_iic_spi_read_write(INT8U ltc6804_num, INT8U (*far buffer)[REG_WITH_PEC_LEN], Ltc6804IICSPIWRFlag w_r_flag,INT8U byte_cnt)
{
	LtcResult result;
	INT8U current_ic = 0;
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
    	ltc6804_num = MAX_LTC6804_IC_NUM;
	}
	if (buffer == NULL)
	{
		return LTC_PTR_NULL;
	}
    if(byte_cnt > 3)
    {
       return LTC_ERROR;
    }
    OSSchedLock();
	//唤醒总线
	wakeup_iso_spi_idle();

	ltc6804_cs_pin_low(); //片选
	//发送WRCOMM命令
	send_ltc6804_wrcomm_cmd();
	//发送COMM寄存器数据,从顶端--->到底端
	for (current_ic = ltc6804_num; current_ic > 0; current_ic--)
	{
		(void)spi_write_array(&ltc6804_high_speed_spi_bus,REG_WITH_PEC_LEN, &buffer[current_ic - 1][0]);
	}
	ltc6804_cs_pin_high(); //片选

	//发送STCOMM命令
	ltc6804_cs_pin_low(); //片选
	send_ltc6804_stcomm_cmd(byte_cnt);
	ltc6804_cs_pin_high(); //取消片选
    OSSchedUnlock();
	if (w_r_flag == kIICSPIWrite)
	{
		return LTC_OK;
	}
	ltc6804_buffer_set(&buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	result = read_ltc6804_register(ltc6804_num, RDCOMM, &buffer[0][0]);
	if (result != LTC_OK)
	{
		return result;
	}
	return LTC_OK;
}
/*****************************************************************************
 *函数名称:ltc6804_init
 *函数功能:LTC初始化,主要是为了,确定采样频率类别,通信引脚置高电平.
 *参    数:ltc6804_num 6804个数
 *返 回 值:LTC_OK 操作成功,其他 失败
 *修订信息:
 ******************************************************************************/
LtcResult ltc6804_init(INT8U ltc6804_num)
{
	INT8U current_ic = 0;
	INT8U refon = 0;
	INT8U adcopt = 0;
	INT8U dcto = 0;
	INT16U tmp_pec = 0, tmp_16u = 0;
	volatile LtcResult result;

	//参数检查
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
		ltc6804_num = MAX_LTC6804_IC_NUM;
	}
	clr_all_ltc6804_balance_ctrl_word();
	//GPIO初始化
	for (current_ic = 0; current_ic < MAX_LTC6804_IC_NUM; current_ic++)
	{   //第一次IO口全部初始化
	    bsu_board_gpio_init(current_ic);
	}
    wakeup_ltc6804_sleep(); //操作前需要对6804进行唤醒
	result = write_cfg_reg_to_ltc6804(ltc6804_num,FALSE);//配置所有芯片
	if (result != LTC_OK)
	{
		return result;
	}
	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));

	result = read_ltc6804_register(ltc6804_num, RDCFG, &ltc6804_reg_buffer[0][0]);//回读取配置寄存器
	if (result != LTC_OK)
	{
		return result;
	}

	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		tmp_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic][0]);
		tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][6] << 8) + ltc6804_reg_buffer[current_ic][7];

		if (tmp_pec == tmp_16u)//PEC校验
		{ //对几个关键参数进行确认
			refon = (ltc6804_reg_buffer[current_ic][0] >> 2) & 0x01;
			adcopt = ltc6804_reg_buffer[current_ic][0] & 0x01;

			if ((refon != CFGR_REFON) || (adcopt != CFGR_ADCOPT))
			{
				return LTC_INIT_ERROR;
			}
		}
		else
		{ //校验错误或通信故障
			return LTC_INIT_ERROR;
		}
	}
	return LTC_OK;
}
/*****************************************************************************
 *函数名称:wakeup_ltc6804_with_write_cfg_reg
 *函数功能:通信中断后重新唤醒6804
 *参    数:ltc6804_num 6804个数     27ms +0.32*n
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void wakeup_ltc6804_with_write_cfg_reg(INT8U ltc6804_num,BOOLEAN balance_en)
{
	//参数检查
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
		ltc6804_num = MAX_LTC6804_IC_NUM;
	}
	wakeup_ltc6804_sleep();
    (void)write_cfg_reg_to_ltc6804(ltc6804_num,balance_en);//配置所有芯片
    OSTimeDly(2); //6804基准上电需要时间
}
/*****************************************************************************
 *函数名称:read_balance_current_from_dsp
 *函数功能:通过LTC6804的IIC来读电流
 *参    数:ltc6804_num 电池数, addr DSP地址
 *返 回 值:LTC_OK成功 ,其他失败。
 *修订信息:
 ******************************************************************************/
LtcResult read_balance_current_from_dsp(INT8U ltc6804_num,INT8U addr,INT8U *FAR balance_current)
{
	LtcResult result;
	INT8U current_ic = 0,cur = 0; //当前IC编号
	INT16U tmp_pec = 0,tmp_16u = 0; //PEC校验

	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
		return LTC6804_NUM_OUT_OF_RANGE;
        addr += IIC_ADDR_READ_BIT;
	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		ltc6804_reg_buffer[current_ic][0] = ((IIC_ICOM_READ_START << 4) & 0xF0) + ((addr >> 4) & 0x0F);
		ltc6804_reg_buffer[current_ic][1] = ((addr << 4) & 0xF0) + IIC_FCOM_READ_ACK_FROM_MIAN;
		ltc6804_reg_buffer[current_ic][2] = ((IIC_ICOM_READ_SDA_LOW << 4) & 0xF0) + 0x0F;
		ltc6804_reg_buffer[current_ic][3] = 0xF0+IIC_FCOM_WRITE_NACK_STOP;
		ltc6804_reg_buffer[current_ic][4] = ((IIC_ICOM_WRITE_NO_TRANSMIT << 4) & 0xF0);
		ltc6804_reg_buffer[current_ic][5] = IIC_FCOM_WRITE_NACK_STOP;
		tmp_pec = pec15_calc(6, &ltc6804_reg_buffer[current_ic][0]);
		ltc6804_reg_buffer[current_ic][6] = (INT8U) (tmp_pec >> 8);
		ltc6804_reg_buffer[current_ic][7] = (INT8U) (tmp_pec);
	}
	result = ltc68041_iic_spi_read_write(ltc6804_num, ltc6804_reg_buffer, kIICSPIRead, 2);
	if(result != LTC_OK) 
	{
	    return result;  
	}
	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		tmp_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic][0]);
		tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][6] << 8) + ltc6804_reg_buffer[current_ic][7];
		if (tmp_pec == tmp_16u)//PEC校验
		{
			cur = ((ltc6804_reg_buffer[current_ic][2] << 4) & 0xF0) + ((ltc6804_reg_buffer[current_ic][3] >> 4) & 0x0F);
			if(cur == 0xFF)
			   cur = 0;
			balance_current[current_ic] = cur;
		}
	}
	return LTC_OK;
}
/*****************************************************************************
 *函数名称:write_balance_info_to_dsp
 *函数功能:通过LTC6804的IIC来写DSP
 *参    数:ltc6804_num 电池数, addr DSP地址,active_balance_word均衡控制字 current均衡电流
 *返 回 值:LTC_OK成功 ,其他失败。
 *修订信息:
 ******************************************************************************/
LtcResult write_balance_info_to_dsp(INT8U ltc6804_num,INT8U addr,INT16U*far active_balance_word,INT8U current)
{
	LtcResult result;
	INT8U current_ic = 0; //当前IC编号
	INT16U tmp_pec = 0; //PEC校验
	INT8U data = 0;
	INT8U i = 0;
	INT16U balance_word = 0;

	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
		return LTC6804_NUM_OUT_OF_RANGE;

	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		data = 0;
		balance_word = balance_ctrl_word_convert(current_ic,active_balance_word[current_ic]);
		for(i = 0;i < LTC6804_MAX_CELL_NUM;i++)
		{
		    if(GET(balance_word,i))
		    {
		        data = i+1;
		        break;
		    }
		}
		if(GET(active_balance_word[current_ic],15))
		{
		    SET(data,4);
		}
		
		data |= ((current &0x07)<<5);
		ltc6804_reg_buffer[current_ic][0] = ((IIC_ICOM_WRITE_START << 4) & 0xF0) + ((addr >> 4) & 0x0F);
		ltc6804_reg_buffer[current_ic][1] = ((addr << 4) & 0xF0) + IIC_FCOM_WRITE_NACK;
		ltc6804_reg_buffer[current_ic][2] = ((IIC_ICOM_WRITE_BLANK << 4) & 0xF0) + ((data >> 4) & 0x0F);
		ltc6804_reg_buffer[current_ic][3] = ((data << 4) & 0xF0) + IIC_FCOM_WRITE_NACK_STOP;
		ltc6804_reg_buffer[current_ic][4] = ((IIC_ICOM_WRITE_NO_TRANSMIT << 4) & 0xF0);
		ltc6804_reg_buffer[current_ic][5] = IIC_FCOM_WRITE_NACK_STOP;
		tmp_pec = pec15_calc(6, &ltc6804_reg_buffer[current_ic][0]);
		ltc6804_reg_buffer[current_ic][6] = (INT8U) (tmp_pec >> 8);
		ltc6804_reg_buffer[current_ic][7] = (INT8U) (tmp_pec);
	}

	result = ltc68041_iic_spi_read_write(ltc6804_num, ltc6804_reg_buffer, kIICSPIWrite, 2);
	return result;

}
/*****************************************************************************
 *函数名称:pcf8574_write_channel
 *函数功能:通过LTC6804的IIC来写PCF8574
 *参    数:ltc6804_num 6804数目, channel通道
 *返 回 值:LTC_OK成功 ,其他失败。
 *修订信息:
 ******************************************************************************/
LtcResult pcf8574_write_channel(INT8U ltc6804_num, INT8U channel)
{
	LtcResult result;
	INT8U current_ic = 0; //当前IC编号
	INT16U tmp_pec = 0; //PEC校验
	INT8U addr_byte = 0; //地址
	INT8U cmd_byte = 0; //命令字
	INT8U pcf8574_addr = 0; //A0A1A2片选
	
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
		return LTC6804_NUM_OUT_OF_RANGE;
	if (channel > PCF8574_CHANNEL_MAX_NUM)
		return PCF8574_CHANNEL_ERROR;
	
	addr_byte = PCF8574_WRITE_ADDR | ((pcf8574_addr << 1) & 0x0E); //写地址
	cmd_byte = channel&0x07; //通道使能

	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		ltc6804_reg_buffer[current_ic][0] = ((IIC_ICOM_WRITE_START << 4) & 0xF0) + ((addr_byte >> 4) & 0x0F);
		ltc6804_reg_buffer[current_ic][1] = ((addr_byte << 4) & 0xF0) + IIC_FCOM_WRITE_NACK;
		ltc6804_reg_buffer[current_ic][2] = ((IIC_ICOM_WRITE_BLANK << 4) & 0xF0) + ((cmd_byte >> 4) & 0x0F);
		ltc6804_reg_buffer[current_ic][3] = ((cmd_byte << 4) & 0xF0) + IIC_FCOM_WRITE_NACK_STOP;
		ltc6804_reg_buffer[current_ic][4] = ((IIC_ICOM_WRITE_NO_TRANSMIT << 4) & 0xF0);
		ltc6804_reg_buffer[current_ic][5] = IIC_FCOM_WRITE_NACK_STOP;
		tmp_pec = pec15_calc(6, &ltc6804_reg_buffer[current_ic][0]);
		ltc6804_reg_buffer[current_ic][6] = (INT8U) (tmp_pec >> 8);
		ltc6804_reg_buffer[current_ic][7] = (INT8U) (tmp_pec);
	}

	result = ltc68041_iic_spi_read_write(ltc6804_num, ltc6804_reg_buffer, kIICSPIWrite, 2);
	return result;

}

#endif