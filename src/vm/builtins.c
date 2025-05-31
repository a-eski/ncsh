/* Copyright ncsh (C) by Alex Eski 2024 */
/* builtins.h: shell builtins implementations for ncsh */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#include <setjmp.h>
#include <string.h>
#endif /* ifndef _DEFAULT_SOURCE */
#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../alias.h"
#include "../args.h"
#include "../defines.h"
#include "../env.h"
#include "../eskilib/ecolors.h"
#include "../readline/history.h"
#include "../z/z.h"
#include "builtins.h"

extern jmp_buf env;
extern int vm_output_fd;

int builtins_disabled_state = 0;

ssize_t builtins_write(int fd, char* buf, size_t len)
{
    ssize_t bytes_written = write(fd, buf, len);
    if (bytes_written == -1 && errno == EPIPE)
        return -1;
    else if (bytes_written == -1)
        longjmp(env, -99);
    return bytes_written;
}

#define Z_COMMAND_NOT_FOUND_MESSAGE "ncsh z: command not found, options not supported.\n"
[[nodiscard]]
int builtins_z(z_Database* rst z_db, char** rst buffer, size_t* rst buf_lens, Arena* rst arena,
               Arena* rst scratch_arena)
{
    assert(z_db);
    assert(buffer && *buffer);

    if (buf_lens[0] == 0) {
        z(NULL, 0, NULL, z_db, arena, *scratch_arena);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    assert(buffer && buffer + 1);

    // skip first position since we know it is 'z'
    char** arg = buffer + 1;
    size_t* arg_lens = buf_lens + 1;
    if (arg_lens[1] == 0) {
        assert(arg && *arg);

        // z print
        if (estrcmp(*arg, *arg_lens, Z_PRINT, sizeof(Z_PRINT))) {
            z_print(z_db);
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
        // z count
        if (estrcmp(*arg, *arg_lens, Z_COUNT, sizeof(Z_COUNT))) {
            z_count(z_db);
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }

        // z
        char cwd[PATH_MAX] = {0};
        if (!getcwd(cwd, PATH_MAX)) {
            perror(RED "ncsh z: Could not load cwd information" RESET);
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        z(*arg, *arg_lens, cwd, z_db, arena, *scratch_arena);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (arg && arg[1] && !arg[2]) {
        assert(arg && *arg);

        // z add
        if (estrcmp(*arg, *arg_lens, Z_ADD, sizeof(Z_ADD))) {
            assert(arg[1] && arg_lens[1]);
            if (z_add(arg[1], arg_lens[1], z_db, arena) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
        // z rm/remove
        else if (estrcmp(*arg, *arg_lens, Z_RM, sizeof(Z_RM)) || estrcmp(*arg, *arg_lens, Z_REMOVE, sizeof(Z_REMOVE))) {
            assert(arg[1] && arg_lens[1]);
            if (z_remove(arg[1], arg_lens[1], z_db) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    if (builtins_write(vm_output_fd, Z_COMMAND_NOT_FOUND_MESSAGE, sizeof(Z_COMMAND_NOT_FOUND_MESSAGE) - 1) == -1) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define HISTORY_COMMAND_NOT_FOUND_MESSAGE "ncsh history: command not found.\n"
[[nodiscard]]
int builtins_history(History* rst history, char** rst buffer, size_t* rst buf_lens, Arena* rst arena,
                     Arena* rst scratch_arena)
{
    if (!buffer || !buffer[1]) {
        return history_command_display(history);
    }

    assert(buffer && *buffer && buffer + 1);

    // skip first position since we know it is 'history'
    char** arg = buffer + 1;
    if (!arg || !*arg) {
        if (builtins_write(vm_output_fd, HISTORY_COMMAND_NOT_FOUND_MESSAGE,
                           sizeof(HISTORY_COMMAND_NOT_FOUND_MESSAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }
        return NCSH_COMMAND_FAILED_CONTINUE;
    }

    size_t* arg_lens = buf_lens + 1;
    if (arg && arg_lens[1] == 0) {
        assert(arg && *arg);

        if (estrcmp(*arg, *arg_lens, NCSH_HISTORY_COUNT, sizeof(NCSH_HISTORY_COUNT))) {
            return history_command_count(history);
        }
        else if (estrcmp(*arg, *arg_lens, NCSH_HISTORY_CLEAN, sizeof(NCSH_HISTORY_CLEAN))) {
            return history_command_clean(history, arena, scratch_arena);
        }
    }

    if (arg && arg[1] && !arg[2]) {
        assert(arg && *arg && arg[1] && *arg[1]);

        // history add
        if (estrcmp(*arg, *arg_lens, NCSH_HISTORY_ADD, sizeof(NCSH_HISTORY_ADD))) {
            assert(arg[1] && arg_lens[1]);
            if (history_command_add(arg[1], arg_lens[1], history, arena) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
        // history rm/remove
        else if (estrcmp(*arg, *arg_lens, NCSH_HISTORY_RM, sizeof(NCSH_HISTORY_RM)) ||
                 estrcmp(*arg, *arg_lens, NCSH_HISTORY_REMOVE, sizeof(NCSH_HISTORY_REMOVE))) {
            assert(arg[1] && arg_lens[1]);
            if (history_command_remove(arg[1], arg_lens[1], history, arena, scratch_arena) != Z_SUCCESS) {
                return NCSH_COMMAND_FAILED_CONTINUE;
            }

            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    if (builtins_write(vm_output_fd, HISTORY_COMMAND_NOT_FOUND_MESSAGE,
                       sizeof(HISTORY_COMMAND_NOT_FOUND_MESSAGE) - 1) == -1) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    return NCSH_COMMAND_FAILED_CONTINUE;
}

#define ALIAS_ADD_USAGE "ncsh: alias: add usage: alias add {alias} {command}.\n"
#define ALIAS_REMOVE_USAGE "ncsh: alias: remove/rm usage: alias {remove/rm} {alias} {command}.\n"
#define ALIAS_USAGE                                                                                                    \
    "ncsh: alias: option not found. Options are alias or alias -p to print aliases, alias add, alias remove/rm, and "  \
    "alias delete to delete all aliases.\n"
int builtins_alias(char** rst buffer, size_t* rst buf_lens, Arena* rst arena)
{
    (void)buf_lens;
    assert(buffer && *buffer);

    if (!buffer || !buffer[1]) {
        alias_print(vm_output_fd);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    // skip first position since we know it is 'alias'
    char** arg = buffer + 1;
    size_t* arg_len = buf_lens + 1;
    if (estrcmp(*arg, *arg_len, NCSH_ALIAS_ADD, sizeof(NCSH_ALIAS_ADD))) {
        ++arg;
        ++arg_len;
        if (!arg || !*arg) {
            if (builtins_write(vm_output_fd, ALIAS_ADD_USAGE, sizeof(ALIAS_ADD_USAGE) - 1) == -1) {
                return NCSH_COMMAND_EXIT_FAILURE;
            }
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
        char* alias = *arg;
        size_t a_len = *arg_len;
        ++arg;
        ++arg_len;
        if (!arg || !*arg) {
            if (builtins_write(vm_output_fd, ALIAS_ADD_USAGE, sizeof(ALIAS_ADD_USAGE) - 1) == -1) {
                return NCSH_COMMAND_EXIT_FAILURE;
            }
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
        char* command = *arg;
        size_t c_len = *arg_len;
        alias_add_new(alias, a_len, command, c_len, arena);
    }
    else if (estrcmp(*arg, *arg_len, NCSH_ALIAS_RM, sizeof(NCSH_ALIAS_RM)) ||
             estrcmp(*arg, *arg_len, NCSH_ALIAS_REMOVE, sizeof(NCSH_ALIAS_REMOVE))) {
        ++arg;
        ++arg_len;
        if (!arg || !*arg) {
            if (builtins_write(vm_output_fd, ALIAS_REMOVE_USAGE, sizeof(ALIAS_REMOVE_USAGE) - 1) == -1) {
                return NCSH_COMMAND_EXIT_FAILURE;
            }
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
        alias_remove(*arg, *arg_len);
    }
    else if (estrcmp(*arg, *arg_len, NCSH_ALIAS_DELETE, sizeof(NCSH_ALIAS_DELETE))) {
        alias_delete();
    }
    else if (estrcmp(*arg, *arg_len, NCSH_ALIAS_PRINT, sizeof(NCSH_ALIAS_PRINT))) {
        alias_print(vm_output_fd);
    }
    else {
        if (builtins_write(vm_output_fd, ALIAS_USAGE, sizeof(ALIAS_USAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }
        return NCSH_COMMAND_FAILED_CONTINUE;
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define UNALIAS_USAGE                                                                                                  \
    "ncsh: unalias: usage is unalias, unalias -a to delete all aliases, or unalias {aliases} to remove specific "      \
    "alias(es).\n"
int builtins_unalias(char** rst buffer, size_t* rst buf_lens)
{
    (void)buf_lens;
    assert(buffer && *buffer);

    if (!buffer || !buffer[1]) {
        alias_print(vm_output_fd);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    // skip first position since we know it is 'unalias'
    char** arg = buffer + 1;
    size_t* arg_len = buf_lens + 1;
    if (estrcmp(*arg, *arg_len, NCSH_UNALIAS_DELETE, sizeof(NCSH_UNALIAS_DELETE)) ||
        estrcmp(*arg, *arg_len, NCSH_UNALIAS_DELETE_ALIAS, sizeof(NCSH_UNALIAS_DELETE_ALIAS))) {
        alias_delete();
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }
    else if (arg) {
        while (arg) {
            alias_remove(*arg, *arg_len);
            ++arg;
            ++arg_len;
        }
    }
    else {
        if (builtins_write(vm_output_fd, UNALIAS_USAGE, sizeof(UNALIAS_USAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }
        return NCSH_COMMAND_FAILED_CONTINUE;
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int builtins_exit(char** rst buffer, size_t* rst buf_lens)
{
    (void)buffer;
    (void)buf_lens;
    return NCSH_COMMAND_EXIT;
}

[[nodiscard]]
int builtins_echo(char** rst buffer, size_t* rst buf_lens)
{
    assert(buffer && *buffer);
    char** arg = buffer + 1;
    size_t* arg_lens = buf_lens + 1;
    if (!arg || !*arg) {
        putchar('\n');
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    bool echo_add_newline = true;
    // process options for echo
    while (arg && *arg) {
        if (estrcmp(*arg, *arg_lens, NCSH_ECHO_NO_NEWLINE, sizeof(NCSH_ECHO_NO_NEWLINE))) {
            echo_add_newline = false;
            break;
        }
        ++arg;
        ++arg_lens;
    }

    // send output for echo
    arg = !echo_add_newline ? arg + 1 : buffer + 1;
    char* prev = NULL;
    while (arg && *arg) {
        prev = *arg;
        if (!prev)
            break;
        ++arg;
        if (!arg || !*arg)
            break;

        dprintf(vm_output_fd, "%s ", prev);
    }
    if (prev)
        dprintf(vm_output_fd, "%s", prev);

    if (echo_add_newline) {
        putchar('\n');
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define NCSH_TITLE "ncsh " NCSH_VERSION "\n"
#define NCSH_COPYRIGHT                                                                                                 \
    "Copyright (C) 2025 Alex Eski\n"                                                                                   \
    "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>. "                                 \
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
    if (builtins_write(vm_output_fd, str, str##_len) == -1) {                                                          \
        perror(RED NCSH_ERROR_STDOUT RESET);                                                                           \
        return NCSH_COMMAND_EXIT_FAILURE;                                                                              \
    }

[[nodiscard]]
int builtins_help(char** rst buffer, size_t* rst buf_lens)
{
    (void)buffer;
    (void)buf_lens;

    constexpr size_t len = sizeof(NCSH_TITLE) - 1;
    if (builtins_write(vm_output_fd, NCSH_TITLE, len) == -1) {
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

    fflush(stdout);
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define NCSH_COULD_NOT_CD_MESSAGE "ncsh cd: could not change directory.\n"
[[nodiscard]]
int builtins_cd(char** rst buffer, size_t* rst buf_lens)
{
    (void)buf_lens;
    assert(buffer && *buffer);

    // skip first position since we know it is 'cd'
    char* arg = *(buffer + 1);
    if (!arg) {
        char* home = getenv("HOME");
        if (!home) {
            if (builtins_write(STDERR_FILENO, NCSH_COULD_NOT_CD_MESSAGE, sizeof(NCSH_COULD_NOT_CD_MESSAGE) - 1) == -1)
                return NCSH_COMMAND_EXIT_FAILURE;
        }
        else if (chdir(home)) {
            perror("ncsh: could not change directory");
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (chdir(arg)) {
        if (builtins_write(STDERR_FILENO, NCSH_COULD_NOT_CD_MESSAGE, sizeof(NCSH_COULD_NOT_CD_MESSAGE) - 1) == -1)
            return NCSH_COMMAND_EXIT_FAILURE;
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int builtins_pwd(char** rst buffer, size_t* rst buf_lens)
{
    (void)buffer;
    (void)buf_lens;

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
int builtins_kill(char** rst buffer, size_t* rst buf_lens)
{
    (void)buf_lens;
    assert(buffer && *buffer && buffer + 1);
    if (!buffer) {
        if (builtins_write(vm_output_fd, KILL_NOTHING_TO_KILL_MESSAGE, sizeof(KILL_NOTHING_TO_KILL_MESSAGE) - 1) ==
            -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    // skip first position since we know it is 'kill'
    char* arg = *(buffer + 1);
    if (!arg || !*arg) {
        if (builtins_write(vm_output_fd, KILL_NOTHING_TO_KILL_MESSAGE, sizeof(KILL_NOTHING_TO_KILL_MESSAGE) - 1) ==
            -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    pid_t pid = atoi(arg);
    if (!pid) {
        if (builtins_write(vm_output_fd, KILL_COULDNT_PARSE_PID_MESSAGE, sizeof(KILL_COULDNT_PARSE_PID_MESSAGE) - 1) ==
            -1) {
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

int builtins_version(char** rst buffer, size_t* rst buf_lens)
{
    (void)buffer;
    (void)buf_lens;
    builtins_write(vm_output_fd, NCSH_TITLE, sizeof(NCSH_TITLE) - 1);
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

// TODO: finish enable & disable implementation
int builtins_disable(char** rst buffer, size_t* rst buf_lens)
{
    assert(buffer && *buffer);

    // skip first position since we know it is 'enable' or 'disable'
    char** arg = buffer + 1;
    if (!arg) {
        builtins_print();
    }
    size_t* arg_lens = buf_lens + 1;

    // check if called by enabled or disabled
    // (enabled has extra option to specify disable, so start at 2 instead of 1 in that case,
    // because 'disable' is 1 arg, but 'enable -n' is 2)
    if (**arg == 'e') {
        ++arg;
        ++arg_lens;
    }

    while (arg && *arg) {
        for (size_t j = 0; j < builtins_count; ++j) {
            if (estrcmp(*arg, *arg_lens, builtins[j].value, builtins[j].length)) {
                if (!(builtins_disabled_state & builtins[j].flag)) {
                    builtins_disabled_state |= builtins[j].flag;
                    printf("ncsh disable: disabled builtin %s.\n", builtins[j].value);
                }
            }
            ++arg;
            ++arg_lens;
        }
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define ENABLE_OPTION_NOT_SUPPORTED_MESSAGE "ncsh enable: command not found, options entered not supported.\n"
int builtins_enable(char** rst buffer, size_t* rst buf_lens)
{
    assert(buffer && *buffer);

    // skip first position since we know it is 'enable'
    char** arg = buffer + 1;
    if (!arg) {
        builtins_print();
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }
    size_t* arg_lens = buf_lens + 1;

    if (*arg_lens == 2) {
        if (CMP_2(*arg, "-a")) {
            builtins_print_enabled();
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
        else if (CMP_2(*arg, "-n")) {
            builtins_disable(buffer, buf_lens);
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    if (builtins_write(vm_output_fd, ENABLE_OPTION_NOT_SUPPORTED_MESSAGE,
                       sizeof(ENABLE_OPTION_NOT_SUPPORTED_MESSAGE) - 1) == -1) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

// TODO: implement export
#define EXPORT_OPTION_NOT_SUPPORTED_MESSAGE "ncsh export: command not found, options entered not supported.\n"
#define EXPORT_OPTIONS_MESSAGE                                                                                         \
    "ncsh export: please pass in at least once argument. export currently supports modifying $PATH and $HOME."
int builtins_export(Args* rst args, size_t* rst buf_lens)
{
    (void)buf_lens;
    assert(args && args->head && args->head->next);

    // skip first position since we know it is 'export'
    Arg* arg = args->head->next->next;
    if (!arg || !arg->val) {
        if (builtins_write(vm_output_fd, EXPORT_OPTION_NOT_SUPPORTED_MESSAGE,
                           sizeof(EXPORT_OPTION_NOT_SUPPORTED_MESSAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }
        return NCSH_COMMAND_FAILED_CONTINUE;
    }

    if (estrcmp(arg->val, arg->len, NCSH_PATH_VAR, sizeof(NCSH_PATH_VAR))) {
        puts("export $PATH found");
    }
    else if (estrcmp(arg->val, arg->len, NCSH_HOME_VAR, sizeof(NCSH_HOME_VAR))) {
        puts("export $HOME found");
    }
    else {
        if (builtins_write(vm_output_fd, EXPORT_OPTION_NOT_SUPPORTED_MESSAGE,
                           sizeof(EXPORT_OPTION_NOT_SUPPORTED_MESSAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

// TODO: implement declare

// NOTE: set is not implemented.
// TODO: finish set/unset implementations
[[nodiscard]]
int builtins_set_e()
{
    puts("set e detected");
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define SET_NOTHING_TO_SET_MESSAGE "ncsh set: nothing to set, please pass in a value to set (i.e. '-e', '-c')\n"
#define SET_VALID_OPERATIONS_MESSAGE "ncsh set: valid set operations are in the form '-e', '-c', etc.\n"
[[nodiscard]]
int builtins_set(Args* rst args, size_t* rst buf_lens)
{
    (void)buf_lens;
    assert(args && args->head && args->head->next);

    // skip first position since we know it is 'set'
    Arg* arg = args->head->next->next;
    if (!arg || !arg->val) {
        if (builtins_write(vm_output_fd, SET_NOTHING_TO_SET_MESSAGE, sizeof(SET_NOTHING_TO_SET_MESSAGE) - 1) == -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (arg->len > 3 || arg->val[0] != '-') {
        if (builtins_write(vm_output_fd, SET_VALID_OPERATIONS_MESSAGE, sizeof(SET_VALID_OPERATIONS_MESSAGE) - 1) ==
            -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    switch (arg->val[1]) {
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
int builtins_unset(Args* rst args, size_t* rst buf_lens)
{
    (void)buf_lens;
    assert(args && args->head && args->head->next);

    // skip first position since we know it is 'unset'
    Arg* arg = args->head->next->next;
    if (!arg || !arg->val) {
        if (builtins_write(vm_output_fd, UNSET_NOTHING_TO_UNSET_MESSAGE, sizeof(UNSET_NOTHING_TO_UNSET_MESSAGE) - 1) ==
            -1) {
            return NCSH_COMMAND_EXIT_FAILURE;
        }

        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    /*bool is_set = var_exists(arg->val, &args->vars);
    if (!is_set) {
        printf("ncsh unset: no value found for '%s' to unset.\n", args->values[1]);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }*/
    // TODO: need a way to unset, var_set doesn't work
    // var_set(args->values[1], NULL, arena, &args->vars)
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}
