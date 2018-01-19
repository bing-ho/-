/**
*
* Copyright (c) 2012 Ligoo Inc. 
* 
* @file  BatBalance.c
* @brief ��ؾ����ļ�
* @note 
* @version 1.00
* @author ��־��
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

OS_STK g_batBalanceTaskStack[BAT_BALANCE_TASK_STK_SIZE]={0};//��ؾ��������ջ

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BYU_SAMPLE
BALANCE_VOL_INFO  g_balance_vol_info;
BalancePower blance_power;
#pragma DATA_SEG DEFAULT

void BatBalanceTask(void *pdata);
/*
*
* @brief        ��ؾ��������ʼ��
* @note         ������ؾ�������
* @param[in]    ��  
* @param[out]   ��
* @return       ��
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
* @brief        ���¾�������
* @note         
* @param[in]    ��  
* @param[out]   ��
* @return       ��
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
        for(j=0;j<LTC_HALF_VOL_NUM;j++)   //ǰ���������͵�ѹ
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
    for(i=0; i<max_addr; i++) //ǰ6����6�����(��)��ѹ������
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

    safe_memset(H_LVN, 0, MAX_LTC6803_NUM*sizeof(INT8U));   //ǰ6����͵�ѹ���
	safe_memset(H_HVN, 0, MAX_LTC6803_NUM*sizeof(INT8U));   //ǰ6����ߵ�ѹ���
	for(i=0; i<MAX_LTC6803_NUM; i++)
	{
	    H_LV[i] = BATTERY_VOLTAGE_MAX_VALUE;
	    H_HV[i] = 0;
	}
	for(i = 0; i < max_addr; i++)
    {
    	for(j=LTC_HALF_VOL_NUM;j<LTC_CELLV_NUM;j++) //�����������͵�ѹ
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
    for(i=0; i<max_addr; i++) //ǰ6����6�����(��)��ѹ������
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
* @brief        ����״̬ת��
* @note         
* @param[in]    ��
* @param[out]   ��
* @return       ��
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
* @brief        �����λ����ʾ�ľ���״̬
* @note         ��
* @param[in]    �� 
* @param[out]   ��
* @return       ��
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
* @brief        ���⿪��
* @note         ��
* @param[in]    �� 
* @param[out]   ��
* @return       ��
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
* @brief        ���bat����״̬
* @note         ��
* @param[in]    �� 
* @param[out]   ��
* @return       ��
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
* @brief        �Ƿ����������
* @note         ��
* @param[in]    �� 
* @param[out]   ��
* @return       ��
*
*/
INT8U allow_blance_check(void)
{
    INT8U i=0,max_addr=0;
    max_addr = get_ltc_num();
    if(max_addr>MAX_LTC6803_NUM)
          max_addr =  MAX_LTC6803_NUM;
    for (i = 0; i < MAX_LTC6803_NUM; i++)                          //�е����Ҫ����
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
* @brief       ��������
* @note         ��
* @param[in]      
* @param[out]   ��
* @return       ��
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
    {                                                                //������������
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
                case 1:      //ֻ��һ·����
                
                    if(g_bmsInfo.batCnt[k]<=LTC_HALF_VOL_NUM) //С�����6�������
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
                    if((balance_max_vol >= config_get(kBYUBalStartVoltIndex))&&//���忪������
    				   (balance_max_vol < (volt + config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (balance_max_vol > (volt + config_get(kBYUBalDiffVoltMinIndex))))  // �����ѹ����С��ѹֵ����Ƿ������С����ѹ��
    		        {
    				    m=k;
    				    n=balance_max_num;
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_CELLV_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //��¼����״̬
                    }
                break;
                case 2:      //ֻ����·����
                    if((g_balance_vol_info.before_six_high_volt[k] >= config_get(kBYUBalStartVoltIndex))&&//���忪������
    				   (g_balance_vol_info.before_six_high_volt[k]< (volt+config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (g_balance_vol_info.before_six_high_volt[k]> (volt+ config_get(kBYUBalDiffVoltMinIndex))))  // �����ѹ����С��ѹֵ����Ƿ������С����ѹ��
    		        {
    				    m=k;
    				    n=g_balance_vol_info.before_six_high_num[k];
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_HALF_VOL_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //��¼����״̬
                    }
                    if( (g_bmsInfo.batCnt[k] > LTC_HALF_VOL_NUM)&&
                       (g_balance_vol_info.after_six_high_volt[k] >= config_get(kBYUBalStartVoltIndex))&&//���忪������
    				   (g_balance_vol_info.after_six_high_volt[k] < (volt+config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (g_balance_vol_info.after_six_high_volt[k] > (volt+ config_get(kBYUBalDiffVoltMinIndex))))  // �����ѹ����С��ѹֵ����Ƿ������С����ѹ��
    		        {
    				    m=k;
    				    n=g_balance_vol_info.after_six_high_num[k];
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_CELLV_NUM)&&(n >= LTC_HALF_VOL_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //��¼����״̬
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
* @brief       ������Դ
* @note         ��
* @param[in]      
* @param[out]   ��
* @return       1��Դ�Ѿ��ȶ�������0��Դû���ȶ�����
*
*/
#define  BALANCE_POWER_OPEN_TIME   4
INT8U is_open_power(void)
{
    INT8U return_temp=0;
    OS_CPU_SR cpu_sr = 0;    
    //��Դ�����ȶ�����ܾ���
   //BALANCE_POWER_IO = 1;//������Դ dlw
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
* @brief       �رյ�Դ
* @note         �����Դ�Ѿ��򿪣�����Ӻ�رա�
* @param[in]      
* @param[out]   ��
* @return       ��
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
            //BALANCE_POWER_IO = 0;      //�رվ����Դ dlw
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
* @brief       ��������
* @note         ��
* @param[in]      
* @param[out]   ��
* @return       ��
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
    {                                                                //������������
        if(!is_open_power())
            return;
        
        //��������
        max_addr=get_ltc_num();
        if(max_addr>MAX_LTC6803_NUM)
            max_addr = MAX_LTC6803_NUM;
        
        if(config_get(kBYUTotalBalEnIndex))       //�����������
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
                case 1:      //ֻ��һ·����
                
                    if(g_bmsInfo.batCnt[k]<=LTC_HALF_VOL_NUM) //С�����6�������
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
                    if((balance_min_vol >= config_get(kBYUBalStartVoltIndex))&&//���忪������
    				   (volt < (balance_min_vol + config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (volt > (balance_min_vol + config_get(kBYUBalDiffVoltMinIndex))))  // �����ѹ����С��ѹֵ����Ƿ������С����ѹ��
    		        {
    				    m=k;
    				    n=balance_min_num;
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_CELLV_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //��¼����״̬
                    }
                break;
                case 2:      //ֻ����·����
                    if((g_balance_vol_info.before_six_low_volt[k] >= config_get(kBYUBalStartVoltIndex))&&//���忪������
    				   (volt < (g_balance_vol_info.before_six_low_volt[k]+config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (volt > (g_balance_vol_info.before_six_low_volt[k]+ config_get(kBYUBalDiffVoltMinIndex))))  // �����ѹ����С��ѹֵ����Ƿ������С����ѹ��
    		        {
    				    m=k;
    				    n=g_balance_vol_info.before_six_low_num[k];
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_HALF_VOL_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //��¼����״̬
                    }
                    if( (g_bmsInfo.batCnt[k] > LTC_HALF_VOL_NUM)&&
                       (g_balance_vol_info.after_six_low_volt[k] >= config_get(kBYUBalStartVoltIndex))&&//���忪������
    				   (volt < (g_balance_vol_info.after_six_low_volt[k]+config_get(kBYUBalDiffVoltMaxIndex)))&&  
    				   (volt > (g_balance_vol_info.after_six_low_volt[k]+ config_get(kBYUBalDiffVoltMinIndex))))  // �����ѹ����С��ѹֵ����Ƿ������С����ѹ��
    		        {
    				    m=k;
    				    n=g_balance_vol_info.after_six_low_num[k];
    				    if((m < MAX_LTC6803_NUM) && (n < LTC_CELLV_NUM)&&(n >= LTC_HALF_VOL_NUM))
    				        g_bmsInfo.batteryBlanceState[m] |= (1U << n);   //��¼����״̬
                    }
                default:
                break;
            }
        }
        
        if(allow_blance_check())
        {
            ControlLtcIO(&g_ltcRegGrop);
            blance_state_transform();   //��������
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
* @brief        ��ȡ��������
* @note         ��
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
* @brief       ��ؾ�������
* @note         ��
* @param[in]    ����ָ��   
* @param[out]   ��
* @return       ��
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
            case PASSIVE_BALANCE_TYPE:         //��������
                close_balance_power();         //�����������Դ
                OSSemPend(g_LTCSPISem, 0, &err);
                OSTimeDly(20);                //�ȴ���ѹ�������
                Passive_balance_control();
                clr_bat_blance_state();
                ControlLtcIO(&g_ltcRegGrop);  //�رվ���
                OSTimeDly(5);                 //������ɵȴ�5ms�ٲɼ� 
                (void)OSSemPost(g_LTCSPISem);
                OSTimeDly(PASSIVE_BALANCE_OFF_TIME); 
          break;
          case INITIATIVE_BALANCE_TYPE:         //��������
                OSSemPend(g_LTCSPISem, 0, &err);
                OSTimeDly(20);                 //�ȴ���ѹ�������
                Initiative_balance_control();      
                clr_bat_blance_state();
                ControlLtcIO(&g_ltcRegGrop);   //�رվ���
                OSTimeDly(20);                 //������ɵȴ�20ms�ٲɼ� 
                (void)OSSemPost(g_LTCSPISem);
                OSTimeDly(500); 
          break;
          default:                           //������
                close_balance_power();       //�����������Դ
                clr_upper_balance_state();   //�����ʾ����״̬
                OSSemPend(g_LTCSPISem, 0, &err);
                OSTimeDly(20);              //�ȴ���ѹ�������
                clr_bat_blance_state();
                ControlLtcIO(&g_ltcRegGrop);//�رվ���
                (void)OSSemPost(g_LTCSPISem);
                OSTimeDly(500); 
          break;
        }
    }    
}
#endif
#endif
/***********END OF FILE***********/