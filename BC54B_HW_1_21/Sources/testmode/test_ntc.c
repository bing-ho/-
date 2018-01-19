#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "run_mode.h"
#include "bms_board_temperature.h"
#include "adc0_intermediate.h"

/*********变更 2016.11.9 新增bc52b 4路温感读取************/
//extern  const struct adc_onchip_channel *_FAR adc_onchip_temp_u3;
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

const struct adc_onchip_channel *const ntc_channels[] = {
		&adc_onchip_temp_u48,
		&adc_onchip_temp_u3_v116,
		&adc_onchip_temp_t1_det_adc_v120,
		&adc_onchip_temp_t2_det_adc_v120,
		&adc_onchip_temp_t3_det_adc_v120,
		&adc_onchip_temp_t4_det_adc_v120
};

const char shell_help_ntc[] = "\
    list\n\
		显示所有温感模拟输入量检测通道名称\n\
	read [name]\n\
		读取温感模拟输入量检测通道, 未指定name读取所有\n";


const char shell_summary_ntc[] =
    "板子温感和主机模拟温感读取";

INT8U test_calc_board_temperature(INT8U which, INT8U *temp)
{
    INT16U vol = 0;

    if(temp == NULL) 
    {
        return 0;
    }
    if (which < 0 || which > 5) 
    {
        return 0;
    }
    
    if (!adc_onchip_read(ntc_channels[which], &vol)) 
    {
        return 0;
    }
    (void)printf("%s:%d\n", "vol", vol);
    //should use NTCTAB_NTCG163JF103FTB, but the param most the same as NTCTAB_NCP18XH103
    *temp = TempQuery(vol, NTCTAB_NCP18XH103) + 10;//offset:-50 
  
    return 1;
}


INT8U ntc_dump_with_name(const char *name) {
    uint8_t ret = 1;
    uint8_t i;
    for (i = 0; i < ARRAY_SIZE(ntc_channels); ++i) {
        if (strcmp(name, ntc_channels[i]->name) == 0) {
            int8_t temp_t = 0;
            if (test_calc_board_temperature(i, (INT8U *)&temp_t)) {
            temp_t = (int)temp_t -50;
                (void)printf("%s:%d\n", ntc_channels[i]->name, temp_t);
                return 1;
            }

            return 0;
        }
    }
    return 0;
}

INT8U ntc_dump_all(void) {
    uint8_t i;
    for (i = 0; i < ARRAY_SIZE(ntc_channels); ++i) {
        int8_t temp = 0;
        if (test_calc_board_temperature(i, (INT8U *)&temp)) {
           temp = (int)temp -50;
            (void)printf("%s:%d\n", ntc_channels[i]->name, temp);
        } else {
            (void)printf("%s:error\n", ntc_channels[i]->name);
            return 0;
        }
    }
    return 1;
}

void ntc_list_all(void) {
    uint8_t i;
    for (i = 0; i < ARRAY_SIZE(ntc_channels); ++i) {
        (void)printf("%s\n", ntc_channels[i]->name);
    }
}


int shell_func_ntc(int argc, char **argv) {
	int rc = 0;
    if (argc < 2) {
        goto __error_params;
    }


    if (strcmp(argv[1], "list") == 0) {
        ntc_list_all();
        return 0;
    }

    if (strcmp(argv[1], "read") != 0) {
        goto __error_params;
    }

    if (argc == 2) {// read all
        if (0 != ntc_dump_all()) {
            return 0;
        }
        return -2;
    }

    if (argc == 3) {
        if (0 != ntc_dump_with_name(argv[2])) {
            return 0;
        }
        return -3;
    }
	
  	__error_params:
    printf("arguments error, \"help %s\" for more information\n", argv[0]);
    return -1;
	
}


