#ifndef __SHELL_PLATFORM_H__
#define __SHELL_PLATFORM_H__

#include "shell.h"

/// shell��Ӧ��ִ�к���.
typedef int(*__FAR shell_handler)(int argc, char **argv);

#ifndef SHELL_SUPPORT_HELP
#define SHELL_SUPPORT_HELP 1
#endif

/// һ��shell����������Ϣ.
struct shell_command {
    const char *cmd; ///< shell����.
#if SHELL_SUPPORT_HELP
    const char *summary; ///< ��Ҫ����.
    const char *help; ///< ��ϸ����
#endif
    shell_handler handler; ///< ��Ӧִ�еĺ���.
};

/// ��������һ��shell����.
#define SHELL_PROTYPE(cmd)  \
    extern int shell_func_##cmd(int argc, char **argv); \
    extern const char shell_help_##cmd[]; \
    extern const char shell_summary_##cmd[]

/// ���ڳ�ʼ��һ��shell����Ľṹ��.
#if SHELL_SUPPORT_HELP
#define SHELL_COMMAND(cmd, shell) {cmd, shell_summary_##shell, shell_help_##shell, shell_func_##shell }
#else
#define SHELL_COMMAND(cmd, shell) {cmd, shell_func_##shell }
#endif


#endif

