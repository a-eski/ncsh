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
#include <unistd.h>

#include "../eskilib/eskilib_string.h"
#include "../eskilib/eskilib_colors.h"
#include "../ncsh_defines.h"
#include "../ncsh_history.h"
#include "../ncsh_parser.h"
#include "ncsh_vm_builtins.h"
#include "../z/z.h"

[[nodiscard]]
int_fast32_t ncsh_builtins_z(struct z_Database* z_db, struct ncsh_Args* args)
{
    assert(z_db);
    assert(args->count > 0);

    if (args->count == 2) {
        assert(args->values[1]);

        // z print
        if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_Z_PRINT, sizeof(NCSH_Z_PRINT))) {
            z_print(z_db);
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }

	// z
        char cwd[PATH_MAX] = {0};
        if (!getcwd(cwd, PATH_MAX)) {
            perror(RED "ncsh z: Could not load cwd information" RESET);
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        z(args->values[1], args->lengths[1], cwd, z_db);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (args->count > 2) {
        assert(args->values[1] && args->values[2]);

	// z add
        if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_Z_ADD, sizeof(NCSH_Z_ADD))) {
            if (z_add(args->values[2], args->lengths[2], z_db) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
	// z rm/remove
        else if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_Z_RM, sizeof(NCSH_Z_RM)) ||
                eskilib_string_compare(args->values[1], args->lengths[1], NCSH_Z_REMOVE, sizeof(NCSH_Z_REMOVE))) {
            if (z_remove(args->values[2], args->lengths[2], z_db) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    ncsh_write_literal("ncsh z: command not found.\n");
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int_fast32_t ncsh_builtins_history(struct ncsh_History* history, struct ncsh_Args* args)
{
    if (args->count == 1) {
        return ncsh_history_command_display(history);
    }

    if (args->count == 2) {
        assert(args->values[1]);

        if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_HISTORY_COUNT,
                                   sizeof(NCSH_HISTORY_COUNT))) {
            return ncsh_history_command_count(history);
        }
        else if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_HISTORY_CLEAN,
                                        sizeof(NCSH_HISTORY_CLEAN))) {
            return ncsh_history_command_clean(history);
        }
    }

    if (args->count == 3) {
        assert(args->values[1] && args->values[2]);

        // z add
        if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_HISTORY_ADD, sizeof(NCSH_HISTORY_ADD))) {
            if (ncsh_history_command_add(args->values[2], args->lengths[2], history) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
	// z rm/remove
        else if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_HISTORY_RM, sizeof(NCSH_HISTORY_RM)) ||
                eskilib_string_compare(args->values[1], args->lengths[1], NCSH_HISTORY_REMOVE, sizeof(NCSH_HISTORY_REMOVE))) {
            if (ncsh_history_command_remove(args->values[2], args->lengths[2], history) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    ncsh_write_literal("ncsh history: command not found.\n");
    return NCSH_COMMAND_FAILED_CONTINUE;
}

[[nodiscard]]
int_fast32_t ncsh_builtins_exit(struct ncsh_Args* args)
{
    (void)args; // to not get compiler warnings
    return NCSH_COMMAND_EXIT;
}

[[nodiscard]]
int_fast32_t ncsh_builtins_echo(struct ncsh_Args* args)
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
    "ncsh: Copyright (C) 2025 Alex Eski\n"                                                                              \
    "This program comes with ABSOLUTELY NO WARRANTY.\n"                                                                \
    "This is free software, and you are welcome to redistribute it "                                                   \
    "under certain conditions.\n\n"

#define HELP_MESSAGE "ncsh help\n\n"
#define HELP_FORMAT "Builtin Commands: {command} {args}\n\n"
#define HELP_QUIT "q:		          To exit, type q, exit, or quit and press enter. You can also use Ctrl+D to exit.\n\n"
#define HELP_CHANGEDIR "cd/z:		          You can change directory with cd or z.\n\n"
#define HELP_Z "z {directory}:            A builtin autojump/z command. An enhanced cd command that keeps track of history and fuzzy matches against previously visited directories.\n\n"
#define HELP_Z_ADD "z add {directory}:        Manually add a directory to your z database.\n\n"
#define HELP_Z_RM "z rm {directory}:         Manually remove a directory from your z database. Can also call using 'z remove {directory}'.\n\n"
#define HELP_Z_PRINT "z print:                  Print out information about the entries in your z database.\n\n"
#define HELP_ECHO "echo:		          You can write things to the screen using echo.\n\n"
#define HELP_HISTORY "history:	          You can see your command history using the history command.\n\n"
#define HELP_HISTORY_COUNT                                                                                        \
    "history count:            You can see the number of entries in your history with history count command.\n\n"
#define HELP_HISTORY_CLEAN "history clean:            Removes all duplicates from the history file and reloads deduplicated history into memory.\n\n"
#define HELP_HISTORY_ADD "history add {command}:    Add a command to your history without executing the command.\n\n"
#define HELP_HISTORY_RM "history rm {command}:     Remove a command from your history. Please note, the history is cleaned first to dededuplicate. Can also call using 'history remove {command}.\n\n"
#define HELP_PWD "pwd:         	          Prints the current working directory.\n\n"
#define HELP_KILL "kill {processId}:         Terminates the process with associated processId.\n"

#define HELP_WRITE(str)                                                                                                \
    constexpr size_t str##_len = sizeof(str) - 1;                                                                      \
    if (write(STDOUT_FILENO, str, str##_len) == -1) {                                                                  \
        perror(RED NCSH_ERROR_STDOUT RESET);                                                                           \
        return NCSH_COMMAND_EXIT_FAILURE;                                                                              \
    }

[[nodiscard]]
int_fast32_t ncsh_builtins_help(struct ncsh_Args* args)
{
    (void)args; // to not get compiler warnings

    HELP_WRITE(NCSH_COPYRIGHT);
    HELP_WRITE(HELP_MESSAGE);
    HELP_WRITE(HELP_FORMAT);
    HELP_WRITE(HELP_QUIT);
    HELP_WRITE(HELP_CHANGEDIR);
    HELP_WRITE(HELP_Z);
    HELP_WRITE(HELP_Z_ADD);
    HELP_WRITE(HELP_Z_RM);
    HELP_WRITE(HELP_Z_PRINT);
    HELP_WRITE(HELP_ECHO);
    HELP_WRITE(HELP_HISTORY);
    HELP_WRITE(HELP_HISTORY_COUNT);
    HELP_WRITE(HELP_HISTORY_CLEAN);
    HELP_WRITE(HELP_HISTORY_ADD);
    HELP_WRITE(HELP_HISTORY_RM);
    HELP_WRITE(HELP_PWD);
    HELP_WRITE(HELP_KILL);

    // controls
    // autocomplete
    // tab autocomplete
    // CTRL + W, CTRL + U

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int_fast32_t ncsh_builtins_cd(struct ncsh_Args* args)
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

[[nodiscard]]
int_fast32_t ncsh_builtins_pwd(struct ncsh_Args* args)
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
[[nodiscard]]
int_fast32_t ncsh_builtins_kill(struct ncsh_Args* args)
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
[[nodiscard]]
int_fast32_t ncsh_builtins_set_e()
{
    puts("sets e detected");
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define SET_NOTHING_TO_SET_MESSAGE "ncsh set: nothing to set, please pass in a value to set (i.e. '-e', '-c')"
#define SET_VALID_OPERATIONS_MESSAGE "ncsh set: valid set operations are in the form '-e', '-c', etc."
[[nodiscard]]
int_fast32_t ncsh_builtins_set(struct ncsh_Args* args)
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
