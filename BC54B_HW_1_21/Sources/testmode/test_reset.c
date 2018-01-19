#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "run_mode.h"

static const enum runmode run_modes[] = {
    RUN_MODE_NORMAL,
    RUN_MODE_TEST,
};

const char shell_help_reset[] = "\
	[mode]\n\
	复位系统.\n\
	mode: 复位到的运行模式:\n\
	    0 正常模式\n\
	    1 测试模式\n\
";
    

const char shell_summary_reset[] =
    "复位系统.";
    
extern void reset_to_mode(unsigned char mode);

extern void print_arg_err_hint(char *arg);
int shell_func_reset(int argc, char **argv) {
    if (argc == 2 && (strlen(argv[1]) == 1)) {
        uint8_t mode = argv[1][0] - '0';
        if (mode <= sizeof(run_modes)/ sizeof(run_modes[0])) {
            runmode_reset_to(run_modes[mode]);
            return 0;
        }
    }
    print_arg_err_hint(argv[0]);
    return -1;
}
