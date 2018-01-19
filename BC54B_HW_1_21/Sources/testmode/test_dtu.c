#include <stdio.h>
#include <stdlib.h>
#include "adc0_intermediate.h"
#include "dtu_m35_impl.h"
#include "test_io.h"


void test_dtu_init(void) {
    uint8_t is_high;
    if (!input_gpio_is_high_with_name("GSM_ID", &is_high)) {
        return;
    }
    
    if (is_high) {
        return;
    }
    
    DTUM35_Init(&g_DTUM35_BspInterface, 1);
}

static void print_ops(void) {
    const char *ops;
    switch (DTUM35_GetOPS()) {
    case DTU_OPS_CHINA_UNIONCOM:
        ops = "中国联通";
        break;
    case DTU_OPS_CHINA_MOBILE:
        ops = "中国移动";
        break;
    case DTU_OPS_CHINA_TELECOM:
        ops = "中国电信";
        break;
    default:
        ops = "未知";
        break;
    }

    (void)printf("OPS:%s\n", ops);
}

/// \brief DTUM35_GetSingalQuality 获取信号强度及误码率.
///
/// \param strength 信号强度存储在这个指针指向的字节.
/// \param err_rate 误码率存储在这个指针指向的字节.
static void print_signal_quality(void) {
    unsigned char s, e;
    DTUM35_GetSingalQuality(&s, &e);
    (void)printf("SignalQuality:%d,%d\n", s, e);
}

static void print_lacci(void) {
    unsigned short lacci[2];
    DTUM35_GetLacCi(lacci);
    (void)printf("LACCI:%04X,%04X\n", lacci[0], lacci[1]);
}


static void print_ccid(void) {
    char ccid[25];
    DTUM35_GetCIMI(ccid, sizeof(ccid));
    (void)printf("CCID:%s\n", ccid);
}


static void print_imei(void) {
    char imei[25];
    DTUM35_GetIMEI(imei, sizeof(imei));
    (void)printf("IMEI:%s\n", imei);
}

static const char *const status_string[] = {
    "UNKNOWN",
    "POWERUP",
    "CONFIGURE",
    "WAIT_REGISTER",
    "SEND_SMS",
    "CONNECT_SERVER",
    "GET_SIGNAL_QUALITY",
    "CHECK_CONNECTION",
    "WAIT_FOR_SEND_DATA",
    "SEND_DATA",
    "RECV_DATA",
    "RECV_SMS",
    "DELAT_TO_RESTART",
};

static void printf_runtime_status(void) {
    INT8U i;
    INT8U status[5];
    (void)memset(status, DTUM35_RUNTIME_STATUS_UNKNOWN, sizeof(status));
    DTUM35_GetRuntimeStatusAll(status, sizeof(status));
    (void)printf("Status:");
    for (i = 0; i < sizeof(status); ++i) {
        if (status[i] < sizeof(status_string) / sizeof(status_string[0])) {
            (void)printf(i == (sizeof(status) - 1) ? "%s\n" : "%s,", status_string[status[i]]);
        }
    }
}

const char shell_help_dtu[] = "\n\
	显示DTU的运行状态.\n\
";

const char shell_summary_dtu[] =
    "DTU相关";


extern void print_arg_err_hint(char *arg);
int shell_func_dtu(int argc, char **argv) {
    int rc = 0;
    uint8_t is_high;
    (void)argv;
    if (argc != 1) {
        goto __error_params;
    }
    
    if (!input_gpio_is_high_with_name("GSM_ID", &is_high)){
        (void)printf("DTU module is installed? check it!\n");
        return -3;
        
    }
    
    if (is_high) {
        (void)printf("DTU module is not installed.\n");
        return -2;
    }

    print_ops();
    print_signal_quality();
    print_lacci();
    print_ccid();
    print_imei();
    printf_runtime_status();

    return 0;

__error_params:
    print_arg_err_hint(argv[0]);
    return -1;
}
