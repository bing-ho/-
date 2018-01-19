/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   hcf4052_intermediate.c                                       

** @brief         ���RTCģ��ʱ�ӽӿں������η�װ           
                
** @copyright   	Ligoo Inc.         
** @date		    2017-04-21.
** @author          �� ��  
*******************************************************************************/ 


#include "bms_clock.h"   
#include "second_datetime.h"
#include "bms_defs.h"
#include "bms_stat.h"
#include "ucos_ii.h"
#include "iic_interface.h" 
#include "bms_bcu.h"
#include "rtc_hardware.h"



#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM


struct pcf8563 __pcf8563_impl = 
{
#if BMS_SUPPORT_CURRENT_DIVIDER 
    &hcs12_i2c_bus_0,
#else
    &hcs12_i2c_bus_1,
#endif
};
const   rtc_dev  *__FAR prtc_dev = NULL;
#pragma pop




const rtc_dev pcf8563_dev =
{
    &pcf8563_oops,
    &__pcf8563_impl
};

Result clock_aquire_resource(INT16U t) {
    (void)t;
    return RES_OK;     
}

void clock_release_resource(void) {
    //OSMutexPost(rtc_mutex);
}

/***********************************************************************
  * @brief           ʱ�ӳ�ʼ��
  * @param[in]       �� 
  * @return          ��
***********************************************************************/
void clock_init(void)
{
#if BMS_SUPPORT_BY5248D == 0
#if BMS_SUPPORT_HARDWARE_BEFORE == 1
    HW_VER_NUM ver;
#endif
#endif
    if(prtc_dev != NULL)
    {
        return; //already inited
    }
#if BMS_SUPPORT_BY5248D == 0

#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    ver = hardware_io_revision_get();
    if(ver == HW_VER_120)
    {
        prtc_dev = &pcf8563_dev;
    }
        else  prtc_dev = &pcf8563_dev;
	#else
        prtc_dev = &pcf8563_dev;
    #endif 
  
#else
    prtc_dev = &pcf8563_dev;
#endif    
    (void)prtc_dev->prtc_ops->rtc_init(prtc_dev->chip);      
}

void clock_uninit(void)
{
    //INT8U err;
    //rtc_mutex = OSMutexDel(rtc_mutex, OS_DEL_ALWAYS, &err);
}

/***********************************************************************
  * @brief           ��ȡʱ��ʱ��
  * @param[in]       seconds��ʱ�� 
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
Result clock_get_seconds(unsigned long *seconds) 
{
    if(prtc_dev == NULL)
    {
        return RES_ERR;
    }
    return prtc_dev->prtc_ops->rtc_read_time(prtc_dev->chip, seconds) ? RES_OK : RES_ERR;
}

/***********************************************************************
  * @brief           ����ʱ���ж�
  * @param[in]       rtc_interrupt_type���ж�����
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
Result clock_config_interrupt(rtc_interrupt_type int_type, rtc_interrupt_single int_type_single) 
{
    if(prtc_dev == NULL)
    {
        return RES_ERR;
    }
    return prtc_dev->prtc_ops->rtc_config_interrupt(prtc_dev->chip, int_type, int_type_single) ? RES_OK : RES_ERR;
}

/***********************************************************************
  * @brief           ��ȡ����ʱ��
  * @param[in]       *seconds��ʱ��ָ��
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
Result clock_get_alarm_time(unsigned long *seconds) 
{
    if(prtc_dev == NULL)
    {
        return RES_ERR;
    }
    return prtc_dev->prtc_ops->rtc_read_time(prtc_dev->chip, seconds) ? RES_OK : RES_ERR;
}

/***********************************************************************
  * @brief           ��������
  * @param[in]       seconds��ʱ��
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
Result clock_set_alarm( uint32_t seconds) 
{
    if(prtc_dev == NULL)
    {
        return RES_ERR;
    }
    return prtc_dev->prtc_ops->rtc_set_alarm(prtc_dev->chip, seconds) ? RES_OK : RES_ERR;
}

/***********************************************************************
  * @brief           ���ʱ���ж�
  * @param[in]       ��
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
Result clock_clear_interrupt(void) 
{
    if(prtc_dev == NULL)
    {
        return RES_ERR;
    }
    return prtc_dev->prtc_ops->rtc_clear_interrupt(prtc_dev->chip) ? RES_OK : RES_ERR;
}

/***********************************************************************
  * @brief           �洢���ݣ�Ԥ�������ݲ�ͬRTCоƬ)
  * @param[in]       
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
Result clock_store_data(const unsigned char *__FAR dat,
                        unsigned char offset,
                        unsigned char len) 
{
    if(prtc_dev == NULL)
    {
        return RES_ERR;
    }
    return prtc_dev->prtc_ops->rtc_store_data(prtc_dev->chip, dat, offset, len) ? RES_OK : RES_ERR;    
}
/***********************************************************************
  * @brief           �洢���ݣ�Ԥ�������ݲ�ͬRTCоƬ)
  * @param[in]       
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
Result clock_restore_data(unsigned char *__FAR dat,
                            unsigned char offset,
                            unsigned char len) 
{
    if(prtc_dev == NULL)
    {
        return RES_ERR;
    }
    return prtc_dev->prtc_ops->rtc_restore_data(prtc_dev->chip, dat, offset, len) ? RES_OK : RES_ERR;   
}

/***********************************************************************
  * @brief           ����ʱ��ʱ��
  * @param[in]       DateTime�ṹ��ָ��
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
Result clock_set(DateTime* time)
{
    unsigned long seconds;
    INT8U rc;

    if(prtc_dev == NULL)
    {
        return RES_ERR;
    }
    if (!datetime_to_second(&seconds,time)) return RES_INVALID_ARG;
    
    if (RES_OK != clock_aquire_resource(1000)) {
        return RES_ERR;
    }
    
    rc = prtc_dev->prtc_ops->rtc_write_time(prtc_dev->chip, seconds);
    clock_release_resource();
    
    if (rc) 
    {             
        g_bcu_system_time_upate_flag = 1; // ����ϵͳʱ�����
        return RES_OK;
    }
    
    return RES_ERR;
}
/***********************************************************************
  * @brief           ��ȡʱ������
  * @param[in]       DateTime�ṹ��ָ��
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
Result clock_get(DateTime* time)
{
    unsigned long seconds;
    Result rc;
    
    if (RES_OK != clock_aquire_resource(1000)) {
        return RES_ERR;
    }
    
    rc = clock_get_seconds(&seconds);
    clock_release_resource();
    
    if (RES_ERR == rc) {
        return RES_ERR;
    }
    
    if (second_to_datetime(time, seconds)) {    
        return RES_OK;
    }
    return RES_ERR;
}

Result clock_set_item(DateTimeFlag flag, ubyte value)
{
    DateTime time;
    
    if (RES_OK != clock_get(&time)) {
        return RES_ERR;
    }

    if (flag == kDateTimeYearBit) {
        time.year = value;
    } else if (flag == kDateTimeMonthBit) {
        time.month = value;
    } else if (flag == kDateTimeDayBit) {
        time.day = value;
    } else if (flag == kDateTimeHourBit) {
        time.hour = value;
    } else if (flag == kDateTimeMinuteBit) {
        time.minute = value;
    } else if (flag == kDateTimeSecondBit) {
        time.second = value;
    }
    
    return clock_set(&time);
}

ubyte clock_get_item(DateTimeFlag flag) {
    DateTime time;
    
    if (RES_OK != clock_get(&time)) {
        return 0xff;
    }
    
    if (flag & (1<<kDateTimeYearBit)) {
        return time.year;
    }
    
    if (flag & (1<<kDateTimeMonthBit)) {
        return time.month;
    }
    if (flag & (1<<kDateTimeDayBit)) {
        return time.day;
    }
    if (flag & (1<<kDateTimeHourBit)) {
        return time.hour;
    }
    if (flag & (1<<kDateTimeMinuteBit)) {
        return time.minute;
    }
    if (flag & (1<<kDateTimeSecondBit)) {
        return time.second;
    }
    
    return 0xff;

}

long clock_to_time_t(DateTime* time)
{
    unsigned long seconds;
    if (!datetime_to_second(&seconds,time)) {
        return 0;
    }
    return seconds;
}
/***********************************************************************
  * @brief           ʱ��������֮��ת��
  * @param[in]       second��ʱ�� DateTime�ṹ��ָ��
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
void clock_from_time_t(unsigned long second, DateTime* time)
{

    (void)second_to_datetime(time, second);
}

uint16_t clock_get_ram_size_bits() 
{
    if(prtc_dev == NULL)
    {
        return RES_ERR;
    }
    return prtc_dev->prtc_ops->rtc_get_ram_size_bit(prtc_dev->chip);
}

