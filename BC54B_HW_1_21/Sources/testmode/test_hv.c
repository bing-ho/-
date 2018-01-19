#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"

#include "ads1015_intermediate.h"
#include "test_io.h"
#include "ucos_ii.h"
#include "hcf4052_intermediate.h"


struct gain_info {
    const char *gain_str;
    float gain;
    enum hcf4052_channel channel;
};

static const struct gain_info gain_table[] = {
    {"0", 37.1, HCF4052_CHANNEL_0},
    {"1", 20.7, HCF4052_CHANNEL_1},
    {"2", 13.0, HCF4052_CHANNEL_2},
    {"3", 8.20, HCF4052_CHANNEL_3},
    {"none", 5.00, HCF4052_CHANNEL_NONE},
};

static const struct gain_info *from_gain_string(const char *s) {
    uint8_t i;
    for (i = 0; i < sizeof(gain_table) / sizeof (gain_table[0]); ++i) {
        if (strcmp(s, gain_table[i].gain_str) == 0) {
            return &gain_table[i];
        }
    }
    return NULL;
}


const char shell_help_insuhv[] = "\n\
	gain_switch pmos_switch nmos_switch [detail]\n\
	gain_switch: 增益选择, 可选值0, 1, 2, 3, none, 分别对应增益:37.1, 20.7, 13.0, 8.2, 5.0\n\
	pmos_switch: PMOS控制, 可选值on, off\n\
	nmos_switch: NMOS控制, 可选值on, off\n\
	detail: 可选参数, 0或1, 是否详细输出\n\
	读2个通道的采集到的绝缘高压\n\
";

const char shell_summary_insuhv[] =
    "绝缘高压";


extern uint8_t adc_onchip_read_with_name(const char *name, uint16_t *v);

extern void print_arg_err_hint(char *arg);
int shell_func_insuhv(int argc, char **argv) {
    int rc = -1;
    uint8_t detail = 0;
    const struct gain_info *gain_info;
    uint8_t pmos_on, nmos_on;
    uint16_t hvp, hvn;
    float v;

    if (argc != 4 && argc != 5) {
        goto __error_args;
    }

    if (argc == 5) {
        if (strcmp(argv[4], "1") == 0) {
            detail = 1;
        } else if (strcmp(argv[4], "0") == 0) {
            detail = 0;
        } else {
            goto __error_args;
        }
    }

    gain_info = from_gain_string(argv[1]);
    if (NULL == gain_info) {
        goto __error_args;
    }

    if (strcmp(argv[2], "on") == 0) {
        pmos_on = 1;
    } else if (strcmp(argv[2], "off") == 0) {
        pmos_on = 0;
    } else {
        goto __error_args;      
    }

    if (strcmp(argv[3], "on") == 0) {
        nmos_on = 1;
    } else if (strcmp(argv[3], "off") == 0) {
        nmos_on = 0;
    } else {
        goto __error_args;
    }


    hcf4052_select_channel(hcf4052_impl, gain_info->channel);


    (void)output_ctrl_set_with_name("CTRL_HV+", pmos_on ? 1 : 0);
    (void)output_ctrl_set_with_name("CTRL_HV-", nmos_on ? 1 : 0);


    OSTimeDly(200);
    if (!adc_onchip_read_with_name("IMD_HV+", &hvp)) {
        rc = -2;
        goto __error;
    }

    if (!adc_onchip_read_with_name("IMD_HV-", &hvn)) {
        rc = -3;
        goto __error;
    }



    (void)output_ctrl_set_with_name("CTRL_HV+", 0);
    (void)output_ctrl_set_with_name("CTRL_HV-", 0);

    v = hvp + hvn;
#if BMS_SUPPORT_BY5248D == 0
    v = v * ((71.5 * 6 + 0.39) / 0.39);
    v = v / gain_info->gain;
#else
#if BMS_SUPPORT_BY5248D_1_1A == 0
    v =  v * 259 / 4; 
#else
    v =  v * 150;
#endif
#endif    
    if (detail) {
        (void)printf("IMD_HV+:%d\n", hvp);
        (void)printf("IMD_HV-:%d\n", hvn);
        (void)printf("GAIN:%.6f\n", gain_info->gain);
        (void)printf("INSUHV:%.0f\n", v);
    } else {
        (void)printf("%.0f\n", v);
    }


    return 0;

__error_args:
    print_arg_err_hint(argv[0]);
__error:
    (void)output_ctrl_set_with_name("CTRL_HV+", 0);
    (void)output_ctrl_set_with_name("CTRL_HV-", 0);

    return rc;
}




const char shell_help_hv[] = "\n\
	读三个通道的采集到的高压\n\
";

const char shell_summary_hv[] =
    "高压采集";


struct name_channel_map {
    const char *name;
    uint8_t channel;
};

static const struct name_channel_map name_channel_maps[] = {
    { "PCH", ADS1015_CHANNEL_0 },
    { "PRE", ADS1015_CHANNEL_1 },
    { "BAT", ADS1015_CHANNEL_2 },
    { "HV3", ADS1015_CHANNEL_3 }
};
#if BMS_SUPPORT_BY5248D == 0
static int __read_all(void) {
    uint8_t i;
    uint16_t v[4];
    
    for (i = 0; i < 4 ; ++i) {
        v[i] = ADS1015_SingleConvert(&g_ADS1015_BspInterface, name_channel_maps[i].channel, ADS1015_FULL_SCALE_4096_MV);
        if ((v[i] <= ADS1015_RESULT_ERROR)&&(v[i] >= (ADS1015_RESULT_ERROR-5))) {
            return (ADS1015_RESULT_ERROR-v[i]+1);
        }
    }
    for (i = 0; i < 4; ++i) {
        float fv;
        signed short vv = ADS1015_ResultToVoltageMv(v[i], ADS1015_FULL_SCALE_4096_MV);
        //(void)printf("%v ADC: %d\n", vv);
    #if BMS_SUPPORT_NEGATIVE_ADHESION
       if(i == 2) {
            vv = vv-420;
        }
    #endif
        fv = (float)vv * ((510 * 5 + 10) / 10);
        (void)printf("%s:%.0f\n", name_channel_maps[i].name, fv);     //1.23中需将第三个值*0.9526
    }
}
#else  //高精度一体机
#if BMS_SUPPORT_BY5248D_1_1A == 0
static int __read_all(void) {
    uint8_t i;
    uint16_t v[4];
    for (i = 0; i < 4 ; ++i) {
        v[i] = ADS1015_SingleConvert(&g_ADS1015_BspInterface, name_channel_maps[i].channel, ADS1015_FULL_SCALE_4096_MV);
        if ((v[i] <= ADS1015_RESULT_ERROR)&&(v[i] >= (ADS1015_RESULT_ERROR-5))) {
            return (ADS1015_RESULT_ERROR-v[i]+1);
        }
    }
    for (i = 0; i < 4; ++i) {
        float fv;
        signed short vv = ADS1015_ResultToVoltageMv(v[i], ADS1015_FULL_SCALE_4096_MV);
        //(void)printf("%v ADC: %d\n", vv);
        if(i==3){   
            vv = vv - 420;
            fv = (float)vv * 60.5;
        }else{
            fv = (float)vv * 61;  
        } 
        (void)printf("%s:%.0f\n", name_channel_maps[i].name, fv);
    }
}
#else
static int __read_all(void) {
    uint8_t i;
    uint16_t v[4];
    DDRC_DDRC3 = 1;
    PORTC_PC3 = 1;
    for (i = 0; i < 4 ; ++i) {
        v[i] = ADS1015_SingleConvert(&g_ADS1015_BspInterface, name_channel_maps[i].channel, ADS1015_FULL_SCALE_4096_MV);
        if ((v[i] <= ADS1015_RESULT_ERROR)&&(v[i] >= (ADS1015_RESULT_ERROR-5))) {
            return (ADS1015_RESULT_ERROR-v[i]+1);
        }
    }
    for (i = 0; i < 4; ++i) {
        float fv;
        signed short vv = ADS1015_ResultToVoltageMv(v[i], ADS1015_FULL_SCALE_4096_MV);
        if(i==3){   
            vv = vv - 420;   
            fv = (float)vv * 147.57; 
        }else{
            fv = (float)vv * 154.3;
        }
        (void)printf("%s:%.0f\n", name_channel_maps[i].name, fv);  
    }
}
#endif
#endif

extern void print_arg_err_hint(char *arg);
int shell_func_hv(int argc, char **argv) {
    if (argc != 1 && argc != 3) {
        goto __error_args;
    }
    if (argc == 1) return __read_all();
#if BMS_SUPPORT_NEGATIVE_ADHESION
    if (argc == 3) 
    {
        DDRC_DDRC3 = 1;
        PORTC_PC3 = 1; 
        return 0;
    }
#endif
__error_args:
    print_arg_err_hint(argv[0]);
    return -1;
}
