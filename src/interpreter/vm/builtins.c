/* Copyright ncsh (C) by Alex Eski 2024 */
/* builtins.h: shell builtins implementations for ncsh */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif /* ifndef _DEFAULT_SOURCE */

#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../alias.h"
#include "../../arena.h"
#include "../../defines.h"
#include "../../io/history.h"
#include "../../ttyio/ttyio.h"
#include "../../shell.h"
#include "../../z/z.h"
#include "vm_types.h"

/* External values */
extern jmp_buf env;      // from main.c, used on unrecoverable failures
extern int vm_output_fd; // from vm.c, used as fd for writing to stdout

/* Shared builtins data and functions */
static int builtins_disabled_state = 0;

int builtins_write(int fd, char* buf, size_t len)
{
    int bytes_written = tty_dwrite(fd, buf, len);
    if (bytes_written == EOF && errno == EPIPE) {
        return EOF;
    }
    else if (bytes_written == EOF) {
        longjmp(env, FAILURE_BUILTIN_WRITE);
    }
    return bytes_written;
}

int builtins_writeln(int fd, char* buf, size_t len)
{
    int bytes_written = tty_dwriteln(fd, buf, len);
    if (bytes_written == EOF && errno == EPIPE) {
        return EOF;
    }
    else if (bytes_written == EOF) {
        longjmp(env, FAILURE_BUILTIN_WRITE);
    }
    return bytes_written;
}

/* Forward Declarations */
#define Z "z" // the base command, changes directory
#define Z_ADD "add"
#define Z_RM "rm"
#define Z_REMOVE "remove" // alias for rm
#define Z_PRINT "print"
#define Z_COUNT "count"
int builtins_z(z_Database* restrict z_db, char** restrict buffer, size_t* restrict buf_lens, Arena* arena, Arena* restrict scratch);

#define NCSH_HISTORY "history" // the base command, displays history
#define NCSH_HISTORY_COUNT "count"
#define NCSH_HISTORY_CLEAN "clean"
#define NCSH_HISTORY_ADD "add"
#define NCSH_HISTORY_RM "rm" // alias for rm
#define NCSH_HISTORY_REMOVE "remove"
int builtins_history(History* restrict history, char** restrict buffer, size_t* restrict buf_lens, Arena* restrict arena,
                     Arena* restrict scratch);

#define NCSH_ALIAS "alias"
#define NCSH_ALIAS_PRINT "-p"
#define NCSH_ALIAS_PRINT_ "print"
#define NCSH_ALIAS_ADD "add"
#define NCSH_ALIAS_RM "rm"
#define NCSH_ALIAS_REMOVE "remove"
#define NCSH_ALIAS_DELETE "delete"
int builtins_alias(char** restrict buffer, size_t* restrict buf_lens, Arena* restrict arena);

#define NCSH_UNALIAS "unalias"
#define NCSH_UNALIAS_DELETE "-a"
#define NCSH_UNALIAS_DELETE_ALIAS "delete"
int builtins_unalias(char** restrict buffer, size_t* restrict buf_lens);

#define NCSH_EXIT "exit" // the base command
#define NCSH_QUIT "quit" // alias for exit
#define NCSH_Q "q"       // alias for exit
int builtins_exit(char** restrict buffer, size_t* restrict buf_lens);

#define NCSH_ECHO "echo"
#define NCSH_ECHO_NO_NEWLINE "-n"
int builtins_echo(char** restrict buffer, size_t* restrict buf_lens);

#define NCSH_HELP "help"
int builtins_help(char** restrict buffer, size_t* restrict buf_lens);

#define NCSH_CD "cd"
int builtins_cd(char** restrict buffer, size_t* restrict buf_lens);

#define NCSH_PWD "pwd"
int builtins_pwd(char** restrict buffer, size_t* restrict buf_lens);

#define NCSH_KILL "kill"
int builtins_kill(char** restrict buffer, size_t* restrict buf_lens);

#define NCSH_VERSION_CMD "version"
int builtins_version(char** restrict buffer, size_t* restrict buf_lens);

#define NCSH_TRUE "true"
int builtins_true(char** restrict buffer, size_t* restrict buf_lens);

#define NCSH_FALSE "false"
int builtins_false(char** restrict buffer, size_t* restrict buf_lens);

// TODO: finish implementation
#define NCSH_ENABLE "enable"
int builtins_enable(char** restrict buffer, size_t* restrict buf_lens);

// TODO: finish implementation
#define NCSH_DISABLE "disable"
int builtins_disable(char** restrict buffer, size_t* restrict buf_lens);

// TODO: finish implementation
// #define NCSH_EXPORT "export"
// int builtins_export(Statements* restrict toks, size_t* restrict buf_lens);
//
// TODO: finish implementation
// #define NCSH_SET "set"
// int builtins_set(Statements* restrict toks, size_t* restrict buf_lens);
//
// TODO: finish implementation
// #define NCSH_UNSET "unset"
// int builtins_unset(Statements* restrict toks, size_t* restrict buf_lens);

/* Types */
// clang-format off
enum Builtins_Disabled : long {
    BF_ALL_ENABLED = 0,
    BF_EXIT =        1 << 0,
    BF_ECHO =        1 << 1,
    BF_HELP =        1 << 2,
    BF_CD =          1 << 3,
    BF_PWD =         1 << 4,
    BF_KILL =        1 << 5,
    BF_VERSION =     1 << 6,
    BF_ALIAS =       1 << 7,
    BF_UNALIAS =     1 << 8,
    BF_TRUE =        1 << 9,
    BF_FALSE =       1 << 10,
    /*BF_ENABLE =      1 << 7,
    BF_DISABLE =     1 << 8,
    BF_EXPORT =      1 << 9,
    BF_SET =         1 << 10,
    BF_UNSET =       1 << 11,*/
};
// clang-format on

typedef struct {
    enum Builtins_Disabled flag;
    size_t length;
    char* value;
    int (*func)(char** restrict r, size_t* restrict l);
} Builtin;

static const Builtin builtins[] = {
    {.flag = BF_EXIT, .length = sizeof(NCSH_EXIT), .value = NCSH_EXIT, .func = &builtins_exit},
    {.flag = BF_EXIT, .length = sizeof(NCSH_QUIT), .value = NCSH_QUIT, .func = &builtins_exit},
    {.flag = BF_EXIT, .length = sizeof(NCSH_Q), .value = NCSH_Q, .func = &builtins_exit},
    {.flag = BF_ECHO, .length = sizeof(NCSH_ECHO), .value = NCSH_ECHO, .func = &builtins_echo},
    {.flag = BF_HELP, .length = sizeof(NCSH_HELP), .value = NCSH_HELP, .func = &builtins_help},
    {.flag = BF_CD, .length = sizeof(NCSH_CD), .value = NCSH_CD, .func = &builtins_cd},
    {.flag = BF_PWD, .length = sizeof(NCSH_PWD), .value = NCSH_PWD, .func = &builtins_pwd},
    {.flag = BF_KILL, .length = sizeof(NCSH_KILL), .value = NCSH_KILL, .func = &builtins_kill},
    {.flag = BF_VERSION, .length = sizeof(NCSH_VERSION_CMD), .value = NCSH_VERSION_CMD, .func = &builtins_version},
    {.flag = BF_UNALIAS, .length = sizeof(NCSH_UNALIAS), .value = NCSH_UNALIAS, .func = &builtins_unalias},
    {.flag = BF_TRUE, .length = sizeof(NCSH_TRUE), .value = NCSH_TRUE, .func = &builtins_true},
    {.flag = BF_FALSE, .length = sizeof(NCSH_FALSE), .value = NCSH_FALSE, .func = &builtins_false},
    /*{.flag = BF_ENABLE, .length = sizeof(NCSH_ENABLE), .value = NCSH_ENABLE, .func = &builtins_enable},
    {.flag = BF_DISABLE, .length = sizeof(NCSH_DISABLE), .value = NCSH_DISABLE, .func = &builtins_disable},
    {.flag = BF_EXPORT, .length = sizeof(NCSH_EXPORT), .value = NCSH_EXPORT, .func = &builtins_export},
    {.flag = BF_SET, .length = sizeof(NCSH_SET), .value = NCSH_SET, .func = &builtins_set},
    {.flag = BF_UNSET, .length = sizeof(NCSH_UNSET), .value = NCSH_UNSET, .func = &builtins_unset},*/
};

static constexpr size_t builtins_count = sizeof(builtins) / sizeof(builtins[0]);

/* Implementations */
#define Z_COMMAND_NOT_FOUND_MESSAGE "ncsh z: command not found, options not supported."

[[nodiscard]]
int builtins_z(z_Database* restrict z_db, char** restrict buffer, size_t* restrict buf_lens, Arena* restrict arena, Arena* restrict scratch)
{
    assert(z_db);
    assert(buffer && *buffer);
    assert(buf_lens && * buf_lens);

    if (buf_lens[1] == 0) {
        z(NULL, 0, NULL, z_db, arena, *scratch);
        return EXIT_SUCCESS;
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
            return EXIT_SUCCESS;
        }
        // z count
        if (estrcmp(*arg, *arg_lens, Z_COUNT, sizeof(Z_COUNT))) {
            z_count(z_db);
            return EXIT_SUCCESS;
        }

        // z
        char cwd[PATH_MAX] = {0};
        if (!getcwd(cwd, PATH_MAX)) {
            tty_perror("ncsh z: Could not load cwd information");
            return EXIT_FAILURE;
        }

        z(*arg, *arg_lens, cwd, z_db, arena, *scratch);
        return EXIT_SUCCESS;
    }

    if (arg && arg[1] && !arg[2]) {
        assert(arg && *arg);

        // z add
        if (estrcmp(*arg, *arg_lens, Z_ADD, sizeof(Z_ADD))) {
            assert(arg[1] && arg_lens[1]);
            if (z_add(arg[1], arg_lens[1], z_db, arena) != Z_SUCCESS) {
                return EXIT_FAILURE_CONTINUE;
            }

            return EXIT_SUCCESS;
        }
        // z rm/remove
        else if (estrcmp(*arg, *arg_lens, Z_RM, sizeof(Z_RM)) || estrcmp(*arg, *arg_lens, Z_REMOVE, sizeof(Z_REMOVE))) {
            assert(arg[1] && arg_lens[1]);
            if (z_remove(arg[1], arg_lens[1], z_db) != Z_SUCCESS) {
                return EXIT_FAILURE_CONTINUE;
            }

            return EXIT_SUCCESS;
        }
    }

    if (builtins_writeln(vm_output_fd, Z_COMMAND_NOT_FOUND_MESSAGE, sizeof(Z_COMMAND_NOT_FOUND_MESSAGE) - 1) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

#define HISTORY_COMMAND_NOT_FOUND_MESSAGE "ncsh history: command not found."

[[nodiscard]]
int builtins_history(History* restrict history, char** restrict buffer, size_t* restrict buf_lens, Arena* restrict arena,
                     Arena* restrict scratch)
{
    if (!buffer || !buffer[1]) {
        return history_command_display(history);
    }

    assert(buffer && *buffer && buffer + 1);

    // skip first position since we know it is 'history'
    char** arg = buffer + 1;
    if (!arg || !*arg) {
        if (builtins_writeln(vm_output_fd, HISTORY_COMMAND_NOT_FOUND_MESSAGE,
                           sizeof(HISTORY_COMMAND_NOT_FOUND_MESSAGE) - 1) == -1) {
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE_CONTINUE;
    }

    size_t* arg_lens = buf_lens + 1;
    if (arg && arg_lens[1] == 0) {
        assert(arg && *arg);

        if (estrcmp(*arg, *arg_lens, NCSH_HISTORY_COUNT, sizeof(NCSH_HISTORY_COUNT))) {
            return history_command_count(history);
        }
        else if (estrcmp(*arg, *arg_lens, NCSH_HISTORY_CLEAN, sizeof(NCSH_HISTORY_CLEAN))) {
            return history_command_clean(history, arena, scratch);
        }
    }

    if (arg && arg[1] && !arg[2]) {
        assert(arg && *arg && arg[1] && *arg[1]);

        // history add
        if (estrcmp(*arg, *arg_lens, NCSH_HISTORY_ADD, sizeof(NCSH_HISTORY_ADD))) {
            assert(arg[1] && arg_lens[1]);
            if (history_command_add(arg[1], arg_lens[1], history, arena) != Z_SUCCESS) {
                return EXIT_FAILURE_CONTINUE;
            }

            return EXIT_SUCCESS;
        }
        // history rm/remove
        else if (estrcmp(*arg, *arg_lens, NCSH_HISTORY_RM, sizeof(NCSH_HISTORY_RM)) ||
                 estrcmp(*arg, *arg_lens, NCSH_HISTORY_REMOVE, sizeof(NCSH_HISTORY_REMOVE))) {
            assert(arg[1] && arg_lens[1]);
            if (history_command_remove(arg[1], arg_lens[1], history, arena, scratch) != Z_SUCCESS) {
                return EXIT_FAILURE_CONTINUE;
            }

            return EXIT_SUCCESS;
        }
    }

    if (builtins_writeln(vm_output_fd, HISTORY_COMMAND_NOT_FOUND_MESSAGE,
                       sizeof(HISTORY_COMMAND_NOT_FOUND_MESSAGE) - 1) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_FAILURE_CONTINUE;
}

#define ALIAS_ADD_USAGE "ncsh: alias: add usage: alias add {alias} {command}."
#define ALIAS_REMOVE_USAGE "ncsh: alias: remove/rm usage: alias {remove/rm} {alias} {command}."
#define ALIAS_USAGE                                                                                                    \
    "ncsh: alias: option not found. Options are alias or alias -p to print aliases, alias add, alias remove/rm, and "  \
    "alias delete to delete all aliases."

[[nodiscard]]
int builtins_alias(char** restrict buffer, [[maybe_unused]] size_t* restrict buf_lens, Arena* restrict arena)
{
    assert(buffer && *buffer);

    if (!buffer || !buffer[1]) {
        alias_print(vm_output_fd);
        return EXIT_SUCCESS;
    }

    // skip first position since we know it is 'alias'
    char** arg = buffer + 1;
    size_t* arg_len = buf_lens + 1;
    if (estrcmp(*arg, *arg_len, NCSH_ALIAS_ADD, sizeof(NCSH_ALIAS_ADD))) {
        ++arg;
        ++arg_len;
        if (!arg || !*arg) {
            if (builtins_writeln(vm_output_fd, ALIAS_ADD_USAGE, sizeof(ALIAS_ADD_USAGE) - 1) == -1) {
                return EXIT_FAILURE;
            }
            return EXIT_FAILURE_CONTINUE;
        }
        char* alias = *arg;
        size_t a_len = *arg_len;
        ++arg;
        ++arg_len;
        if (!arg || !*arg) {
            if (builtins_writeln(vm_output_fd, ALIAS_ADD_USAGE, sizeof(ALIAS_ADD_USAGE) - 1) == -1) {
                return EXIT_FAILURE;
            }
            return EXIT_FAILURE_CONTINUE;
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
            if (builtins_writeln(vm_output_fd, ALIAS_REMOVE_USAGE, sizeof(ALIAS_REMOVE_USAGE) - 1) == -1) {
                return EXIT_FAILURE;
            }
            return EXIT_FAILURE_CONTINUE;
        }
        alias_remove(*arg, *arg_len);
    }
    else if (estrcmp(*arg, *arg_len, NCSH_ALIAS_DELETE, sizeof(NCSH_ALIAS_DELETE))) {
        alias_delete();
    }
    else if (estrcmp(*arg, *arg_len, NCSH_ALIAS_PRINT, sizeof(NCSH_ALIAS_PRINT)) ||
             estrcmp(*arg, *arg_len, NCSH_ALIAS_PRINT_, sizeof(NCSH_ALIAS_PRINT_))) {
        alias_print(vm_output_fd);
    }
    else {
        if (builtins_writeln(vm_output_fd, ALIAS_USAGE, sizeof(ALIAS_USAGE) - 1) == -1) {
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE_CONTINUE;
    }

    return EXIT_SUCCESS;
}

#define UNALIAS_USAGE                                                                                                  \
    "ncsh: unalias: usage is unalias, unalias -a to delete all aliases, or unalias {aliases} to remove specific "      \
    "alias(es)."

[[nodiscard]]
int builtins_unalias(char** restrict buffer, [[maybe_unused]] size_t* restrict buf_lens)
{
    assert(buffer && *buffer);

    if (!buffer || !buffer[1]) {
        alias_print(vm_output_fd);
        return EXIT_SUCCESS;
    }

    // skip first position since we know it is 'unalias'
    char** arg = buffer + 1;
    size_t* arg_len = buf_lens + 1;
    if (estrcmp(*arg, *arg_len, NCSH_UNALIAS_DELETE, sizeof(NCSH_UNALIAS_DELETE)) ||
        estrcmp(*arg, *arg_len, NCSH_UNALIAS_DELETE_ALIAS, sizeof(NCSH_UNALIAS_DELETE_ALIAS))) {
        alias_delete();
        return EXIT_SUCCESS;
    }
    else if (arg) {
        while (arg) {
            alias_remove(*arg, *arg_len);
            ++arg;
            ++arg_len;
        }
    }
    else {
        if (builtins_writeln(vm_output_fd, UNALIAS_USAGE, sizeof(UNALIAS_USAGE) - 1) == -1) {
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE_CONTINUE;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int builtins_exit([[maybe_unused]] char** restrict buffer, [[maybe_unused]] size_t* restrict buf_lens)
{
    return EXIT_SUCCESS_END;
}

[[nodiscard]]
int builtins_echo(char** restrict buffer, size_t* restrict buf_lens)
{
    assert(buffer && *buffer);
    char** arg = buffer + 1;
    size_t* arg_lens = buf_lens + 1;
    if (!arg || !*arg) {
        tty_send(&tcaps.newline);
        return EXIT_SUCCESS;
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
        if (!prev) {
            break;
        }
        ++arg;
        if (!arg || !*arg) {
            break;
        }

        tty_dprint(vm_output_fd, "%s ", prev);
    }
    if (prev) {
        tty_dprint(vm_output_fd, "%s", prev);
    }

    if (echo_add_newline) {
        tty_send(&tcaps.newline);
    }

    return EXIT_SUCCESS;
}

#define NCSH_TITLE "ncsh " NCSH_VERSION ""

#define NCSH_COPYRIGHT                                                                                                 \
    "Copyright (C) 2025 Alex Eski"

#define NCSH_LICENSE                                                                                                   \
    "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>. "
#define NCSH_LICENSE_NO_WARRANTY                                                                                       \
    "This program comes with ABSOLUTELY NO WARRANTY."
#define NCSH_LICENSE_INFO                                                                                              \
    "This is free software, and you are welcome to redistribute it "                                                   \
    "under certain conditions."

#define HELP_MESSAGE "ncsh help"
#define HELP_FORMAT "Builtin Commands: {command} {args}"
#define HELP_QUIT                                                                                                      \
    "q:		          To exit, type q, exit, or quit and press enter. You can also use Ctrl+D to exit."
#define HELP_CHANGEDIR "cd/z:		          You can change directory with cd or z."
#define HELP_Z                                                                                                         \
    "z {directory}:            A builtin autojump/z command. An enhanced cd command that keeps track of history and "  \
    "fuzzy matches against previously visited directories."
#define HELP_Z_ADD "z add {directory}:        Manually add a directory to your z database."
#define HELP_Z_RM                                                                                                      \
    "z rm {directory}:         Manually remove a directory from your z database. Can also call using 'z remove "       \
    "{directory}'."
#define HELP_Z_PRINT "z print:                  Print out information about the entries in your z database."
#define HELP_ECHO "echo:		          You can write things to the screen using echo."
#define HELP_HISTORY "history:	          You can see your command history using the history command."
#define HELP_HISTORY_COUNT                                                                                             \
    "history count:            You can see the number of entries in your history with history count command."
#define HELP_HISTORY_CLEAN                                                                                             \
    "history clean:            Removes all duplicates from the history file and reloads deduplicated history into "    \
    "memory."
#define HELP_HISTORY_ADD "history add {command}:    Add a command to your history without executing the command."
#define HELP_HISTORY_RM                                                                                                \
    "history rm {command}:     Remove a command from your history. Please note, the history is cleaned first to "      \
    "dededuplicate. Can also call using 'history remove {command}."
#define HELP_PWD "pwd:         	          Prints the current working directory."
#define HELP_KILL "kill {processId}:         Terminates the process with associated processId."

#define HELP_WRITE(str)                                                                                                \
    constexpr size_t str##_len = sizeof(str) - 1;                                                                      \
    if (builtins_writeln(vm_output_fd, str, str##_len) == -1) {                                                          \
        tty_perror(NCSH_ERROR_STDOUT);                                                                           \
        return EXIT_FAILURE;                                                                                           \
    }

#define HELP_WRITELN(str)                                                                                                \
    constexpr size_t str##_len = sizeof(str) - 1;                                                                      \
    if (builtins_writeln(vm_output_fd, str, str##_len) == -1) {                                                          \
        tty_perror(NCSH_ERROR_STDOUT);                                                                           \
        return EXIT_FAILURE;                                                                                           \
    } \
    tty_dsend(vm_output_fd, &tcaps.newline);


[[nodiscard]]
int builtins_help([[maybe_unused]] char** restrict buffer,
                  [[maybe_unused]] size_t* restrict buf_lens)
{
    constexpr size_t len = sizeof(NCSH_TITLE) - 1;
    if (builtins_writeln(vm_output_fd, NCSH_TITLE, len) == -1) {
        tty_perror(NCSH_ERROR_STDOUT);
        return EXIT_FAILURE;
    }

    HELP_WRITE(NCSH_COPYRIGHT);
    HELP_WRITE(NCSH_LICENSE);
    HELP_WRITE(NCSH_LICENSE_NO_WARRANTY);
    HELP_WRITELN(NCSH_LICENSE_INFO);
    HELP_WRITELN(HELP_MESSAGE);
    HELP_WRITELN(HELP_FORMAT);
    HELP_WRITELN(HELP_QUIT);
    HELP_WRITELN(HELP_CHANGEDIR);
    HELP_WRITELN(HELP_Z);
    HELP_WRITELN(HELP_Z_ADD);
    HELP_WRITELN(HELP_Z_RM);
    HELP_WRITELN(HELP_Z_PRINT);
    HELP_WRITELN(HELP_ECHO);
    HELP_WRITELN(HELP_HISTORY);
    HELP_WRITELN(HELP_HISTORY_COUNT);
    HELP_WRITELN(HELP_HISTORY_CLEAN);
    HELP_WRITELN(HELP_HISTORY_ADD);
    HELP_WRITELN(HELP_HISTORY_RM);
    HELP_WRITELN(HELP_PWD);
    HELP_WRITELN(HELP_KILL);

    // controls
    // HELP_WRITE(HELP_BASIC_CONTROLS);
    // CTRL + W, CTRL + U, etc.
    // autocomplete
    // HELP_WRITE(HELP_AUTOCOMPLETIONS);
    // HELP_WRITE(HELP_TAB_AUTOCOMPLETIONS);
    // HELP_WRITE(HELP_AUTOCOMPLETIONS_MORE_INFO);

    fflush(stdout);
    return EXIT_SUCCESS;
}

#define NCSH_COULD_NOT_CD_MESSAGE "ncsh cd: could not change directory."

[[nodiscard]]
int builtins_cd(char** restrict buffer, [[maybe_unused]] size_t* restrict buf_lens)
{
    assert(buffer && *buffer);

    // skip first position since we know it is 'cd'
    char* arg = *(buffer + 1);
    if (!arg) {
        char* home = getenv("HOME");
        if (!home) {
            if (builtins_writeln(STDERR_FILENO, NCSH_COULD_NOT_CD_MESSAGE, sizeof(NCSH_COULD_NOT_CD_MESSAGE) - 1) == -1)
                return EXIT_FAILURE;
        }
        else if (chdir(home)) {
            tty_perror("ncsh: could not change directory");
        }

        return EXIT_SUCCESS;
    }

    if (chdir(arg)) {
        if (builtins_writeln(STDERR_FILENO, NCSH_COULD_NOT_CD_MESSAGE, sizeof(NCSH_COULD_NOT_CD_MESSAGE) - 1) == -1)
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int builtins_pwd([[maybe_unused]] char** restrict buffer, [[maybe_unused]] size_t* restrict buf_lens)
{
    char path[PATH_MAX];
    if (!getcwd(path, sizeof(path))) {
        tty_perror("ncsh pwd: Error when getting current directory");
        return EXIT_FAILURE;
    }

    tty_puts(path);
    return EXIT_SUCCESS;
}

#define KILL_NOTHING_TO_KILL_MESSAGE "ncsh kill: nothing to kill, please pass in a process ID (PID)."
#define KILL_COULDNT_PARSE_PID_MESSAGE "ncsh kill: could not parse process ID (PID) from arguments."

[[nodiscard]]
int builtins_kill(char** restrict buffer, [[maybe_unused]] size_t* restrict buf_lens)
{
    assert(buffer && *buffer && buffer + 1);
    if (!buffer) {
        if (builtins_writeln(vm_output_fd, KILL_NOTHING_TO_KILL_MESSAGE, sizeof(KILL_NOTHING_TO_KILL_MESSAGE) - 1) ==
            -1) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    // skip first position since we know it is 'kill'
    char* arg = *(buffer + 1);
    if (!arg || !*arg) {
        if (builtins_writeln(vm_output_fd, KILL_NOTHING_TO_KILL_MESSAGE, sizeof(KILL_NOTHING_TO_KILL_MESSAGE) - 1) ==
            -1) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    pid_t pid = atoi(arg);
    if (!pid) {
        if (builtins_writeln(vm_output_fd, KILL_COULDNT_PARSE_PID_MESSAGE, sizeof(KILL_COULDNT_PARSE_PID_MESSAGE) - 1) ==
            -1) {
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE_CONTINUE;
    }

    if (kill(pid, SIGTERM) != 0) {
        tty_println("ncsh kill: could not kill process with process ID (PID): %d", pid);
        return EXIT_FAILURE_CONTINUE;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int builtins_version([[maybe_unused]] char** restrict buffer, [[maybe_unused]] size_t* restrict buf_lens)
{
    builtins_writeln(vm_output_fd, NCSH_TITLE, sizeof(NCSH_TITLE) - 1);
    return EXIT_SUCCESS;
}

[[nodiscard]]
int builtins_true([[maybe_unused]] char** restrict buffer, [[maybe_unused]] size_t* restrict buf_lens)
{
    return EXIT_SUCCESS;
}

[[nodiscard]]
int builtins_false([[maybe_unused]] char** restrict buffer, [[maybe_unused]] size_t* restrict buf_lens)
{
    return EXIT_FAILURE;
}

void builtins_print()
{
    for (size_t i = 0; i < builtins_count; ++i) {
        tty_println("%s", builtins[i].value);
    }
}

void builtins_print_enabled()
{
    if (!builtins_disabled_state) {
        for (size_t i = 0; i < builtins_count; ++i) {
            tty_println("%s: enabled", builtins[i].value);
        }
    }
    else {
        for (size_t i = 0; i < builtins_count; ++i) {
            if ((builtins_disabled_state & builtins[i].flag)) {
                tty_println("%s: disabled", builtins[i].value);
            }
            else {
                tty_println("%s: enabled", builtins[i].value);
            }
        }
    }
}

// TODO: finish enable & disable implementation
[[nodiscard]]
int builtins_disable(char** restrict buffer, size_t* restrict buf_lens)
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
                    tty_println("ncsh disable: disabled builtin %s.", builtins[j].value);
                }
            }
            ++arg;
            ++arg_lens;
        }
    }

    return EXIT_SUCCESS;
}

#define ENABLE_OPTION_NOT_SUPPORTED_MESSAGE "ncsh enable: command not found, options entered not supported."

[[nodiscard]]
int builtins_enable(char** restrict buffer, size_t* restrict buf_lens)
{
    assert(buffer && *buffer);

    // skip first position since we know it is 'enable'
    char** arg = buffer + 1;
    if (!arg) {
        builtins_print();
        return EXIT_SUCCESS;
    }
    size_t* arg_lens = buf_lens + 1;

    if (*arg_lens == 2) {
        if (CMP_2(*arg, "-a")) {
            builtins_print_enabled();
            return EXIT_SUCCESS;
        }
        else if (CMP_2(*arg, "-n")) {
            return builtins_disable(buffer, buf_lens);
        }
    }

    if (builtins_writeln(vm_output_fd, ENABLE_OPTION_NOT_SUPPORTED_MESSAGE,
                       sizeof(ENABLE_OPTION_NOT_SUPPORTED_MESSAGE) - 1) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// TODO: implement export
/*#define EXPORT_OPTION_NOT_SUPPORTED_MESSAGE "ncsh export: command not found, options entered not supported."
#define EXPORT_OPTIONS_MESSAGE \*/
//     "ncsh export: please pass in at least once argument. export currently supports modifying $PATH and $HOME."
//
// [[nodiscard]]
// int builtins_export(Tokens* restrict toks, size_t* restrict buf_lens)
// {
//     (void)buf_lens;
//     assert(toks && toks->head && toks->head->next);
//
//     // skip first position since we know it is 'export'
//     Token* tok = toks->head->next->next;
//     if (!tok || !tok->val) {
//         if (builtins_write(vm_output_fd, EXPORT_OPTION_NOT_SUPPORTED_MESSAGE,
//                            sizeof(EXPORT_OPTION_NOT_SUPPORTED_MESSAGE) - 1) == -1) {
//             return EXIT_FAILURE;
//         }
//         return EXIT_FAILURE_CONTINUE;
//     }
//
//     if (estrcmp(tok->val, tok->len, NCSH_PATH_VAR, sizeof(NCSH_PATH_VAR))) {
//         puts("export $PATH found");
//     }
//     else if (estrcmp(tok->val, tok->len, NCSH_HOME_VAR, sizeof(NCSH_HOME_VAR))) {
//         puts("export $HOME found");
//     }
//     else {
//         if (builtins_write(vm_output_fd, EXPORT_OPTION_NOT_SUPPORTED_MESSAGE,
//                            sizeof(EXPORT_OPTION_NOT_SUPPORTED_MESSAGE) - 1) == -1) {
//             return EXIT_FAILURE;
//         }
//     }
//
//     return EXIT_SUCCESS;
// }
//
// // TODO: implement declare
//
// // NOTE: set is not implemented.
// // TODO: finish set/unset implementations
// [[nodiscard]]
// int builtins_set_e()
// {
//     puts("set e detected");
//     return EXIT_SUCCESS;
// }
//
// #define SET_NOTHING_TO_SET_MESSAGE "ncsh set: nothing to set, please pass in a value to set (i.e. '-e', '-c')"
// #define SET_VALID_OPERATIONS_MESSAGE "ncsh set: valid set operations are in the form '-e', '-c', etc."
// [[nodiscard]]
// int builtins_set(Tokens* restrict toks, size_t* restrict buf_lens)
// {
//     (void)buf_lens;
//     assert(toks && toks->head && toks->head->next);
//
//     // skip first position since we know it is 'set'
//     Token* tok = toks->head->next->next;
//     if (!tok || !tok->val) {
//         if (builtins_write(vm_output_fd, SET_NOTHING_TO_SET_MESSAGE, sizeof(SET_NOTHING_TO_SET_MESSAGE) - 1) == -1) {
//             return EXIT_FAILURE;
//         }
//
//         return EXIT_SUCCESS;
//     }
//
//     if (tok->len > 3 || tok->val[0] != '-') {
//         if (builtins_write(vm_output_fd, SET_VALID_OPERATIONS_MESSAGE, sizeof(SET_VALID_OPERATIONS_MESSAGE) - 1) ==
//             -1) {
//             return EXIT_FAILURE;
//         }
//
//         return EXIT_SUCCESS;
//     }
//
//     switch (tok->val[1]) {
//     case 'e': {
//         return builtins_set_e();
//     }
//     default: {
//         break;
//     }
//     }
//
//     return EXIT_SUCCESS;
// }
//
// // not implemented
// #define UNSET_NOTHING_TO_UNSET_MESSAGE "ncsh unset: nothing to unset, please pass in a value to unset."
// [[nodiscard]]
// int builtins_unset(Tokens* restrict toks, size_t* restrict buf_lens)
// {
//     (void)buf_lens;
//     assert(toks && toks->head && toks->head->next);
//
//     // skip first position since we know it is 'unset'
//     Token* tok = toks->head->next->next;
//     if (!tok || !tok->val) {
//         if (builtins_write(vm_output_fd, UNSET_NOTHING_TO_UNSET_MESSAGE, sizeof(UNSET_NOTHING_TO_UNSET_MESSAGE) - 1)
//         ==
//             -1) {
//             return EXIT_FAILURE;
//         }
//
//         return EXIT_SUCCESS;
//     }
//
//     /*bool is_set = var_exists(arg->val, &args->vars);
//     if (!is_set) {
//         printf("ncsh unset: no value found for '%s' to unset.", args->values[1]);
//         return EXIT_SUCCESS;
//     }*/
//     // TODO: need a way to unset, var_set doesn't work
//     // var_set(args->values[1], NULL, arena, &args->vars)
//     return EXIT_SUCCESS;
// }

/* builtins_check_and_run
 * Checks current command against builtins, and if matches runs the builtin.
 */
[[nodiscard]]
bool builtins_check_and_run(Vm_Data* restrict vm, Shell* restrict shell, Arena* restrict scratch)
{
    if (shell) {
        if (estrcmp(vm->buffer[0], vm->buffer_lens[0], Z, sizeof(Z))) {
            vm->status = builtins_z(&shell->z_db, vm->buffer, vm->buffer_lens, &shell->arena, scratch);
            return true;
        }

        if (estrcmp(vm->buffer[0], vm->buffer_lens[0], NCSH_HISTORY, sizeof(NCSH_HISTORY))) {
            vm->status = builtins_history(&shell->input.history, vm->buffer, vm->buffer_lens, &shell->arena, scratch);
            return true;
        }

        if (estrcmp(vm->buffer[0], vm->buffer_lens[0], NCSH_ALIAS, sizeof(NCSH_ALIAS))) {
            vm->status = builtins_alias(vm->buffer, vm->buffer_lens, &shell->arena);
            return true;
        }
    }

    for (size_t i = 0; i < builtins_count; ++i) {
        if (estrcmp(vm->buffer[0], vm->buffer_lens[0], builtins[i].value, builtins[i].length)) {
            vm->status = (*builtins[i].func)(vm->buffer, vm->buffer_lens);
            return true;
        }
    }

    return false;
}
