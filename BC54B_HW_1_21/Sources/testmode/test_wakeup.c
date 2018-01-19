#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "test_mode.h"
#include "cp_pwm.h"
#include "pwm.h"


const char shell_help_wakeup[] = "\n\
	唤醒测试\n\
";
    

const char shell_summary_wakeup[] = "\
seconds\n\
    关闭电源, 然后再给定时间之后唤醒.\n\
";
    
    
extern void print_arg_err_hint(char *arg);
int shell_func_cppwm(int argc, char **argv) {
    if (argc == 1) {
        float duty = ((float)cp_pwm_get_duty()) / 10;
        (void)printf("duty:%.1f\n", duty);
        return 0;
    }
    print_arg_err_hint(argv[0]);
    return -1;
}