#include "xgate.h"
#include "adc_xgate.hxgate"
#include "adc_xgate.h"
#include "ucos_ii.h"

#include "ad_hardware.h"
#include "pit_hardware.h"
#include "derivative.h" 

/*
 xgate 协议处理器支持一级中断嵌套，中断级别4到7的 可以抢占中断级别 1到3的 中断，在adc0 和adc1的操作过程中，
 有共同调用的函数，所以设置中断时候 中断级别之间不能存在抢占关系
*/
/*
 ADC 中断0处理
*/
//#pragma push
//#pragma DATA_SEG __RPAGE_SEG XGATE_DATA

struct xadc0_xgate_data xadc0_data;
static OS_EVENT *finished_event;



//#pragma DATA_SEG DEFAULT
//#pragma pop



//定义adc0 配置参数
const AdcParam  adc0_param = 
{ 
 kAdcGroup0,          //设备号
 kAdcChannel15,       //起始通道
 kAdc12bit,           //采样精度
 kAdcETRIG2,          //外部触发源
 kAdcRising,          //外部触发信号源
 kAdcRight,           //采样对齐方式
 kAdc12Cycles,        //采样时钟周期
 2,                   //预分频
 kAdcSingle,          //采样方式
 kAdcSingleChannel,   //多通道或单通道采样
}; 

//函数写在main文件中没有单独的.h文件
extern unsigned long BSP_BUS_ClkFreq(void);    

/***********************************************************************
  * @brief           计算pit模块计数初值
  * @param[in]       interval 定时时长单位毫秒
  * @param[in]       pit_context PitContext型pit配置
  * @return          无
***********************************************************************/
void pit_time_base_computation(unsigned char interval,PitContext* pit_context) 
{
    unsigned long clk;
    unsigned long div;
    clk = BSP_BUS_ClkFreq();
    // PIT timer clock
    pit_context->TimerBase = (clk / 1000000) - 1; // 1M//设置PIT模块中的8位计数器初值    溢出一次时间是1us  
    div = clk / (pit_context->TimerBase + 1);      //16位计数器频率
    div = div * interval / 1000 - 1;        //定时1ms的 计数次数
    if (div > 65535) {                             //超过最大计数值
        
        pit_context->TimerBase = 255;             //微定时器 使用最大计数值
        div = clk / (pit_context->TimerBase + 1);
        div = div * interval / 1000 - 1;   //重新计算 16位计数频率
        if(div>65535)div=65535;             //最大时长
        //Hanhui  PITMUX_PMUX0 = 1; // 通道0选择时基1
        pit_context->mt_dev=kPitMtDev1;
    } else {
        //Hanhui  PITMUX_PMUX0 = 0; // 通道0选择时基0
        pit_context->mt_dev=kPitMtDev0;
    }

     pit_context->Timer = (unsigned short)div;//设置PIT模块中的16位计数器初值
}
  
  
/***********************************************************************
  * @brief           adc0外部中断初始化，使用外部触发pit0中断初始化
  * @param[in]       无
  * @return          无
***********************************************************************/
char xadc0_init(void) 
{
    finished_event = OSSemCreate(0);        //创建信号量
    // Channel 69 - ATD0 to XGATE
    interrupt_route_request(0x69, 1, 1);    //adc0 使用协处理器中断
    // Channel 0x3D - Periodic Interrupt Timer to XGATE
    interrupt_route_request(0x3D, 1, 2);    //pit0 使用协处理器中断

   
    PITCFLMT_PITE = 1; // 开启定时器总开关
    
  

    return 1;
}


/***********************************************************************
  * @brief           adc0转化初始化 
  * @param[in]       param  xadc0_param结构类型变量
  * @return          1: Ok   0: Err
***********************************************************************/
char xadc0_start(const struct xadc0_param *__FAR param) {
    INT8U err;
    
    //pit0
    PitContext pit0_param =
   { 
      kPitMtDev0,  //微定时器通道号
      kPitDev0,    //定时器通道号   
      255,  //PIT模块中的8位计数器初值                    
      65535    //PIT模块中的16位计数器初值
   };
 
    OS_INIT_CRITICAL();

    if (!param) return 0;

    if (xadc0_data.param.times != 0) return 0;


    if (XADC0_SAMPLE_MAX_TIMES < param->times) return 0;
    if (param->drop_high + param->drop_low >= param->times) return 0;


    OS_ENTER_CRITICAL();
    if (XADC0_SAMPLE_MAX_TIMES < param->times) {
        OS_EXIT_CRITICAL();
        return 0;
    }
    xadc0_data.param.interval = param->interval;
    xadc0_data.param.times = param->times;
    xadc0_data.param.drop_high = param->drop_high;
    xadc0_data.param.drop_low = param->drop_low;
    xadc0_data.param.channel = param->channel;
    xadc0_data.left_times = param->times - param->drop_high - param->drop_low;
    xadc0_data.sample_index = 0;


    OS_EXIT_CRITICAL();

    OSSemSet(finished_event, 0, &err);

    (void)Adc_HardwareInit(&adc0_param); //初始化adc0参数
    //(void)Adc_SetTrigger(adc0_param->dev,1);
    
    
    pit_time_base_computation(xadc0_data.param.interval,&pit0_param); //计算采样周期
    
    (void)Pit_Hardware_Init(&pit0_param);  //初始化定时器通道参数
    
    Pit_MicroTimer_Init(&pit0_param) ;    //初始化基础定时器计数值
    
    //清除溢出标志位 默认为0
    Pit_Hardware_Clear_Time_out_Flag(kPitDev0);  //清除中断标志位
    // 关闭定时器通道0
    Pit_Hardware_Stop(kPitDev0); 
    
    (void)Adc_StartConversion(pit0_param.dev,xadc0_data.param.channel);//开启转换  
    //设置倒计数器模式
    Pit_Hardware_down_counter(kPitDev0,1);
     //开启定时器通道0
    Pit_Hardware_Start(kPitDev0);
    return 1;
}

/***********************************************************************
  * @brief           获取adc0转换结构 
  * @param[in]       timeout_tick  超时时间
  * @param[in]       result        转换结构
  * @return          1：  Ok   0: Err
***********************************************************************/
char xadc0_wait(uint16_t *near result, unsigned short timeout_tick) {
    INT8U err;
    OSSemPend(finished_event, timeout_tick, &err);//等待finished_event信号量，阻塞时间timeout_tick
    if (err != OS_ERR_NONE) {
        Pit_Hardware_Stop(kPitDev0);
        Pit_Hardware_Clear_Time_out_Flag(kPitDev0);
        xadc0_data.param.times = 0;
        return 0;
    }
    *result = xadc0_data.result;
    return 1;
}

/***********************************************************************
  * @brief           adc0 hc12中断处理
  * @param[in]       无
  * @return          无
***********************************************************************/
void xadc0_adc_isr(void) {
    XGIF_6F_60 = 1UL << 9; // 清除中断.
    xadc0_data.param.times = 0;
    (void)OSSemPost(finished_event);
}

/*
 ADC 中断1处理soc  ad转换
*/

//#pragma push 
//#pragma DATA_SEG __RPAGE_SEG XGATE_DATA
//struct soc_adc_param soc_adc_param;
struct soc_adc_xgate_data soc_xadc1_data;
unsigned char is_independence = 0;
//unsigned char left_times;
static OS_EVENT *adc1_finished_event;
static OS_EVENT *adc1_finished_event_x;
//#pragma DATA_SEG DEFAULT
//#pragma pop
 

//定义adc1 配置参数
const AdcParam  adc1_param = 
{ 
 kAdcGroup1,        //设备号
 kAdcChannel15,     //起始通道
 kAdc12bit,         //采样精度
 kAdcETRIG3,        //外部触发源
 kAdcNoTriggerSignal,        //外部触发信号源
 kAdcRight,         //采样对齐方式
 kAdc12Cycles,      //采样时钟周期
 2,                 //预分频
 kAdcSingle,        //采样方式
 kAdcSingleChannel, //多通道或单通道采样 
 
}; 

/***********************************************************************
  * @brief           adc1 中断初始化
  * @param[in]       无
  * @return          1: ok
***********************************************************************/
char soc_adc_init(void) {
    adc1_finished_event = OSSemCreate(0);
    adc1_finished_event_x = OSSemCreate(0);
    // Channel 38 - XGATE Software Trigger 1
    interrupt_route_request(0x38, 1, 2);       //xgate软件中断1
    // Channel 68 - ATD1 to XGATE
    interrupt_route_request(0x68, 1, 2);       //adc1中断使用协处理中断
    // Channel 0x3C - Periodic Interrupt Timer to XGATE
    interrupt_route_request(0x3C, 1, 2);       //PIT1中断使用协处理中断

    // pit 计数允许使能
    PITCFLMT_PITE = 1;

    return 1;
}



/***********************************************************************
  * @brief           adc1 中断初始化
  * @param[in]       无
  * @return          1: ok
***********************************************************************/
static void adc1_init(void) 
{ 
  //pit1
  PitContext pit1_param =
  { 
      kPitMtDev1,  //微定时器通道号
      kPitDev1,    //定时器通道号   
      255,  //PIT模块中的8位计数器初值                    
      65535    //PIT模块中的16位计数器初值
  };

   #if 0
    // 12bit
    // ETRIG3
    ATD1CTL1 = 0xD3;
    // ATD Fast Flag Clear All
    // Enable external trigger
    // Rising edge trigger
    // ATD Sequence Complete Interrupt Enable
    ATD1CTL2 = 0x4E;
    // Right justified data in the result registers.
    // in Freeze Mode: Finish current conversion, then freeze
    ATD1CTL3 = 0x82;
    // ADTCLK
    ATD1CTL4 = 0x22;
    #endif 
    
    (void)Adc_HardwareInit(&adc1_param);  
//    (void)Adc_SetTrigger(param->dev,0);      
    
    pit_time_base_computation(soc_xadc1_data.param.interval,&pit1_param);

    // 使用间隔中断通道1.
    // 设置中断时间.
    (void)Pit_Hardware_Init(&pit1_param);  //初始化定时器通道参数
    
    Pit_MicroTimer_Init(&pit1_param) ;    //初始化基础定时器计数值

    // 触发启动.
    XGSWT = 0x0202;    
}


/***********************************************************************
  * @brief           soc  adc转换初始化
  * @param[in]       param           soc_adc_param结构体类型变量      
  * @param[in]       independence    独立采集
  * @return          1: ok
***********************************************************************/
static char soc_adc_start_x(const struct soc_adc_param *__FAR param, unsigned char independence) {

    if (!param) return 0;

    if (SOC_ADC_SAMPLE_MAX_TIMES < param->times) return 0;
    if (param->drop_high + param->drop_low >= param->times) return 0;

    do {
        CPU_SET_SEM(SOC_ADC_SEM_NUM);
    } while (!CPU_SEM_IS_SET(SOC_ADC_SEM_NUM));

   // if (soc_xadc1_data.param.times > 0) {    //此条件 初始化一次之后 再次初始化adc1 失败
   if (soc_xadc1_data.param.times > SOC_ADC_SAMPLE_MAX_TIMES) {      //liqing 改
        CPU_RELEASE_SEM(SOC_ADC_SEM_NUM);
        return 0;
    }
    
    is_independence = independence;

    soc_xadc1_data.param.interval = param->interval;
    soc_xadc1_data.param.times = param->times;
    soc_xadc1_data.param.drop_high = param->drop_high;
    soc_xadc1_data.param.drop_low = param->drop_low;
    soc_xadc1_data.left_times = param->times - param->drop_high - param->drop_low;
    soc_xadc1_data.sample_index = 0;

    CPU_RELEASE_SEM(SOC_ADC_SEM_NUM);

    adc1_init();

    return 1;
}

/***********************************************************************
  * @brief           soc  adc转换初始化结构函数
  * @param[in]       param           soc_adc_param结构体类型变量 
  * @return          1: ok
***********************************************************************/
char soc_adc_start(const struct soc_adc_param *__FAR param) {
    return soc_adc_start_x(param, 0);
}

/***********************************************************************
  * @brief           soc  adc转换初始化结构函数独立方式
  * @param[in]       param           soc_adc_param结构体类型变量 
  * @return          1: ok
***********************************************************************/
char soc_adc_start_independence(const struct soc_adc_param *__FAR param) {
    return soc_adc_start_x(param, 1);
}

/***********************************************************************
  * @brief           停止adc转换
  * @param[in]       无
  * @return          无
***********************************************************************/
void soc_adc_stop() {
    INT8U err;
    OSSemSet(finished_event, 0, &err);
    // 关闭定时器.
    PITCE_PCE1 = 0;
    PITINTE_PINTE1 = 0;
    do {
        CPU_SET_SEM(SOC_ADC_SEM_NUM);
    } while (!CPU_SEM_IS_SET(SOC_ADC_SEM_NUM));

    soc_xadc1_data.param.times = 0;
    CPU_RELEASE_SEM(SOC_ADC_SEM_NUM);
}

/***********************************************************************
  * @brief           获取adc1 转换值 获取方式延时获取
  * @param[in]       high   转换结果
  * @param[in]       low    转换结果
  * @param[in]       single 转换结果
  * @param[in]       t      等待时长
  * @return          1：ok  0：err
***********************************************************************/
char soc_wait_timeout(uint16_t *near high, uint16_t *near low, uint16_t *near single, unsigned short t) {
    INT8U err;
    
    if (is_independence) {
        return 0;
    }

    OSSemPend(adc1_finished_event, t, &err);
    if (err != OS_ERR_NONE) {
        adc1_init();
        return 0;
    }
    
    OSSemSet(adc1_finished_event, 0, &err);

    do {
        CPU_SET_SEM(SOC_ADC_SEM_NUM);
    } while (!CPU_SEM_IS_SET(SOC_ADC_SEM_NUM));

    *high = soc_xadc1_data.high;
    *low = soc_xadc1_data.low;
    *single = soc_xadc1_data.single;
    CPU_RELEASE_SEM(SOC_ADC_SEM_NUM);

    return 1;
}

/***********************************************************************
  * @brief           获取adc1 转换值 按通道延时获取方式
  * @param[in]       channel转换通道
  * @param[in]       value  转换结果
  * @param[in]       t      等待时长
  * @return          1：ok  0：err
***********************************************************************/
char soc_wait_channel_timeout(uint8_t channel, uint16_t *near value, unsigned short t) {
    INT8U err;
    
    OS_EVENT *event;
    
    if (!is_independence) {
        return 0;
    }
   
    if (channel == 0) event = adc1_finished_event;
    else if (channel == 1) event = adc1_finished_event_x;
    else return 0;

    OSSemPend(event, t, &err);
    if (err != OS_ERR_NONE) {
        adc1_init();
        return 0;
    }
    
    OSSemSet(event, 0, &err);

    do {
        CPU_SET_SEM(SOC_ADC_SEM_NUM);
    } while (!CPU_SEM_IS_SET(SOC_ADC_SEM_NUM));

    *value = channel == 0 ? soc_xadc1_data.high : soc_xadc1_data.low;
    CPU_RELEASE_SEM(SOC_ADC_SEM_NUM);

    return 1;
}

/***********************************************************************
  * @brief           获取adc1 转换值接口函数 获取方式等待阻塞信号
  * @param[in]       high、 low、  single转换结果
  * @return          1：ok  0：err
***********************************************************************/
char soc_wait(uint16_t *near high, uint16_t *near low, uint16_t *near single) {
    return soc_wait_timeout(high, low,single, 0); 
}

/***********************************************************************
  * @brief           soc 信号量值清零
  * @param[in]       无
  * @return          无
***********************************************************************/
void soc_clear_result(void) {
    INT8U err;
    OSSemSet(adc1_finished_event, 0, &err);
    if (is_independence) {
        OSSemSet(adc1_finished_event_x, 0, &err);
    }
}

/***********************************************************************
  * @brief           adc1中断使用 hc12x方式
  * @param[in]       无
  * @return          无
***********************************************************************/
void soc_adc1_isr(void) {
    //XGIF_6F_60_XGIF_68 = 1; // 清除中断.
    XGIF_6F_60 = 1UL << 8; // 清除中断.
    (void)OSSemPost(adc1_finished_event);
    if (is_independence) {
        (void)OSSemPost(adc1_finished_event_x);
    }
}
