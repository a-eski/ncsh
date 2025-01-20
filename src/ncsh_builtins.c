// Copyright (c) ncsh by Alex Eski 2024

#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_defines.h"
#include "ncsh_defines.h"
#include "ncsh_parser.h"
#include "ncsh_builtins.h"

eskilib_nodiscard int_fast32_t ncsh_builtins_exit(struct ncsh_Args *args)
{
    (void)args; // to not get compiler warnings
    return NCSH_COMMAND_EXIT;
}

eskilib_nodiscard int_fast32_t ncsh_builtins_echo(struct ncsh_Args *args)
{
    for (uint_fast32_t i = 1; i < args->count; ++i)
        printf("%s ", args->values[i]);

    if (args->count > 0)
        putchar('\n');

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define NCSH_COPYRIGHT \
"ncsh Copyright (C) 2025 Alex Eski\n"                                 \
"This program comes with ABSOLUTELY NO WARRANTY.\n"                     \
"This is free software, and you are welcome to redistribute it "         \
"under certain conditions.\n\n"                                           \

#define NCSH_HELP "ncsh help:\n"
#define NCSH_HELP_FORMAT \
"Builtin Commands: {command} {args}\n"
#define NCSH_HELP_QUIT \
"q:		      To exit, type q, exit, or quit and press enter. You can also use Ctrl+D to exit.\n"
#define NCSH_HELP_CHANGEDIR \
"cd/z:		      You can change directory with cd or z.\n"
#define NCSH_HELP_ECHO \
"echo:		      You can write things to the screen using echo.\n"
#define NCSH_HELP_HISTORY \
"history:	      You can see your command history using the history command.\n"
#define NCSH_HELP_HISTORY_COUNT \
"history count:        You can see the number of entries in your history with history count command.\n"
#define NCSH_HELP_PWD \
"pwd:         	      Prints the current working directory.\n"

#define NCSH_HELP_WRITE(str) \
if (write(STDOUT_FILENO, str, sizeof(str) - 1) == -1)   \
{                                                       \
    perror(RED NCSH_ERROR_STDOUT RESET);                \
    return NCSH_COMMAND_EXIT_FAILURE;                   \
}

eskilib_nodiscard int_fast32_t ncsh_builtins_help(struct ncsh_Args *args)
{
    (void)args; // to not get compiler warnings

    NCSH_HELP_WRITE(NCSH_COPYRIGHT);
    NCSH_HELP_WRITE(NCSH_HELP);
    NCSH_HELP_WRITE(NCSH_HELP_FORMAT);
    NCSH_HELP_WRITE(NCSH_HELP_QUIT);
    NCSH_HELP_WRITE(NCSH_HELP_CHANGEDIR);
    NCSH_HELP_WRITE(NCSH_HELP_ECHO);
    NCSH_HELP_WRITE(NCSH_HELP_HISTORY);
    NCSH_HELP_WRITE(NCSH_HELP_HISTORY_COUNT);
    NCSH_HELP_WRITE(NCSH_HELP_PWD);

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

eskilib_nodiscard int_fast32_t ncsh_builtins_cd(struct ncsh_Args *args)
{
    if (!args->values[1])
    {
        char *home = getenv("HOME");
        if (!home)
            fputs("ncsh: could not change directory.\n", stderr);
        else if (chdir(home) != 0)
            perror("ncsh: could not change directory");

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (chdir(args->values[1]) != 0)
        fputs("ncsh: could not change directory.\n", stderr);

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

eskilib_nodiscard int_fast32_t ncsh_builtins_pwd(struct ncsh_Args *args)
{
    (void)args;
    char path[PATH_MAX];
    if (!getcwd(path, sizeof(path)))
    {
        perror(RED "ncsh pwd: Error when getting current directory" RESET);
        fflush(stderr);
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    puts(path);
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

eskilib_nodiscard int_fast32_t ncsh_builtins_set_e()
{
	puts("sets e detected");
	return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define SET_NOTHING_TO_SET_MESSAGE "ncsh set: nothing to set"
#define SET_VALID_OPERATIONS_MESSAGE "ncsh set: valid set operations are in the form '-e', '-c', etc."
eskilib_nodiscard int_fast32_t ncsh_builtins_set(struct ncsh_Args *args)
{
    if (!args->values[1])
    {
        if (write(STDOUT_FILENO, SET_NOTHING_TO_SET_MESSAGE, sizeof(SET_NOTHING_TO_SET_MESSAGE) - 1) == -1)
        {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (args->lengths[1] > 3 || args->values[1][0] != '-')
    {
        if (write(STDOUT_FILENO, SET_VALID_OPERATIONS_MESSAGE, sizeof(SET_VALID_OPERATIONS_MESSAGE) - 1) == -1)
        {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    switch (args->values[1][1])
    {
    case 'e': { return ncsh_builtins_set_e();}
    default: { break; }
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}
