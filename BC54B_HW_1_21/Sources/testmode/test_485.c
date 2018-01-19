#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "rs485_hardware.h"
#include "bms_config.h"
#include "ucos_ii.h"
#include "shell.h"

//#define config_get(a) kRs485Dev1

void test_rs485_init(void) {
    //rs485_init(HMI_DEV_NAME, 9600, 0, 0);//config_get(kHmiRS485ChannelIndex)
    Rs485Param rs485_param = {9600, 0, 0, 0};
    Rs485_Hardware_Init(HMI_DEV_NAME, &rs485_param, NULL, NULL);
}

static void __write_to_rs485(Rs485Dev dev, char *s) {
    rs485_t ctx = rs485_get_context(dev);
    while (*s) {
        rs485_send(ctx, *s++);
    }
}

static int this_recv_data(Rs485Dev context, char chr, void *user_data) {
    (void)context;
    (void)user_data;
    putchar(chr);
    return 1;
}

static void __recv(Rs485Dev dev, uint16_t timeout) {
    rs485_t ctx = rs485_get_context(dev);
    (void)rs485_set_recv_handler(ctx, this_recv_data, NULL);
    OSTimeDly(timeout);
    rs485_set_recv_handler(ctx, 0, 0);
}


const char shell_help_rs485[] = "\
	dev ops xx\n\
	dev: 设备号, 必须为0\n\
	ops xxxx\n\
		read xx: 从RS485接收数据并打印, xx为超时时间(ms);\n\
		send xx: 通过RS485发送数据, xx为需要发送的数据;\n\
";

const char shell_summary_rs485[] =
    "RS485操作";

extern void print_arg_err_hint(char *arg);
int shell_func_rs485(int argc, char **argv) {
    if (argc != 4) {
        goto __error_params;
    }
    if (strcmp(argv[1], "0") != 0) {
        goto __error_params;
    }

    if (strcmp(argv[2], "read") == 0) { // read recv buffer
        __recv(HMI_DEV_NAME, atoi(argv[3]));//config_get(kHmiRS485ChannelIndex)
        putchar('\n');
        return 0;
    }

    if (strcmp(argv[2], "send") == 0) { // send argv[3]
        __write_to_rs485(HMI_DEV_NAME, argv[3]);//config_get(kHmiRS485ChannelIndex)
        OSTimeDly(strlen(argv[3]) * 2);
        return 0;
    }

__error_params:
    print_arg_err_hint(argv[0]);
    return -1;

}
