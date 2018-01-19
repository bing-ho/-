#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "stdint.h"
#include "adc_xgate.h"
#include "bms_bcu.h"
#include "bms_soc_impl.h"

#include "ads1015_intermediate.h"

const char shell_help_hall[] = "\n\
		读取HALL电池传感器的输出电压(mv)\n\
";

const char shell_summary_hall[] =
    "HALL电流传感器";
    
    
    
    /* bms_get_ad1_value_h(void)
{
    return ad1_high_value;                                     
}

INT16U bms_get_ad1_value_l(void) 
{
    return ad1_low_value;
}
INT16U bms_get_ad1_value_single(void) 
{
    return ad1_single_value;
}
    */
static const struct soc_adc_param param = {
    2, // 1ms
    40, // 40次滤波
    10, // 丢弃10个最大值
    10, // 丢弃10个最小值
};

void test_hall_init(void) {
    (void)soc_adc_init();
}

static int dump(void) {
    uint16_t  high, low, c3;
    float vhigh, vlow, vc3;
    uint16_t vref;
#if BMS_SUPPORT_BY5248D == 0
    if(hardware_io_revision_get() >= HW_VER_120)
    {
        vref = 4096;
    }
    else
    {
        vref = 5000;
    }
#else
    vref = 4096;
#endif    
    if(!soc_adc_start(&param)) {
        return -1;
    }
    
    soc_clear_result();
    if (!soc_wait_timeout(&high, &low, &c3,200)) {
        soc_adc_stop();
        return -1;
    }
    soc_adc_stop();
    
    vhigh = high*1.1;
    vlow = low*1.1;
    vc3 = c3*2.26-vref;
    (void)printf("HALL_C1:%d\nHALL_C2:%d\nHALL_C3:%d\n", (int)vlow, (int)vhigh,(int)vc3);
    return 0;
}
     

extern void print_arg_err_hint(char *arg);
int shell_func_hall(int argc, char **argv) {
    if (argc == 1) { // 读取RTC时间
    //(void)printf("HALL_C1:%d\n", bms_get_ad1_value_l());
        return dump();
        //(void)printf("HALL_C1:%d\n", bms_get_ad1_value_l());
    }

   print_arg_err_hint(argv[0]);
   return -1;
}
