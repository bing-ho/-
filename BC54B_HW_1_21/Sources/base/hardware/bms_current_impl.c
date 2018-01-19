/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_current_impl.c
 * @brief
 * @note
 * @author
 * @date 2012-5-9
 *
 */
#include "bms_current_impl.h"
#include "bms_config.h"
#include "adc_xgate.h"
#include "bms_soc_impl.h"
#if BMS_SUPPORT_CURRENT

#pragma MESSAGE DISABLE C1420   // Result of function-call is ignored
#pragma MESSAGE DISABLE C2705   // Possible loss of data
#pragma MESSAGE DISABLE C5919   // Conversion of floating to unsigned integral
#pragma MESSAGE DISABLE C12056  // SP debug info incorrect because of optimization or inline assembler
#pragma MESSAGE DISABLE C4301   // Inline expansion done for function call

#define CURRENT_LOW_RANGE_ADS1013_CONFIG    0x4E3
#define CURRENT_HIGH_RANGE_ADS1013_CONFIG   0x4E3


/* Current = (2260*V_AD - (range_max_volt + range_min_volt)/2 - ref_voltage)*2*Senssor_max_range/(range_max_volt - range_min_volt)   */
#define ZOOM_IN_RATIO           ((float)2.260)    // 226K/100K
#define SINGLE_RANGE_MAX_VOLT          ((INT16S)4500)   //4500mV ==> 500A  右边满偏时候对应邋AD采样电压值
#define SINGLE_RANGE_MIN_VOLT          ((INT16S) 500)   //500mV ==> -500A 左边满偏时候对应邋AD采样电压值
#define DOUBLE_RANGE_MAX_VOLT          ((INT16S)4000)   //4000mV ==> 500A  右边满偏时候对应邋AD采样电压值
#define DOUBLE_RANGE_MIN_VOLT         ((INT16S)-4000)   //-4000mV ==> -500A 左边满偏时候对应邋AD采样电压值
#define REF_VOLTAGE                   4096         //5000mV or 4096mV   参考电压
#define ADC_CHG_VOL_ADJUST              4               //单位mv, 此处两个adjust 为修正量，用于补偿从输入口到芯片IO 的衰减量
#define ADC_DCHG_VOL_ADJUST             24
#define SENSSOR_MAX_RANGE(CURRENT_GET_HIGH_or_LOW_RANGE_MAX)       current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_HIGH_or_LOW_RANGE_MAX)//满量程时候电流值

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

static CurrentSampleContext g_current_sample_context;
static INT16S g_currentAutoCheckHighCurInZero = 0;
static INT16S g_currentAutoCheckLowCurInZero = 0;
static INT16S g_currentAutoCheckSingleCurInZero = 0;
static INT16U g_currentADPowerSupplyVolt = CURRENT_SUPPLY_VOLT_DEFAULT;
static INT32U g_currentAutoToZeroTick = 0;
static INT8U  g_currentAutoToZeroState=0;
static BOOLEAN g_currentAutoToZeroFlag=0;
static INT16U g_current_ref_voltage = CURRENT_REF_VOLTAGE_BEFORE_V120_5000mV;

#if BMS_LOG_LEVEL==INFO_LEVEL
static INT8U g_debugPrintInterval = 0;
#endif

#pragma DATA_SEG DEFAULT


#if USE_TBL_INSTEAD_OF_MACRO  

typedef enum
{
    CURRENT_RANGE_PARAM_LOW,
    CURRENT_RANGE_PARAM_HIGH
}CURRENT_RANGE_PARAM_TYPE;

typedef enum
{
    CURRENT_GET_LOW_OFFSET_VOLT,
    CURRENT_GET_LOW_SENSITIVITY,
    CURRENT_GET_HIGH_OFFSET_VOLT,
    CURRENT_GET_HIGH_SENSITIVITY,
    CURRENT_GET_LOW_RANGE_MIN,
    CURRENT_GET_LOW_RANGE_MAX,
    CURRENT_GET_HIGH_RANGE_MIN,
    CURRENT_GET_HIGH_RANGE_MAX
}CURRENT_PARAM_TYPE;

typedef struct
{
    INT16U low_ofst_volt;
    INT16U low_sensitivity;
    INT16U high_ofst_volt;
    INT16U high_sensitivity;
    INT16S low_range_min;
    INT16S low_range_max;
    INT16S high_range_min;
    INT16S high_range_max;
}CurParam;

typedef struct
{
    INT16U current_senor_range;
    union
    {
        CurParam param;
        INT16U  data[8];
    }u;
}CurrentParam;

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
CurrentParam current_param[] = 
{
    {
        150,
        CURRENT_150A_LOW_OFFSET_VOLT,
        CURRENT_150A_LOW_SENSITIVITY,
        CURRENT_150A_HIGH_OFFSET_VOLT,
        CURRENT_150A_HIGH_SENSITIVITY,
        CURRENT_150A_LOW_RANGE_MIN,
        CURRENT_150A_LOW_RANGE_MAX,
        CURRENT_150A_HIGH_RANGE_MIN,
        CURRENT_150A_HIGH_RANGE_MAX,
    },
    {
        200,
        CURRENT_200A_LOW_OFFSET_VOLT,
        CURRENT_200A_LOW_SENSITIVITY,
        CURRENT_200A_HIGH_OFFSET_VOLT,
        CURRENT_200A_HIGH_SENSITIVITY,
        CURRENT_200A_LOW_RANGE_MIN,
        CURRENT_200A_LOW_RANGE_MAX,
        CURRENT_200A_HIGH_RANGE_MIN,
        CURRENT_200A_HIGH_RANGE_MAX,
    },
    {
        350,
        CURRENT_350A_LOW_OFFSET_VOLT,
        CURRENT_350A_LOW_SENSITIVITY,
        CURRENT_350A_HIGH_OFFSET_VOLT,
        CURRENT_350A_HIGH_SENSITIVITY,
        CURRENT_350A_LOW_RANGE_MIN,
        CURRENT_350A_LOW_RANGE_MAX,
        CURRENT_350A_HIGH_RANGE_MIN,
        CURRENT_350A_HIGH_RANGE_MAX,
    },
    {
        500,
        CURRENT_500A_LOW_OFFSET_VOLT,
        CURRENT_500A_LOW_SENSITIVITY,
        CURRENT_500A_HIGH_OFFSET_VOLT,
        CURRENT_500A_HIGH_SENSITIVITY,
        CURRENT_500A_LOW_RANGE_MIN,
        CURRENT_500A_LOW_RANGE_MAX,
        CURRENT_500A_HIGH_RANGE_MIN,
        CURRENT_500A_HIGH_RANGE_MAX,
    },
    {
        600,
        CURRENT_600A_LOW_OFFSET_VOLT,
        CURRENT_600A_LOW_SENSITIVITY,
        CURRENT_600A_HIGH_OFFSET_VOLT,
        CURRENT_600A_HIGH_SENSITIVITY,
        CURRENT_600A_LOW_RANGE_MIN,
        CURRENT_600A_LOW_RANGE_MAX,
        CURRENT_600A_HIGH_RANGE_MIN,
        CURRENT_600A_HIGH_RANGE_MAX,
    }
};
#pragma pop

INT16S current_sessor_param_get(INT16U cur_sensor_range, CURRENT_PARAM_TYPE type)
{
    static INT8U    g_sensor_idx = 0;
    INT16S temp = 0;
    if(current_param[g_sensor_idx].current_senor_range != cur_sensor_range)
    {
        INT8U i;
        for(i=0; i<5; i++)
        {
            if(current_param[i].current_senor_range == cur_sensor_range)
            {
                g_sensor_idx = i;
                break;
            }
        }
        if(i == 5)
        {
            return 0xFFFF;
        }
    }
    temp = current_param[g_sensor_idx].u.data[type];

    return temp;
}
#endif
/*
*
* @brief        初始化
* @note         无
* @param[in]    
* @param[out]   无
* @return       RES_OK 
*
*/
Result current_sample_init(INT16U sample_peroid, INT8U sample_count, INT16S* _PAGED low_buffer,
        INT16S* _PAGED high_buffer, INT16S* _PAGED single_buffer,INT8U min_sample_count, INT8U filter_num, INT16U range, INT16U range1,CURRENT_RANGE_TYPE range_type)
{
    
    if (low_buffer == NULL || high_buffer == NULL||single_buffer == NULL) return RES_INVALID_ARG;

    safe_memset(&g_current_sample_context, 0, sizeof(g_current_sample_context));

    g_current_sample_context.event = OSSemCreate(0);
    g_current_sample_context.adc_data0 = 0;
    g_current_sample_context.adc_data1 = 0;
    g_current_sample_context.sample_count = sample_count;
    g_current_sample_context.sample_peroid = sample_peroid;
    g_current_sample_context.low_buffer = low_buffer;
    g_current_sample_context.high_buffer = high_buffer;
    g_current_sample_context.single_buffer= single_buffer;
    g_current_sample_context.min_sample_count = min_sample_count;
    g_current_sample_context.filter_num = filter_num;
    g_current_sample_context.current_senor_range = (range == 0 ? ISENSOR_TYPE_DEF : range);
    g_current_sample_context.current_senor_range1 = (range1 == 0 ? ISENSOR_TYPE_DEF : range1);
    g_current_sample_context.range_type = range_type;
    if(range_type == kCurrentRangeTypeDouble) 
    {
      g_current_sample_context.get_current_type_func = get_current_range_type_double; 
      g_current_sample_context.current_auto_check_func = current_sample_auto_check_double; 
      g_current_sample_context.CurrentCalibration_func = CurrentCalibration_double;
    } 
    else if(range_type == kCurrentRangeTypeSingle) 
    {
      g_current_sample_context.get_current_type_func = get_current_range_type_single;
      g_current_sample_context.current_auto_check_func = current_sample_auto_check_single;
      g_current_sample_context.CurrentCalibration_func = CurrentCalibration_single;
    } 
    else if(range_type == kCurrentRangeTypeTwoSingle) 
    {
      g_current_sample_context.get_current_type_func = get_current_range_type_twosingle;
      g_current_sample_context.current_auto_check_func = current_sample_auto_check_twosingle;
      g_current_sample_context.CurrentCalibration_func = CurrentCalibration_single;
    }

#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    if(hardware_io_revision_get() == HW_VER_120)
    {
        g_current_ref_voltage = CURRENT_REF_VOLTAGE_V120_8096mV;
    }
    else
    {
        g_current_ref_voltage = CURRENT_REF_VOLTAGE_BEFORE_V120_5000mV;
    }
#else
    g_current_ref_voltage = CURRENT_REF_VOLTAGE_V120_8096mV;
#endif
    return RES_OK;
}

void current_sample_uninit(void)
{
    if (g_current_sample_context.event)
    {
        INT8U err;
        OSSemDel(g_current_sample_context.event, 0, &err);
        g_current_sample_context.adc_data0 = 0;
        g_current_sample_context.adc_data1 = 0;
        g_current_sample_context.event = NULL;
        g_current_sample_context.low_buffer = NULL;
        g_current_sample_context.high_buffer = NULL;
        g_current_sample_context.single_buffer = NULL;
    }
}
/*
*
* @brief        单量程电流传感器误差校准
* @note         配置为两路单量程电流传感器类型时：只对计算SOC的主回路电流进行校准
* @param[in]    电流 0.1bit/A
* @param[out]   电流 0.1bit/A
* @return       电流 0.1bit/A
*
*/
float CurrentCalibration_single(float current) 
{
  static float current_data;
  if(current > 0) 
    {
      current_data = (INT16S)config_get(kCurrentCalibrationChgbIndex);
      current_data = (float)config_get(kCurrentCalibrationChgkIndex)*current + current_data;
      current_data = current_data/1000; 
      return current_data;
    } 
  else 
    {
      current_data = (INT16S)config_get(kCurrentCalibrationDchgbIndex);
      current_data = (float)config_get(kCurrentCalibrationDchgkIndex)*current + current_data;
      current_data = current_data/1000; 
      return current_data;
    }

}
/*
*
* @brief        莱姆双量程电流误差校准
* @note         只在大量程时候进行误差校准
* @param[in]    电流 0.1bit/A
* @param[out]   电流 0.1bit/A
* @return       电流 0.1bit/A
*
*/
float CurrentCalibration_double(float current) 
{
  static float current_data;
  INT16S temp = 0;
  if(current > 0) 
    {
      temp = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_LOW_RANGE_MAX);
      if(current > temp) 
      {
        current_data = (INT16S)config_get(kCurrentCalibrationChgbIndex);
        current_data = (float)config_get(kCurrentCalibrationChgkIndex)*current + current_data;
        current_data = current_data/1000; 
        return current_data; 
      }
    } 
  else 
    {
      temp = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_LOW_RANGE_MIN);
      if(current < temp) 
      {
        current_data = (INT16S)config_get(kCurrentCalibrationDchgbIndex);
        current_data = (float)config_get(kCurrentCalibrationDchgkIndex)*current + current_data;
        current_data = current_data/1000; 
        return current_data;
      }
    }

}
/*
*
* @brief        莱姆双量程电压换算电流
* @note         无
* @param[in]    AD电压：volt大小量程范围type
* @param[out]   电流 0.1bit/A
* @return       电流 0.1bit/A
*
*/
static float get_current_in_100mA_by_ad_volt(float volt, CURRENT_RANGE_PARAM_TYPE type)//type only support CURRENT_GET_HIGH_SENSITIVITY & CURRENT_GET_LOW_SENSITIVITY
{
    float current = 0;
    INT16S temp = 0;
    INT16U sensity = 0;
    INT16U range_min = 0,range_max = 0,zero_data = 0;

    if(type == CURRENT_RANGE_PARAM_LOW)
    {
        sensity = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_LOW_SENSITIVITY);
        range_min = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_LOW_RANGE_MIN); 
        range_max = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_LOW_RANGE_MAX);
        zero_data = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_LOW_OFFSET_VOLT); 
    }
    else
    {
        sensity = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_HIGH_SENSITIVITY);
        range_min = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_HIGH_RANGE_MIN);
        range_max = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_HIGH_RANGE_MAX);
        zero_data = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_HIGH_OFFSET_VOLT); 
    }
    
   // current = (ZOOM_IN_RATIO * volt - (SINGLE_RANGE_MAX_VOLT + SINGLE_RANGE_MIN_VOLT)/2.0 - REF_VOLTAGE) * 2.0
   //                                                     * range / (SINGLE_RANGE_MAX_VOLT - SINGLE_RANGE_MIN_VOLT) * 10; // @0.1A
    current = (((float)(volt*11 - zero_data*10))* (range_max - range_min)) / (SINGLE_RANGE_MAX_VOLT - SINGLE_RANGE_MIN_VOLT);
   
    //current = ((volt*11 - 25000UL)* 2.0* range) / (SINGLE_RANGE_MAX_VOLT - SINGLE_RANGE_MIN_VOLT);
   
    return current;
}
        
/*
*
* @brief        通过AD的电压值换算成电流值
* @note         配置为单量程电流传感器情况下的电流值计算
* @param[in]    Current_platform
* @param[out]   电流：platform.current  0.1bit/A
* @return       RES_OK
*
*/
Result get_current_range_type_single(Current_platform platform) 
{
  INT8U average_temperature;
  INT16S tmp = 0;
  float single_cur = 0;
    
  g_current_sample_context.signal_invalid_flag1 = CURRENT_NO_CONNECT;
  bcu_set_current1(0);//platform.current1 = 0;
  single_cur = platform.ad_single_value; 
    
  tmp = DOUBLE_RANGE_MAX_VOLT;
  tmp -= DOUBLE_RANGE_MIN_VOLT;
  average_temperature = board_average_temperature_get();
  if(bmu_is_valid_temperature(average_temperature))
    {
     if(average_temperature > CURRENT_NORMAL_TEMPERATURE)
        {
        single_cur = single_cur + (INT8U)(average_temperature - CURRENT_NORMAL_TEMPERATURE) / 10;
      }
     else if(average_temperature < CURRENT_NORMAL_TEMPERATURE)
            {
        single_cur = single_cur -(INT8U)(CURRENT_NORMAL_TEMPERATURE - average_temperature) / 10; //0.1mV/℃
            }
        }
   g_currentAutoCheckSingleCurInZero=(INT16S)single_cur;
   g_currentAutoCheckHighCurInZero = 0;
   g_currentAutoCheckLowCurInZero = 0;
        
   single_cur = single_cur + (float)platform.high_calib_value;//calibration_value;
   single_cur = (ZOOM_IN_RATIO * single_cur - REF_VOLTAGE) * 2.0;
        
   single_cur = single_cur * g_current_sample_context.current_senor_range * 10;
   single_cur = single_cur / tmp; // @0.1A
        
   if(abs(single_cur) < platform.cur_zero_filter) single_cur = 0;
   if(config_get(kCurSensorReverseIndex)) single_cur = -1 * single_cur;
   single_cur = g_current_sample_context.CurrentCalibration_func(single_cur);
   bcu_set_current((INT16S)single_cur);//platform.current = (INT16S)single_cur;
   
   if(abs(single_cur)>(g_current_sample_context.current_senor_range * 10))  g_current_sample_context.signal_invalid_flag = CURRENT_OVER_RANGE;   
   else g_current_sample_context.signal_invalid_flag = CURRENT_NORMAL; 
   return RES_OK;
    }
    
/*
*
* @brief        通过AD的电压值换算成电流值
* @note         配置为莱姆双量程电流传感器情况下的电流值计算
* @param[in]    Current_platform
* @param[out]   电流：platform.current  0.1bit/A
* @return       RES_OK
*
*/
Result get_current_range_type_double(Current_platform platform) 
    {
   INT16U low_value = 0, high_value = 0,single_value=0;
   float low_cur = 0, high_cur = 0;
   INT16S temp;
   
    /** 检测电流值 */
   high_value = platform.ad_high_value;
   low_value = platform.ad_low_value;
    
        g_current_sample_context.signal_invalid_flag1 = CURRENT_NO_CONNECT;
   bcu_set_current1(0);//platform.current1 = 0;
   low_value = (INT16S)((INT32S)low_value* REF_VOLTAGE / 4096);
   high_value = (INT16S)((INT32S)high_value* REF_VOLTAGE / 4096);
        if (low_value <= CURRENT_DOUBLE_SENSOR_OUTPUT_VOLT_MIN_1)
        {
        bcu_set_current(0);//platform.current = 0;
            g_current_sample_context.signal_invalid_flag = CURRENT_NO_CONNECT;
            g_currentAutoCheckHighCurInZero = 0;
            g_currentAutoCheckLowCurInZero = 0; 
            return RES_OK;
        }
   if (low_value > 0) low_cur = low_value + platform.calibration_value;
   if (high_value > 0) high_cur = high_value + platform.high_calib_value;        
        g_currentAutoCheckHighCurInZero = (INT16S)high_cur;
        g_currentAutoCheckLowCurInZero = (INT16S)low_cur;
        
        high_cur = get_current_in_100mA_by_ad_volt(high_cur, CURRENT_RANGE_PARAM_HIGH); // @0.1A
        low_cur = get_current_in_100mA_by_ad_volt(low_cur, CURRENT_RANGE_PARAM_LOW); // @0.1A
        
        if(low_cur >= 0)
        {
            float chg_over_cur_max;
        
            temp = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_LOW_RANGE_MAX);
            chg_over_cur_max = (float)(CURRENT_100_MA_FROM_A(temp));
            chg_over_cur_max -= chg_over_cur_max * CURRENT_ALLOW_OVER_PERCENT;
            if(low_cur > chg_over_cur_max && high_cur > low_cur) low_cur = high_cur;
        }
        else
        {
            float dchg_over_cur_max;
        
            temp = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_LOW_RANGE_MIN);
            dchg_over_cur_max = (float)(CURRENT_100_MA_FROM_A(temp));
            dchg_over_cur_max -= dchg_over_cur_max * CURRENT_ALLOW_OVER_PERCENT;
            
            if(low_cur < dchg_over_cur_max && high_cur < low_cur) low_cur = high_cur;
        }
        
    if(abs((INT16S)low_cur) < platform.cur_zero_filter) low_cur = 0;
        if(config_get(kCurSensorReverseIndex)) low_cur = -1 * low_cur;
    low_cur = g_current_sample_context.CurrentCalibration_func(low_cur);
    bcu_set_current((INT16S)low_cur);//platform.current = (INT16S)low_cur;
   
        if(abs(low_cur)>(g_current_sample_context.current_senor_range * 10))  g_current_sample_context.signal_invalid_flag = CURRENT_OVER_RANGE;   
        else g_current_sample_context.signal_invalid_flag = CURRENT_NORMAL;   
    return RES_OK;
    }

/*
*
* @brief        通过AD的电压值换算成电流值
* @note         配置为两路单量程电流传感器情况下的电流值计算，其中计算SOC的通道为大量程通道，对应的硬件通道为通道2
* @param[in]    Current_platform
* @param[out]   电流：platform.current platform.current1   0.1bit/A
* @return       RES_OK
*
*/
Result get_current_range_type_twosingle(Current_platform platform) 
             {    
  INT16U low_value = 0, high_value = 0,single_value=0;
  float low_cur = 0, high_cur = 0;
             
  high_value = platform.ad_high_value;
  low_value = platform.ad_low_value;
  high_value = (INT16S)((INT32S)high_value* REF_VOLTAGE / 4096);
      if (high_value <= CURRENT_DOUBLE_SENSOR_OUTPUT_VOLT_MIN_1)
        {
            bcu_set_current(0);//platform.current =0;
            g_current_sample_context.signal_invalid_flag = CURRENT_NO_CONNECT; //电流传感器没有连接
            g_currentAutoCheckHighCurInZero = 0;
        } 
      else
        { 
          high_cur = high_value + platform.high_calib_value;
          g_currentAutoCheckHighCurInZero = (INT16S)high_cur;
          high_cur = ((high_cur*11 - 25000UL)* 2.0* g_current_sample_context.current_senor_range) / (SINGLE_RANGE_MAX_VOLT - SINGLE_RANGE_MIN_VOLT); 
          if(abs(high_cur) < platform.cur_zero_filter) high_cur = 0;
          if(config_get(kCurSensorReverseIndex)) high_cur = -1 * high_cur; 
          high_cur = g_current_sample_context.CurrentCalibration_func(high_cur);
          bcu_set_current(high_cur);//platform.current = high_cur;
          if(abs(high_cur)>(g_current_sample_context.current_senor_range * 10))  g_current_sample_context.signal_invalid_flag = CURRENT_OVER_RANGE; //电流传感器超出量程
          else g_current_sample_context.signal_invalid_flag = CURRENT_NORMAL; 
        }
      low_value = (INT16S)((INT32S)low_value* REF_VOLTAGE / 4096); 
      if (low_value <= CURRENT_DOUBLE_SENSOR_OUTPUT_VOLT_MIN_1)
        {
            bcu_set_current1(0);//platform.current1 = 0;
            g_current_sample_context.signal_invalid_flag1 = CURRENT_NO_CONNECT;   //电流传感器1没有连接
            g_currentAutoCheckLowCurInZero = 0; 
        } 
      else
       {
          low_cur = low_value + platform.calibration_value;
          g_currentAutoCheckLowCurInZero = (INT16S)low_cur;
          low_cur = ((low_cur*11 - 25000UL)* 2.0* g_current_sample_context.current_senor_range1) / (SINGLE_RANGE_MAX_VOLT - SINGLE_RANGE_MIN_VOLT);
          if(abs(low_cur) < platform.cur_zero_filter) low_cur = 0;
          if(config_get(kCurSensorReverseIndex)) low_cur = -1 * low_cur;
          bcu_set_current1(low_cur);//platform.current1 = low_cur;
          if(abs(low_cur)>(g_current_sample_context.current_senor_range1 * 10))  g_current_sample_context.signal_invalid_flag1 = CURRENT_OVER_RANGE;//电流传感器1超出量程
          else g_current_sample_context.signal_invalid_flag1 = CURRENT_NORMAL;    
       }              
   return RES_OK;            
}

/*
*
* @brief        获取电流值
* @note         无
* @param[in]    无  
* @param[out]   无
* @return       RES_OK
*
*/
Result get_sample_current(Current_platform platform) 
{
  Result res = 0;
  if(g_current_sample_context.get_current_type_func == NULL) return res;
  res = g_current_sample_context.get_current_type_func(platform);
  return res;
}
 
/*
*
* @brief        计算零飘值
* @note         配置为单量程电流传感器情况下的零飘值计算
* @param[in]    无  
* @param[out]   无
* @return       RES_OK
*
*/
CurrentCheck g_current_check={0};
Result current_sample_auto_check_single(void) 
{
    Result res;
    INT16S diff_current[2] = {0};
    float cur = 0;
    INT16S tmp;
    Current_platform platform={0};
    platform.ad_high_value = bms_get_ad1_value_h();
    platform.ad_low_value = bms_get_ad1_value_l();
    platform.ad_single_value = bms_get_ad1_value_single();
    platform.calibration_value = 0;
    platform.high_calib_value = 0;
    platform.cur_zero_filter =0;
    res = get_current_range_type_single(platform);
    
    if (res != RES_OK)
        return RES_ERR;
    diff_current[1] = (float)REF_VOLTAGE/ZOOM_IN_RATIO - g_currentAutoCheckSingleCurInZero;
    diff_current[0] = 0;
    tmp = DOUBLE_RANGE_MAX_VOLT;
    tmp -= DOUBLE_RANGE_MIN_VOLT;
        //cur = (ZOOM_IN_RATIO * (float)g_currentAutoCheckSingleCurInZero - (DOUBLE_RANGE_MAX_VOLT + DOUBLE_RANGE_MIN_VOLT)/2.0 - REF_VOLTAGE) * 2.0;
    cur = (ZOOM_IN_RATIO * (float)g_currentAutoCheckSingleCurInZero  - REF_VOLTAGE) * 2.0;
    cur = cur * g_current_sample_context.current_senor_range * 10;
    cur = cur / tmp; // @0.1A
    //if(abs(cur) > g_current_sample_context.current_senor_range) //10%
    //return RES_OK;
    if((abs(cur) < g_current_sample_context.current_senor_range) && (abs(diff_current[1]) <= CCHK_CUR_OUT_VOLT_MAX_DEF)) 
    {
      g_current_check.diff1 = diff_current[1]; 
    }
    return RES_OK;
}

/*
*
* @brief        计算零飘值
* @note         配置为莱姆双量程电流传感器情况下的零飘值计算
* @param[in]    无  
* @param[out]   无
* @return       RES_OK
*
*/
Result current_sample_auto_check_double(void) 
{
   Result res;
   INT16S offset;
   INT16S diff_current[2] = {0};
   float cur = 0;
 
   Current_platform platform={0};
   platform.ad_high_value = bms_get_ad1_value_h();
   platform.ad_low_value = bms_get_ad1_value_l();
   platform.ad_single_value = bms_get_ad1_value_single();
   platform.calibration_value = 0;
   platform.high_calib_value = 0;
   platform.cur_zero_filter =0;
   res = get_current_range_type_double(platform);
   if (res != RES_OK)
        return RES_ERR;
   offset = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_HIGH_OFFSET_VOLT);
   diff_current[1] = (offset*10)/11 - g_currentAutoCheckHighCurInZero;
   offset = current_sessor_param_get(g_current_sample_context.current_senor_range, CURRENT_GET_LOW_OFFSET_VOLT);
   diff_current[0] = (offset*10)/11 - g_currentAutoCheckLowCurInZero; 
   if(abs(diff_current[1]) <= CCHK_CUR_OUT_VOLT_MAX_DEF)
        g_current_check.diff1=diff_current[1];
   if(abs(diff_current[0]) <= CCHK_CUR_OUT_VOLT_MAX_DEF)
        g_current_check.diff0=diff_current[0];
   return RES_OK;
}

/*
*
* @brief        计算零飘值
* @note         配置为两路单量程电流传感器情况下的零飘值计算
* @param[in]    无  
* @param[out]   无
* @return       RES_OK
*
*/
Result current_sample_auto_check_twosingle(void) 
{
   Result res;
   INT16S diff_current[2] = {0};
   float cur = 0;
   Current_platform platform={0};
   platform.ad_high_value = bms_get_ad1_value_h();
   platform.ad_low_value = bms_get_ad1_value_l();
   platform.ad_single_value = bms_get_ad1_value_single();
   platform.calibration_value = 0;
   platform.high_calib_value = 0;
   platform.cur_zero_filter =0;
   res = get_current_range_type_twosingle(platform);
   if (res != RES_OK)
        return RES_ERR; 
   diff_current[1] = (2500*10)/11-g_currentAutoCheckHighCurInZero;
   diff_current[0] = (2500*10)/11-g_currentAutoCheckLowCurInZero;
   if(abs(diff_current[1]) <= CCHK_CUR_OUT_VOLT_MAX_DEF) 
   {
     g_current_check.diff1=diff_current[1]; 
   } 
   if(abs(diff_current[0]) <= CCHK_CUR_OUT_VOLT_MAX_DEF) 
   {
     g_current_check.diff0=diff_current[0];
   }
   return RES_OK;
}

/*
*
* @brief        零飘校正值
* @note         无
* @param[in]    无  
* @param[out]   无
* @return       零飘校正值 
*
*/
CurrentCheck bcu_get_current_check_diff(void)
{
  CurrentCheck check;
  OS_CPU_SR cpu_sr = 0;
  OS_ENTER_CRITICAL();
  check = g_current_check;
  OS_EXIT_CRITICAL();
  return check; 
}

/*
*
* @brief        上电自动校准零飘
* @note         上电400MS以内回路有小于6A电流，则认为是零飘电流，启动零飘校正
* @param[in]    无  
* @param[out]   无
* @return       无
*
*/
static INT8U g_current_check_count=0,g_current_auto_check_flag=0;
void bcu_check_current_auto_to_zero_self(void)
{
   INT16S current = bcu_get_current(); 
   if(g_current_sample_context.current_auto_check_func == NULL) return;
   if(g_current_auto_check_flag == 1) return;
   if((get_tick_count() < BMS_CURRENT_AUTO_CHECK_DELAY) && (abs(current) < CURRENT_START_CHECK_TO_ZERO_CURRENT))
    {
      g_current_check_count++;
      if(g_current_check_count > 2) 
      {
        g_current_sample_context.current_auto_check_func();//current_sample_auto_check(bcu_update_current_check);
        g_current_auto_check_flag = 1; 
        g_current_check_count = 0; 
      }
    }
} 

/*
*
* @brief        主路电流传感器状态
* @note         无
* @param[in]    无  
* @param[out]   无
* @return       正常：0x00 CURRENT_NORMAL    没有连接：0x01 CURRENT_NO_CONNECT   超出量程范围：0x02 CURRENT_OVER_RANGE  
*
*/
INT8U current_signal_is_valid(void)
{
   return  g_current_sample_context.signal_invalid_flag;
}

/*
*
* @brief        第二路电流传感器状态
* @note         无
* @param[in]    无  
* @param[out]   无
* @return       正常：0x00 CURRENT_NORMAL    没有连接：0x01 CURRENT_NO_CONNECT   超出量程范围：0x02 CURRENT_OVER_RANGE  
*
*/
INT8U current1_signal_is_valid(void)
{
   return  g_current_sample_context.signal_invalid_flag1;
}

void current_sample_filter_max_min(INT16S* _PAGED data, INT8U num)
{
    INT8U min_index = 0, max_index;
    INT16S min_value = 32767, max_value = -32768;
    INT8U index;

    for (index = 0; index < num; ++index)
    {
        if (data[index] >= max_value)
        {
            max_index = index;
            max_value = data[index];
        }

        if (data[index] < min_value)
        {
            min_index = index;
            min_value = data[index];
        }
    }

    if (max_index < num - 2)
    {
        data[max_index] = data[num - 1];
        if (min_index == num - 1) min_index = max_index;
    }

    if (min_index < num - 2)
    {
        data[min_index] = data[num - 2];
    }
}


INT16U current_sample_average_filter(INT16S* _PAGED data, INT8U num)
{
    INT32S temp = 0;
    register INT8U index;

    if (num > g_current_sample_context.filter_num * 2)
    {
        index = g_current_sample_context.filter_num;
        while(index-- > 0)
        {
            current_sample_filter_max_min(data, num);
            num -= 2;
        }
    }

    for (index = 0; index < num;  index++)
    {
        temp += data[index];
    }
    temp = temp / num;
    return (INT16U)temp;
}
#endif
