#ifndef __SHELL_CONFIG_H__
#define __SHELL_CONFIG_H__

// =========================================================
// 在这下面定义SHELL的行为
// =========================================================
#define SHELL_SUPPORT_HELP 1
#define SHELL_WELCOM_MESSAGE \
"\n\
================================================\n\
Hello, this is a simple shell for BC52B test mode.\n\
type \"help<cr>\" for more information.\n\
================================================\n\
"

#define SHELL_PROMPT   "PROMPT# "

// =========================================================
// 在这下面定义SHELL命令集
// =========================================================
#include "shell_platform.h"


SHELL_PROTYPE(echo);


#define SHELL_COMMAND_CUSTOM_LIST \
    SHELL_COMMAND("echo", echo), \

#endif

