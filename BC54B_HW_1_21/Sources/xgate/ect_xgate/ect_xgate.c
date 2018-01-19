#include "xgate.h"
#include "xgate.hxgate"
#include "ect_intermediate.h"

extern unsigned long ect_clock;


/***********************************************************************
  * @brief           初始化PWM捕获设置
  * @param[in]       无
  * @return          无
***********************************************************************/
void pwm_init(void) {
    // Channel 6F - Enhanced Capture Timer overflow to XGATE
    //interrupt_route_request(0x6F, 1, 2);
    
    (void)Ect_Tcnt_Init(&count_ect_context,1,2);
    // Channel 39 - XGATE Software Trigger 0 to XGATE
    interrupt_route_request(0x39, 1, 1);
    // Trigger Soft 0 to invoke init
    XGSWT = 0x0101;

}

/***********************************************************************
  * @brief           获取输入PWM的占空比
  * @param[in]       pwm_data  CaptureSignal结构体型参数
  * @return          占空比大小
***********************************************************************/
unsigned short pwm_get_duty( CaptureSignal *__FAR pwm_data) {
    unsigned long h;
    unsigned long l;

    do {
        CPU_SET_SEM(PWM_SEM_NUM);
    } while (!CPU_SEM_IS_SET(PWM_SEM_NUM));

    if ((pwm_data->high_cnt & 0xFF000000) || (pwm_data->low_cnt & 0xFF000000)) {
        h = pwm_data->high_cnt >> 12;
        l = pwm_data->low_cnt >> 12;
    } else if ((pwm_data->high_cnt & 0x00F00000) || (pwm_data->low_cnt & 0x00F00000)) {
        h = pwm_data->high_cnt >> 4;
        l = pwm_data->low_cnt >> 4;
    } else {
        h = pwm_data->high_cnt;
        l = pwm_data->low_cnt;
    }

    CPU_RELEASE_SEM(PWM_SEM_NUM);

    if (h == 0) { // 确保 h+l != 0
        return 0;
    }
    return (unsigned short)((unsigned long)h * 1000 / (h + l));
}

/***********************************************************************
  * @brief           获取输入PWM的周期
  * @param[in]       pwm_data  CaptureSignal结构体型参数
  * @return          周期大小
***********************************************************************/
unsigned long pwm_get_cycle( CaptureSignal *__FAR pwm_data) {
    unsigned long cycle;
    do {
        CPU_SET_SEM(PWM_SEM_NUM);
    } while (!CPU_SEM_IS_SET(PWM_SEM_NUM));
    cycle =  pwm_data->high_cnt + pwm_data->low_cnt;
    if (cycle < pwm_data->high_cnt) {
        cycle = 0xFFFFFFFF;
    } else if (cycle < pwm_data->low_cnt) {
        cycle = 0xFFFFFFFF;
    }
    CPU_RELEASE_SEM(PWM_SEM_NUM);
    
    return cycle;
}

/***********************************************************************
  * @brief           获取输入PWM的频率
  * @param[in]       pwm_data  CaptureSignal结构体型参数
  * @return          频率大小
***********************************************************************/
unsigned long pwm_get_frequence( CaptureSignal *__FAR pwm_data) {
    unsigned long cycle = pwm_get_cycle(pwm_data);
    return ect_clock / cycle;    
}


