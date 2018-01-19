#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"

const char shell_help_flag[] = "\
	[xxx]\n\
	设置或读取标志, 用于判断是否重启.\n\
	xxx: 需要设置的标志, uint8_t\n\
";

const char shell_summary_flag[] =
    "设置或读取标";

extern void print_arg_err_hint(char *arg);
int shell_func_flag(int argc, char **argv) {
    static uint8_t flag = 0;
    
    if (argc == 1) {
        (void)printf("%d\n", flag);
        return 0;
    }
    
    if (argc == 2) {
        flag = (uint8_t)atoi(argv[1]);
        return 0;
    }

    print_arg_err_hint(argv[0]);
    return -1;
}
