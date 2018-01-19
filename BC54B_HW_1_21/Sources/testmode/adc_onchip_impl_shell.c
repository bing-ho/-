#include <stdio.h>
#include <stdlib.h>
#include "adc0_intermediate.h"


const char shell_help_adc[] = "\n\
	list\n\
		��ʾ����ģ�����������ͨ������\n\
	read [name|all]\n\
		��ȡģ�����������ͨ��, δָ��name��ȡ����\n\
";

const char shell_summary_adc[] =
    "ģ���������";

extern void print_arg_err_hint(char *arg) ;
int shell_func_adc(int argc, char **argv) {
    int rc = 0;
    if (argc < 2) {
        goto __error_params;
    }


    if (strcmp(argv[1], "list") == 0) {
        adc_onchip_list_all();
        return 0;
    }

    if (strcmp(argv[1], "read") != 0) {
        goto __error_params;
    }

    if (argc == 2) {// read all
        if (0 != adc_onchip_dump_all()) {
            return 0;
        }
        return -2;
    }

    if (argc == 3) {
        if (strcmp(argv[2], "all") == 0) {
            if (0 != adc_onchip_dump_all()) {
                return 0;
            }
            return -2;
        }
        if (0 != adc_onchip_dump_with_name(argv[2])) {
            return 0;
        }
        return -3;
    }

__error_params:
    print_arg_err_hint(argv[0]);
    return -1;
}
