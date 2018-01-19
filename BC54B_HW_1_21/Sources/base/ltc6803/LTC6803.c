/**
*
* Copyright (c) 2012 Ligoo Inc. 
* 
* @file  LTC6803.c
* @brief LTC6803-4底层驱动  
* @note 
* @version 1.00
* @author 
* @date 2012/10/4  
*
*/ 
#ifndef LTC6803_SOURCE
#define LTC6803_SOURCE

#include "LTC6803.h"

/*
* @brief        LTC6803-4CRC校验
* @note         无
* @param[in]    buf：校验数据首地址len：数据长度
* @param[out]   无
* @return       校验结果
*
*/
INT8U   Calcrc8(INT8U* _PAGED buf, INT8U len)   
{   
    INT8U   i,j=0;   
    INT8U   crc   =   0x41;   
  
    while(len--!=0)   
    {   
        for   (i= 0x80;i!=0;i/=2)   
        {   
            if ((crc & 0x80)!=0)   
            {   
                crc *= 2;   
                crc ^= 0x07;   
             }   
             else 
             {  
                crc *= 2;  
             }
        
             if ((*(buf+j) & i)!=0)
             {
                crc^=0x07;  
             }
          }   
          j++;
    }   
    return   crc;  
} 
/*
* @brief        片选
* @note         无
* @param[in]    无
* @param[out]   无
* @return       无
*
*/
void   LTC_CS_ON()   
{   
    DN_LTC_CS;
}
/*
* @brief        片选
* @note         无
* @param[in]    无
* @param[out]   无
* @return       无
*
*/
void   LTC_CS_OFF()   
{   
    UP_LTC_CS;
} 
 
/*
* @brief        LTC6803-4读取命令
* @note         无
* @param[in]    command：命令类型ltc_addr：LTC6803地址，ltcRegGrop，
* @param[out]   无
* @return       无
*
*/
INT8U TLCReceiveCommand(INT8U ltc_addr,INT8U command, LTC_REG_GROP * _PAGED ltcRegGrop) 
{
    INT8U i=0;
    INT8U pec=0;
    INT8U address=0;
    INT8U recdata_pec=0;
    
    if(!ltcRegGrop)                       //参数检测
        return FALSE;
    
    if((command < RDCFG_REG) || (command > RDDGN_REG))
        return FALSE;
    
    if(ltc_addr >= MAX_LTC6803_NUM)
        return FALSE;
    
    address = ltc_addr|0x80;
    
    LTC_CS_ON();                           //片选
    
	  pec = Calcrc8(&address, 1); //发送地址及PEC校验
    (void)SendReadSPI(address);    
    (void)SendReadSPI(pec);      
	  
	  pec = Calcrc8(&command, 1);//发送命令及命令PEC校验
    (void)SendReadSPI(command);    
    (void)SendReadSPI(pec);      
    
    switch(command)
    {
        case  RDCFG_REG:                 //读取配置寄存器数据
                for (i = 0; i < CFGREG_LEN; i++) 
                {
                    ltcRegGrop->RDCfgReg[i] = SendReadSPI(0xff);  //保存数据
                }
                recdata_pec = SendReadSPI(0xff);
                pec = Calcrc8(ltcRegGrop->RDCfgReg, CFGREG_LEN);
              break;
              
        case  RDCV_REG:                  //读取电压数据               
                for (i = 0; i < CVREG_LEN; i++) 
                {
                    ltcRegGrop->VReg[i] = SendReadSPI(0xff);  //保存数据
                }
                recdata_pec = SendReadSPI(0xff);
                pec = Calcrc8(ltcRegGrop->VReg, CVREG_LEN);
            break;
        case RDTMP_REG:                                        //读取温度数据 
                for (i = 0; i < TMPREG_LEN; i++) 
                {
                    ltcRegGrop->TReg[i] = SendReadSPI(0xff); //保存数据
                }
                recdata_pec = SendReadSPI(0xff);
                pec = Calcrc8(ltcRegGrop->TReg, TMPREG_LEN);
              break;
     #if LTC_TEST_SUPPORT
        case  RDFLG_REG:                                        //读取标志数据 
                for (i = 0; i < FLGREG_LEN; i++) 
                {
                    ltcRegGrop->FlagReg[i] = SendReadSPI(0xff); //保存数据
                }
                recdata_pec = SendReadSPI(0xff);
                pec = Calcrc8(ltcRegGrop->FlagReg, FLGREG_LEN);
             break;
        case RDDGN_REG:                                        //读取诊断数据 
                for (i = 0; i < DGNREG_LEN; i++) 
                {
                    ltcRegGrop->DgnReg[i] = SendReadSPI(0xff); //保存数据
                }
                recdata_pec = SendReadSPI(0xff);
                pec = Calcrc8(ltcRegGrop->DgnReg, DGNREG_LEN);
              break;
     #endif
        default:
            return FALSE;
              break;
    }
    LTC_CS_OFF() ;
    
    if (pec!=recdata_pec)                           //校验
    {
        return FALSE;    
    }
    
   return TRUE;           
} 
/*
* @brief        LTC6803-4发送命令
* @note         无
* @param[in]    command：命令类型
* @param[out]   无
* @return       无
*
*/
void LTCSendCommand (INT8U ltc_addr,INT8U command,  LTC_REG_GROP * _PAGED ltcRegGrop)
{
    INT8U i;
    INT8U command_pec;
    INT8U regdata_pec; 
    INT8U address;
    
     
    LTC_CS_ON();
    
    if(ltc_addr != LTC_BROADCAST_ADDR)
    {
        address = ltc_addr|0x80;
        command_pec = Calcrc8(&address, 1); //发送地址及PEC校验
        (void)SendReadSPI(address);    
        (void)SendReadSPI(command_pec);      
    }
    command_pec = Calcrc8(&command, 1);            //计算校验
    (void)SendReadSPI(command);                    //发命令
    (void)SendReadSPI(command_pec);                //发校验

    switch(command)
    {
        case WRCFG_REG:
             
                regdata_pec = Calcrc8(ltcRegGrop->CfgReg, CFGREG_LEN);
                                             
                for (i=0;i<CFGREG_LEN;i++)
                {
                    (void)SendReadSPI(ltcRegGrop->CfgReg[i]);          
                }
                (void)SendReadSPI(regdata_pec);            //发校验    
            
            break;
            
        default:
        
            break;
    }
    
    for (i=0;i<SPIDLY100US;i++);
    {
        
    }
    
    LTC_CS_OFF() ;  
    
}


/* @brief  广播命令*/
void BroadCastCommand(INT8U command)
{
    INT8U i;
    INT8U command_pec;
    
    LTC_CS_ON();
    
    command_pec = Calcrc8(&command, 1);            //计算校验
    (void)SendReadSPI(command);                    //发命令
    (void)SendReadSPI(command_pec);                //发校验
    
    for (i=0;i<SPIDLY100US;i++);
    {
        
    }
    LTC_CS_OFF() ;  
}


/*@brief 写配置寄存器*/
void WriteLtcCfgReg(INT8U ltc_addr,LTC_REG_GROP * _PAGED ltcRegGrop )
{
    //OS_CPU_SR cpu_sr = 0;
    if(!ltcRegGrop)
       return;
    //OS_ENTER_CRITICAL();
    LTCSendCommand (ltc_addr, WRCFG_REG, ltcRegGrop);
    //OS_EXIT_CRITICAL();
}

/*@brief 写所有LTC配置寄存器*/
void WriteAllLtcCfgReg()
{
    //OS_CPU_SR cpu_sr = 0;
    //OS_ENTER_CRITICAL();
    BroadCastCommand(WRCFG_REG);
    //OS_EXIT_CRITICAL();
}

/*@brief 启动全部电压采集*/
void StartLtcVAD(void)
{
    //OS_CPU_SR cpu_sr = 0;
    //OS_ENTER_CRITICAL();
    BroadCastCommand(STCVAD_REG);
    //OS_EXIT_CRITICAL();
}

/*@brief  启动全部温度采集*/
void StartLtcTAD(void)
{
    //OS_CPU_SR cpu_sr = 0;
    //OS_ENTER_CRITICAL();
    BroadCastCommand(STTMPAD_REG);
    //OS_EXIT_CRITICAL();
}


/*@brief LTC电压寄存器清零*/
void ClrLtcVReg(void)
{
    //OS_CPU_SR cpu_sr = 0;
    //OS_ENTER_CRITICAL();
    BroadCastCommand(CLRV_REG);
    //OS_EXIT_CRITICAL();
}

/*@brief 电压排线检测*/
void VWireOff(void)
{
    //OS_CPU_SR cpu_sr = 0;
    //OS_ENTER_CRITICAL();
    BroadCastCommand(STOWAD_REG);
    //OS_EXIT_CRITICAL();
}

/*@brief  读取电压*/
INT8U ReadLtcVoltage(INT8U ltc_addr,LTC_REG_GROP * _PAGED ltcRegGrop)
{
   return TLCReceiveCommand(ltc_addr,RDCV_REG,ltcRegGrop); 
}

/*@brief 读取温度*/
INT8U ReadLtcTemperature(INT8U ltc_addr,LTC_REG_GROP * _PAGED ltcRegGrop)
{
   return TLCReceiveCommand(ltc_addr,RDTMP_REG,ltcRegGrop); 
}

/*@brief  读取配置寄存器 */
INT8U ReadLtcCfgReg(INT8U ltc_addr,LTC_REG_GROP * _PAGED ltcRegGrop)
{
   return TLCReceiveCommand(ltc_addr,RDCFG_REG,ltcRegGrop); 
}

/*@brief  读取配置寄存器*/
INT8U ReadLtcDgnReg(INT8U ltc_addr,LTC_REG_GROP * _PAGED ltcRegGrop)
{
   return TLCReceiveCommand(ltc_addr,RDDGN_REG,ltcRegGrop); 
}

/*@brief  测试电压采集功能*/
void TestLtcVADCommand(void)
{
    BroadCastCommand(SELFVADTEST1_REG);
}

/*@brief 测试温度采集功能*/
void TestLtcTADCommand(void)
{
    BroadCastCommand(SELFTADTEST1_REG);
}

/*@brief 测试LTC电压基准*/
void TestLtcREFCommand(void)
{
    BroadCastCommand(STDGN_REG);
}

/*
* @brief        memset
* @note         buffer    value    size
* @param[in]    无
* @param[out]   无
* @return       无
*
*/
/*void safe_memset(void* _PAGED  buffer, INT8U value, INT16U size)
{
  INT8U * data = (INT8U * _PAGED )buffer;
  INT16U index;
  for (index = 0; index < size; ++index)
    data[index] = value;
}*/
/*
* @brief        校验写入配置寄存器的数据
* @note         
* @param[in]    ltccfgreg  
* @param[out]   无
* @return       FALSE，TRUE
*
*/
INT8U read_and_check_cfg_reg(LTC_REG_GROP * _PAGED ltccfgreg)
{
     INT8U j=0;
    for(j = 0; j < CFGREG_LEN; j++)
    {
        if(j == 0) 
        {
            if((ltccfgreg->CfgReg[0]&0x0f) != (ltccfgreg->RDCfgReg[0]&0x0f))
            {
                 return FALSE;   
            }
        }
        else
        {
            if(ltccfgreg->CfgReg[j] != ltccfgreg->RDCfgReg[j])
            {
                 return FALSE;   
            } 
        }
    }
    return TRUE; 
}

/*
* @brief        检测有效LTC数
* @note         无
* @param[in]    无
* @param[out]   无
* @return       有效LTC数
*
*/
INT8U valid_ltc_num(void)
{
    INT8U i;
    INT16U cnt;
    for(i=0; i<MAX_LTC6803_NUM; i++)
    {
        cnt = config_get(kBYUVoltTempCnt1Index + i);
        cnt = cnt&0xFF00;
         cnt = cnt>>8;
        if(cnt==0)
            return i;
    }
    return i;
}

/*
* @brief        LTC6803-4初始化
* @note         无
* @param[in]    无
* @param[out]   无
* @return       无
*
*/
INT8U Ltc6803Init(LTC_REG_GROP * _PAGED ltcRegGrop)
{
    //OS_CPU_SR cpu_sr = 0;
    INT8U i=0,j=0,ltcerr=0,ltc_num=0;
    
    if(!ltcRegGrop)                                 //参数检测
        return FALSE;
    
    safe_memset(ltcRegGrop, 0, sizeof(LTC_REG_GROP));
    
    ltcRegGrop->dcc =0x0000;
    ltcRegGrop->CfgReg[0] = (LTC6803_WDT<<7) + (LTC6803_GPIO2<<6) + (LTC6803_GPIO1<<5) + (LTC6803_LVLPL<<4) + (LTC6803_CELL10<<3) + (LTC6803_CDC);
    ltcRegGrop->CfgReg[1] = (INT8U)(ltcRegGrop->dcc &0x00ff); 
    ltcRegGrop->CfgReg[2] = (INT8U)((ltcRegGrop->dcc>>8)&0x000f) + (INT8U)((LTC6803_MCXI<<4) &0x00f0);
    ltcRegGrop->CfgReg[3] = (INT8U)((LTC6803_MCXI>>4) &0x00ff); 
    ltcRegGrop->CfgReg[4] = (INT8U)((LTC6803_UV>>4) &0x00ff); 
    ltcRegGrop->CfgReg[5] = (INT8U)((LTC6803_OV>>4) &0x00ff); 
    
    
    //OS_ENTER_CRITICAL();
    LTCSendCommand (LTC_BROADCAST_ADDR, WRCFG_REG, ltcRegGrop);
    ltc_num  = valid_ltc_num(); 
    //OS_EXIT_CRITICAL(); 
    
    if(ltc_num > MAX_LTC6803_NUM)
         return FALSE;  
    
    //OS_ENTER_CRITICAL(); 
    for(i = 0; i < ltc_num; i++)  
    {
        WriteLtcCfgReg(i, ltcRegGrop);
        ltcerr = ReadLtcCfgReg(i, ltcRegGrop);     // 读配置寄存器
        if(ltcerr == TRUE)
        {
            
            if( !(read_and_check_cfg_reg(ltcRegGrop)) )    //检测配置寄存器的值是否已经写入
            {
                //OS_EXIT_CRITICAL();
                return FALSE;
            }
        }
        else
        {
            //OS_EXIT_CRITICAL();
            return FALSE;   
        }
    }
    //OS_EXIT_CRITICAL();
    return TRUE;
}
#if LTC_TEST_SUPPORT
/*
* @brief        LTC电压采集功能诊断
* @note         此函数仅用来测试芯片好坏（测试情况下使用，其他情况不用）
* @param[in]    无
* @param[out]   无
* @return       无
*
*/
INT8U VADDiagnose(INT8U ltc_addr)
{
   INT8U  ltcerr,err;
   INT16U  tmp;
   OSSemPend(g_LTCSPISem, 0, &err);
   TestLtcVADCommand();
   OSTimeDly(20); 
   ltcerr = ReadLtcVoltage(ltc_addr, &g_ltcRegGrop);  //读电压寄存器
   (void)OSSemPost(g_LTCSPISem);
   if(ltcerr == TRUE)  //通讯成功
   {      		  
         tmp = ((INT16U)g_ltcRegGrop.VReg[1] << 8) & 0x0f00;
    	 tmp += g_ltcRegGrop.VReg[0];
         if ((tmp == 0x555)||(tmp == 0xAAA))
         {
             return TRUE;
         }
         else
         {
             return FALSE;
         }
   }
   else   //通讯失败
   {
      return FALSE;
   }
    
}
/*
* @brief        LTC温度采集功能诊断
* @note         此函数仅用来测试芯片好坏（测试情况下使用，其他情况不用）
* @param[in]    无
* @param[out]   无
* @return       无
*
*/
INT8U TADDiagnose(INT8U ltc_addr)
{
   INT8U  ltcerr,err;
   INT16U  tmp;
   OSSemPend(g_LTCSPISem, 0, &err);
   TestLtcTADCommand();
   OSTimeDly(20); 
   ltcerr = ReadLtcTemperature(ltc_addr, &g_ltcRegGrop);  //读电压寄存器
   (void)OSSemPost(g_LTCSPISem); 
   if(ltcerr == TRUE)  //通讯成功
   {      		  
         tmp = ((INT16U)g_ltcRegGrop.TReg[1] << 8) & 0x0f00;
		 tmp += g_ltcRegGrop.TReg[0];
         if ((tmp == 0x555)||(tmp == 0xAAA))
         {
             return TRUE;
         }
         else
         {
             return FALSE;
         }
   }
   else   //通讯失败
   {
      return FALSE;
   }
   
}
/*
* @brief        LTC基准诊断
* @note         此函数仅用来测试芯片好坏（测试情况下使用，其他情况不用）
* @param[in]    无
* @param[out]   无
* @return       无
*
*/
INT8U REFDiagnose(INT8U ltc_addr)
{
   INT8U  ltcerr,err;
   INT16U  tmp;
   OSSemPend(g_LTCSPISem, 0, &err);
   TestLtcREFCommand();
   OSTimeDly(20); 
   ltcerr = ReadLtcDgnReg(ltc_addr, &g_ltcRegGrop);  //读电压寄存器
   (void)OSSemPost(g_LTCSPISem);
   if(ltcerr == TRUE)  //通讯成功
   {      		  
         tmp = ((INT16U)g_ltcRegGrop.DgnReg[1] << 8) & 0x0f00;
		 tmp += g_ltcRegGrop.DgnReg[0];
		 tmp = tmp-512;
		 tmp = tmp + (tmp >> 1);
         if((tmp >= 2100)&&(tmp <= 2900)&&((g_ltcRegGrop.DgnReg[1]&0x10) == 0x00))
         {
             return TRUE;
         }
         else
         {
             return FALSE;
         }
   }
   else   //通讯失败
   {
      return FALSE;
   }
    
}
#endif
#endif
/***********END OF FILE***********/