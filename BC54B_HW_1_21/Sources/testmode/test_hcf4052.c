#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "test_io.h"

#include "hcf4052_intermediate.h"


const char shell_help_hcf4052[] = "\
    chip [channel]\n\
    设置DAC的输出\n\
    chip u37\n\
    channel 0,1,2,3,none, if not specified, display the current select channel\n";

const char shell_summary_hcf4052[] =
    "HCF4052相关.";

extern void print_arg_err_hint(char *arg);
int shell_func_hcf4052(int argc, char **argv) {
    if (argc < 2) {
        goto __error_args;
    }

    if (strcmp("u37", argv[1]) != 0) {
        goto __error_args;
    }

    if (argc == 2) { // display the current channel
        enum hcf4052_channel channel = hcf4052_get_current_channel(hcf4052_impl);
        if (HCF4052_CHANNEL_NONE == channel) {
            (void)printf("none\n");
        } else {
            (void)printf("%d\n", channel);
        }

        return 0;
    }

    if (argc == 3) { // select channel
        uint8_t channel;
        if (strcmp("none", argv[2]) == 0) {
            channel = HCF4052_CHANNEL_NONE;
        } else if (strlen(argv[2]) == 1) {
            if (argv[2][0] >= '0' && argv[2][0] <= '3') {
                channel = argv[2][0] - '0';
            } else {
                goto __error_args;
            }
        } else {
            goto __error_args;
        }

        hcf4052_select_channel(hcf4052_impl, channel);
        return 0;
    }
__error_args:
    print_arg_err_hint(argv[0]);
    return -1;
}
