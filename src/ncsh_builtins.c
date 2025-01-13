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

#include "ncsh_builtins.h"
#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_defines.h"
#include "ncsh_defines.h"
#include "ncsh_parser.h"

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

eskilib_nodiscard int_fast32_t ncsh_builtins_help(struct ncsh_Args *args)
{
    (void)args; // to not get compiler warnings

    if (write(STDOUT_FILENO, "ncsh by Alex Eski: help\n\n", 25) == -1)
    {
        perror("ncsh: Error writing to stdout");
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    if (write(STDOUT_FILENO, "Builtin Commands {command} {args}\n", 34) == -1)
    {
        perror("ncsh: Error writing to stdout");
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    if (write(STDOUT_FILENO,
              "q:		To exit, type q, exit, or quit and press enter. You can also use Ctrl+D to exit.\n", 85) == -1)
    {
        perror("ncsh: Error writing to stdout");
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    if (write(STDOUT_FILENO, "cd/z:		You can change directory with cd or z.\n", 46) == -1)
    {
        perror("ncsh: Error writing to stdout");
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    if (write(STDOUT_FILENO, "echo:		You can write things to the screen using echo.\n", 54) == -1)
    {
        perror("ncsh: Error writing to stdout");
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    if (write(STDOUT_FILENO, "history:	You can see your command history using the history command.\n", 69) == -1)
    {
        perror("ncsh: Error writing to stdout");
        return NCSH_COMMAND_EXIT_FAILURE;
    }

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

#define NOTHING_TO_SET "ncsh set: nothing to set"
#define VALID_SET_OPERATIONS "ncsh set: valid set operations are in the form '-e', '-c', etc."
eskilib_nodiscard int_fast32_t ncsh_builtins_set(struct ncsh_Args *args)
{
    if (!args->values[1])
    {
        if (write(STDOUT_FILENO, NOTHING_TO_SET, sizeof(NOTHING_TO_SET) - 1) == -1)
        {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (args->lengths[1] > 3 || args->values[1][0] != '-')
    {
        if (write(STDOUT_FILENO, VALID_SET_OPERATIONS, sizeof(VALID_SET_OPERATIONS) - 1) == -1)
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

