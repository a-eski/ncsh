// Copyright (c) ncsh by Alex Eski 2024

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_defines.h"
#include "ncsh_defines.h"
#include "ncsh_history.h"
#include "ncsh_builtins.h"
#include "ncsh_parser.h"
#include "z/z.h"

eskilib_nodiscard int_fast32_t ncsh_builtins_z(struct z_Database* z_db, struct ncsh_Args* args)
{
    assert(z_db);
    assert(args->count > 0);

    if (args->values[1] &&
        eskilib_string_compare(args->values[1], args->lengths[1], NCSH_Z_PRINT, sizeof(NCSH_Z_PRINT))) {
        z_print(z_db);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (args->count > 2) {
        if (!args->values[1] || !args->values[2]) {
            return NCSH_COMMAND_FAILED_CONTINUE;
        }

        if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_Z_ADD, sizeof(NCSH_Z_ADD))) {
            size_t length = strlen(args->values[2]) + 1;
            if (z_add(args->values[2], length, z_db) == Z_SUCCESS) {
                return NCSH_COMMAND_SUCCESS_CONTINUE;
            }
            else {
                return NCSH_COMMAND_EXIT_FAILURE;
            }
        }
        else {
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    size_t length = !args->values[1] ? 0 : strlen(args->values[1]) + 1;
    char cwd[PATH_MAX] = {0};
    char* cwd_result = getcwd(cwd, PATH_MAX);
    if (!cwd_result) {
        perror(RED "ncsh z: Could not load cwd information" RESET);
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    z(args->values[1], length, cwd, z_db);
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

eskilib_nodiscard int_fast32_t ncsh_builtins_history(struct ncsh_History* history, struct ncsh_Args* args)
{
    if (args->values[1]) {
        if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_HISTORY_COUNT,
                                   sizeof(NCSH_HISTORY_COUNT))) {
            return ncsh_history_command_count(history);
        }
        else if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_HISTORY_CLEAN,
                                        sizeof(NCSH_HISTORY_CLEAN))) {
            return ncsh_history_command_clean(history);
        }
        // else { Invalid options }
    }

    return ncsh_history_command_display(history);
}

eskilib_nodiscard int_fast32_t ncsh_builtins_exit(struct ncsh_Args* args)
{
    (void)args; // to not get compiler warnings
    return NCSH_COMMAND_EXIT;
}

eskilib_nodiscard int_fast32_t ncsh_builtins_echo(struct ncsh_Args* args)
{
    for (uint_fast32_t i = 1; i < args->count; ++i) {
        printf("%s ", args->values[i]);
    }

    if (args->count > 0) {
        putchar('\n');
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define NCSH_COPYRIGHT                                                                                                 \
    "ncsh Copyright (C) 2025 Alex Eski\n"                                                                              \
    "This program comes with ABSOLUTELY NO WARRANTY.\n"                                                                \
    "This is free software, and you are welcome to redistribute it "                                                   \
    "under certain conditions.\n\n"

#define NCSH_HELP_MESSAGE "ncsh help:\n"
#define NCSH_HELP_FORMAT "Builtin Commands: {command} {args}\n"
#define NCSH_HELP_QUIT "q:		      To exit, type q, exit, or quit and press enter. You can also use Ctrl+D to exit.\n"
#define NCSH_HELP_CHANGEDIR "cd/z:		      You can change directory with cd or z.\n"
#define NCSH_HELP_ECHO "echo:		      You can write things to the screen using echo.\n"
#define NCSH_HELP_HISTORY "history:	      You can see your command history using the history command.\n"
#define NCSH_HELP_HISTORY_COUNT                                                                                        \
    "history count:        You can see the number of entries in your history with history count command.\n"
#define NCSH_HELP_PWD "pwd:         	      Prints the current working directory.\n"

#define NCSH_HELP_WRITE(str)                                                                                           \
    if (write(STDOUT_FILENO, str, sizeof(str) - 1) == -1) {                                                            \
        perror(RED NCSH_ERROR_STDOUT RESET);                                                                           \
        return NCSH_COMMAND_EXIT_FAILURE;                                                                              \
    }

eskilib_nodiscard int_fast32_t ncsh_builtins_help(struct ncsh_Args* args)
{
    (void)args; // to not get compiler warnings

    NCSH_HELP_WRITE(NCSH_COPYRIGHT);
    NCSH_HELP_WRITE(NCSH_HELP_MESSAGE);
    NCSH_HELP_WRITE(NCSH_HELP_FORMAT);
    NCSH_HELP_WRITE(NCSH_HELP_QUIT);
    NCSH_HELP_WRITE(NCSH_HELP_CHANGEDIR);
    // z
    // z add
    // z print
    NCSH_HELP_WRITE(NCSH_HELP_ECHO);
    NCSH_HELP_WRITE(NCSH_HELP_HISTORY);
    NCSH_HELP_WRITE(NCSH_HELP_HISTORY_COUNT);
    // history clean
    NCSH_HELP_WRITE(NCSH_HELP_PWD);
    // kill

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

eskilib_nodiscard int_fast32_t ncsh_builtins_cd(struct ncsh_Args* args)
{
    if (!args->values[1]) {
        char* home = getenv("HOME");
        if (!home) {
            fputs("ncsh: could not change directory.\n", stderr);
        }
        else if (chdir(home) != 0) {
            perror("ncsh: could not change directory");
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (chdir(args->values[1]) != 0) {
        fputs("ncsh: could not change directory.\n", stderr);
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

eskilib_nodiscard int_fast32_t ncsh_builtins_pwd(struct ncsh_Args* args)
{
    (void)args;
    char path[PATH_MAX];
    if (!getcwd(path, sizeof(path))) {
        perror(RED "ncsh pwd: Error when getting current directory" RESET);
        fflush(stderr);
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    puts(path);
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define KILL_NOTHING_TO_KILL_MESSAGE "ncsh kill: nothing to kill, please pass in a process ID (PID).\n"
#define KILL_COULDNT_PARSE_PID_MESSAGE "ncsh kill: could not parse process ID (PID) from arguments.\n"
eskilib_nodiscard int_fast32_t ncsh_builtins_kill(struct ncsh_Args* args)
{
    if (!args->values[1]) {
        if (write(STDOUT_FILENO, KILL_NOTHING_TO_KILL_MESSAGE, sizeof(KILL_NOTHING_TO_KILL_MESSAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    pid_t pid = atoi(args->values[1]);
    if (!pid) {
        if (write(STDOUT_FILENO, KILL_COULDNT_PARSE_PID_MESSAGE, sizeof(KILL_COULDNT_PARSE_PID_MESSAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }
        return NCSH_COMMAND_FAILED_CONTINUE;
    }
    if (kill(pid, SIGTERM) != 0) {
        printf("ncsh kill: could not kill process with process ID (PID): %d\n", pid);
        return NCSH_COMMAND_FAILED_CONTINUE;
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

// NOTE: set is not fully implemented.
eskilib_nodiscard int_fast32_t ncsh_builtins_set_e()
{
    puts("sets e detected");
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define SET_NOTHING_TO_SET_MESSAGE "ncsh set: nothing to set, please pass in a value to set (i.e. '-e', '-c')"
#define SET_VALID_OPERATIONS_MESSAGE "ncsh set: valid set operations are in the form '-e', '-c', etc."
eskilib_nodiscard int_fast32_t ncsh_builtins_set(struct ncsh_Args* args)
{
    if (!args->values[1]) {
        if (write(STDOUT_FILENO, SET_NOTHING_TO_SET_MESSAGE, sizeof(SET_NOTHING_TO_SET_MESSAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (args->lengths[1] > 3 || args->values[1][0] != '-') {
        if (write(STDOUT_FILENO, SET_VALID_OPERATIONS_MESSAGE, sizeof(SET_VALID_OPERATIONS_MESSAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    switch (args->values[1][1]) {
    case 'e': {
        return ncsh_builtins_set_e();
    }
    default: {
        break;
    }
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}


