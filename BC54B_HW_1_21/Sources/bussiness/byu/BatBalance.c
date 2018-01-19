/**
*
* Copyright (c) 2012 Ligoo Inc. 
* 
* @file  BatBalance.c
* @brief 电池均衡文件
* @note 
* @version 1.00
* @author 曹志勇
* @date 2012/10/6  
*
*/
#ifndef BAT_BALANCE_SOURCE
#define BAT_BALANCE_SOURCE

#include "includes.h"
#include "BatBalance.h"
#include "ADSample.h"
#if  BMS_SUPPORT_HARDWARE_LTC6803 == 1 
#pragma MESSAGE DISABLE C4003
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

OS_STK g_batBalanceTaskStack[BAT_BALANCE_TASK_STK_SIZE]={0};//电池均衡任务堆栈

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BYU_SAMPLE
BALANCE_VOL_INFO  g_balance_vol_info;
BalancePower blance_power;
#pragma DATA_SEG DEFAULT

void BatBalanceTask(void *pdata);
/*
*
* @brief        电池均衡任务初始化
* @note         创建电池均衡任务
* @param[in]    无  
* @param[out]   无
* @return       无
*
*/ 

void BatBalanceInit(void)
{
    (void)OSTaskCreate(BatBalanceTask,
                            (void *)0,
                            (OS_STK *)&g_batBalanceTaskStack[BAT_BALANCE_TASK_STK_SIZE - 1],
                            BAT_BALANCE_TASK_PRIO);   
}

/*
*
* @brief        更新均衡条件
* @note         
* @param[in]    无  
* @param[out]   无
* @return       无
*
*/

void balance_condition_update(void)
{
    INT8U max_addr;
    INT8U i, j;
    INT8U H_LVN[MAX_LTC6803_NUM]={0},H_HVN[MAX_LTC6803_NUM]={0};
    INT16U H_LV[MAX_LTC6803_NUM]={0},H_HV[MAX_LTC6803_NUM]={0};
    OS_CPU_SR cpu_sr = 0;

    max_addr = get_ltc_num();
    if(max_addr>MAX_LTC6803_NUM)
        max_addr=MAX_LTC6803_NUM;
    
    for(i=0;i<MAX_LTC6803_NUM;i++)
	{
	    H_LV[i] = BATTERY_VOLTAGE_MAX_VALUE;
	    H_HV[i] = 0;
	}
    for(i = 0; i < max_addr; i++)
    {
        for(j=0;j<LTC_HALF_VOL_NUM;j++)   //前六串最高最低电压
        {
            if((g_bmsInfo.volt[i][j] < H_LV[i])&&(g_bmsInfo.volt[i][j]!=0))
	      	{
	      		H_LV[i] = g_bmsInfo.volt[i][j];
	            H_LVN[i] = j;
	        }
	        
            if(g_bmsInfo.volt[i][j] > H_HV[i])
	      	{
	      		H_HV[i] = g_bmsInfo.volt[i][j];
	            H_HVN[i] = j;
	        }
        }
    }
    OS_ENTER_CRITICAL();
    for(i=0; i<max_addr; i++) //前6串后6串最低(高)电压极其编号
	{
	    if((H_LV[i]<=BATTERY_VOLTAGE_MIN_VALUE)||(H_LV[i]==BATTERY_VOLTAGE_MAX_VALUE)||(H_LVN[i]>=LTC_AFTER_HALF_VOL_INIT_NO))
	    {
	        g_balance_vol_info.before_six_low_volt[i]=0;
	        g_balance_vol_info.before_six_low_num[i]=0;
	    }
	    else
	    {
	        g_balance_vol_info.before_six_low_volt[i]=H_LV[i];
	        g_balance_vol_info.before_six_low_num[i]=H_LVN[i];
	    }
	    
	    if((H_HV[i]<=BATTERY_VOLTAGE_MIN_VALUE)||(H_HV[i]>=BATTERY_VOLTAGE_MAX_VALUE)||(H_HVN[i]>=LTC_AFTER_HALF_VOL_INIT_NO))
	    {
	        g_balance_vol_info.before_six_high_volt[i]=0;
	        g_balance_vol_info.before_six_high_num[i]=0;
	    }
	    else
	    {
	        g_balance_vol_info.before_six_high_volt[i]=H_HV[i];
	        g_balance_vol_info.before_six_high_num[i]=H_HVN[i]; 
	    }
	}
	OS_EXIT_CRITICAL();

    safe_memset(H_LVN, 0, MAX_LTC6803_NUM*sizeof(INT8U));   //前6串最低电压编号
	safe_memset(H_HVN, 0, MAX_LTC6803_NUM*sizeof(INT8U));   //前6串最高电压编号
	for(i=0; i<MAX_LTC6803_NUM; i++)
	{
	    H_LV[i] = BATTERY_VOLTAGE_MAX_VALUE;
	    H_HV[i] = 0;
	}
	for(i = 0; i < max_addr; i++)
    {
    	for(j=LTC_HALF_VOL_NUM;j<LTC_CELLV_NUM;j++) //后六串最高最低电压
        {
            if((g_bmsInfo.volt[i][j] < H_LV[i])&&(g_bmsInfo.volt[i][j]!=0))
          	{
          		H_LV[i] = g_bmsInfo.volt[i][j];
                H_LVN[i] = j;
            }
            
            if(g_bmsInfo.volt[i][j] > H_HV[i])
          	{
          		H_HV[i] = g_bmsInfo.volt[i][j];
                H_HVN[i] = j;
            }
        }
    }
    OS_ENTER_CRITICAL();
    for(i=0; i<max_addr; i++) //前6串后6串最低(高)电压极其编号
	{
	    if((H_LV[i]<=BATTERY_VOLTAGE_MIN_VALUE)||(H_LV[i]==BATTERY_VOLTAGE_MAX_VALUE)||(H_LVN[i]<LTC_AFTER_HALF_VOL_INIT_NO)||(H_LVN[i]>LTC_CELLV_NUM))
        {
            g_balance_vol_info.after_six_low_volt[i]=0;
            g_balance_vol_info.after_six_low_num[i]=LTC_AFTER_HALF_VOL_INIT_NO;
        }
        else
        {
            g_balance_vol_info.after_six_low_volt[i]=H_LV[i];
            g_balance_vol_info.after_six_low_num[i]=H_LVN[i];
        }
        if((H_HV[i]<=BATTERY_VOLTAGE_MIN_VALUE)||(H_HV[i]>=BATTERY_VOLTAGE_MAX_VALUE)||(H_HVN[i]<LTC_AFTER_HALF_VOL_INIT_NO)||(H_HVN[i]>LTC_CELLV_NUM))
        {
            g_balance_vol_info.after_six_high_volt[i]=0;
            g_balance_vol_info.after_six_high_num[i]=LTC_AFTER_HALF_VOL_INIT_NO; 
        }
        else
        {
            g_balance_vol_info.after_six_high_volt[i]=H_HV[i];
            g_balance_vol_info.after_six_high_num[i]=H_HVN[i]; 
        }
	}
    OS_EXIT_CRITICAL();    
}

/*
*
* @brief        均衡状态转换
* @note         
* @param[in]    无
* @param[out]   无
* @return       无
*
*/

void blance_state_transform(void)
{    
    INT8U i, j, max_addr, bat_num;
    INT8U bal_buff_index, value, offset;
    INT16U blance_state = 0;
    OS_CPU_SR cpu_sr = 0;
    
    max_addr = get_ltc_num();
    if(max_addr > MAX_LTC6803_NUM)
	    max_addr = MAX_LTC6803_NUM;
    
    bal_buff_index = 0;
    value = 0;
    offset = 0;
    //OS_ENTER_CRITICAL();
    for(i=0; i<max_addr; i++)
    {
        blance_state=g_bmsInfo.batteryBlanceState[i];
        bat_num = g_bmsInfo.batCnt[i];
        if(bat_num > LTC_CELLV_NUM) bat_num= LTC_CELLV_NUM;
        
        for(j=0; j<bat_num; j++)
        {
            if((blance_state >> j) & 0x01)
            {
                value |= (1 << offset);
            }
            else
            {
                value &= ~(1 << offset);
            }
            offset++;
            if(offset >= 8 || j >= bat_num-1)
            {
                bmu_set_balance_state(BYU_USE_SLAVE_INDEX, bal_buff_index, value);
                bal_buff_index++;
                offset = 0;
                value = 0;
            }
        }
    }
    //OS_EXIT_CRITICAL();
}
/*
*
* @brief        清除上位机显示的均衡状态
* @note         无
* @param[in]    无 
* @param[out]   无
* @return       无
*
*/ 
void clr_upper_balance_state(void)
{ 
    INT8U i;
    OS_CPU_SR cpu_sr = 0;
    
    OS_ENTER_CRITICAL();
    for(i=0; i<BMS_BMU_BALANCE_STATE_BYTE_NUM; i++)
        bmu_set_balance_state(BYU_USE_SLAVE_INDEX, i, 0x00);
    OS_EXIT_CRITICAL();
}
/*
*
* @brief        均衡开关
* @note         无
* @param[in]    无 
* @param[out]   无
* @return       无
*
*/ 
void ControlLtcIO(LTC_REG_GROP * _PAGED ltcRegGrop)
{
    INT8U i=0;
    for(i = 0; i < MAX_LTC6803_NUM; i++)
    {
        ltcRegGrop->dcc = g_bmsInfo.batteryBlanceState[i];
        ltcRegGrop->CfgReg[1] = (INT8U)(ltcRegGrop->dcc & 0xff); 
        ltcRegGrop->CfgReg[2] = ((ltcRegGrop->CfgReg[2] & 0xf0) | ((ltcRegGrop->dcc >> 8) & 0x000f));
        WriteLtcCfgReg(i,ltcRegGrop);
    }
}
/*
*
* @brief        清除bat均衡状态
* @note         无
* @param[in]    无 
* @param[out]   无
* @return       无
*
*/ 
void clr_bat_blance_state(void)
{
    INT8U i=0;
    for (i = 0; i < MAX_LTC6803_NUM; i++)                        
    {
        g_bmsInfo.batteryBlanceState[i] = 0x0000;
    }         
}
/*
*
* @brief        是否均衡允许检测
* @note         无
* @param[in]    无 
* @param[out]   无
* @return       无
*
*/
INT8U allow_blance_check(void)
{
    INT8U i=0,max_addr=0;
    max_addr = get_ltc_num();
    if(max_addr>MAX_LTC6803_NUM)
          max_addr =  MAX_LTC6803_NUM;
    for (i = 0; i < MAX_LTC6803_NUM; i++)                          //有电池需要均衡
    {
        if(g_bmsInfo.batteryBlanceState[i] != 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}


/*
*
* @brief       被动均衡
* @note         无
* @param[in]      
* @param[out]   无
* @return       无
*
*/
void Passive_balance_control(void) 
{
    INT8U   i=0,k=0,m=0,n=0,max_addr=0,blance_num=0,balance_max_num=0;
    INT8U   high_temp=0;
    INT16U  high_volt=0, low_volt=0, volt=0,balance_max_vol=0;
    OS_CPU_SR cpu_sr = 0;

        if(config_get(kBYUTotalBalEnIndex))
           volt = g_bms_info.average_voltage;
        else
           volt = bcu_get_low_voltage();  
    
    bmu_get_high_voltage(BYU_USE_SLAVE_INDEX, &high_volt, NULL);    
    bmu_get_low_voltage(BYU_USE_SLAVE_INDEX, &low_volt, NULL);    
    bmu_get_high_temperature(BYU_USE_SLAVE_INDEX, &high_temp, NULL);
    
    if ((bcu_is_charging() == TRUE)&&
        (config_get(kBmuBalanceEnableIndex))&&                                    
        (high_temp < config_get(kChgHTTrdAlarmIndex))&&
        (high_temp < config_get(kDChgHTTrdAlarmIndex))&&
        (low_volt > BL_OFF_VOLT)&&
        (config_get(kBYUBalNumIndex)>0)&&
        (high_volt >= config_get(kBYUBalStartVoltIndex))&&
        (high_volt < (volt+config_get(kBYUBalDiffVoltMaxIndex)))&&  
    	(high_volt > (volt+ config_get(kBYUBalDiffVoltMinIndex)))&&
    	(high_volt < config_get(kChgHVTrdAlarmIndex))&&
    	(high_volt < config_get(kDChgHVTrdAlarmIndex)))
    {                                                                //均衡条件满足
        max_addr=get_ltc_num();
        if(max_addr>MAX_LTC6803_NUM)
            max_addr = MAX_LTC6803_NUM;
        
        for(k = 0; k < max_addr; k++)
        {
            if(config_get(kBYUBalNumIndex) > MAX_BLANCE_BUF_LEN)
                 blance_num = MAX_BLANCE_BUF_LEN;
            else
                blance_num = (INT8U)config_get(kBYUBalNumIndex);

            switch(blance_num)
            {
                case 1:      //只开一路均衡
                
                    if(g_bmsInfo.batCnt[k]<=LTC_HALF_VOL_NUM) //小与等于6串的情况
                    {
                       balance_max_vol = g_balance_vol_info.before_six_high_volt[k];
                       balance_max_num = g_balance_vol_info.before_six_high_num[k];
                    }
                    else
                    {
                        if(g_balance_vol_info.before_six_high_volt[k]>=g_balance_vol_info.after_six_high_volt[k])
                        {
                            balance_max_vol = g_balance_vol_info.before_six_high_volt[k];
                             balance_max_num = g_balance_vol_info.before_six_high_num[k]; 
                        }
                        else
                        {
                             balance_max_vol = g_balance_vol_info.after_six_high_volt[k];
                             balance_max_num = g_balance_vol_info.after_six_high_num[k];
                        }
                    }
                    if((balance_max_vol >= config_get(kBYUBalStartVoltIndex))&&//单体开启条件
    				   (balance_max_vol < (volt + config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (balance_max_vol > (volt + config_get(kBYUBalDiffVoltMinIndex))))  // 单体电压与最小电压值相比是否大于最小均衡压差
    		        {
    				    m=k;
    				    n=balance_max_num;
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_CELLV_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //记录均衡状态
                    }
                break;
                case 2:      //只开两路均衡
                    if((g_balance_vol_info.before_six_high_volt[k] >= config_get(kBYUBalStartVoltIndex))&&//单体开启条件
    				   (g_balance_vol_info.before_six_high_volt[k]< (volt+config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (g_balance_vol_info.before_six_high_volt[k]> (volt+ config_get(kBYUBalDiffVoltMinIndex))))  // 单体电压与最小电压值相比是否大于最小均衡压差
    		        {
    				    m=k;
    				    n=g_balance_vol_info.before_six_high_num[k];
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_HALF_VOL_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //记录均衡状态
                    }
                    if( (g_bmsInfo.batCnt[k] > LTC_HALF_VOL_NUM)&&
                       (g_balance_vol_info.after_six_high_volt[k] >= config_get(kBYUBalStartVoltIndex))&&//单体开启条件
    				   (g_balance_vol_info.after_six_high_volt[k] < (volt+config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (g_balance_vol_info.after_six_high_volt[k] > (volt+ config_get(kBYUBalDiffVoltMinIndex))))  // 单体电压与最小电压值相比是否大于最小均衡压差
    		        {
    				    m=k;
    				    n=g_balance_vol_info.after_six_high_num[k];
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_CELLV_NUM)&&(n >= LTC_HALF_VOL_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //记录均衡状态
                    }
                default:
                break;
            }
        }
        
        if(allow_blance_check())
        {
            ControlLtcIO(&g_ltcRegGrop);
            blance_state_transform();
            OSTimeDly(PASSIVE_BALANCE_ON_TIME);             
        }
        else
        {
            clr_upper_balance_state();
        }
    }
    else
    {
            clr_upper_balance_state();
    }
}
/*
*
* @brief       开启电源
* @note         无
* @param[in]      
* @param[out]   无
* @return       1电源已经稳定开启，0电源没有稳定开启
*
*/
#define  BALANCE_POWER_OPEN_TIME   4
INT8U is_open_power(void)
{
    INT8U return_temp=0;
    OS_CPU_SR cpu_sr = 0;    
    //电源开启稳定后才能均衡
   //BALANCE_POWER_IO = 1;//开启电源 dlw
   blance_power.flag = 1;
   if(blance_power.open_dly++>BALANCE_POWER_OPEN_TIME)  
   {
      blance_power.open_dly =BALANCE_POWER_OPEN_TIME+1;
      return_temp = 1;
   }
   else
   {
      clr_upper_balance_state();
      return_temp = 0; 
   }
    blance_power.close_dly = 0;
    return  return_temp;
}
/*
*
* @brief       关闭电源
* @note         如果电源已经打开，半分钟后关闭。
* @param[in]      
* @param[out]   无
* @return       无
*
*/
#define  BALANCE_POWER_CLOSE_TIME   20
void close_balance_power(void)
{
    if(blance_power.flag == 1)
    {
        if(blance_power.close_dly++>BALANCE_POWER_CLOSE_TIME)
        {
            blance_power.close_dly = 0;  
            //BALANCE_POWER_IO = 0;      //关闭均衡电源 dlw
            blance_power.flag = 0;
        } 
    }
    else
    {
       blance_power.close_dly = 0;
       //BALANCE_POWER_IO = 0; // dlw
    }
    blance_power.open_dly =0;
    
}

/*
*
* @brief       主动均衡
* @note         无
* @param[in]      
* @param[out]   无
* @return       无
*
*/
void Initiative_balance_control(void) 
{
    INT8U   i=0,k=0,m=0,n=0,max_addr=0,blance_num=0,balance_min_num=0;
    INT16U  volt=0,balance_min_vol=0;
    OS_CPU_SR cpu_sr = 0;

    if ((bcu_is_charging() == TRUE)&&
        (config_get(kBmuBalanceEnableIndex))&&                                    
        (bcu_get_high_temperature_state() == kAlarmNone)&&
        (bcu_get_low_voltage() >= config_get(kBYUBalStartVoltIndex))&&
        (volt >= bcu_get_low_voltage()+config_get(kBYUBalDiffVoltMinIndex))&&
        (volt < bcu_get_low_voltage()+config_get(kBYUBalDiffVoltMaxIndex))&&
        (bcu_get_high_voltage()<config_get(kChgHVSndAlarmIndex))&&
        (config_get(kBYUBalNumIndex)>0))
    {                                                                //均衡条件满足
        if(!is_open_power())
            return;
        
        //开启均衡
        max_addr=get_ltc_num();
        if(max_addr>MAX_LTC6803_NUM)
            max_addr = MAX_LTC6803_NUM;
        
        if(config_get(kBYUTotalBalEnIndex))       //考虑整组均衡
           volt = g_bms_info.average_voltage;
        else
           volt = bcu_get_high_voltage();
        
        for(k = 0; k < max_addr; k++)
        {
            if(config_get(kBYUBalNumIndex) > MAX_BLANCE_BUF_LEN)
                 blance_num = MAX_BLANCE_BUF_LEN;
            else
                blance_num = (INT8U)config_get(kBYUBalNumIndex);
            switch(blance_num)
            {
                case 1:      //只开一路均衡
                
                    if(g_bmsInfo.batCnt[k]<=LTC_HALF_VOL_NUM) //小与等于6串的情况
                    {
                       balance_min_vol = g_balance_vol_info.before_six_low_volt[k];
                       balance_min_num = g_balance_vol_info.before_six_low_num[k];
                    }
                    else
                    {
                        if(g_balance_vol_info.before_six_low_volt[k]>=g_balance_vol_info.after_six_low_volt[k])
                        {
                            balance_min_vol = g_balance_vol_info.after_six_low_volt[k];
                             balance_min_num = g_balance_vol_info.after_six_low_num[k]; 
                        }
                        else
                        {
                             balance_min_vol = g_balance_vol_info.before_six_low_volt[k];
                             balance_min_num = g_balance_vol_info.before_six_low_num[k];
                        }
                    }
                    if((balance_min_vol >= config_get(kBYUBalStartVoltIndex))&&//单体开启条件
    				   (volt < (balance_min_vol + config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (volt > (balance_min_vol + config_get(kBYUBalDiffVoltMinIndex))))  // 单体电压与最小电压值相比是否大于最小均衡压差
    		        {
    				    m=k;
    				    n=balance_min_num;
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_CELLV_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //记录均衡状态
                    }
                break;
                case 2:      //只开两路均衡
                    if((g_balance_vol_info.before_six_low_volt[k] >= config_get(kBYUBalStartVoltIndex))&&//单体开启条件
    				   (volt < (g_balance_vol_info.before_six_low_volt[k]+config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (volt > (g_balance_vol_info.before_six_low_volt[k]+ config_get(kBYUBalDiffVoltMinIndex))))  // 单体电压与最小电压值相比是否大于最小均衡压差
    		        {
    				    m=k;
    				    n=g_balance_vol_info.before_six_low_num[k];
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_HALF_VOL_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //记录均衡状态
                    }
                    if( (g_bmsInfo.batCnt[k] > LTC_HALF_VOL_NUM)&&
                       (g_balance_vol_info.after_six_low_volt[k] >= config_get(kBYUBalStartVoltIndex))&&//单体开启条件
    				   (volt < (g_balance_vol_info.after_six_low_volt[k]+config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (volt > (g_balance_vol_info.after_six_low_volt[k]+ config_get(kBYUBalDiffVoltMinIndex))))  // 单体电压与最小电压值相比是否大于最小均衡压差
    		        {
    				    m=k;
    				    n=g_balance_vol_info.after_six_low_num[k];
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_CELLV_NUM)&&(n >= LTC_HALF_VOL_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //记录均衡状态
                    }
                default:
                break;
            }
        }
        
        if(allow_blance_check())
        {
            ControlLtcIO(&g_ltcRegGrop);
            blance_state_transform();   //开启均衡
            OSTimeDly(500);
        }
        else
        {
            clr_upper_balance_state();
        }
    }
    else
    {
        close_balance_power();
        clr_upper_balance_state();
    }
}

/*
* @brief        读取均衡类型
* @note         无
* @param[in]    
* @param[out]   
* @return       
*
*/
INT8U ReadConfigBalanceType(void)
{  
   INT8U temp=0;
   OS_CPU_SR cpu_sr = 0;
   
   OS_ENTER_CRITICAL(); 
   if(config_get(kBYUBalTypeIndex)<=BALANCE_TYPE_NUM)
        temp = (INT8U)config_get(kBYUBalTypeIndex);
    else
        temp =  0;
   OS_EXIT_CRITICAL();
   return temp;
}
/*
*
* @brief       电池均衡任务
* @note         无
* @param[in]    参数指针   
* @param[out]   无
* @return       无
*
*/ 

void BatBalanceTask(void *pdata)
{
    
    INT8U   err=0;
    INT8U   balance_type=0;
    OS_CPU_SR cpu_sr = 0;
    pdata = pdata;
    OSTimeDly(3000);
    for(;;)
    {
        balance_type =ReadConfigBalanceType();
        balance_condition_update();
        switch(balance_type)
        {
            case PASSIVE_BALANCE_TYPE:         //被动均衡
                close_balance_power();         //关主动均衡电源
                OSSemPend(g_LTCSPISem, 0, &err);
                OSTimeDly(20);                //等待电压采样完毕
                Passive_balance_control();
                clr_bat_blance_state();
                ControlLtcIO(&g_ltcRegGrop);  //关闭均衡
                OSTimeDly(5);                 //均衡完成等待5ms再采集 
                (void)OSSemPost(g_LTCSPISem);
                OSTimeDly(PASSIVE_BALANCE_OFF_TIME); 
          break;
          case INITIATIVE_BALANCE_TYPE:         //主动均衡
                OSSemPend(g_LTCSPISem, 0, &err);
                OSTimeDly(20);                 //等待电压采样完毕
                Initiative_balance_control();      
                clr_bat_blance_state();
                ControlLtcIO(&g_ltcRegGrop);   //关闭均衡
                OSTimeDly(20);                 //均衡完成等待20ms再采集 
                (void)OSSemPost(g_LTCSPISem);
                OSTimeDly(500); 
          break;
          default:                           //不均衡
                close_balance_power();       //关主动均衡电源
                clr_upper_balance_state();   //清除显示均衡状态
                OSSemPend(g_LTCSPISem, 0, &err);
                OSTimeDly(20);              //等待电压采样完毕
                clr_bat_blance_state();
                ControlLtcIO(&g_ltcRegGrop);//关闭均衡
                (void)OSSemPost(g_LTCSPISem);
                OSTimeDly(500); 
          break;
        }
    }    
}
#endif
#endif
/***********END OF FILE***********/