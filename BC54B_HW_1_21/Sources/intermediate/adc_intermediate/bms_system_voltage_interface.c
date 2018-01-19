         /**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_system_voltage_impl.c
 * @brief
 * @note
 * @author Liwei Dong
 * @date 2014-9-23
 *
 */
#include "bms_system_voltage_interface.h"
#include "adc_xgate.h"
#include "adc0_intermediate.h"
#include "ad_hardware.h"     //AAAAAAAAAA 
 

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE C5919 // Conversion of floating to unsigned integral


//static INT16U g_systemVoltageValue = 0;     liqing  20170428 to paged

#if BMS_SUPPORT_SYSTEM_VOLTAGE  

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
static INT16U g_system24vDCValue = 0;
static INT16U g_systemLeadAcidValue = 0;
static INT16U g_system5vHallDetValue    = 0;
static INT16U g_system8vHallDetValue    = 0;
static INT16U g_system5vDetValue        = 0;
static INT16U g_systemBatDetValue       = 0;

static const struct adc_onchip_channel *g_chgdc_voltage_ad_param;

//static ADC_ChannelParameter g_system_voltage_ad;              liqing 20170428 ����
//INT16U system_voltage_ad_buff[BCU_SYSTEM_VOLT_BUFF_MAX] = {0};  liqing 20170428 ����

static INT16U g_systemVoltageValue = 0; 

#pragma DATA_SEG DEFAULT

/***********************************************************************
  * @brief           bms������ѹ����ʼ��
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
void system_voltage_init(void)
{
#if BMS_SUPPORT_BY5248D == 0
#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    if(hardware_io_revision_get() >= HW_VER_116){//V1.16 & V1.20
        g_chgdc_voltage_ad_param = &adc_onchip_v24_det_v116;
    }
    else{
        g_chgdc_voltage_ad_param = &adc_onchip_v24_det_v114;
    }
#else
    g_chgdc_voltage_ad_param = &adc_onchip_v24_det_v116;
#endif    
#else
    g_chgdc_voltage_ad_param = &adc_onchip_v24_det_v116;
#endif    
    job_schedule(MAIN_JOB_GROUP, BCU_SYSTEM_VOLTAGE_JOB_PERIODIC, bcu_system_voltage_detect, NULL);
    bcu_calc_system_voltage(&g_systemVoltageValue);
}

/***********************************************************************
  * @brief           ����ϵͳ��ѹֵ
  * @param[in]       ��  
  * @return          ��ѹֵ
***********************************************************************/ 
inline INT16U system_voltage_get(void)
{
    return g_systemVoltageValue;
}

/***********************************************************************
  * @brief           �ж�ϵͳ�����ѹ�Ƿ���Ч
  * @param[in]       ��  
  * @return          1��ok  0��err
***********************************************************************/
INT8U system_voltage_is_usful(void)
{
    INT16U volt = g_systemVoltageValue;

    if(volt < BMS_SYSTEM_VOLTAGE_MIN)
    {
        return FALSE;
    }
    if(volt > BMS_SYSTEM_VOLTAGE_MAX)
    {
        return FALSE;
    }

    return TRUE;
}


/***********************************************************************
  * @brief           �ж�ϵͳ��ѹ�Ƿ���Ч
  * @param[in]       ��  
  * @return          1��ok  0��err
***********************************************************************/
INT8U system_voltage_is_valid(void)
{
    INT16U volt = g_systemVoltageValue;

    if(volt < BMS_SYSTEM_VOLTAGE_LOW_THRESHOLD)
    {
        return FALSE;
    }
    
    return TRUE;
}

/***********************************************************************
  * @brief           ��ȡϵͳ�����ѹֵ��С
  * @param[in]       *volt ���ص�ѹֵ 
  * @return          1��ok  0��err
***********************************************************************/
INT8U bcu_calc_battery_v_voltage(INT16U *volt)
{
    INT16U sum = 0;
    INT8U flag = 0;
    
    if(volt == NULL) return 0;
          
    if (!adc_onchip_read(&adc_onchip_battery_v, volt)) {
        return 0;
    }
    *volt += BCU_BATTERY_V_VOLT_OFFSET;
    
    //g_systemLeadAcidValue = (INT16U)*volt;
    g_systemLeadAcidValue = *volt;
    
    return 1;
}

/***********************************************************************
  * @brief           ��ȡ24V�����ѹֵ��С
  * @param[in]       ��
  * @return          ���ص�ѹֵ
***********************************************************************/
inline INT16U dc_24V_voltage_get(void)
{
    return g_system24vDCValue;
}


/***********************************************************************
  * @brief           �ж�24v��ѹֵ�Ƿ���Ч
  * @param[in]       ��
  * @return          1��ok  0��err
***********************************************************************/
INT8U dc_24V_voltage_is_usful(void)
{
    INT16U volt = g_system24vDCValue;

    if(volt < BMS_24V_DC_VOLTAGE_MIN)
    {
        return FALSE;
    }
    if(volt > BMS_24V_DC_VOLTAGE_MAX)
    {
        return FALSE;
    }

    return TRUE;
}   

/***********************************************************************
  * @brief           �ж�24v��ѹֵ�Ƿ���Ч
  * @param[in]       ��
  * @return          1��ok  0��err
***********************************************************************/
INT8U dc_24V_voltage_is_valid(void)
{
    INT16U volt = g_system24vDCValue;

    if(volt < BMS_24V_DC_VOLTAGE_LOW_THRESHOLD)
    {
        return FALSE;
    }
    
    return TRUE;
}

/***********************************************************************
  * @brief           ���ֱ����ѹ���
  * @param[in]       *volt  ���ص�ѹֵ��ַ
  * @return          1��ok  0��err
***********************************************************************/
INT8U bcu_calc_24V_dc_voltage(INT16U *volt)
{
    INT16U sum = 0;
   // INT8U flag = 0;
    
//    g_system_voltage_ad.pdata = &flag;
        
    if(volt == NULL) return 0;
            
    if (!adc_onchip_read(g_chgdc_voltage_ad_param, volt)) {
        return 0;
    }
    
    g_system24vDCValue = (INT16U)*volt;  
    return 1;
}


/** 5V_HALL_DET ��ѹ���, Range: 5000 +/- 50 mv (1%) */
/***********************************************************************
  * @brief           ����5Vϵͳ��ѹֵ
  * @param[in]       ��
  * @return          ��ѹֵ
***********************************************************************/
inline INT16U hall_5V_voltage_get(void)
{
    return g_system5vHallDetValue;
}

/***********************************************************************
  * @brief           �ж�5Vϵͳ��ѹ�Ƿ���Ч
  * @param[in]       ��
  * @return          1��ok  0��err
***********************************************************************/
INT8U hall_5V_voltage_is_usful(void)
{
    INT16U volt = g_system5vHallDetValue;

    if((volt < 5000 - 50) || (volt > 5000 + 50))
    {
        return FALSE;
    }

    return TRUE;
}

/***********************************************************************
  * @brief           ��ȡ5Vϵͳ��ѹֵ
  * @param[in]       *volt ���ص�ѹֵ��ַ
  * @return          1��ok  0��err
***********************************************************************/
INT8U bcu_calc_hall_5V_voltage(INT16U *volt)
{
    INT16U sum = 0;
   // INT8U flag = 0;
    
    if(volt == NULL) return 0;
    
  //  g_system_voltage_ad.pdata = &flag; 
    if (!adc_onchip_read(&adc_onchip_5vhall_adc_v116, volt)) {
        return 0;
    }
    
    g_system5vHallDetValue = (INT16U)*volt;
    
    return 1;
}


/** 8V_HALL_DET ��ѹ��� , Range: 8000 +/- 160 mv (2%)*/
/***********************************************************************
  * @brief           ����8Vϵͳ��ѹֵ
  * @param[in]       ��
  * @return          ��ѹֵ
***********************************************************************/
inline INT16U hall_8V_voltage_get(void)
{
    return g_system8vHallDetValue;
}


/***********************************************************************
  * @brief           �ж�8Vϵͳ��ѹ�Ƿ���Ч
  * @param[in]       ��
  * @return          1��ok  0��err
***********************************************************************/
INT8U hall_8V_voltage_is_usful(void)
{
    INT16U volt = g_system8vHallDetValue;

    if((volt < 8000 - 160) || (volt > 8000 + 160))
    {
        return FALSE;
    }

    return TRUE;
}


/***********************************************************************
  * @brief           ��ȡ8Vϵͳ��ѹֵ
  * @param[in]       *volt ���ص�ѹֵ��ַ
  * @return          1��ok  0��err
***********************************************************************/
INT8U bcu_calc_hall_8V_voltage(INT16U *volt)
{
    INT16U sum = 0;
   // INT8U flag = 0;
    
  //  g_system_voltage_ad.pdata = &flag;
        
    if(volt == NULL) return 0;
            
    if (!adc_onchip_read(&adc_onchip_8vhall_adc_v116, volt)) {
        return 0;
    }
    
    g_system8vHallDetValue = (INT16U)*volt;    
    return 1;
}

/** 5V_DET ��ѹ��� , Range: 2500 +/- 25 mv (1%) */
/***********************************************************************
  * @brief           ����2.5V��׼��ѹֵ
  * @param[in]       ��
  * @return          ��ѹֵ
***********************************************************************/
inline INT16U det_5V_voltage_get(void)
{
    return g_system5vDetValue;
}

/***********************************************************************
  * @brief           �ж�2.5V��׼��ѹ�Ƿ���Ч
  * @param[in]       ��
  * @return          1��ok  0��err
***********************************************************************/
INT8U det_5V_voltage_is_usful(void)
{
    INT16U volt = g_system5vDetValue;

    if((volt < 2500 - 25) || (volt > 2500 + 25))
    {
        return FALSE;
    }

    return TRUE;
}

/***********************************************************************
  * @brief           ��ȡ2.5V��׼��ѹֵ
  * @param[in]       *volt ���ص�ѹֵ��ַ
  * @return          1��ok  0��err
***********************************************************************/
INT8U bcu_calc_det_5V_voltage(INT16U *volt)
{
    INT16U sum = 0;
  //  INT8U flag = 0;
    
   // g_system_voltage_ad.pdata = &flag;
        
    if(volt == NULL) return 0;
            
    if (!adc_onchip_read(&adc_onchip_5vdet_adc_v116, volt)) {
        return 0;
    }
    
    g_system5vDetValue = (INT16U)*volt;    
    return 1;
}

/** BATTERY_DET ��ѹ��� , Range: (9000~36000)/21 +/- 2% */
/***********************************************************************
  * @brief           ����2.5V��׼��ѹֵ
  * @param[in]       ��
  * @return          ��ѹֵ
***********************************************************************/
inline INT16U bat_det_voltage_get(void)
{
    INT16U volt = 0;
    OS_INIT_CRITICAL();
    OS_ENTER_CRITICAL();
    volt = g_systemBatDetValue; 
    OS_EXIT_CRITICAL();
    return volt;
}

/***********************************************************************
  * @brief           �ж����ص�ѹ�Ƿ���Ч
  * @param[in]       ��
  * @return          1��ok  0��err
***********************************************************************/
INT8U bat_det_voltage_is_usful(void)
{
    INT16U  volt = bat_det_voltage_get();

    if((volt < BMS_BAT_DET_VOLTAGE_MIN) || (volt > BMS_BAT_DET_VOLTAGE_MAX))
    {
        return FALSE;
    }
    
    return TRUE;
}

/***********************************************************************
  * @brief           ��ȡ���ص�ѹֵ
  * @param[in]       *volt ���ص�ѹֵ��ַ
  * @return          1��ok  0��err
***********************************************************************/
INT8U bcu_calc_bat_det_voltage(INT16U *volt)
{
    INT16U sum = 0;
  //  INT8U flag = 0;
    OS_INIT_CRITICAL();
  //  g_system_voltage_ad.pdata = &flag;
        
    if(volt == NULL) return 0;
            
    if (!adc_onchip_read(&adc_onchip_vbat_det_adc_v116, volt)) {
        return 0;
    }
    OS_ENTER_CRITICAL();
    g_systemBatDetValue = (INT16U)*volt; 
    OS_EXIT_CRITICAL();   
    return 1;
}

/***********************************************************************
  * @brief           ��ȡϵͳ��ѹֵ
  * @param[in]       ��
  * @return          1��ok  0��err
***********************************************************************/
INT8U bcu_calc_ext_voltage(void)
{
    INT8U   res     =   0;
    INT16U  volt_t1 =   0;

#if BMS_SUPPORT_BY5248D == 0
#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    HW_VER_NUM ver  = hardware_io_revision_get();
    
    if(ver == HW_VER_116)
    {
        res |= bcu_calc_det_5V_voltage(&volt_t1);
        res |= bcu_calc_hall_5V_voltage(&volt_t1);
        res |= bcu_calc_hall_8V_voltage(&volt_t1);
        res |= bcu_calc_bat_det_voltage(&volt_t1);
    }
    else if(ver == HW_VER_120)
    {
        res |= bcu_calc_det_5V_voltage(&volt_t1);
        res |= bcu_calc_hall_5V_voltage(&volt_t1);
        //res |= bcu_calc_hall_8V_voltage(&volt_t1); //V120 removed 8V_hall
        res |= bcu_calc_bat_det_voltage(&volt_t1);
    }
    else
    {
        g_system5vHallDetValue    = 5000;
        g_system8vHallDetValue    = 8000;
        g_system5vDetValue        = 2500;
        g_systemBatDetValue       = 24000;
    }
#else
    res |= bcu_calc_det_5V_voltage(&volt_t1);
    res |= bcu_calc_hall_5V_voltage(&volt_t1);
    //res |= bcu_calc_hall_8V_voltage(&volt_t1); //V120 removed 8V_hall
    res |= bcu_calc_bat_det_voltage(&volt_t1);
#endif     
  
#else
    res |= bcu_calc_bat_det_voltage(&volt_t1);
#endif
       
    return res;
}

/***********************************************************************
  * @brief           ��ȡϵͳ�����ѹֵ
  * @param[in]       *__far volt  ϵͳ�����ѹ��ַ
  * @return          1��ok  0��err
***********************************************************************/
INT8U bcu_calc_system_voltage(INT16U *__far volt)   //liqing 20170428
{
    INT8U res=0,i = 0;
    INT16U volt_t1=0, volt_t2;
    
    res |= bcu_calc_battery_v_voltage(&volt_t1);
    res |= bcu_calc_24V_dc_voltage(&volt_t2);
    if(volt_t2 >= BMS_SYSTEM_VOLTAGE_MIN && volt_t2 <= BMS_SYSTEM_VOLTAGE_MAX)  //24v �Ƿ�����Ҫ��
    {
        if(volt_t1 >= BMS_SYSTEM_VOLTAGE_MIN && volt_t1 <= BMS_SYSTEM_VOLTAGE_MAX)
        {
            if(volt_t1 >= volt_t2) *volt = volt_t1;
            else *volt = volt_t2;
        }
        else
        {
            *volt = volt_t2;
        }
    }
    else
    {
        *volt = volt_t1;
    }
    res |= bcu_calc_ext_voltage();
    
    //Adc_StartConversion (0,15);//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    
    return res;
}

/***********************************************************************
  * @brief           ��ȡϵͳ�����ѹֵ
  * @param[in]       *data  δʹ��
  * @return          ��
***********************************************************************/
void bcu_system_voltage_detect(void* data)//ϵͳ�����ѹ���
{
    INT16U volt;
    
    UNUSED(data); 
    if(bcu_calc_system_voltage(&volt))    
    {
        g_systemVoltageValue = volt;
    }
}

/***********************************************************************
  * @brief           �������صĵ�ѹֵ
  * @param[in]       ��
  * @return          ��ѹֵ
***********************************************************************/
inline INT16U bcu_get_lead_acid_volt(void)
{
    return g_systemLeadAcidValue;
}

#endif

