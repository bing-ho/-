#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "test_mode.h"
#include "ect_intermediate.h"
#include "ect_xgate.h"


void test_cppwm_init(void) {
    pwm_init();
    (void)cp_pwm_start();
}

const char shell_help_cppwm[] = "\n\
	显示检测到的CP信号的占空比\n\
";
    

const char shell_summary_cppwm[] =
    "CP PWM占空比.";
    
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