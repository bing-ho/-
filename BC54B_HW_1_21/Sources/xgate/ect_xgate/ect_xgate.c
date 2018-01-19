#include "xgate.h"
#include "xgate.hxgate"
#include "ect_intermediate.h"

extern unsigned long ect_clock;


/***********************************************************************
  * @brief           ��ʼ��PWM��������
  * @param[in]       ��
  * @return          ��
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
  * @brief           ��ȡ����PWM��ռ�ձ�
  * @param[in]       pwm_data  CaptureSignal�ṹ���Ͳ���
  * @return          ռ�ձȴ�С
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

    if (h == 0) { // ȷ�� h+l != 0
        return 0;
    }
    return (unsigned short)((unsigned long)h * 1000 / (h + l));
}

/***********************************************************************
  * @brief           ��ȡ����PWM������
  * @param[in]       pwm_data  CaptureSignal�ṹ���Ͳ���
  * @return          ���ڴ�С
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
  * @brief           ��ȡ����PWM��Ƶ��
  * @param[in]       pwm_data  CaptureSignal�ṹ���Ͳ���
  * @return          Ƶ�ʴ�С
***********************************************************************/
unsigned long pwm_get_frequence( CaptureSignal *__FAR pwm_data) {
    unsigned long cycle = pwm_get_cycle(pwm_data);
    return ect_clock / cycle;    
}


