#ifndef __SHELL_CONFIG_H__
#define __SHELL_CONFIG_H__

// =========================================================
// �������涨��SHELL����Ϊ
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
// �������涨��SHELL���
// =========================================================
#include "shell_platform.h"


SHELL_PROTYPE(echo);


#define SHELL_COMMAND_CUSTOM_LIST \
    SHELL_COMMAND("echo", echo), \

#endif

