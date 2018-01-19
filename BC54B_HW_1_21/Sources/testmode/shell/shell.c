#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "shell_config.h"


#ifndef SHELL_WELCOM_MESSAGE
#define SHELL_WELCOM_MESSAGE     "Hello, this is a shell."
#endif

#ifndef SHELL_PROMPT
#define SHELL_PROMPT              "SHELL> "
#endif

#define SHELL_MAXSIZE             50
#define SHELL_ERRMSG              "Invalid command, type 'help' for help\n"
#define SHELL_ALT_SPACE           '\x07'
#define SHELL_MAX_ARGS            10


static int rc = 0;




static const char shell_summary_exit[] = "exit the shell";
static const char shell_help_exit[] = "\nExits the shell.\n";
static int shell_func_exit(int argc, char **argv) {
    (void)argc;
    (void)argv;
    (void)printf("byte ...\n");
    return 0;
}

#if SHELL_SUPPORT_HELP
static void shellh_show_help(const char *cmd, const char *helptext) {
    (void)printf("Usage: %s %s", cmd, helptext);
}


static const char shell_summary_help[] = "shell help";
static const char shell_help_help[] = "\
[<command>]\n\
    [<command>] - the command to get help on.\n\
Without arguments it shows a summary of all the shell commands.\n\
";
static int shell_func_help(int argc, char **argv);

#endif

/// shell命令列表的结束标记.

#if SHELL_SUPPORT_HELP
#define SHELL_COMMAND_END() {(const char *__FAR)0, (const char *__FAR)0, (const char *__FAR)0, (const char *__FAR)0}
#else
#define SHELL_COMMAND_END() {(const char *__FAR)0, (const char *__FAR)0}
#endif

static const struct shell_command buildin_shell_commands[] = {
#ifdef SHELL_COMMAND_CUSTOM_LIST
    SHELL_COMMAND_CUSTOM_LIST
#endif

#if SHELL_SUPPORT_HELP
    SHELL_COMMAND("help", help),
#endif
    SHELL_COMMAND("exit", exit),
    SHELL_COMMAND_END(),
};


#if SHELL_SUPPORT_HELP
static void shell_list_summary_help(const struct shell_command *cmds) {
    const struct shell_command *__FAR pcmd;
    for (pcmd = cmds; pcmd->cmd != NULL; ++pcmd) {
        if (strlen(pcmd->summary) > 0) {
            (void)printf("  %-6s - %s\n", pcmd->cmd, pcmd->summary);
        }
    }

    (void)printf("\nhelp <command> for more information of the command.\n\n");
}

static const struct shell_command *shell_detail_help(const struct shell_command *cmds, const char *cmd) {
    const struct shell_command *pcmd;
    for (pcmd = cmds; pcmd->cmd != NULL; ++pcmd) {
        if (!strcmp(pcmd->cmd, cmd) && strlen(pcmd->summary) > 0) {
            (void)printf("%s - %s\nUsage:\n %s %s\n\n",
                         pcmd->cmd,
                         pcmd->summary,
                         pcmd->cmd,
                         pcmd->help);
            return pcmd;
        }
    }
    return NULL;
}



static int shell_func_help(int argc, char **argv) {
    if (argc > 2) {
        shellh_show_help(argv[0], shell_help_help);
        return -1;
    }

    if (argc == 1) {
        (void)printf("Shell commands:\n");
        shell_list_summary_help(buildin_shell_commands);
        return 0;
    }

    if (NULL != shell_detail_help(buildin_shell_commands, argv[1])) {
        return 0;
    }

    (void)printf("Unknown command '%s'.\n", argv[ 1 ]);
    (void)printf("For more information use 'help <command>'.\n");
    return -2;
}

#endif

static const struct shell_command *shell_execute_command_in_commands(const struct shell_command *cmds, int argc, char **argv) {
    const struct shell_command *pcmd;
    for (pcmd = cmds; pcmd->cmd != NULL; ++pcmd) {
        if (!strcmp(pcmd->cmd, argv[0])) {
            rc = pcmd->handler(argc, argv);
            return pcmd;
        }
    }
    return NULL;
}

void shell_execute_command(char *cmd) {
    char *p, *temp;
    int i, inside_quotes;
    char quote_char;
    int argc;
    char *argv[ SHELL_MAX_ARGS ];

    if (strlen(cmd) == 0) {
        return;
    }

    if (cmd[ strlen(cmd) - 1 ] != '\n') {
        (void)strcat(cmd, "\n");
    }

    // Change '\r', '\n' and '\t' chars to ' ' to ease processing
    p = cmd;
    while (*p) {
        if (*p == '\r' || *p == '\n' || *p == '\t') {
            *p = ' ';
        }
        p ++;
    }

    // Transform ' ' characters inside a '' or "" quoted string in
    // a 'special' char. We do this to let the user execute something
    // like "lua -e 'quoted string'" without disturbing the quoted
    // string in any way.
    for (i = 0, inside_quotes = 0, quote_char = '\0'; i < strlen(cmd); i ++)
        if ((cmd[ i ] == '\'') || (cmd[ i ] == '"')) {
            if (!inside_quotes) {
                inside_quotes = 1;
                quote_char = cmd[ i ];
            } else {
                if (cmd[ i ] == quote_char) {
                    inside_quotes = 0;
                    quote_char = '\0';
                }
            }
        } else if ((cmd[ i ] == ' ') && inside_quotes) {
            cmd[ i ] = SHELL_ALT_SPACE;
        }
    if (inside_quotes) {
        (void)printf("Invalid quoted string\n");
        return;
    }

    // Transform consecutive sequences of spaces into a single space
    p = strchr(cmd, ' ');
    while (p) {
        temp = p + 1;
        while (*temp && *temp == ' ') {
            (void)memmove(temp, temp + 1, strlen(temp));
        }
        p = strchr(p + 1, ' ');
    }
    if (!strcmp(cmd, " ")) {
        return;
    }

    // Skip over the initial space char if it exists
    p = cmd;
    if (*p == ' ') {
        p ++;
    }

    // Add a final space if it does not exist
    if (p[ strlen(p) - 1 ] != ' ') {
        (void)strcat(p, " ");
    }

    // Compute argc/argv
    for (argc = 0; argc < SHELL_MAX_ARGS; argc ++) {
        argv[ argc ] = NULL;
    }
    argc = 0;
    for(;;) 
    {
        temp = strchr(p, ' ');
        if (temp == NULL) {
            break;
        }
        *temp = 0;
        if (argc == SHELL_MAX_ARGS) {
            (void)printf("Error: too many arguments\n");
            argc = -1;
            break;
        }
        argv[ argc ++ ] = p;
        p = temp + 1;
    }

    if (argc == -1) {
        return;
    }

    // Additional argument processing happens here
    for (i = 0; i < argc; i ++) {
        p = argv[ i ];
        // Put back spaces if needed
        for (inside_quotes = 0; inside_quotes < strlen(argv[ i ]); inside_quotes ++) {
            if (p[ inside_quotes ] == SHELL_ALT_SPACE) {
                argv[ i ][ inside_quotes ] = ' ';
            }
        }
        // Remove quotes
        if ((p[ 0 ] == '\'' || p [ 0 ] == '"') && (p[ 0 ] == p[ strlen(p) - 1 ])) {
            argv[ i ] = p + 1;
            p[ strlen(p) - 1 ] = '\0';
        }
    }

    if (NULL != shell_execute_command_in_commands(buildin_shell_commands, argc, argv)) {
        return;
    }


    (void)printf("Command \"%s\" cannot be found.\n", argv[0]);
}


static void getline(char *buf, int buf_size) {
    signed int i;
    int c;
    for (i = 0; i < buf_size - 1;) {
        c = getchar();
        if (c == '\b' || c == 0x7F) {
            if (i > 0) {
                --i;
                --buf;
            }
            continue;
        }

        if (c == '\r') {
            *buf = 0;
            return;
        }
        *buf++ = (char)c;
        ++i;
    }

    *buf = 0;
}

void shell_loop(void) {
    char cmd[SHELL_MAXSIZE];


    (void)printf("\n");
    (void)printf(SHELL_WELCOM_MESSAGE);
    (void)printf("\n");

    for(;;) 
    {
        (void)printf("[%d]" SHELL_PROMPT, rc);
        (void)fflush(stdout);
        getline(cmd, sizeof(cmd) - 1);
        if (strlen(cmd) == 0) {
            continue;
        }
        shell_execute_command(cmd);
    }
}

void print_arg_err_hint(char *arg) 
{
    (void)printf("arguments error, \"help %s\" for more information\n", arg);
}
    