#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "run_mode.h"
#include "bms_bcu.h"
#include "test_io.h"

const char shell_help_bdinfo[] = "\
	\n\
	显示硬件版本.\n\
";
    

const char shell_summary_bdinfo[] =
    "显示硬件版本.";

    
extern void print_arg_err_hint(char *arg);
int shell_func_bdinfo(int argc, char **argv) {
    if (argc == 1) {
        uint8_t is_high;
        const char *name;
        input_gpio_is_high_with_name("GSM_ID", &is_high);
        if(is_high) {
            name = "BC52B";
        } else {
            name = "BC54B";
        }
        printf("Board: %s\n", name);
        printf("HwVer: %d\n", hardware_io_revision_get());
        return 0;
    }
    print_arg_err_hint(argv[0]);
    return -1;
}