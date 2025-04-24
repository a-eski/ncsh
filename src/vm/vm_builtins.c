/* Copyright ncsh (C) by Alex Eski 2024 */

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../defines.h"
#include "../eskilib/ecolors.h"
#include "../eskilib/estr.h"
#include "../parser.h"
#include "../readline/history.h"
#include "../z/z.h"
#include "vm_builtins.h"

int builtins_disabled_state = 0;

#define Z_COMMAND_NOT_FOUND_MESSAGE "ncsh z: command not found, options not supported.\n"
[[nodiscard]]
int builtins_z(struct z_Database* const restrict z_db, struct Args* const restrict args, struct Arena* const arena,
               struct Arena* const scratch_arena)
{
    assert(z_db);
    assert(args->count > 0);

    if (args->count == 1) {
        z(NULL, 0, NULL, z_db, arena, *scratch_arena);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (args->count == 2) {
        assert(args->values[1]);

        // z print
        if (estrcmp_c(args->values[1], args->lengths[1], Z_PRINT, sizeof(Z_PRINT))) {
            z_print(z_db);
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }

        // z
        char cwd[PATH_MAX] = {0};
        if (!getcwd(cwd, PATH_MAX)) {
            perror(RED "ncsh z: Could not load cwd information" RESET);
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        z(args->values[1], args->lengths[1], cwd, z_db, arena, *scratch_arena);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (args->count > 2) {
        assert(args->values[1] && args->values[2]);

        // z add
        if (estrcmp_c(args->values[1], args->lengths[1], Z_ADD, sizeof(Z_ADD))) {
            if (z_add(args->values[2], args->lengths[2], z_db, arena) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
        // z rm/remove
        else if (estrcmp_c(args->values[1], args->lengths[1], Z_RM, sizeof(Z_RM)) ||
                 estrcmp_c(args->values[1], args->lengths[1], Z_REMOVE, sizeof(Z_REMOVE))) {
            if (z_remove(args->values[2], args->lengths[2], z_db) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    if (write(STDOUT_FILENO, Z_COMMAND_NOT_FOUND_MESSAGE, sizeof(Z_COMMAND_NOT_FOUND_MESSAGE)) == -1) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define HISTORY_COMMAND_NOT_FOUND_MESSAGE "ncsh history: command not found.\n"
[[nodiscard]]
int builtins_history(struct History* const restrict history, struct Args* const restrict args,
                     struct Arena* const arena, struct Arena* const scratch_arena)
{
    if (args->count == 1) {
        return history_command_display(history);
    }

    if (args->count == 2) {
        assert(args->values[1]);

        if (estrcmp_c(args->values[1], args->lengths[1], NCSH_HISTORY_COUNT, sizeof(NCSH_HISTORY_COUNT))) {
            return history_command_count(history);
        }
        else if (estrcmp_c(args->values[1], args->lengths[1], NCSH_HISTORY_CLEAN, sizeof(NCSH_HISTORY_CLEAN))) {
            return history_command_clean(history, arena, scratch_arena);
        }
    }

    if (args->count == 3) {
        assert(args->values[1] && args->values[2]);

        // z add
        if (estrcmp_c(args->values[1], args->lengths[1], NCSH_HISTORY_ADD, sizeof(NCSH_HISTORY_ADD))) {
            if (history_command_add(args->values[2], args->lengths[2], history, arena) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
        // z rm/remove
        else if (estrcmp_c(args->values[1], args->lengths[1], NCSH_HISTORY_RM, sizeof(NCSH_HISTORY_RM)) ||
                 estrcmp_c(args->values[1], args->lengths[1], NCSH_HISTORY_REMOVE, sizeof(NCSH_HISTORY_REMOVE))) {
            if (history_command_remove(args->values[2], args->lengths[2], history, arena, scratch_arena) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    if (write(STDOUT_FILENO, HISTORY_COMMAND_NOT_FOUND_MESSAGE, sizeof(HISTORY_COMMAND_NOT_FOUND_MESSAGE)) == -1) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    return NCSH_COMMAND_FAILED_CONTINUE;
}

[[nodiscard]]
int builtins_exit(struct Args* const restrict args)
{
    (void)args;
    return NCSH_COMMAND_EXIT;
}

[[nodiscard]]
int builtins_echo(struct Args* const restrict args)
{
    if (args->count <= 1) {
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    bool echo_add_newline = true;
    size_t i = 1;

    for (size_t j = 1; j < args->count; ++j) {
        if (args->lengths[j] != 3) {
            break;
        }

        if (CMP_2(args->values[j], "-n")) {
            echo_add_newline = false;
            i = 2;
        }
    }

    for (; i < args->count - 1; ++i) {
        printf("%s ", args->values[i]);
    }
    printf("%s", args->values[i]);

    if (echo_add_newline) {
        putchar('\n');
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define NCSH_TITLE "ncsh " NCSH_VERSION "\n"
#define NCSH_COPYRIGHT                                                                                                 \
    "Copyright (C) 2025 Alex Eski\n"                                                                                   \
    "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>."                                  \
    "This program comes with ABSOLUTELY NO WARRANTY.\n"                                                                \
    "This is free software, and you are welcome to redistribute it "                                                   \
    "under certain conditions.\n\n"

#define HELP_MESSAGE "ncsh help\n\n"
#define HELP_FORMAT "Builtin Commands: {command} {args}\n\n"
#define HELP_QUIT                                                                                                      \
    "q:		          To exit, type q, exit, or quit and press enter. You can also use Ctrl+D to exit.\n\n"
#define HELP_CHANGEDIR "cd/z:		          You can change directory with cd or z.\n\n"
#define HELP_Z                                                                                                         \
    "z {directory}:            A builtin autojump/z command. An enhanced cd command that keeps track of history and "  \
    "fuzzy matches against previously visited directories.\n\n"
#define HELP_Z_ADD "z add {directory}:        Manually add a directory to your z database.\n\n"
#define HELP_Z_RM                                                                                                      \
    "z rm {directory}:         Manually remove a directory from your z database. Can also call using 'z remove "       \
    "{directory}'.\n\n"
#define HELP_Z_PRINT "z print:                  Print out information about the entries in your z database.\n\n"
#define HELP_ECHO "echo:		          You can write things to the screen using echo.\n\n"
#define HELP_HISTORY "history:	          You can see your command history using the history command.\n\n"
#define HELP_HISTORY_COUNT                                                                                             \
    "history count:            You can see the number of entries in your history with history count command.\n\n"
#define HELP_HISTORY_CLEAN                                                                                             \
    "history clean:            Removes all duplicates from the history file and reloads deduplicated history into "    \
    "memory.\n\n"
#define HELP_HISTORY_ADD "history add {command}:    Add a command to your history without executing the command.\n\n"
#define HELP_HISTORY_RM                                                                                                \
    "history rm {command}:     Remove a command from your history. Please note, the history is cleaned first to "      \
    "dededuplicate. Can also call using 'history remove {command}.\n\n"
#define HELP_PWD "pwd:         	          Prints the current working directory.\n\n"
#define HELP_KILL "kill {processId}:         Terminates the process with associated processId.\n"

#define HELP_WRITE(str)                                                                                                \
    constexpr size_t str##_len = sizeof(str) - 1;                                                                      \
    if (write(STDOUT_FILENO, str, str##_len) == -1) {                                                                  \
        perror(RED NCSH_ERROR_STDOUT RESET);                                                                           \
        return NCSH_COMMAND_EXIT_FAILURE;                                                                              \
    }

[[nodiscard]]
int builtins_help(struct Args* const restrict args)
{
    (void)args;

    constexpr size_t len = sizeof(NCSH_TITLE) - 1;
    if (write(STDOUT_FILENO, NCSH_TITLE, len) == -1) {
        perror(RED NCSH_ERROR_STDOUT RESET);
        return NCSH_COMMAND_EXIT_FAILURE;
    }

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
    // HELP_WRITE(HELP_BASIC_CONTROLS);
    // CTRL + W, CTRL + U, etc.
    // HELP_WRITE(HELP_READLINE_INFO);
    // HELP_WRITE(HELP_READLINE_MORE_INFO);
    // autocomplete
    // HELP_WRITE(HELP_AUTOCOMPLETIONS);
    // HELP_WRITE(HELP_TAB_AUTOCOMPLETIONS);
    // HELP_WRITE(HELP_AUTOCOMPLETIONS_MORE_INFO);

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int builtins_cd(struct Args* const restrict args)
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
int builtins_pwd(struct Args* const restrict args)
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
int builtins_kill(struct Args* const restrict args)
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

int builtins_version(struct Args* const restrict args)
{
    (void)args;

    if (write(STDOUT_FILENO, NCSH_TITLE, sizeof(NCSH_TITLE)) == -1) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

void builtins_print()
{
    for (size_t i = 0; i < builtins_count; ++i) {
        printf("%s\n", builtins[i].value);
    }
}

void builtins_print_enabled()
{
    if (!builtins_disabled_state) {
        for (size_t i = 0; i < builtins_count; ++i) {
            printf("%s: enabled\n", builtins[i].value);
        }
    }
    else {
        for (size_t i = 0; i < builtins_count; ++i) {
            if ((builtins_disabled_state & builtins[i].flag))
                printf("%s: disabled\n", builtins[i].value);
            else
                printf("%s: enabled\n", builtins[i].value);
        }
    }
}

int builtins_disable(struct Args* const restrict args)
{
    // check if called by enabled or disabled
    size_t i = args->values[0][0] == 'e' ? 2 : 1;

    for (; i < args->count; ++i) {
        for (size_t j = 0; j < builtins_count; ++j) {
            if (estrcmp_c(args->values[i], args->lengths[i], builtins[j].value, builtins[j].length)) {
                if (!(builtins_disabled_state & builtins[j].flag)) {
                    builtins_disabled_state |= builtins[j].flag;
                    printf("ncsh disable: disabled builtin %s.\n", builtins[j].value);
                }
            }
        }
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define ENABLE_OPTION_NOT_SUPPORTED_MESSAGE "ncsh enable: command not found, option not supported.\n"
int builtins_enable(struct Args* const restrict args)
{
    if (!args->values[1]) {
        builtins_print();
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (args->lengths[1] == 3) {
        if (CMP_2(args->values[1], "-a")) {
            builtins_print_enabled();
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
        else if (CMP_2(args->values[1], "-n")) {
            builtins_disable(args);
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    if (write(STDOUT_FILENO, ENABLE_OPTION_NOT_SUPPORTED_MESSAGE, sizeof(ENABLE_OPTION_NOT_SUPPORTED_MESSAGE)) == -1) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

// NOTE: set is not implemented.
[[nodiscard]]
int builtins_set_e()
{
    puts("set e detected");
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define SET_NOTHING_TO_SET_MESSAGE "ncsh set: nothing to set, please pass in a value to set (i.e. '-e', '-c')\n"
#define SET_VALID_OPERATIONS_MESSAGE "ncsh set: valid set operations are in the form '-e', '-c', etc.\n"
[[nodiscard]]
int builtins_set(struct Args* const restrict args)
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
        return builtins_set_e();
    }
    default: {
        break;
    }
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

// not implemented
#define UNSET_NOTHING_TO_UNSET_MESSAGE "ncsh unset: nothing to unset, please pass in a value to unset.\n"
[[nodiscard]]
int builtins_unset(struct Args* const restrict args)
{
    if (!args->values[1]) {
        if (write(STDOUT_FILENO, UNSET_NOTHING_TO_UNSET_MESSAGE, sizeof(UNSET_NOTHING_TO_UNSET_MESSAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    bool is_set = var_exists(args->values[1], &args->vars);
    if (!is_set) {
        printf("ncsh unset: no value found for '%s' to unset.\n", args->values[1]);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }
    // TODO: need a way to unset, var_set doesn't work
    // var_set(args->values[1], NULL, arena, &args->vars)
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}
