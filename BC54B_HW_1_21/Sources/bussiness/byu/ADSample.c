/**
*
* Copyright (c) 2012 Ligoo Inc. 
* 
* @file  ADSample.c
* @brief ��ز������� 
* @note 
* @version 1.00
* @author ��־��
* @date 2012/10/6  
*
*/
#ifndef AD_SAMPLE_SOURCE
#define AD_SAMPLE_SOURCE

#include "ADSample.h"
#include "NTC.h"
#include "bms_bmu.h"
#if  BMS_SUPPORT_HARDWARE_LTC6803 == 1 
#pragma MESSAGE DISABLE C1853 // Unary minus operator applied to unsigned type
#pragma MESSAGE DISABLE C2705
#pragma MESSAGE DISABLE C4000 // Condition always TRUE
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler


OS_STK     ADSampleTaskStack[AD_SAMPLE_TASK_STK_SIZE] = {0};            //��ѹ�ɼ������ջ
OS_EVENT        *g_LTCSPISem;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BYU_SAMPLE
LTC_REG_GROP    g_ltcRegGrop;
BMS_BYU_CXT     g_byuCxt;
VOL_BUF_GROP    g_vbuf;                //��ѹ��⻺������
static INT8U g_ltcInitOk = LTC_INIT_OK;
static INT8U g_ltcInitCnt = 0;
#if LTC_COMERR_CNT_SUPPORT
  INT8U g_ltcComErrorCnt[MAX_LTC6803_NUM]= {0};  //ͨ��ʧ�ܼ���
#endif
static INT8U g_tFilterCnt[MAX_LTC6803_NUM][LTC_CELLT_NUM] = {0};
static INT8U g_ltcPowerWireOffFlag[MAX_LTC6803_NUM] = {0};
static INT8U g_history_temp_buf[MAX_LTC6803_NUM][LTC_CELLT_NUM];
static INT8U g_before_temp_buf[MAX_LTC6803_NUM][LTC_CELLT_NUM];
static INT8U g_current_temp_buf[MAX_LTC6803_NUM][LTC_CELLT_NUM];
#pragma DATA_SEG DEFAULT

void ADSampleTask(void *pdata);
void ReadBatVoltage(INT16U* _PAGED ltcbuf,VOL_BUF_GROP* _PAGED vbufgrop);
void ReadBatTemperature(INT16U* _PAGED ltcbuf,INT8U t_extend_cnt);
void get_cellAB_vol(INT16U _PAGED cellABbuf[][LTC_CELLV_NUM]);
void copy_vol(INT16U _PAGED from_buf[][LTC_CELLV_NUM],INT16U _PAGED to_buf[][LTC_CELLV_NUM],VOL_BUF_GROP * _PAGED vbufgrop);
void wireoff_vol_fliter(BMS_INFO * _PAGED bmsinfo,VOL_BUF_GROP * _PAGED vbufgrop);
void ClrLtcPowerWireOffFlag(VOL_BUF_GROP * _PAGED vbufgrop);
INT8U voltage_is_abnormal(BMS_INFO * _PAGED bmsinfo,VOL_BUF_GROP * _PAGED vbuf);
void SelectTemperature(INT8U num);

INT8U valid_ltc_num(void);
void ControlLtcIO(LTC_REG_GROP * _PAGED ltcRegGrop);
void byu_update_bcu_info(void);

/*
* @brief        ADC���������ʼ��
* @note         ��
* @param[in]    ��
* @param[out]   ��
* @return       ��
*
*/ 
void ADSampleInit(void) 
{
    INT8U i;
    
    LTC_CS_OUT;
    SPI2_Init();                                        //��ʼ��SPI 
    
    for(i=0; i<MAX_LTC6803_NUM; i++)
        g_bmsInfo.batCnt[i] = config_get(kBYUVoltCnt1Index + i);
    for(i=0; i<MAX_LTC6803_NUM; i++)
        g_bmsInfo.tempCnt[i] = config_get(kBYUTempCnt1Index + i);
    
    g_LTCSPISem = OSSemCreate(1);                             //SPI�ź�����ʼ��,���Ա�֤ͬʱ���һ���������SPIͨ��

    (void) OSTaskCreate(ADSampleTask,                         //��ѹ�ɼ�����
                            (void *)0,
                            (OS_STK *)&ADSampleTaskStack[AD_SAMPLE_TASK_STK_SIZE - 1],
                            AD_SAMPLE_TASK_PRIO);
     watch_dog_register(WDT_AD_ID, WDT_AD_TIME);
}

/*
* @brief        ��ȡLTC��
* @note         ��
* @param[in]    ��
* @param[out]   ��
* @return       ��ЧLTC��
*
*/
INT8U get_ltc_num(void)
{
    INT8U num=0; 
    num = valid_ltc_num();
    if(num > MAX_LTC6803_NUM)
        num = MAX_LTC6803_NUM;
    return  num;
}
/*
*
* @brief       ��ѹ�ɼ�����
* @note         ��
* @param[in]    ����ָ��  
* @param[out]   ��
* @return       ��
*
*/ 
void  ADSampleTask(void *pdata)
{
    INT8U   err,i=0,k=0,m=0,t_extend_cnt=0;
    OS_CPU_SR cpu_sr = 0;
    INT16U ltc_reg_buf[LTC_CELLV_NUM]={0}; //��ѹ���¶��������黺��
    pdata = pdata;
    
    g_ltcInitOk = Ltc6803Init(&g_ltcRegGrop);  
                                                  
    for(;;)
    {
        watch_dog_feed(WDT_AD_ID);   
        if(g_ltcInitOk == LTC_INIT_OK)
        {
            
            if( (k++)%CLR_LTC6803_REG_PERIOD == 0)
            {
              OSSemPend(g_LTCSPISem, 0, &err);
              ClrLtcVReg();                 //���ѹ�¶Ȳɼ����ݼĴ���
              (void)OSSemPost(g_LTCSPISem);
              OSTimeDly(20);
            }
            
            //��ѹ�ɼ�����
            
            OSSemPend(g_LTCSPISem, 0, &err); //������ѹ�ɼ�
            StartLtcVAD();                   
            (void)OSSemPost(g_LTCSPISem);  

            OSTimeDly(23);                   //�ȴ���ѹ�ɼ����
             
            ReadBatVoltage(&ltc_reg_buf[0],&g_vbuf);                //��ȡ��ѹ����
            
            if(voltage_is_abnormal(&g_bmsInfo,&g_vbuf))       //�ж��Ƿ�����������
            {
                 get_cellAB_vol(g_vbuf.cellA);               //��һ�ο�������Դ�ɼ���ѹ
                 get_cellAB_vol(g_vbuf.cellB);              //�ڶ��ο�������Դ�ɼ���ѹ
                 wireoff_vol_fliter(&g_bmsInfo,&g_vbuf);    //������������ĵ�ѹ����
            }
            ClrLtcPowerWireOffFlag(&g_vbuf);
            copy_vol(g_vbuf.cellVBuf,g_bmsInfo.volt,&g_vbuf);  //�����յĵ�ѹ����g_vbuf.cellVBuf�ŵ�g_bmsInfo.volt��
            
            //�¶Ȳɼ�����
            
            if(t_extend_cnt>=LTC_CELLT_EXTEND_CNT)
                   t_extend_cnt = 0;
            OSSemPend(g_LTCSPISem, 0, &err);//ѡ���¶�
            SelectTemperature(t_extend_cnt);
            (void)OSSemPost(g_LTCSPISem);
            
            OSTimeDly(30);
            
            OSSemPend(g_LTCSPISem, 0, &err);//�����¶Ȳɼ�
            StartLtcTAD();                   
            (void)OSSemPost(g_LTCSPISem);  
            
            OSTimeDly(23);                   //�ȴ��¶Ȳɼ����
             
            ReadBatTemperature(&ltc_reg_buf[0],t_extend_cnt);            //��ȡ�¶�����
            
            t_extend_cnt++;
            
        }
        else
        {
            if(g_ltcInitCnt++ < LTC6803_INIT_CNT)           //4�β��ɹ��Ͳ��ٳ�ʼ��
            {
                 OSSemPend(g_LTCSPISem, 0, &err);
                 g_ltcInitOk = Ltc6803Init(&g_ltcRegGrop);
                 (void)OSSemPost(g_LTCSPISem); 
            }
            else
            {
               g_ltcInitOk = LTC_INIT_OK;
            }
        } 
        
        byu_update_bcu_info(); 
        OSTimeDly(3);
    }
}
/*
* @brief        ��LTC�е�ѹ��������
* @note         ��
* @param[in]    ��
* @param[out]   ��
* @return       ��
*
*/

void ClrLtcPowerWireOffFlag(VOL_BUF_GROP * _PAGED vbufgrop)
{
    INT8U ltc_addr=0;
    for(ltc_addr = 0;ltc_addr< MAX_LTC6803_NUM; ltc_addr++)
    {
        if((vbufgrop->cellVBuf[ltc_addr][0] != 0)&&
          (g_bmsInfo.batCnt[ltc_addr]>0)&&
          (g_bmsInfo.batCnt[ltc_addr]<=LTC_CELLV_NUM)&&
          (vbufgrop->cellVBuf[ltc_addr][g_bmsInfo.batCnt[ltc_addr]-1] != 0))    
        {    
            g_ltcPowerWireOffFlag[ltc_addr] =FALSE;
        } 
    }
}
/*
* @brief        ��LTC�е�ѹ��������
* @note         ��
* @param[in]    ��
* @param[out]   ��
* @return       ��
*
*/
void voltage_data_reorganize(INT16U * _PAGED buf,INT8U len)
{
   INT8U i=0;
   INT16U  voltagedata=0;
    if(!buf)
        return;
    if(len > VOL_REORGANIZE_DATA_LEN)
        len = VOL_REORGANIZE_DATA_LEN;
    for(i = 0; i < len; i++)                    
    {
        voltagedata = ((INT16U)g_ltcRegGrop.VReg[i*3+1] << 8) & 0x0f00;
        voltagedata += g_ltcRegGrop.VReg[i*3];
        if((i*2) <= LTC_CELLV_NUM)
            buf[i*2] = voltagedata;
        voltagedata = ((g_ltcRegGrop.VReg[i*3+1] >> 4) & 0x0f) + (((INT16U)g_ltcRegGrop.VReg[i*3+2] << 4) & 0x0ff0);
        if((i*2+1) <= LTC_CELLV_NUM)
            buf[i*2+1] = voltagedata;        
    } 

}
/*
* @brief       �����ص�ѹ
* @note         ��
* @param[in]    LTC��ַ
* @param[out]   ��
* @return       ��
*
*/
void calculate_bat_voltage(INT8U ltc_addr,INT16U * _PAGED buf,VOL_BUF_GROP * _PAGED vbuf,INT16U vcheck)
{
    INT8U i=0;
    INT16U tmp=0;
    OS_CPU_SR cpu_sr = 0;
    if(ltc_addr > MAX_LTC6803_NUM)
        return;
    if((!buf)||(!vbuf))
        return;
    for (i = 0; i < LTC_CELLV_NUM; i++)                 
    {
        if((buf[i] > LTC_ZERO_VALUE)&&(buf[i]<0x0FFF))
            tmp = buf[i]-LTC_ZERO_VALUE;  
        else
            tmp = 0;
        OS_ENTER_CRITICAL();
        if(tmp!=0)
            vbuf->cellVBuf[ltc_addr][i] = tmp + (tmp >> 1) + vcheck;//y=(x-512)*1.5+У׼ֵ    ��λ mv
        else
            vbuf->cellVBuf[ltc_addr][i] = 0;
        if(vbuf->cellVBuf[ltc_addr][i] > NEGATIVE_CRITICAL_POINT)
           vbuf->cellVBuf[ltc_addr][i] = 0;
        OS_EXIT_CRITICAL();    
    }
}
/*
* @brief        ��ȡ���е�ص�ѹ
* @note         ��
* @param[in]    ��
* @param[out]   ��
* @return       ��
*
*/
void ReadBatVoltage(INT16U* _PAGED  ltcbuf,VOL_BUF_GROP * _PAGED vbufgrop)
{
    INT8U  ltc_addr=0,i=0,err=0,ltcerr=0,max_addr=0,temp=0;
    OS_CPU_SR cpu_sr = 0;
    if((!ltcbuf)||(!vbufgrop))
        return;     
    max_addr = get_ltc_num();
    if(max_addr>MAX_LTC6803_NUM)
        max_addr = MAX_LTC6803_NUM;
    for(ltc_addr = 0;ltc_addr < max_addr; ltc_addr++)
    {
        OSSemPend(g_LTCSPISem, 0, &err);
        //OS_ENTER_CRITICAL();
        ltcerr = ReadLtcVoltage(ltc_addr, &g_ltcRegGrop);  //����ѹ�Ĵ���
        //OS_EXIT_CRITICAL();
        (void)OSSemPost(g_LTCSPISem); 
        
        if (ltcerr == TRUE)
        {   
            g_bmsInfo.LtcVHeartBeat[ltc_addr] = 0;           //ͨѶ����
            
            for (i = 0; i < LTC_CELLV_NUM; i++)
                ltcbuf[i] = 0x0000;
                
            voltage_data_reorganize(ltcbuf,VOL_REORGANIZE_DATA_LEN);
            
            calculate_bat_voltage(ltc_addr,ltcbuf,vbufgrop,config_get(kBYUVoltCorrectIndex));
            
            if(g_bmsInfo.batCnt[ltc_addr] > LTC_CELLV_NUM)
                continue;
            
            for(i = g_bmsInfo.batCnt[ltc_addr]; i < LTC_CELLV_NUM; i++) 
            {
                OS_ENTER_CRITICAL();
                vbufgrop->cellVBuf[ltc_addr][i] = 0;        //û��ʹ�õĺ����ѹ��������
                OS_EXIT_CRITICAL();   
            }
        }
        else
        {
           #if LTC_COMERR_CNT_SUPPORT
            g_ltcComErrorCnt[ltc_addr]++;
           #endif
           if(g_bmsInfo.LtcVHeartBeat[ltc_addr]++ > LTC_COM_ERR_CNT)                //4������û��ͨѶ�ɹ�
           {
                g_bmsInfo.LtcVHeartBeat[ltc_addr] = LTC_COM_ERR_CNT + 1;               //�̶�ס����
                for (i = 0; i < LTC_CELLV_NUM; i++)
                {
                    OS_ENTER_CRITICAL();
                    vbufgrop->cellVBuf[ltc_addr][i] = 0;      //��LTCͨѶʧ����������
                    OS_EXIT_CRITICAL();    
                }
           }
        }
        
    }
    for(ltc_addr = max_addr;ltc_addr < MAX_LTC6803_NUM; ltc_addr++)
    {
        for (i = 0; i < LTC_CELLV_NUM; i++)
        {
            OS_ENTER_CRITICAL();
            vbufgrop->cellVBuf[ltc_addr][i] = 0;     
            OS_EXIT_CRITICAL();    
        }
    }
}
/*
* @brief        �жϵ�ѹ�Ƿ��쳣
* @note         ��ǰ�ɼ����ĵ�ѹ�����Ƿ��ǰһ�βɼ����ĵ�ѹ���ݴ� CHANGED_VOL��30��mv
* @param[in]    BMS_INFO *bmsinfo��VOL_BUF_GROP *vbufgrop
* @param[out]   ��
* @return       FALSE�� TRUE
*
*/
INT8U voltage_is_abnormal(BMS_INFO * _PAGED bmsinfo,VOL_BUF_GROP * _PAGED vbufgrop)
{
    INT8U  ltc_addr=0,max_addr=0,i=0;
    if((!bmsinfo)||(!vbufgrop))
        return FALSE;     
    max_addr = get_ltc_num();
    if(max_addr>MAX_LTC6803_NUM)
        max_addr = MAX_LTC6803_NUM; ;  
    for(ltc_addr = 0;ltc_addr < max_addr; ltc_addr++)
    {
        if(bmsinfo->batCnt[ltc_addr] > LTC_CELLV_NUM)       //ģ���ظ������ڹ涨ֵ
            continue;  
        if(bmsinfo->LtcVHeartBeat[ltc_addr] > LTC_COM_ERR_CNT) //ltcģ����DZ60ͨѶ�ж�
            continue;
        for(i = 0; i < bmsinfo->batCnt[ltc_addr]; i++)
        {
             if(ABS(vbufgrop->cellVBuf[ltc_addr][i] - bmsinfo->volt[ltc_addr][i]) > CHANGED_VOL) //�Ƚ�
                return TRUE;
        }
    }
    return FALSE;
}



/*
* @brief        ��ȡcellA��cellB��ֵ
* @note         ��ȡ��������Դ�ĵ�ѹֵ
* @param[in]    ��
* @param[out]   AB��ѹ����cellABbuf[][LTC_CELLV_NUM]
* @return       ��
*
*/
void get_cellAB_vol(INT16U _PAGED cellABbuf[][LTC_CELLV_NUM])
{
    INT8U  ltc_addr=0,i=0,err=0,ltcerr=0,max_addr=0;
    OS_CPU_SR cpu_sr = 0;
    
    max_addr = get_ltc_num();
    if(max_addr>MAX_LTC6803_NUM)
        return;
    OSSemPend(g_LTCSPISem, 0, &err);
    VWireOff();                             //��������Դ���ɼ�
    (void)OSSemPost(g_LTCSPISem);
    OSTimeDly(40);                          //��ʱ

    for(ltc_addr = 0;ltc_addr < max_addr; ltc_addr++)
    {
        OSSemPend(g_LTCSPISem, 0, &err);
        //OS_ENTER_CRITICAL();
        ltcerr = ReadLtcVoltage(ltc_addr, &g_ltcRegGrop);  //����ѹ�Ĵ���
        //OS_EXIT_CRITICAL();
        (void)OSSemPost(g_LTCSPISem); 
        
        if (ltcerr == TRUE)
        {   
            for (i = 0; i < LTC_CELLV_NUM; i++)
                cellABbuf[ltc_addr][i] = 0x0000;
            voltage_data_reorganize(cellABbuf[ltc_addr],VOL_REORGANIZE_DATA_LEN);
        }
        else
        {
           #if LTC_COMERR_CNT_SUPPORT
            g_ltcComErrorCnt[ltc_addr]++;
           #endif
            for (i = 0; i < LTC_CELLV_NUM; i++)
                cellABbuf[ltc_addr][i] = 0x0000;
        }
    }
}
/*
* @brief        ������cellVBuf�����ݸ��Ƶ�g_bmsInfo.volt
* @note         ��
* @param[in]    from_buf�� to_buf��VOL_BUF_GROP *vbufgrop
* @param[out]   ��
* @return       ��
*
*/
void copy_vol(INT16U _PAGED from_buf[][LTC_CELLV_NUM],INT16U _PAGED to_buf[][LTC_CELLV_NUM],VOL_BUF_GROP* _PAGED vbufgrop)
{
    INT8U m=0,i=0;
    OS_CPU_SR cpu_sr = 0; 
    if(!vbufgrop)
        return;
    for(m = 0; m < MAX_LTC6803_NUM; m++)
    {
        for(i = 0; i < LTC_CELLV_NUM; i++)
        {
           if(( from_buf[m][i] == 0 ) || (from_buf[m][i] >= BATTERY_VOLTAGE_MAX_VALUE)||(from_buf[m][i] <= BATTERY_VOLTAGE_MIN_VALUE))  //�����ѹ�쳣������ȷ�ϵ�ѹ�쳣���ٸ���
           {
               if(vbufgrop->vfiltercnt[m][i]++ > V_FILTER_CNT)
               {
                    from_buf[m][i] = 0;
                    OS_ENTER_CRITICAL(); 
                    to_buf[m][i] = from_buf[m][i];
                    OS_EXIT_CRITICAL(); 
                    vbufgrop->vfiltercnt[m][i] = 0;
               }
           }
           else                                //���쳣ֱ�Ӹ���
           {
               vbufgrop->vfiltercnt[m][i] = 0;
               OS_ENTER_CRITICAL(); 
               to_buf[m][i] = from_buf[m][i];
               OS_EXIT_CRITICAL();
           }
        }
    }
}
/*
* @brief        ����������ĵ�ص�ѹ���˵�
* @note         ��
* @param[in]    BMS_INFO *bmsinfo,VOL_BUF_GROP *vbufgrop
* @param[out]   ��
* @return       ��
*
*/
void wireoff_vol_fliter(BMS_INFO * _PAGED bmsinfo,VOL_BUF_GROP * _PAGED vbufgrop)
{
    INT8U m=0,i=0,max_addr=0,k=0;
    
    if((!bmsinfo)||(!vbufgrop))
        return;
    max_addr = get_ltc_num();
    if(max_addr>MAX_LTC6803_NUM)
        return; 
    
    for(m = 0; m < max_addr; m++)
    {
        if(bmsinfo->batCnt[m]>LTC_CELLV_NUM)
            continue;
        
        for( i= 0; i < bmsinfo->batCnt[m];i++)
        {  //����CELLB(n+1)-CELLA(n+1)>200mV ����CELLB(n+1)�_��5.375V��ȫ�˶�ֵ���tC n �_·��
           if( (vbufgrop->cellB[m][i] >= LTC_MAX_AD_DATA) ||
               (vbufgrop->cellB[m][i] > (vbufgrop->cellA[m][i]+OPEN_STOWAD_VOL_CHANGE)))
           {
                vbufgrop->cellVBuf[m][i] = 0;
                if ((i<=(bmsinfo->batCnt[m] -1))&&(bmsinfo->batCnt[m]>1))
                {  
                    if(i>0)
                    {
                      vbufgrop->cellVBuf[m][i-1] = 0; 
                    }
                }
                if((vbufgrop->cellVBuf[m][1] == 0)&&(vbufgrop->cellVBuf[m][0] <= FIRST_BAT_MIN_VOL))
                {
                    vbufgrop->cellVBuf[m][0] = 0;
                }
           } //����CELLA(1)<0��CELLB(1)<0���t V�ض��_·��
           else if((vbufgrop->cellB[m][i] < LTC_ZERO_VALUE)||
                   (vbufgrop->cellA[m][i] < LTC_ZERO_VALUE))
           {
                vbufgrop->cellVBuf[m][i]=0;
                if ((i == 0)||(i == (bmsinfo->batCnt[m]-1)))                  //v-��v+������ȫ������
                {  
                    for(k=0;k<LTC_CELLV_NUM;k++)
                    {
                      vbufgrop->cellVBuf[m][k]=0;
                    }
                    g_ltcPowerWireOffFlag[m] = TRUE;
                }
           }
        }
    }
}

/*
* @brief        ��LTC���¶���������
* @note         ��
* @param[in]    ��
* @param[out]   ��
* @return       ��
*
*/
void temperature_data_reorganize(INT16U * _PAGED tempbuf)
{
    INT16U  temperature=0;
    if(!tempbuf)
        return;
    temperature = ((INT16U)g_ltcRegGrop.TReg[1] << 8) & 0x0f00;      //��LTC���¶���������
    temperature += g_ltcRegGrop.TReg[0];
    tempbuf[0] = temperature;
    temperature = ((g_ltcRegGrop.TReg[1] >> 4) & 0x0f) + ((INT16U)g_ltcRegGrop.TReg[2] << 4);
    tempbuf[1] = temperature;
}
/*
* @brief       �������¶�
* @note         ��
* @param[in]    LTC��ַ
* @param[out]   ��
* @return       ��
*
*/
void calculate_tmperature(INT8U ltc_addr,INT16U * _PAGED buf,INT8U t_extend_cnt)
{
    INT8U i=0,index=0;
    INT16U tmp=0;
    OS_CPU_SR cpu_sr = 0;
    if(ltc_addr > MAX_LTC6803_NUM)
        return;
    if(!buf)
        return;
    for (i = 0; i < LTC_CELLT_MIN_NUM; i++)
    {
        if((buf[i] > LTC_ZERO_VALUE)&&(buf[i]<0x0FFF))
            tmp = buf[i]-LTC_ZERO_VALUE;   
        else
            tmp = 0;
        tmp = tmp + (tmp >> 1) ;           //y=(x-512)*1.5
        
        index = i+t_extend_cnt*2;
        if(index >= LTC_CELLT_NUM)
           continue;
        
        tmp = TempQuery(tmp, NTCTAB_HFSJ);       //���ݲɼ����ĵ�ѹ���
        if(g_ltcPowerWireOffFlag[ltc_addr] == TRUE)
        {
            if(g_tFilterCnt[ltc_addr][index]++ > T_FILTER_CNT)
                g_tFilterCnt[ltc_addr][index] = T_FILTER_CNT+1;
            tmp = 0;
        }
        else
        {
            g_tFilterCnt[ltc_addr][index] = 0;
        }
        if((g_tFilterCnt[ltc_addr][index] == 0)||(g_tFilterCnt[ltc_addr][index]>T_FILTER_CNT))
        {
            //��Ҫ�Ƿ�ֹ��EFTʵ�鵼���¶Ȳɼ�ֵ�仯���Ż�����EFTʵ��,��ǰ�ɼ��¶Ⱥ�ǰһ�βɼ��¶ȱȽϱ仯������LTC_TEMP_CHANGE_MIN �棬
            //��ǰǰһ�βɼ��¶ȱȽϱ仯������LTC_TEMP_CHANGE_MAX ��
            g_current_temp_buf[ltc_addr][index] = (INT8U)tmp;
            
            if((abs(g_current_temp_buf[ltc_addr][index]-g_before_temp_buf[ltc_addr][index])<=LTC_TEMP_CHANGE_MIN)
            &&(abs(g_current_temp_buf[ltc_addr][index]-g_history_temp_buf[ltc_addr][index])<=LTC_TEMP_CHANGE_MAX))
            {
                OS_ENTER_CRITICAL();
                g_bmsInfo.temp[ltc_addr][index] = (INT8U)tmp;
                OS_EXIT_CRITICAL();
            }
            g_history_temp_buf[ltc_addr][index] = g_before_temp_buf[ltc_addr][index];
            g_before_temp_buf[ltc_addr][index] = g_current_temp_buf[ltc_addr][index];
        }
    } 
    
}

/*
* @brief        ��ȡ���е���¶�
* @note         ��
* @param[in]    ��
* @param[out]   ��
* @return       ��
*
*/
void ReadBatTemperature(INT16U* _PAGED  ltcbuf,INT8U t_extend_cnt)
{
    INT8U  ltc_addr=0,i=0,err=0,ltcerr=0,max_addr=0;
    OS_CPU_SR cpu_sr = 0;
    if(!ltcbuf)
        return;
    max_addr = get_ltc_num();
    if(max_addr > MAX_LTC6803_NUM)
        return;
    
    for(ltc_addr = 0;ltc_addr < max_addr; ltc_addr++)
    {
        
        OSSemPend(g_LTCSPISem, 0, &err);//���¶ȼĴ���
        //OS_ENTER_CRITICAL();
        ltcerr = ReadLtcTemperature(ltc_addr, &g_ltcRegGrop);        
        //OS_EXIT_CRITICAL();
        (void)OSSemPost(g_LTCSPISem); 
        
        if (ltcerr == TRUE)
        {   
            g_bmsInfo.LtcTHeartBeat[ltc_addr] = 0;
            ltcbuf[0]=0x0000;
            ltcbuf[1]=0x0000;
            temperature_data_reorganize(ltcbuf);
            calculate_tmperature(ltc_addr,ltcbuf,t_extend_cnt);        
        }
        else
        {
           #if LTC_COMERR_CNT_SUPPORT
            g_ltcComErrorCnt[ltc_addr]++;
           #endif
           
            if(g_bmsInfo.LtcTHeartBeat[ltc_addr]++ > LTC_COM_ERR_CNT)             //LTC_COM_ERR_CNT������û��ͨѶ�ɹ�
            {
                g_bmsInfo.LtcTHeartBeat[ltc_addr] = LTC_COM_ERR_CNT+1;             //�̶�ס����
                  for (i = 0; i < LTC_CELLT_NUM; i++)
                {
                    OS_ENTER_CRITICAL();
                    g_bmsInfo.temp[ltc_addr][i] = 0;     //��LTCͨѶʧ����������
                    OS_EXIT_CRITICAL();
                }
            }
        }
    }
    for(ltc_addr = max_addr;ltc_addr < MAX_LTC6803_NUM; ltc_addr++)
    {
          for (i = 0; i < LTC_CELLT_NUM; i++)
        {
            OS_ENTER_CRITICAL();
            g_bmsInfo.temp[ltc_addr][i] = 0;    
            OS_EXIT_CRITICAL();
        }
    }
}
/*
* @brief        ѡ���¶ȴ�����
* @note         ��
* @param[in]    ��
* @param[out]   ��
* @return       ��
*
*/
void SelectTemperature(INT8U t_extend_cnt)
{
    if(t_extend_cnt >=LTC_CELLT_EXTEND_CNT)
          return;
    
        g_ltcRegGrop.CfgReg[0] &=~0x20;
        g_ltcRegGrop.CfgReg[0] &=~0x40;    
        switch(t_extend_cnt)
        {
            case 0 :
                g_ltcRegGrop.CfgReg[0] &=~0x20;
                g_ltcRegGrop.CfgReg[0] &=~0x40;
                break;
            case 1:
                g_ltcRegGrop.CfgReg[0] |=0x20;
                g_ltcRegGrop.CfgReg[0] &=~0x40;
                break;
            case 2:
                g_ltcRegGrop.CfgReg[0] &=~0x20;
                g_ltcRegGrop.CfgReg[0] |=0x40;
                
                break;
            case 3:
                g_ltcRegGrop.CfgReg[0] |=0x20;
                g_ltcRegGrop.CfgReg[0] |=0x40;
                break;
            
            default:
                break;
        }
           
        ControlLtcIO(&g_ltcRegGrop);
}
#endif
#endif
/***********END OF FILE***********/      