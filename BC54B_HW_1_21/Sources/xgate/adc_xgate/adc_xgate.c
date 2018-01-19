#include "xgate.h"
#include "adc_xgate.hxgate"
#include "adc_xgate.h"
#include "ucos_ii.h"

#include "ad_hardware.h"
#include "pit_hardware.h"
#include "derivative.h" 

/*
 xgate Э�鴦����֧��һ���ж�Ƕ�ף��жϼ���4��7�� ������ռ�жϼ��� 1��3�� �жϣ���adc0 ��adc1�Ĳ��������У�
 �й�ͬ���õĺ��������������ж�ʱ�� �жϼ���֮�䲻�ܴ�����ռ��ϵ
*/
/*
 ADC �ж�0����
*/
//#pragma push
//#pragma DATA_SEG __RPAGE_SEG XGATE_DATA

struct xadc0_xgate_data xadc0_data;
static OS_EVENT *finished_event;



//#pragma DATA_SEG DEFAULT
//#pragma pop



//����adc0 ���ò���
const AdcParam  adc0_param = 
{ 
 kAdcGroup0,          //�豸��
 kAdcChannel15,       //��ʼͨ��
 kAdc12bit,           //��������
 kAdcETRIG2,          //�ⲿ����Դ
 kAdcRising,          //�ⲿ�����ź�Դ
 kAdcRight,           //�������뷽ʽ
 kAdc12Cycles,        //����ʱ������
 2,                   //Ԥ��Ƶ
 kAdcSingle,          //������ʽ
 kAdcSingleChannel,   //��ͨ����ͨ������
}; 

//����д��main�ļ���û�е�����.h�ļ�
extern unsigned long BSP_BUS_ClkFreq(void);    

/***********************************************************************
  * @brief           ����pitģ�������ֵ
  * @param[in]       interval ��ʱʱ����λ����
  * @param[in]       pit_context PitContext��pit����
  * @return          ��
***********************************************************************/
void pit_time_base_computation(unsigned char interval,PitContext* pit_context) 
{
    unsigned long clk;
    unsigned long div;
    clk = BSP_BUS_ClkFreq();
    // PIT timer clock
    pit_context->TimerBase = (clk / 1000000) - 1; // 1M//����PITģ���е�8λ��������ֵ    ���һ��ʱ����1us  
    div = clk / (pit_context->TimerBase + 1);      //16λ������Ƶ��
    div = div * interval / 1000 - 1;        //��ʱ1ms�� ��������
    if (div > 65535) {                             //����������ֵ
        
        pit_context->TimerBase = 255;             //΢��ʱ�� ʹ��������ֵ
        div = clk / (pit_context->TimerBase + 1);
        div = div * interval / 1000 - 1;   //���¼��� 16λ����Ƶ��
        if(div>65535)div=65535;             //���ʱ��
        //Hanhui  PITMUX_PMUX0 = 1; // ͨ��0ѡ��ʱ��1
        pit_context->mt_dev=kPitMtDev1;
    } else {
        //Hanhui  PITMUX_PMUX0 = 0; // ͨ��0ѡ��ʱ��0
        pit_context->mt_dev=kPitMtDev0;
    }

     pit_context->Timer = (unsigned short)div;//����PITģ���е�16λ��������ֵ
}
  
  
/***********************************************************************
  * @brief           adc0�ⲿ�жϳ�ʼ����ʹ���ⲿ����pit0�жϳ�ʼ��
  * @param[in]       ��
  * @return          ��
***********************************************************************/
char xadc0_init(void) 
{
    finished_event = OSSemCreate(0);        //�����ź���
    // Channel 69 - ATD0 to XGATE
    interrupt_route_request(0x69, 1, 1);    //adc0 ʹ��Э�������ж�
    // Channel 0x3D - Periodic Interrupt Timer to XGATE
    interrupt_route_request(0x3D, 1, 2);    //pit0 ʹ��Э�������ж�

   
    PITCFLMT_PITE = 1; // ������ʱ���ܿ���
    
  

    return 1;
}


/***********************************************************************
  * @brief           adc0ת����ʼ�� 
  * @param[in]       param  xadc0_param�ṹ���ͱ���
  * @return          1: Ok   0: Err
***********************************************************************/
char xadc0_start(const struct xadc0_param *__FAR param) {
    INT8U err;
    
    //pit0
    PitContext pit0_param =
   { 
      kPitMtDev0,  //΢��ʱ��ͨ����
      kPitDev0,    //��ʱ��ͨ����   
      255,  //PITģ���е�8λ��������ֵ                    
      65535    //PITģ���е�16λ��������ֵ
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

    (void)Adc_HardwareInit(&adc0_param); //��ʼ��adc0����
    //(void)Adc_SetTrigger(adc0_param->dev,1);
    
    
    pit_time_base_computation(xadc0_data.param.interval,&pit0_param); //�����������
    
    (void)Pit_Hardware_Init(&pit0_param);  //��ʼ����ʱ��ͨ������
    
    Pit_MicroTimer_Init(&pit0_param) ;    //��ʼ��������ʱ������ֵ
    
    //��������־λ Ĭ��Ϊ0
    Pit_Hardware_Clear_Time_out_Flag(kPitDev0);  //����жϱ�־λ
    // �رն�ʱ��ͨ��0
    Pit_Hardware_Stop(kPitDev0); 
    
    (void)Adc_StartConversion(pit0_param.dev,xadc0_data.param.channel);//����ת��  
    //���õ�������ģʽ
    Pit_Hardware_down_counter(kPitDev0,1);
     //������ʱ��ͨ��0
    Pit_Hardware_Start(kPitDev0);
    return 1;
}

/***********************************************************************
  * @brief           ��ȡadc0ת���ṹ 
  * @param[in]       timeout_tick  ��ʱʱ��
  * @param[in]       result        ת���ṹ
  * @return          1��  Ok   0: Err
***********************************************************************/
char xadc0_wait(uint16_t *near result, unsigned short timeout_tick) {
    INT8U err;
    OSSemPend(finished_event, timeout_tick, &err);//�ȴ�finished_event�ź���������ʱ��timeout_tick
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
  * @brief           adc0 hc12�жϴ���
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void xadc0_adc_isr(void) {
    XGIF_6F_60 = 1UL << 9; // ����ж�.
    xadc0_data.param.times = 0;
    (void)OSSemPost(finished_event);
}

/*
 ADC �ж�1����soc  adת��
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
 

//����adc1 ���ò���
const AdcParam  adc1_param = 
{ 
 kAdcGroup1,        //�豸��
 kAdcChannel15,     //��ʼͨ��
 kAdc12bit,         //��������
 kAdcETRIG3,        //�ⲿ����Դ
 kAdcNoTriggerSignal,        //�ⲿ�����ź�Դ
 kAdcRight,         //�������뷽ʽ
 kAdc12Cycles,      //����ʱ������
 2,                 //Ԥ��Ƶ
 kAdcSingle,        //������ʽ
 kAdcSingleChannel, //��ͨ����ͨ������ 
 
}; 

/***********************************************************************
  * @brief           adc1 �жϳ�ʼ��
  * @param[in]       ��
  * @return          1: ok
***********************************************************************/
char soc_adc_init(void) {
    adc1_finished_event = OSSemCreate(0);
    adc1_finished_event_x = OSSemCreate(0);
    // Channel 38 - XGATE Software Trigger 1
    interrupt_route_request(0x38, 1, 2);       //xgate����ж�1
    // Channel 68 - ATD1 to XGATE
    interrupt_route_request(0x68, 1, 2);       //adc1�ж�ʹ��Э�����ж�
    // Channel 0x3C - Periodic Interrupt Timer to XGATE
    interrupt_route_request(0x3C, 1, 2);       //PIT1�ж�ʹ��Э�����ж�

    // pit ��������ʹ��
    PITCFLMT_PITE = 1;

    return 1;
}



/***********************************************************************
  * @brief           adc1 �жϳ�ʼ��
  * @param[in]       ��
  * @return          1: ok
***********************************************************************/
static void adc1_init(void) 
{ 
  //pit1
  PitContext pit1_param =
  { 
      kPitMtDev1,  //΢��ʱ��ͨ����
      kPitDev1,    //��ʱ��ͨ����   
      255,  //PITģ���е�8λ��������ֵ                    
      65535    //PITģ���е�16λ��������ֵ
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

    // ʹ�ü���ж�ͨ��1.
    // �����ж�ʱ��.
    (void)Pit_Hardware_Init(&pit1_param);  //��ʼ����ʱ��ͨ������
    
    Pit_MicroTimer_Init(&pit1_param) ;    //��ʼ��������ʱ������ֵ

    // ��������.
    XGSWT = 0x0202;    
}


/***********************************************************************
  * @brief           soc  adcת����ʼ��
  * @param[in]       param           soc_adc_param�ṹ�����ͱ���      
  * @param[in]       independence    �����ɼ�
  * @return          1: ok
***********************************************************************/
static char soc_adc_start_x(const struct soc_adc_param *__FAR param, unsigned char independence) {

    if (!param) return 0;

    if (SOC_ADC_SAMPLE_MAX_TIMES < param->times) return 0;
    if (param->drop_high + param->drop_low >= param->times) return 0;

    do {
        CPU_SET_SEM(SOC_ADC_SEM_NUM);
    } while (!CPU_SEM_IS_SET(SOC_ADC_SEM_NUM));

   // if (soc_xadc1_data.param.times > 0) {    //������ ��ʼ��һ��֮�� �ٴγ�ʼ��adc1 ʧ��
   if (soc_xadc1_data.param.times > SOC_ADC_SAMPLE_MAX_TIMES) {      //liqing ��
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
  * @brief           soc  adcת����ʼ���ṹ����
  * @param[in]       param           soc_adc_param�ṹ�����ͱ��� 
  * @return          1: ok
***********************************************************************/
char soc_adc_start(const struct soc_adc_param *__FAR param) {
    return soc_adc_start_x(param, 0);
}

/***********************************************************************
  * @brief           soc  adcת����ʼ���ṹ����������ʽ
  * @param[in]       param           soc_adc_param�ṹ�����ͱ��� 
  * @return          1: ok
***********************************************************************/
char soc_adc_start_independence(const struct soc_adc_param *__FAR param) {
    return soc_adc_start_x(param, 1);
}

/***********************************************************************
  * @brief           ֹͣadcת��
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void soc_adc_stop() {
    INT8U err;
    OSSemSet(finished_event, 0, &err);
    // �رն�ʱ��.
    PITCE_PCE1 = 0;
    PITINTE_PINTE1 = 0;
    do {
        CPU_SET_SEM(SOC_ADC_SEM_NUM);
    } while (!CPU_SEM_IS_SET(SOC_ADC_SEM_NUM));

    soc_xadc1_data.param.times = 0;
    CPU_RELEASE_SEM(SOC_ADC_SEM_NUM);
}

/***********************************************************************
  * @brief           ��ȡadc1 ת��ֵ ��ȡ��ʽ��ʱ��ȡ
  * @param[in]       high   ת�����
  * @param[in]       low    ת�����
  * @param[in]       single ת�����
  * @param[in]       t      �ȴ�ʱ��
  * @return          1��ok  0��err
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
  * @brief           ��ȡadc1 ת��ֵ ��ͨ����ʱ��ȡ��ʽ
  * @param[in]       channelת��ͨ��
  * @param[in]       value  ת�����
  * @param[in]       t      �ȴ�ʱ��
  * @return          1��ok  0��err
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
  * @brief           ��ȡadc1 ת��ֵ�ӿں��� ��ȡ��ʽ�ȴ������ź�
  * @param[in]       high�� low��  singleת�����
  * @return          1��ok  0��err
***********************************************************************/
char soc_wait(uint16_t *near high, uint16_t *near low, uint16_t *near single) {
    return soc_wait_timeout(high, low,single, 0); 
}

/***********************************************************************
  * @brief           soc �ź���ֵ����
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void soc_clear_result(void) {
    INT8U err;
    OSSemSet(adc1_finished_event, 0, &err);
    if (is_independence) {
        OSSemSet(adc1_finished_event_x, 0, &err);
    }
}

/***********************************************************************
  * @brief           adc1�ж�ʹ�� hc12x��ʽ
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void soc_adc1_isr(void) {
    //XGIF_6F_60_XGIF_68 = 1; // ����ж�.
    XGIF_6F_60 = 1UL << 8; // ����ж�.
    (void)OSSemPost(adc1_finished_event);
    if (is_independence) {
        (void)OSSemPost(adc1_finished_event_x);
    }
}
