#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const char shell_help_echo[] = "\
	...\n\
	回显输入的参数\
";

const char shell_summary_echo[] =
    "回显";

int shell_func_echo(int argc, char **argv) {
    int i;
    for (i = 1; i < argc; ++i) {
        (void)printf(i == argc - 1 ? "%s\n" : "%s ", argv[i]);
    }
    return 0;
}
