#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"

#include "ads1015_intermediate.h"



const char shell_help_ads1015[] = "\
	chip [channel]\n\
	读取指定通道的电压\n\
	chip 指定芯片, 取值u14\n\
	channel 通道, 如果没有指定, 则读取所有通道的电压.\n\
";

const char shell_summary_ads1015[] =
    "ADS1015相关";


static const ADS1015_Channel ADS1015_Channels[] = {
    ADS1015_CHANNEL_0,
    ADS1015_CHANNEL_1,
    ADS1015_CHANNEL_2,
    ADS1015_CHANNEL_3
};

static int __read_all(void) {
    uint8_t i;
    uint16_t v[4];
    for (i = 0; i < 4; ++i) {
        v[i] = ADS1015_SingleConvert(&g_ADS1015_BspInterface, ADS1015_Channels[i], ADS1015_FULL_SCALE_4096_MV);
        if (v[i] == ADS1015_RESULT_ERROR) {
            return 1;
        }
    }
    for (i = 0; i < 4; ++i) {
        (void)printf(i != 3 ? "%d " : "%d\n", ADS1015_ResultToVoltageMv(v[i], ADS1015_FULL_SCALE_4096_MV));
    }
}

static int __read(uint8_t c) {
    uint16_t v;
    if (c >= 4) return 1;
    v = ADS1015_SingleConvert(&g_ADS1015_BspInterface, ADS1015_Channels[c], ADS1015_FULL_SCALE_4096_MV);
    if (v == ADS1015_RESULT_ERROR) {
        return 1;
    }
    (void)printf("%d\n", ADS1015_ResultToVoltageMv(v, ADS1015_FULL_SCALE_4096_MV));
    return 0;
}

extern void print_arg_err_hint(char *arg);
int shell_func_ads1015(int argc, char **argv) {
    if (argc < 2) {
        goto __error_args;
    }


    if (strcmp("u14", argv[1]) != 0) {
        goto __error_args;
    }

    if (argc == 2) {
        return __read_all();
    }

    if (argc == 3) {
        uint8_t c = (uint8_t)atoi(argv[2]);
        return __read(c);
    }

__error_args:
    print_arg_err_hint(argv[0]);
    return -1;
}
