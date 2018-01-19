/**
*
* Copyright (c) 2011 Ligoo Inc. 
* 
* @file  BmsMain.c
* @brief 电池分析,及参数更新,控制 
* @note 
* @version 1.00
* @author 
* @date 2011/11/30  
* @正负极双MOS开关测试
*/

#include "bms_insulation_impl.h"
#include "adc_xgate.h"
#include "adc0_intermediate.h"
#include "hcf4052_intermediate.h"
#include "app_cfg.h"

#pragma MESSAGE DISABLE C1420
#pragma MESSAGE DISABLE C2705
#pragma MESSAGE DISABLE C5905
#pragma MESSAGE DISABLE C5917
#pragma MESSAGE DISABLE C5919

#if BMS_SUPPORT_INSULATION_DIAGNOSE  

OS_STK 	g_res_scan_task_stack[RES_SAMPLE_TASK_STK_SIZE] = {0};	        //绝缘电阻采集任务堆栈
OS_STK 	g_volt_scan_task_stack[VOLT_SAMPLE_TASK_STK_SIZE] = {0};	    //绝缘总压采集任务堆栈

/** test for mos delay */

#define SAMPLE_MOS_ON_DELAY()       sleep(config_get(kEepromMosOnDelayIndex));

#if BMS_SUPPORT_BY5248D == 1
  #if BMS_SUPPORT_BY5248D_1_1A
    #define POS_FEN_YA_RES              600
    #define POS_CAI_YANG_RES            4.0  
  #else
    #define POS_FEN_YA_RES              255.0
    #define POS_CAI_YANG_RES            4.0
  #endif    
#else
   #define POS_FEN_YA_RES              429.0
   #define POS_CAI_YANG_RES            0.392
#endif




#define SYS_LEAK_RES_MAX            5     //5kOhm
#define TOTAL_VOLT_MIN              360
#define TOTAL_VOLT_INSU_MIN              5
#define TOTAL_VOLT_DIFF_VALID       50

/** test end */

#define TOTAL_VOLT_IS_INVALID()   (g_insu_info.bat_volt < TOTAL_VOLT_MIN)
#define ISO_ALARM_RES             (g_insu_info.bat_volt * 1/2)

#define BAT_TOTAL_VOLT_FACT       g_insu_info.bat_volt

#define BAT_VOLT_LEVEL_NUM        6

#define INSU_XADC0_RETRY_NUM      8
#define INSU_XADC0_RETRY_DELAY      15


static INT8U   g_need_revert = 0; //V1.20 need revert
const struct hcs12_gpio_detail insu_mos_ctrl[] = {
    //v120 renamed to CTRL_HV+_MCU
    { "CTRL_HV+",     { &PTIS,  &PTS, &DDRS, 1 << 1}, 0 , 0},
    //v120 renamed to CTRL_HV-_MCU
    { "CTRL_HV-",     { &PTIS,  &PTS, &DDRS, 1 << 0}, 0 , 0},
#if BMS_SUPPORT_NEGATIVE_ADHESION
    { "CTRL_B2-",     { &PORTC, &PORTC, &DDRC, 1 << 3 }, 0, 0 },
#endif
    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0}, 0, 0},
};
/*MOS开关控制*/

#define PositiveMosOn()  hcs12_gpio_set_output(&insu_mos_ctrl[0].io, 0 ^ g_need_revert)
#define PositiveMosOff() hcs12_gpio_set_output(&insu_mos_ctrl[0].io, 1 ^ g_need_revert)
#define NegativeMosOn()  hcs12_gpio_set_output(&insu_mos_ctrl[1].io, 0 ^ g_need_revert)
#define NegativeMosOff() hcs12_gpio_set_output(&insu_mos_ctrl[1].io, 1 ^ g_need_revert)

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_INSULATION
InsulationInfo g_insu_info;                             //绝缘信息结构体
INT16U g_adc_buff_positive[AD_CNT];                      //绝缘采样AD缓存
INT16U g_adc_buff_negative[AD_CNT];                      //绝缘采样AD缓存
INT16S g_adc_buff_extend[EXTEND_TV_AD_CNT];           //绝缘扩展总压采样AD缓存

FP64 g_insu_volt_positive[2];
FP64 g_insu_volt_negative[2];

INT16U g_insu_positive_origin_res = 0;               //上一次原始绝缘正极阻值，未滤波
INT16U g_insu_negative_origin_res = 0;               //上一次原始绝缘负极阻值，未滤波
INT16U g_insu_positive_filter_buff[RES_FILTER_BUFF_NUM]={0};//正极绝缘阻值滤波缓存
INT8U g_insu_positive_filter_buff_index=0;                  //正极绝缘阻值滤波索引
INT8U g_insu_positive_filter_buff_cnt=0;                    //正极绝缘阻值滤波计数
INT16U g_insu_negative_filter_buff[RES_FILTER_BUFF_NUM]={0};//负极绝缘阻值滤波缓存
INT8U g_insu_negative_filter_buff_index=0;                  //负极绝缘阻值滤波索引
INT8U g_insu_negative_filter_buff_cnt=0;                    //负极绝缘阻值滤波计数

INT16U g_insu_alarm_res_positive_buff[ALARM_RES_BUFF_NUM]={0};      //最新n次正极绝缘阻值缓存
INT8U g_insu_alarm_res_positive_buff_cnt=0;                          //最新n次正极绝缘阻值缓存计数
INT8U g_insu_alarm_res_positive_buff_index=0;                        //最新n次正极绝缘阻值缓存索引
INT16U g_insu_alarm_res_negative_buff[ALARM_RES_BUFF_NUM]={0};      //最新n次负极绝缘阻值缓存
INT8U g_insu_alarm_res_negative_buff_cnt=0;                          //最新n次负极绝缘阻值缓存计数
INT8U g_insu_alarm_res_negative_buff_index=0;                        //最新n次负极绝缘阻值缓存索引

const INT16U g_isoBatVoltBuff[BAT_VOLT_LEVEL_NUM]=
{
0, 1000, 2000, 3000, 5000, 30000
};

const float g_insu_res_factor_arr[BAT_VOLT_LEVEL_NUM-1] =
{
//37, 19.6, 13.6, 8.21, 5
37.1, 20.7, 13, 8.2, 5,
};
#if BMS_SUPPORT_BY5248D == 1
    float g_insu_res_gain_factor = 1;
#else
float g_insu_res_gain_factor = 5;
#endif

#pragma DATA_SEG DEFAULT
void insu_res_analyze_task(void *pdata);            //绝缘电阻采集任务
void insu_volt_analyze_task(void *pdata);           //绝缘电压采集任务
void reset_mos_status(void);//复位正负极开关
void reset_insu_para(void); //复位绝缘信息
void insu_calc_insu_res(void);
void insu_calc_insu_res_with_mos(void);
INT8U insu_total_volt_is_valid(void);
INT16U insu_calc_total_volt(FP64 hvp, FP64 hvn);
INT16U insu_get_res_calc_cycle(void);

void insu_gain_res_with_voltage(void);
void update_system_insu(void);
void all_mos_on(void) ;

INT16U insu_calc_pre_volt(void);
INT16U insu_calc_pch_volt(void);
INT16U insu_calc_bat_hv_volt(void);
INT16U insu_calc_hv3_hv_volt(void);
float insu_get_leak_res_min(void);

/*
* @brief        ADC采样任务初始化
* @note         无
* @param[in]    无
* @param[out]   无
* @return       无
*
*/ 
void insu_res_analyze_init(void) 
{
    uint8_t i;    

#if BMS_SUPPORT_BY5248D == 0         
#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    if(hardware_io_revision_get() == HW_VER_120)
    {
        g_need_revert = 1;
    }
    else
    {
        g_need_revert = 0;
    }
#else
    g_need_revert = 1;
#endif    
#else
    if(hardware_io_revision_get() == HW_VER_BYU_110)  //江铃项目 CTRL_HV MOS低 ON
    {
        g_need_revert = 0;
    }
    else 
    {
        g_need_revert = 1;    
    }
#endif    
   
    for (i = 0; ; ++i) 
    {
        if(insu_mos_ctrl[i].name == '\0')
        {
            break;
        }
        (void)hcs12_gpio_init(&insu_mos_ctrl[i].io, GPIO_MODE_OUTPUT_PUSHPULL);
        (void)hcs12_gpio_set_output(&insu_mos_ctrl[i].io, insu_mos_ctrl[i].def_val ^ g_need_revert);
    }

  
    hcf4052_impl_init();

    reset_mos_status();
    reset_insu_para();
    
    if(!(insu_type_is_on_board() || insu_type_is_both())) return;
    
    
    (void) OSTaskCreate(insu_res_analyze_task,                         //绝缘电阻采集任务
                            (void *)0,
                            (OS_STK *)&g_res_scan_task_stack[RES_SAMPLE_TASK_STK_SIZE - 1],
                            RES_SCAN_TASK_PRIO);
                            
#if 1                           
    ADS1015_Init(&g_ADS1015_BspInterface);
    
    (void) OSTaskCreate(insu_volt_analyze_task,                         //绝缘电压采集任务
                            (void *)0,
                            (OS_STK *)&g_volt_scan_task_stack[VOLT_SAMPLE_TASK_STK_SIZE - 1],
                            VOLT_SCAN_TASK_PRIO);
#endif 
    
//江铃100~200V 第一次总压采集前放大5倍 
    hcf4052_select_channel(hcf4052_impl, HCF4052_CHANNEL_NONE);
}



/*
*
* @brief       绝缘阻值采集任务
* @note         无
* @param[in]    参数指针  
* @param[out]   无
* @return       无
*
*/    
void  insu_res_analyze_task(void *pdata)
{
    INT32U calc_tv_last_tick, calc_res_last_tick, now_tick, calc_res_interval;
    INT8U power_on_calc_res_flag = 1; //上电绝缘阻值检测标志位
    
    pdata = pdata;
    
    now_tick = get_tick_count();
    calc_tv_last_tick = now_tick;
    calc_res_last_tick = now_tick;
    
    for(;;)
    {   
        OSTimeDly(50);
        //计算总压
        if(charger_is_connected() != 0) 
        {
            if(config_get(kEepromInsuResSwitchInChgIndex) == 0)
            {
                reset_insu_para();
                continue;
            }
        }
        
        now_tick = get_tick_count();
        
        //计算绝缘阻值
        if(config_get(kEepromInsuResCalibFlagIndex) == 0 || relay_control_is_on(kRelayTypePreCharging) == 1) //预充时停止绝缘阻值检测
        {
            reset_insu_para();
            continue;
        }
        if(power_on_calc_res_flag) //上电绝缘阻值检测延时
        {
            calc_res_interval = POWER_ON_CALC_RES_DELAY;
        }
        else
            calc_res_interval = insu_get_res_calc_cycle();
        
        
        if(get_interval_by_tick(calc_res_last_tick, now_tick) > calc_res_interval)
        {
            power_on_calc_res_flag = 0;
            calc_res_last_tick = now_tick;
            if(config_get(kEepromMosOnDelayIndex) == 0) //无延时认为MOS常闭
                insu_calc_insu_res();
            else
                insu_calc_insu_res_with_mos();
        }
        
        bcu_set_positive_insulation_resistance(g_insu_info.positive_res);
        bcu_set_negative_insulation_resistance(g_insu_info.negative_res);
        bcu_set_insulation_work_state(g_insu_info.insu_status);
    }
}

/*
*
* @brief       绝缘总压采集任务
* @note         无
* @param[in]    参数指针  
* @param[out]   无
* @return       无
*
*/    
void  insu_volt_analyze_task(void *pdata)
{
    pdata = pdata;
    
    for(;;)
    {
        OSTimeDly(50);
        //计算总压
        if(charger_is_connected() != 0)
        {
            if(config_get(kEepromInsuResSwitchInChgIndex) == 0)
            {
                continue;
            }
        }
        
        g_insu_info.pre_volt = insu_calc_pre_volt();
        g_insu_info.pch_volt = insu_calc_pch_volt();
        g_insu_info.bat_volt = insu_calc_bat_hv_volt();
        
    #if BMS_SUPPORT_NEGATIVE_ADHESION
        if(hcs12_gpio_output_is_high(&insu_mos_ctrl[2].io))    //光耦是否闭合
         {
            g_insu_info.hv3_volt = insu_calc_hv3_hv_volt();
         } 
        else 
         {
            g_insu_info.hv3_volt = 0;
         }
    #else
        g_insu_info.hv3_volt = insu_calc_hv3_hv_volt();
        
    #endif

    #if BMS_SUPPORT_BY5248D == 0   
        insu_gain_res_with_voltage();
    #endif    
        if(g_insu_info.insu_status == 0x01)
          bcu_set_insulation_total_voltage(0);
        else
          bcu_set_insulation_total_voltage((INT32U)g_insu_info.bat_volt*100);
        
        bcu_set_insulation_pre_total_voltage((INT32U)g_insu_info.pre_volt*100);
        bcu_set_insulation_pch_total_voltage((INT32U)g_insu_info.pch_volt*100);
        bcu_set_insulation_hv3_total_voltage((INT32U)g_insu_info.hv3_volt*100);

    }
    

}

void all_mos_on(void) 
{
    PositiveMosOn();//总正极开关
    NegativeMosOn();//总负极开关
}

void reset_mos_status(void)
{
    PositiveMosOff();//总正极开关
    NegativeMosOff();//总负极开关
}

void reset_insu_para(void)
{
    g_insu_info.insu_status = 0x00;
    g_insu_info.positive_res=65535;
    g_insu_info.negative_res=65535;
    reset_mos_status();
    
    bcu_set_positive_insulation_resistance(g_insu_info.positive_res);
    bcu_set_negative_insulation_resistance(g_insu_info.negative_res);
    bcu_set_insulation_work_state(g_insu_info.insu_status);
}

void reset_ext_insu_para(void)
{
    g_insu_info.ext_insu_status = 0x00;
    g_insu_info.ext_positive_res=65535;
    g_insu_info.ext_negative_res=65535;
    
    bcu_set_ext_positive_insulation_resistance(g_insu_info.ext_positive_res);
    bcu_set_ext_negative_insulation_resistance(g_insu_info.ext_negative_res);
    bcu_set_ext_insulation_work_state(g_insu_info.ext_insu_status);
}

INT16U insu_positive_res_filter(INT16U data)
{
    INT8U i;
    INT32U temp=0;
    
    g_insu_positive_origin_res = data;
    temp = g_insu_info.positive_res;
    if(data < ISO_ALARM_RES) //异常阻值
    {
        //复位正常阻值缓存
        for(i=0; i<RES_FILTER_BUFF_NUM; i++) g_insu_positive_filter_buff[i] = 0xFFFF;
        g_insu_positive_filter_buff_cnt = 0;
        g_insu_positive_filter_buff_index = 0;
        //滤波
        if(g_insu_alarm_res_positive_buff_index > ALARM_RES_BUFF_NUM-1)
            g_insu_alarm_res_positive_buff_index = 0;
        g_insu_alarm_res_positive_buff[g_insu_alarm_res_positive_buff_index++]=data;
        if(g_insu_alarm_res_positive_buff_cnt<ALARM_RES_BUFF_NUM)
            g_insu_alarm_res_positive_buff_cnt++;
        else
        {
            temp = 0;
            for(i=0;i<ALARM_RES_BUFF_NUM;i++)
                temp += g_insu_alarm_res_positive_buff[i];
            temp /= ALARM_RES_BUFF_NUM;
        }
    }
    else //正常阻值
    {
        //复位异常阻值缓存
        for(i=0; i<ALARM_RES_BUFF_NUM; i++) g_insu_alarm_res_positive_buff[i] = 0xFFFF;
        g_insu_alarm_res_positive_buff_cnt = 0;
        g_insu_alarm_res_positive_buff_index = 0;
        //滤波
        if(g_insu_positive_filter_buff_index > RES_FILTER_BUFF_NUM-1)
            g_insu_positive_filter_buff_index=0;
        g_insu_positive_filter_buff[g_insu_positive_filter_buff_index++]=data;
        if(g_insu_positive_filter_buff_cnt<RES_FILTER_BUFF_NUM)
            g_insu_positive_filter_buff_cnt++;
        
        temp = 0;
        for(i=0;i<g_insu_positive_filter_buff_cnt;i++)
            temp += g_insu_positive_filter_buff[i];
        temp /= g_insu_positive_filter_buff_cnt;
    }
    return temp;
}
INT16U insu_negative_res_filter(INT16U data)
{
    INT8U i;
    INT32U temp=0;
    
    g_insu_negative_origin_res = data;
    temp = g_insu_info.negative_res;
    if(data < ISO_ALARM_RES) //异常阻值
    {
        //复位正常阻值缓存
        for(i=0; i<RES_FILTER_BUFF_NUM; i++) g_insu_negative_filter_buff[i] = 0xFFFF;
        g_insu_negative_filter_buff_cnt = 0;
        g_insu_negative_filter_buff_index = 0;
        //滤波
        if(g_insu_alarm_res_negative_buff_index > ALARM_RES_BUFF_NUM-1)
            g_insu_alarm_res_negative_buff_index = 0;
        g_insu_alarm_res_negative_buff[g_insu_alarm_res_negative_buff_index++]=data;
        if(g_insu_alarm_res_negative_buff_cnt<ALARM_RES_BUFF_NUM)
            g_insu_alarm_res_negative_buff_cnt++;
        else
        {
            temp = 0;
            for(i=0;i<ALARM_RES_BUFF_NUM;i++)
                temp += g_insu_alarm_res_negative_buff[i];
            temp /= ALARM_RES_BUFF_NUM;
        }
    }
    else //正常阻值
    {
        //复位异常缓存
        for(i=0; i<ALARM_RES_BUFF_NUM; i++) g_insu_alarm_res_negative_buff[i] = 0xFFFF;
        g_insu_alarm_res_negative_buff_cnt = 0;
        g_insu_alarm_res_negative_buff_index = 0;
        //滤波
        if(g_insu_negative_filter_buff_index > RES_FILTER_BUFF_NUM-1)
            g_insu_negative_filter_buff_index=0;
        g_insu_negative_filter_buff[g_insu_negative_filter_buff_index++]=data;
        if(g_insu_negative_filter_buff_cnt<RES_FILTER_BUFF_NUM)
            g_insu_negative_filter_buff_cnt++;
        
        temp = 0;
        for(i=0;i<g_insu_negative_filter_buff_cnt;i++)
            temp += g_insu_negative_filter_buff[i];
        temp /= g_insu_negative_filter_buff_cnt;
    }
    return temp;
}

FP64 calc_spi_hv_max(void)
{
    FP64 temp_r, temp_v = g_insu_info.total_volt;
    
    temp_r = POS_FEN_YA_RES;
    temp_r += POS_CAI_YANG_RES;
    temp_v = temp_v * temp_r / (temp_r+SYS_LEAK_RES_MAX);
    temp_v = temp_v * POS_CAI_YANG_RES / temp_r;// * RES_GAIN_FACTOR;
    
    if(temp_v > AD_REF_VOLT)
        temp_v = AD_REF_VOLT;
    return temp_v;
}

/*
*
* @brief       正极绝缘电阻计算函数
* @note         无
* @param[in]    无
* @param[out]   0:阻值公式计算正常 1:阻值公式计算异常
* @return       无
*
*/ 
INT8U insu_res_calc_negative(INT16U *res)
{
    FP64 temp_v, temp_r, mdata;
    
    if(res == NULL) return 0xFF;
    
    temp_v = calc_spi_hv_max();
    if(g_insu_volt_positive[0] > temp_v)// || g_spiHVP[1] > temp_v)
    {
        *res = SYS_SHORT_CIRCUIT_RES;
        return 1;   
    } 
    else if(g_insu_volt_negative[0] > temp_v)
    {
        *res = 0xFFFF;
        return 1;
    }
    
    if(g_insu_volt_positive[1] == 0) 
    {
        *res = 0xFFFF;
        return 1;
    }
    temp_r = (POS_FEN_YA_RES+POS_CAI_YANG_RES)*(g_insu_volt_positive[0] + g_insu_volt_negative[0]- g_insu_volt_positive[1] - g_insu_volt_negative[1])/g_insu_volt_positive[1];
    temp_r *= 10;
    
    if(temp_r > 20000)
    {
      temp_v = g_insu_volt_positive[0] + g_insu_volt_negative[0];//U
    
      temp_r = POS_FEN_YA_RES;
    
      mdata = g_insu_volt_negative[0] * g_insu_volt_positive[1];
      if(g_insu_volt_positive[0] >= g_insu_volt_positive[1])
        temp_r = temp_v * temp_r * (g_insu_volt_positive[0] - g_insu_volt_positive[1])/mdata;
      else
        temp_r = temp_v * temp_r * (g_insu_volt_positive[1] - g_insu_volt_positive[0])/mdata;
    
      temp_r *= 10;
    }
    if(temp_r > 65535)temp_r = 65535;
    
    //g_isoInfo.Res_N = IsoNFilter(temp_r);//TODO:(INT16U)temp_r;
    *res = temp_r;
    
    return 0;
}
/*
*
* @brief       负极绝缘电阻计算函数
* @note         无
* @param[in]    无
* @param[out]   0:阻值公式计算正常 1:阻值公式计算异常
* @return       无
*
*/ 
INT8U insu_res_calc_positive(INT16U *res)
{
    FP64 temp_v, temp_r, mdata;
    
    if(res == NULL) return 0xFF;
    
    temp_v = calc_spi_hv_max();
    if(g_insu_volt_negative[0] > temp_v)// || g_spiHVN[1] > temp_v)
    {
        *res = SYS_SHORT_CIRCUIT_RES;
        return 1;   
    }
    else if(g_insu_volt_positive[0] > temp_v)
    {
        *res = 0xFFFF;
        return 1;
    }
    
    if(g_insu_volt_negative[1] == 0) 
    {
        *res = 0xFFFF;
        return 1;
    }
    
    temp_r = (POS_FEN_YA_RES+POS_CAI_YANG_RES)*(g_insu_volt_positive[0] + g_insu_volt_negative[0]- g_insu_volt_positive[1] - g_insu_volt_negative[1])/g_insu_volt_negative[1];
    temp_r *= 10;

    if(temp_r >20000)
    {
      temp_v = g_insu_volt_positive[0] + g_insu_volt_negative[0];//U
    
      temp_r = POS_FEN_YA_RES;
    
      mdata = g_insu_volt_positive[0] * g_insu_volt_negative[1];
      if(g_insu_volt_negative[0] >= g_insu_volt_negative[1])
        temp_r = temp_v * temp_r * (g_insu_volt_negative[0] - g_insu_volt_negative[1])/mdata;
      else
        temp_r = temp_v * temp_r * (g_insu_volt_negative[1] - g_insu_volt_negative[0])/mdata;
    
      temp_r *= 10;
    }

    if(temp_r > 65535)temp_r = 65535;
    
    //g_isoInfo.Res_P = IsoPFilter(temp_r);//TODO:(INT16U)temp_r;
    *res = temp_r;

    return 0;
}

INT8U insu_get_spi_volt(INT16U *near p_volt, INT16U *near n_volt)
{
    //INT8U flag = 0,flag_n = 0,fail_count = 0;
    INT8U fail_count = 0;
    
    if((p_volt == NULL)||(n_volt == NULL))
        return 0;
    
    
    fail_count = 0;
    while(!adc_onchip_read(&adc_onchip_imd_hvp, p_volt)) {
        if(fail_count++ < INSU_XADC0_RETRY_NUM)     
            OSTimeDly(INSU_XADC0_RETRY_DELAY);
        else
            return 0;
    }
    
    fail_count = 0;
    while(!adc_onchip_read(&adc_onchip_imd_hvn, n_volt)) {
        if(fail_count++ < INSU_XADC0_RETRY_NUM)     
            OSTimeDly(INSU_XADC0_RETRY_DELAY);
        else
            return 0;
    }
    
    
    return 1;
}

INT8U insu_get_first_spi_volt(void)
{
    INT16U temp_p, temp_n;
    INT8U temp = 0;

    PositiveMosOn();//总正极开关
    NegativeMosOn();//总负极开关
    SAMPLE_MOS_ON_DELAY();//OSTimeDly(MOS_ON_DELAY);
    
    temp = insu_get_spi_volt(&temp_p, &temp_n);
    if(!temp) return 0;
    
    //g_insu_volt_positive[0] = (FP64)temp_p * AD_REF_VOLT / AD_TO_VOLT_FACTOR / g_insu_res_gain_factor/1000;
    //g_insu_volt_negative[0] = (FP64)temp_n * AD_REF_VOLT / AD_TO_VOLT_FACTOR / g_insu_res_gain_factor/1000;
    g_insu_volt_positive[0] = (FP64)temp_p / g_insu_res_gain_factor/1000;
    g_insu_volt_negative[0] = (FP64)temp_n / g_insu_res_gain_factor/1000; 
	  
	  return 1;   
}

INT8U insu_get_second_spi_volt(void)
{
    INT16U temp_v1, temp_v2;
    INT8U temp = 0;
    
    PositiveMosOn();//总正极开关
    NegativeMosOff();//总负极开关
    SAMPLE_MOS_ON_DELAY();//OSTimeDly(MOS_ON_DELAY);
    
    temp = insu_get_spi_volt(&temp_v1, &temp_v2);
    if(!temp) return 0;
    
    g_insu_volt_positive[1] = (FP64)temp_v1 / g_insu_res_gain_factor/1000; 
    PositiveMosOff();//总正极开关
    NegativeMosOn();//总负极开关
    SAMPLE_MOS_ON_DELAY();//OSTimeDly(MOS_ON_DELAY);
    
    temp = insu_get_spi_volt(&temp_v1, &temp_v2);
    if(!temp) return 0;
    
    g_insu_volt_negative[1] = (FP64)temp_v2 / g_insu_res_gain_factor/1000;
    
    return 1;
}

void insu_calc_insu_res(void)
{
    INT8U calc_flag = 0,temp = 0;
    INT16U temp_p=0, temp_n=0, res_min;
    FP64 temp_v, res_p, res_n;
    
    all_mos_on();
    temp_v = calc_spi_hv_max();
    
    temp = insu_get_spi_volt(&temp_p, &temp_n);
    if(!temp) return;
    
    g_insu_volt_positive[0] = (FP64)temp_p / g_insu_res_gain_factor/1000;
    g_insu_volt_negative[0] = (FP64)temp_n / g_insu_res_gain_factor/1000; 
    
    g_insu_info.total_volt = insu_calc_total_volt(g_insu_volt_positive[0], g_insu_volt_negative[0]);//先检测总压是否正常            
    if(insu_total_volt_is_valid() == FALSE) return;
    	
    if(g_insu_volt_negative[0] > temp_v)// || g_spiHVN[1] > temp_v)
    {
        calc_flag = 1;
        temp_p = SYS_SHORT_CIRCUIT_RES;   
        temp_n = 0xFFFF;
    }
    else if(g_insu_volt_positive[0] > temp_v)
    {
        calc_flag = 1;
        temp_p = 0xFFFF;
        temp_n = SYS_SHORT_CIRCUIT_RES;
    }
    else
    {
        res_p = POS_FEN_YA_RES + POS_CAI_YANG_RES;
        if(g_insu_volt_positive[0] == g_insu_volt_negative[0])
        {
            res_p = 0xFFFF;
            res_n = 0xFFFF;
        }
        else if(g_insu_volt_positive[0] > g_insu_volt_negative[0])
        {
            temp_v = (FP64)(g_insu_volt_positive[0] - g_insu_volt_negative[0]) / g_insu_volt_negative[0];
            res_n = res_p / temp_v * 10;
            res_p = 0xFFFF;
        }
        else
        {
            res_n = 0xFFFF;
            temp_v = (FP64)(g_insu_volt_negative[0] - g_insu_volt_positive[0]) / g_insu_volt_positive[0];
            res_p = res_p / temp_v * 10;
        }

        temp_p = res_p;
        temp_n = res_n;
        res_min = insu_get_leak_res_min();
        if(temp_p < res_min && temp_n < res_min) //不可能同时都很小
        {
            if(g_insu_volt_positive[0] < g_insu_volt_negative[0]) temp_n = 0xFFFF;
            else temp_p = 0xFFFF;
        }
    }
    g_insu_info.positive_res = insu_positive_res_filter(temp_p);//TODO:(INT16U)temp_r;
    g_insu_info.negative_res = insu_negative_res_filter(temp_n);//TODO:(INT16U)temp_r;
    
    if(calc_flag && (g_insu_info.positive_res == SYS_SHORT_CIRCUIT_RES || g_insu_info.negative_res == SYS_SHORT_CIRCUIT_RES))
        g_insu_info.insu_status = 0x02;
    else
        g_insu_info.insu_status = 0x00;
    
    update_system_insu();
}

void insu_calc_insu_res_with_mos(void)
{   
#if 1
    INT8U calc_flag = 0,temp = 0;
    INT16U res_p, res_n, res_min;
    
    temp = insu_get_first_spi_volt();
    if(!temp) return;
    
    temp = insu_get_second_spi_volt();
    if(!temp) return;
    
    reset_mos_status();//复位MOS
    
    g_insu_info.total_volt = insu_calc_total_volt(g_insu_volt_positive[0], g_insu_volt_negative[0]);//先检测总压是否正常            
    if(insu_total_volt_is_valid() == FALSE) return;
    
    calc_flag |= insu_res_calc_positive(&res_p);
    calc_flag |= insu_res_calc_negative(&res_n);
    
    res_min = insu_get_leak_res_min();
    if(res_p < res_min && res_n < res_min) //不可能同时都很小
    {
        if(g_insu_volt_positive[0] < g_insu_volt_negative[0]) res_n = 0xFFFF;
        else res_p = 0xFFFF;
    }
    g_insu_info.positive_res = insu_positive_res_filter(res_p);
    g_insu_info.negative_res = insu_negative_res_filter(res_n);
    
    if(calc_flag && (g_insu_info.positive_res == SYS_SHORT_CIRCUIT_RES || g_insu_info.negative_res == SYS_SHORT_CIRCUIT_RES))
        g_insu_info.insu_status = 0x02;
    else
        g_insu_info.insu_status = 0x00;
    
    update_system_insu();
#else //待验证
    float tmp = 0;
    uint8_t fail_count = 0;
    struct insu_res_unbalanced_bridge_result        result;
    struct insu_res_unbalanced_bridge_sample_value  sample_val = {0};


    PositiveMosOn();//总正极开关
    NegativeMosOn();//总负极开关
    SAMPLE_MOS_ON_DELAY();
        
    fail_count = 0;
    while(!adc_onchip_read(&adc_onchip_imd_hvp, &sample_val.p_val_pmos_on_nmos_on)) {
        if(fail_count++ < INSU_XADC0_RETRY_NUM)     
            OSTimeDly(INSU_XADC0_RETRY_DELAY);
        else
            return ;
    }

    fail_count = 0;
    while(!adc_onchip_read(&adc_onchip_imd_hvn, &sample_val.n_val_pmos_on_nmos_on)) {
        if(fail_count++ < INSU_XADC0_RETRY_NUM)     
            OSTimeDly(INSU_XADC0_RETRY_DELAY);
        else
            return ;
    }
        
    PositiveMosOff();//总正极开关
    NegativeMosOn();//总负极开关
    SAMPLE_MOS_ON_DELAY();
    fail_count = 0;
    while(!adc_onchip_read(&adc_onchip_imd_hvn, &sample_val.n_val_pmos_off_nmos_on)) {
        if(fail_count++ < INSU_XADC0_RETRY_NUM)     
            OSTimeDly(INSU_XADC0_RETRY_DELAY);
        else
            return ;
    }
        
    
    PositiveMosOn();//总正极开关
    NegativeMosOff();//总负极开关
    SAMPLE_MOS_ON_DELAY();
    fail_count = 0;
    while(!adc_onchip_read(&adc_onchip_imd_hvp, &sample_val.p_val_pmos_on_nmos_off)) {
        if(fail_count++ < INSU_XADC0_RETRY_NUM)     
            OSTimeDly(INSU_XADC0_RETRY_DELAY);
        else
            return ;
    }

    reset_mos_status();//复位MOS
    
    insu_res_unbalanced_bridge_calculate(&result,  &sample_val) ;

    
    g_insu_info.insu_status = 0x00;
    if(result.ratio_p <= 0)
    {
        g_insu_info.positive_res = SYS_SHORT_CIRCUIT_RES;
        g_insu_info.insu_status = 0x02;
    }
    else
    {
        if(result.ratio_p >= (65535.0/(POS_FEN_YA_RES + POS_CAI_YANG_RES)))
        {
            g_insu_info.positive_res = 65535;
        }
        else
        {
            result.ratio_p = result.ratio_p * (POS_FEN_YA_RES + POS_CAI_YANG_RES);
            g_insu_info.positive_res = (INT16U)result.ratio_p;
        }
    }
    
    if(result.ratio_n <= 0)
    {
        g_insu_info.negative_res = SYS_SHORT_CIRCUIT_RES;
        g_insu_info.insu_status = 0x02;
    }
    else
    {
        if(result.ratio_n >= (65535.0/(POS_FEN_YA_RES + POS_CAI_YANG_RES)))
        {
            g_insu_info.negative_res = 65535;
        }
        else
        {
            result.ratio_n = result.ratio_n * (POS_FEN_YA_RES + POS_CAI_YANG_RES);
            g_insu_info.negative_res = (INT16U)result.ratio_n;
        }
    }
    
    tmp = sample_val.p_val_pmos_on_nmos_on + sample_val.n_val_pmos_on_nmos_on;
    tmp = tmp * g_insu_res_gain_factor/1000;
    tmp = tmp * (POS_FEN_YA_RES + POS_CAI_YANG_RES) / POS_CAI_YANG_RES;

    g_insu_info.total_volt = tmp;
    //g_insu_info.total_volt = (float)(sample_val.p_val_pmos_on_nmos_on + sample_val.n_val_pmos_on_nmos_on) 
    //                            * g_insu_res_gain_factor/1000  * (POS_FEN_YA_RES + POS_CAI_YANG_RES) / POS_CAI_YANG_RES;
    
    update_system_insu();
#endif
}
//计算绝缘总压
INT16U insu_calc_total_volt(FP64 hvp, FP64 hvn)
{
    INT16U volt;
    
    volt =  (hvp + hvn) * (POS_FEN_YA_RES + POS_CAI_YANG_RES) / POS_CAI_YANG_RES;
    
    if(volt < TOTAL_VOLT_INSU_MIN)
        volt = 0;
    
    return volt;
}

INT16U insu_calc_pre_volt(void) //pre
{
    INT32S volt = 0;
    INT16S temp = 0;
    INT8U index = 0, err_count = 0,dat_valid_count = 0;
    
    //g_adcZVCount = 0;
    
    while(index < EXTEND_TV_AD_CNT)
    {
        temp = ADS1015_SingleConvert(&g_ADS1015_BspInterface, ADS1015_CHANNEL_1, ADS1015_FULL_SCALE_4096_MV);
        if(temp == ADS1015_RESULT_ERROR) 
        {
            dat_valid_count = 0;
            
            err_count++;
            if(err_count < 2) continue;        
            
            temp = 0;
        }
        else if(temp < 0)
        {
            err_count = 0;
            
            dat_valid_count++;
            if(dat_valid_count < 2) continue;
            
            temp = 0;
        }
        err_count = 0;
        dat_valid_count = 0;
        g_adc_buff_extend[index] = temp;
        index++;
        
       // OSTimeDly(1);
    }
    
    volt = bms_middle_average_value_filter(g_adc_buff_extend, EXTEND_TV_AD_CNT, 1);
    
    if(volt >= 0x0800) 
        volt = 0;
    else if(volt == 0x07FF) 
        volt = PRE_TV_FULL_VALUE;
    else 
        volt = volt * ADS1013_REFERENCE / ADS1013_AD_MAX;
#if BMS_SUPPORT_BY5248D == 1
	volt = volt * (3*PRE_TV_FEN_YA_RES_COMMON + PRE_TV_CAI_YANG_RES_COMMON) / PRE_TV_CAI_YANG_RES_COMMON;
#else
    volt = volt * (5*PRE_TV_FEN_YA_RES_COMMON + PRE_TV_CAI_YANG_RES_COMMON) / PRE_TV_CAI_YANG_RES_COMMON;
#endif   
    volt = MV_TO_100MV(volt);
    
    if(volt < TOTAL_VOLT_MIN_DEFAULT) volt = 0;
    
    return (INT16U)volt;
}

INT16U insu_calc_pch_volt(void) //pch
{
    INT32S volt = 0;
    INT16S temp = 0;
    INT8U index = 0, err_count = 0,dat_valid_count = 0;
    
    //g_adcZVCount = 0;
    
    while(index < EXTEND_TV_AD_CNT)
    {
	#if BMS_SUPPORT_BY5248D == 1
        temp = ADS1015_SingleConvert(&g_ADS1015_BspInterface, ADS1015_CHANNEL_2, ADS1015_FULL_SCALE_4096_MV);
    #else
        temp = ADS1015_SingleConvert(&g_ADS1015_BspInterface, ADS1015_CHANNEL_0, ADS1015_FULL_SCALE_4096_MV);
	#endif   
	   
        if(temp == ADS1015_RESULT_ERROR) 
        {
            dat_valid_count = 0;
            
            err_count++;
            if(err_count < 2) continue;        
            
            temp = 0;
        }
        else if(temp < 0)
        {
            err_count = 0;
            
            dat_valid_count++;
            if(dat_valid_count < 2) continue;
            
            temp = 0;
        }
        err_count = 0;
        dat_valid_count = 0;
        g_adc_buff_extend[index] = temp;
        index++;
        
        //OSTimeDly(1);
    }
    
    volt = bms_middle_average_value_filter(g_adc_buff_extend, EXTEND_TV_AD_CNT, 1);

    if(volt >= 0x0800) 
        volt = 0;
    else if(volt == 0x07FF) 
        volt = PCH_TV_FULL_VALUE;
    else 
        volt = volt * ADS1013_REFERENCE / ADS1013_AD_MAX;
#if BMS_SUPPORT_BY5248D == 1
	volt = volt * (3*PCH_TV_FEN_YA_RES_COMMON + PCH_TV_CAI_YANG_RES_COMMON) / PCH_TV_CAI_YANG_RES_COMMON;
#else
    volt = volt * (5*PCH_TV_FEN_YA_RES_COMMON + PCH_TV_CAI_YANG_RES_COMMON) / PCH_TV_CAI_YANG_RES_COMMON;
#endif
    
    volt = MV_TO_100MV(volt);
    
    if(volt < TOTAL_VOLT_MIN_DEFAULT) volt = 0;
    
    return (INT16U)volt;
}

INT16U insu_calc_bat_hv_volt(void) //BAT_HV
{
    INT32S volt = 0;
    INT16S temp = 0;
    INT8U index = 0, i = 0, err_count = 0,dat_valid_count = 0;
    
    //g_adcZVCount = 0;
    
    while(index < EXTEND_TV_AD_CNT)
    {
	#if BMS_SUPPORT_BY5248D == 1
        temp = ADS1015_SingleConvert(&g_ADS1015_BspInterface, ADS1015_CHANNEL_0, ADS1015_FULL_SCALE_4096_MV);
    #else
        temp = ADS1015_SingleConvert(&g_ADS1015_BspInterface, ADS1015_CHANNEL_3, ADS1015_FULL_SCALE_4096_MV);
	#endif    
        if(temp == ADS1015_RESULT_ERROR) 
        {
            dat_valid_count = 0;
            
            err_count++;
            if(err_count < 2) continue;        
            
            temp = 0;
        }
        else if(temp < 0)
        {
            err_count = 0;
            
            dat_valid_count++;
            if(dat_valid_count < 2) continue;
            
            temp = 0;
        }
        err_count = 0;
        dat_valid_count = 0;
        g_adc_buff_extend[index] = temp;
        index++;
        
        //OSTimeDly(1);
    }
    
    
    //g_adcZVCount = 0;
    volt = bms_middle_average_value_filter(g_adc_buff_extend, EXTEND_TV_AD_CNT, 1);
    
    if(volt >= 0x0800) 
        volt = 0;
    else if(volt == 0x07FF) 
        volt = PRE_TV_FULL_VALUE;
    else 
        volt = volt * ADS1013_REFERENCE / ADS1013_AD_MAX;
#if BMS_SUPPORT_BY5248D == 1
	volt = volt * (3*BAT_TV_FEN_YA_RES_COMMON + BAT_TV_CAI_YANG_RES_COMMON) / BAT_TV_CAI_YANG_RES_COMMON;
#else
    volt = volt * (5*BAT_TV_FEN_YA_RES_COMMON + BAT_TV_CAI_YANG_RES_COMMON) / BAT_TV_CAI_YANG_RES_COMMON;
#endif    
    volt = MV_TO_100MV(volt);
        
    if(volt < TOTAL_VOLT_MIN_DEFAULT) volt = 0;
    
    return (INT16U)volt;
}

INT16U insu_calc_hv3_hv_volt(void) //HV3_AD
{
    INT32S volt = 0;
    INT16S temp = 0;
    INT8U index = 0, i = 0, err_count = 0,dat_valid_count = 0;
    
    //g_adcZVCount = 0;
    
    while(index < EXTEND_TV_AD_CNT)
    {
	#if BMS_SUPPORT_BY5248D == 1
        temp = ADS1015_SingleConvert(&g_ADS1015_BspInterface, ADS1015_CHANNEL_3, ADS1015_FULL_SCALE_4096_MV);
    #else
        temp = ADS1015_SingleConvert(&g_ADS1015_BspInterface, ADS1015_CHANNEL_2, ADS1015_FULL_SCALE_4096_MV);
    #endif   
        if(temp == ADS1015_RESULT_ERROR) 
        {
            dat_valid_count = 0;
            
            err_count++;
            if(err_count < 2) continue;        
            
            temp = 0;
        }
        else if(temp < 0)
        {
            err_count = 0;
            
            dat_valid_count++;
            if(dat_valid_count < 2) continue;
            
            temp = 0;
        }
        err_count = 0;
        dat_valid_count = 0;
        g_adc_buff_extend[index] = temp;
        index++;
        
        //OSTimeDly(1);
    }
    
    //g_adcZVCount = 0;
    volt = bms_middle_average_value_filter(g_adc_buff_extend, EXTEND_TV_AD_CNT, 1);
    
    if(volt >= 0x0800) 
        volt = 0;
    else if(volt == 0x07FF) 
        volt = PRE_TV_FULL_VALUE;
    else 
        volt = volt * ADS1013_REFERENCE / ADS1013_AD_MAX;
    
#if BMS_SUPPORT_NEGATIVE_ADHESION     
    if(volt < 420)
    {
        volt = 0;
    }else
    {
        volt = volt - 420;    //减去二极管上的电压
    }
#endif    
    
#if BMS_SUPPORT_BY5248D == 0
   #if BMS_SUPPORT_NEGATIVE_ADHESION
        volt = volt * (5*BAT_TV_FEN_YA_RES_COMMON + BAT_TV_CAI_YANG_RES_COMMON + HV3_TV_OPTOCOUPLER_RES_COMMON) / (BAT_TV_CAI_YANG_RES_COMMON + HV3_TV_OPTOCOUPLER_RES_COMMON);
   #else
        volt = volt * (5*BAT_TV_FEN_YA_RES_COMMON + BAT_TV_CAI_YANG_RES_COMMON) / BAT_TV_CAI_YANG_RES_COMMON;
    #endif
#else
    #if BMS_SUPPORT_NEGATIVE_ADHESION
	    volt = volt * (3*BAT_TV_FEN_YA_RES_COMMON + BAT_TV_CAI_YANG_RES_COMMON + HV3_TV_OPTOCOUPLER_RES_COMMON) / (BAT_TV_CAI_YANG_RES_COMMON + HV3_TV_OPTOCOUPLER_RES_COMMON);    
    #else
    	volt = volt * (3*BAT_TV_FEN_YA_RES_COMMON + BAT_TV_CAI_YANG_RES_COMMON) / BAT_TV_CAI_YANG_RES_COMMON;	
    #endif

#endif    
    volt = MV_TO_100MV(volt);
        
    if(volt < TOTAL_VOLT_MIN_DEFAULT) volt = 0;
    
    return (INT16U)volt;
}

#define VOLT_EXCEPT_MIN 50

//总压有效性检查
INT8U insu_total_volt_is_valid(void)
{
    if(g_insu_info.bat_volt < VOLT_EXCEPT_MIN)
        g_insu_info.insu_status = 0x01;
    else
        g_insu_info.insu_status = 0x00;
    
    if(((config_get(kBCUCommModeIndex) == BMS_SELF_CHECK_MODE) && g_insu_info.bat_volt < TOTAL_VOLT_MIN_TEST_MODE) || //测试模式，总压只要大于60V
        ((config_get(kBCUCommModeIndex) != BMS_SELF_CHECK_MODE) && g_insu_info.bat_volt < TOTAL_VOLT_MIN) ||
        (g_insu_info.bat_volt > 1000 && abs(TV_100_MV_TO_V(g_insu_info.bat_volt) - g_insu_info.total_volt) > TOTAL_VOLT_DIFF_VALID))
    {
        
        g_insu_info.positive_res=65535;
        g_insu_info.negative_res=65535;
        
        return FALSE;
    }
    else
    {
        if(g_insu_info.insu_status == 0x01)
            g_insu_info.insu_status = 0x00;
    }
    return TRUE;
}

INT16U insu_get_res_calc_cycle(void)    //周期性的更新绝缘信息
{
    INT16U alarm_res;
    
    alarm_res = ISO_ALARM_RES;
    
    if((g_insu_info.insu_status == 0x01) 
        || g_insu_info.insu_sys_res < alarm_res 
        || (config_get(kBCUCommModeIndex) == BMS_SELF_CHECK_MODE)
        || g_insu_positive_origin_res < alarm_res
        || g_insu_negative_origin_res < alarm_res) 
        return RES_CALC_CYCLE_FAST;
    
    return config_get(kEepromNormalCheckCycleIndex);
}

float insu_get_leak_res_min(void)
{
    INT16U range;
    float res = 0.0;
    
    range = config_get(kEepromTvRangeIndex);
    if(range == kTVRange72V)
        res = SYS_LEAK_RES_MIN_IN_72V;
    else if(range == kTVRange400V)
        res = SYS_LEAK_RES_MIN_IN_400V;
    else
        res = SYS_LEAK_RES_MIN_IN_1000V;
    return res;
}

void update_system_insu(void)//更新绝缘阻值
{
    if(g_insu_info.positive_res >= g_insu_info.negative_res)
        g_insu_info.insu_sys_res = g_insu_info.negative_res;
    else
        g_insu_info.insu_sys_res = g_insu_info.positive_res;
}

INT8U volt_range_index = 0;
void insu_gain_res_with_voltage(void)
{    
    volt_range_index = (INT8U)bms_get_word_range_index(g_isoBatVoltBuff,BAT_VOLT_LEVEL_NUM,BAT_TOTAL_VOLT_FACT);
    g_insu_res_gain_factor = g_insu_res_factor_arr[volt_range_index];
    switch(volt_range_index)   
    {
        case 0:
            hcf4052_select_channel(hcf4052_impl, HCF4052_CHANNEL_0);
            break;
        case 1:
            hcf4052_select_channel(hcf4052_impl, HCF4052_CHANNEL_1);
            
            break;
        case 2:
            hcf4052_select_channel(hcf4052_impl, HCF4052_CHANNEL_2);
            break;
        case 3:
            hcf4052_select_channel(hcf4052_impl, HCF4052_CHANNEL_3);
            break;        
        case 4:
            hcf4052_select_channel(hcf4052_impl, HCF4052_CHANNEL_NONE);
            break;
        default:
        break;
    }
}
#endif

/***********END OF FILE***********/      