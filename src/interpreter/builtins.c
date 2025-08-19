/* Copyright ncsh (C) by Alex Eski 2024 */
/* builtins.h: shell builtins implementations for ncsh */

/* TODO: --- this file is a mess and needs some work --- */

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

#include "../alias.h"
#include "../arena.h"
#include "../defines.h"
#include "../env.h"
#include "../io/history.h"
#include "../ttyio/ttyio.h"
#include "../types.h"
#include "../z/z.h"
#include "vm_types.h"

/* External values */
extern jmp_buf env_jmp_buf;      // from main.c, used on unrecoverable failures
extern int vm_output_fd; // from vm.c, used as fd for writing to stdout

/* Shared builtins data and functions */
static long unsigned int builtins_disabled_state = 0;

/*static int builtins_write(int fd, char* buf, size_t len)
{
    int bytes_written = tty_dwrite(fd, buf, len);
    if (bytes_written == EOF && errno == EPIPE) {
        return EOF;
    }
    else if (bytes_written == EOF) {
        longjmp(env_jmp_buf, FAILURE_BUILTIN_WRITE);
    }
    return bytes_written;
}*/

static int builtins_writeln(int fd, char* buf, size_t len)
{
    int bytes_written = tty_dwriteln(fd, buf, len);
    if (bytes_written == EOF && errno == EPIPE) {
        return EOF;
    }
    else if (bytes_written == EOF) {
        longjmp(env_jmp_buf, FAILURE_BUILTIN_WRITE);
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
static int builtins_z(z_Database* restrict z_db, Str* restrict strs, Arena* arena, Arena* restrict scratch);

#define NCSH_HISTORY "history" // the base command, displays history
#define NCSH_HISTORY_COUNT "count"
#define NCSH_HISTORY_CLEAN "clean"
#define NCSH_HISTORY_ADD "add"
#define NCSH_HISTORY_RM "rm" // alias for rm
#define NCSH_HISTORY_REMOVE "remove"
static int builtins_history(History* restrict history, Str* restrict strs, Arena* restrict arena,
                     Arena* restrict scratch);

#define NCSH_ALIAS "alias"
#define NCSH_ALIAS_PRINT "-p"
#define NCSH_ALIAS_PRINT_ "print"
#define NCSH_ALIAS_ADD "add"
#define NCSH_ALIAS_RM "rm"
#define NCSH_ALIAS_REMOVE "remove"
#define NCSH_ALIAS_DELETE "delete"
static int builtins_alias(Str* restrict strs, Arena* restrict arena);

#define NCSH_UNALIAS "unalias"
#define NCSH_UNALIAS_DELETE "-a"
#define NCSH_UNALIAS_DELETE_ALIAS "delete"
static int builtins_unalias(Str* restrict strs);

#define NCSH_EXIT "exit" // the base command
#define NCSH_QUIT "quit" // alias for exit
#define NCSH_Q "q"       // alias for exit
static int builtins_exit(Str* restrict strs);

#define NCSH_ECHO "echo"
#define NCSH_ECHO_NO_NEWLINE "-n"
static int builtins_echo(Str* restrict strs);

#define NCSH_HELP "help"
static int builtins_help(Str* restrict strs);

#define NCSH_CD "cd"
static int builtins_cd(Str* restrict strs);

#define NCSH_PWD "pwd"
static int builtins_pwd(Str* restrict strs);

#define NCSH_KILL "kill"
static int builtins_kill(Str* restrict strs);

#define NCSH_VERSION_CMD "version"
static int builtins_version(Str* restrict strs);

#define NCSH_TRUE "true"
static int builtins_true(Str* restrict strs);

#define NCSH_FALSE "false"
static int builtins_false(Str* restrict strs);

#define NCSH_ENABLE "enable"
static int builtins_enable(Str* restrict strs);

#define NCSH_DISABLE "disable"
static int builtins_disable(Str* restrict strs);

// TODO: finish implementation
// #define NCSH_EXPORT "export"
// int builtins_export(Statements* restrict toks, size_t* restrict buf_lens);

// #define NCSH_SET "set"
// static int builtins_set(Str* restrict strs, Env* restrict env);

#define NCSH_UNSET "unset"
static int builtins_unset(Str* restrict strs, Env* restrict env);

/* Types */
// clang-format off
enum Builtins_Disabled : long unsigned int {
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
    BF_ENABLE =      1 << 11,
    BF_DISABLE =     1 << 12,
    BF_Z =           1 << 13,
    BF_HISTORY =     1 << 14,
    BF_UNSET =       1 << 16
    // BF_SET =         1 << 13,
    // BF_EXPORT =      1 << 9,
};
// clang-format on

typedef struct {
    enum Builtins_Disabled flag;
    Str str;
    int (*func)(Str* restrict strs);
} Builtin;

static const Builtin builtins[] = {
    {.flag = BF_EXIT, .str.length = sizeof(NCSH_EXIT), .str.value = NCSH_EXIT, .func = &builtins_exit},
    {.flag = BF_EXIT, .str.length = sizeof(NCSH_QUIT), .str.value = NCSH_QUIT, .func = &builtins_exit},
    {.flag = BF_EXIT, .str.length = sizeof(NCSH_Q), .str.value = NCSH_Q, .func = &builtins_exit},
    {.flag = BF_ECHO, .str.length = sizeof(NCSH_ECHO), .str.value = NCSH_ECHO, .func = &builtins_echo},
    {.flag = BF_HELP, .str.length = sizeof(NCSH_HELP), .str.value = NCSH_HELP, .func = &builtins_help},
    {.flag = BF_CD, .str.length = sizeof(NCSH_CD), .str.value = NCSH_CD, .func = &builtins_cd},
    {.flag = BF_PWD, .str.length = sizeof(NCSH_PWD), .str.value = NCSH_PWD, .func = &builtins_pwd},
    {.flag = BF_KILL, .str.length = sizeof(NCSH_KILL), .str.value = NCSH_KILL, .func = &builtins_kill},
    {.flag = BF_VERSION, .str.length = sizeof(NCSH_VERSION_CMD), .str.value = NCSH_VERSION_CMD, .func = &builtins_version},
    {.flag = BF_UNALIAS, .str.length = sizeof(NCSH_UNALIAS), .str.value = NCSH_UNALIAS, .func = &builtins_unalias},
    {.flag = BF_TRUE, .str.length = sizeof(NCSH_TRUE), .str.value = NCSH_TRUE, .func = &builtins_true},
    {.flag = BF_FALSE, .str.length = sizeof(NCSH_FALSE), .str.value = NCSH_FALSE, .func = &builtins_false},
    {.flag = BF_ENABLE, .str.length = sizeof(NCSH_ENABLE), .str.value = NCSH_ENABLE, .func = &builtins_enable},
    {.flag = BF_DISABLE, .str.length = sizeof(NCSH_DISABLE), .str.value = NCSH_DISABLE, .func = &builtins_disable},
    /*{.flag = BF_EXPORT, .str.length = sizeof(NCSH_EXPORT), .str.value = NCSH_EXPORT, .func = &builtins_export},
    {.flag = BF_SET, .str.length = sizeof(NCSH_SET), .str.value = NCSH_SET, .func = &builtins_set},*/
};

static constexpr size_t builtins_count = sizeof(builtins) / sizeof(builtins[0]);

/* Implementations */
#define Z_COMMAND_NOT_FOUND_MESSAGE "ncsh z: command not found, options not supported."

[[nodiscard]]
static int builtins_z(z_Database* restrict z_db, Str* restrict strs, Arena* restrict arena, Arena* restrict scratch)
{
    assert(z_db); assert(strs && strs->value); assert(arena); assert(scratch);

    if (!strs[1].length) {
        z(&Str_Empty, NULL, z_db, arena, *scratch);
        return EXIT_SUCCESS;
    }

    // skip first position since we know it is 'z'
    Str* args = strs + 1;
    if (!args || !args[1].length) {
        assert(args->value);

        // z print
        if (estrcmp(*args, Str_New_Literal(Z_PRINT))) {
            z_print(z_db);
            return EXIT_SUCCESS;
        }
        // z count
        if (estrcmp(*args, Str_New_Literal(Z_COUNT))) {
            z_count(z_db);
            return EXIT_SUCCESS;
        }

        // z
        char cwd[PATH_MAX] = {0};
        if (!getcwd(cwd, PATH_MAX)) {
            tty_perror("ncsh z: Could not load cwd information");
            return EXIT_FAILURE;
        }

        z(args, cwd, z_db, arena, *scratch);
        return EXIT_SUCCESS;
    }

    if (args && args[1].value && !args[2].value) {
        // z add
        if (estrcmp(*args, Str_New_Literal(Z_ADD))) {
            if (z_add(&args[1], z_db, arena) != Z_SUCCESS) {
                return EXIT_FAILURE_CONTINUE;
            }

            return EXIT_SUCCESS;
        }
        // z rm/remove
        else if (estrcmp(*args, Str_New_Literal(Z_RM)) ||
                estrcmp(*args, Str_New_Literal(Z_REMOVE))) {
            if (z_remove(&args[1], z_db) != Z_SUCCESS) {
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
static int builtins_history(History* restrict history, Str* restrict strs, Arena* restrict arena,
                     Arena* restrict scratch)
{
    assert(history); assert(strs); assert(arena); assert(scratch);

    if (!strs[1].value) {
        return history_command_display(history);
    }

    assert(strs && *strs->value && strs->value[1]);

    // skip first position since we know it is 'history'
    Str* args = strs + 1;
    if (!args || !args->value) {
        if (builtins_writeln(vm_output_fd, HISTORY_COMMAND_NOT_FOUND_MESSAGE,
                           sizeof(HISTORY_COMMAND_NOT_FOUND_MESSAGE) - 1) == -1) {
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE_CONTINUE;
    }

    if (args && !args[1].length) {
        if (estrcmp(*args, Str_New_Literal(NCSH_HISTORY_COUNT))) {
            return history_command_count(history);
        }
        else if (estrcmp(*args, Str_New_Literal(NCSH_HISTORY_CLEAN))) {
            return history_command_clean(history, arena, scratch);
        }
    }

    if (args && args[1].value && !args[2].value) {
        // history add
        if (estrcmp(*args, Str_New_Literal(NCSH_HISTORY_ADD))) {
            return history_command_add(args[1], history, arena);
        }
        // history rm/remove
        else if (estrcmp(*args, Str_New_Literal(NCSH_HISTORY_RM)) ||
                 estrcmp(*args, Str_New_Literal(NCSH_HISTORY_REMOVE))) {
            return history_command_remove(args[1], history, arena, scratch);
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
static int builtins_alias(Str* restrict strs, Arena* restrict arena)
{
    assert(strs && strs->value && *strs->value);

    if (!strs[1].value) {
        alias_print(vm_output_fd);
        return EXIT_SUCCESS;
    }

    // skip first position since we know it is 'alias'
    Str* args = strs + 1;
    if (!args || !args->length) {
        if (builtins_writeln(vm_output_fd, ALIAS_USAGE, sizeof(ALIAS_USAGE) - 1) == -1) {
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE_CONTINUE;
    }
    else if (estrcmp(*args, Str_New_Literal(NCSH_ALIAS_ADD))) {
        ++args;
        if (!args || !args->value) {
            if (builtins_writeln(vm_output_fd, ALIAS_ADD_USAGE, sizeof(ALIAS_ADD_USAGE) - 1) == -1) {
                return EXIT_FAILURE;
            }
            return EXIT_FAILURE_CONTINUE;
        }
        Str alias = *args;
        ++args;
        if (!args || !args->value) {
            if (builtins_writeln(vm_output_fd, ALIAS_ADD_USAGE, sizeof(ALIAS_ADD_USAGE) - 1) == -1) {
                return EXIT_FAILURE;
            }
            return EXIT_FAILURE_CONTINUE;
        }
        Str command = *args;
        alias_add_new(alias, command, arena);
    }
    else if (estrcmp(*args, Str_New_Literal(NCSH_ALIAS_RM)) ||
             estrcmp(*args, Str_New_Literal(NCSH_ALIAS_REMOVE))) {
        ++args;
        if (!args || !args->value) {
            if (builtins_writeln(vm_output_fd, ALIAS_REMOVE_USAGE, sizeof(ALIAS_REMOVE_USAGE) - 1) == -1) {
                return EXIT_FAILURE;
            }
            return EXIT_FAILURE_CONTINUE;
        }
        alias_remove(*args);
    }
    else if (estrcmp(*args, Str_New_Literal(NCSH_ALIAS_DELETE))) {
        alias_delete();
    }
    else if (estrcmp(*args, Str_New_Literal(NCSH_ALIAS_PRINT)) ||
             estrcmp(*args, Str_New_Literal(NCSH_ALIAS_PRINT_))) {
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
static int builtins_unalias(Str* restrict strs)
{
    assert(strs && strs->value);

    if (!strs[1].value) {
        alias_print(vm_output_fd);
        return EXIT_SUCCESS;
    }

    // skip first position since we know it is 'unalias'
    Str* args = strs + 1;
    if (!args || !args->length) {
        if (builtins_writeln(vm_output_fd, UNALIAS_USAGE, sizeof(UNALIAS_USAGE) - 1) == -1) {
            return EXIT_FAILURE;
        }
        return EXIT_FAILURE_CONTINUE;
    }
    else if (estrcmp(*args, Str_New_Literal(NCSH_UNALIAS_DELETE)) ||
        estrcmp(*args, Str_New_Literal(NCSH_UNALIAS_DELETE_ALIAS))) {
        alias_delete();
        return EXIT_SUCCESS;
    }
    else if (args) {
        while (args) {
            alias_remove(*args);
            ++args;
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
static int builtins_exit([[maybe_unused]] Str* strs)
{
    return EXIT_SUCCESS_END;
}

[[nodiscard]]
static int builtins_echo(Str* restrict strs)
{
    assert(strs && *strs->value);
    Str* args = strs + 1;
    if (!args || !args->value) {
        tty_send(&tcaps.newline);
        return EXIT_SUCCESS;
    }

    bool echo_add_newline = true;
    // process options for echo
    while (args && args->value) {
        if (estrcmp(*args, Str_New_Literal(NCSH_ECHO_NO_NEWLINE))) {
            echo_add_newline = false;
            break;
        }
        ++args;
    }

    // send output for echo
    args = !echo_add_newline ? args + 1 : strs + 1;
    Str* prev = NULL;
    while (args && args->value) {
        prev = args;
        if (!prev) {
            break;
        }
        ++args;
        if (!args || !args->value) {
            break;
        }

        tty_dprint(vm_output_fd, "%s ", prev->value);
    }
    if (prev) {
        tty_dprint(vm_output_fd, "%s", prev->value);
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
static int builtins_help([[maybe_unused]] Str* restrict strs)
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
static int builtins_cd(Str* restrict strs)
{
    assert(strs && strs->value);

    // skip first position since we know it is 'cd'
    Str* args = strs + 1;
    if (!args || !args->value) {
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

    if (chdir(args->value)) {
        if (builtins_writeln(STDERR_FILENO, NCSH_COULD_NOT_CD_MESSAGE, sizeof(NCSH_COULD_NOT_CD_MESSAGE) - 1) == -1)
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
static int builtins_pwd([[maybe_unused]] Str* restrict strs)
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
static int builtins_kill(Str* restrict strs)
{
    assert(strs);
    if (!strs->value) {
        if (builtins_writeln(vm_output_fd, KILL_NOTHING_TO_KILL_MESSAGE, sizeof(KILL_NOTHING_TO_KILL_MESSAGE) - 1) ==
            -1) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    // skip first position since we know it is 'kill'
    Str* args = strs + 1;
    if (!args || !args->value) {
        if (builtins_writeln(vm_output_fd, KILL_NOTHING_TO_KILL_MESSAGE, sizeof(KILL_NOTHING_TO_KILL_MESSAGE) - 1) ==
            -1) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    pid_t pid = atoi(args->value);
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
static int builtins_version([[maybe_unused]] Str* restrict strs)
{
    builtins_writeln(vm_output_fd, NCSH_TITLE, sizeof(NCSH_TITLE) - 1);
    return EXIT_SUCCESS;
}

[[nodiscard]]
static int builtins_true([[maybe_unused]] Str* restrict strs)
{
    return EXIT_SUCCESS;
}

[[nodiscard]]
static int builtins_false([[maybe_unused]] Str* restrict strs)
{
    return EXIT_FAILURE;
}

void builtins_print()
{
    for (size_t i = 0; i < builtins_count; ++i) {
        tty_println("%s", builtins[i].str.value);
    }
}

void builtins_print_enabled()
{
    if (!builtins_disabled_state) {
        for (size_t i = 0; i < builtins_count; ++i) {
            tty_println("%s: enabled", builtins[i].str.value);
        }
    }
    else {
        for (size_t i = 0; i < builtins_count; ++i) {
            if ((builtins_disabled_state & builtins[i].flag)) {
                tty_println("%s: disabled", builtins[i].str.value);
            }
            else {
                tty_println("%s: enabled", builtins[i].str.value);
            }
        }
    }
}

static int builtins_disable__(Str* restrict str)
{
    for (size_t i = 0; i < builtins_count; ++i) {
        if (estrcmp(*str, builtins[i].str)) {
            if (builtins_disabled_state & builtins[i].flag) {
                tty_println("ncsh disable: the builtin '%s' was already disable", builtins[i].str.value);
                return EXIT_SUCCESS;
            }
            if (builtins_disabled_state == 0 || builtins_disabled_state | builtins[i].flag) {
                builtins_disabled_state |= builtins[i].flag;
                tty_println("ncsh disable: disabled builtin %s.", builtins[i].str.value);
                return EXIT_SUCCESS;
            }
        }
    }
    return EXIT_FAILURE_CONTINUE;
}

#define DISABLE_BUILTIN_NOT_FOUND "ncsh disable: command not found, could not disable."
#define DISABLE_NO_COMMAND_ARG "ncsh enable: no command passed in to disable."
[[nodiscard]]
static int builtins_disable(Str* restrict strs)
{
    assert(strs && strs->value);

    // skip first position since we know it is 'enable' or 'disable'
    Str* args = strs + 1;
    if (!args || !args->value) {
        builtins_print();
        return EXIT_SUCCESS;
    }

    if (!builtins_disable__(args)) {
        return EXIT_SUCCESS;
    }

    builtins_writeln(vm_output_fd, DISABLE_BUILTIN_NOT_FOUND, sizeof(DISABLE_BUILTIN_NOT_FOUND) - 1);
    return EXIT_SUCCESS;
}

#define ENABLE_OPTION_NOT_SUPPORTED_MESSAGE "ncsh enable: command not found, options entered not supported."

[[nodiscard]]
static int builtins_enable(Str* restrict strs)
{
    assert(strs && strs->value);

    // skip first position since we know it is 'enable'
    Str* args = strs + 1;
    if (!args || !args->value) {
        builtins_print();
        return EXIT_SUCCESS;
    }

    if (args->length == 3) {
        if (estrcmp(*args, Str_New_Literal("-a"))) {
            builtins_print_enabled();
            return EXIT_SUCCESS;
        }
        else if (estrcmp(*args, Str_New_Literal("-n"))) {
            ++args;
            return builtins_disable__(args);
        }
    }

    for (size_t i = 0; i < builtins_count; ++i) {
        if (estrcmp(*args, builtins[i].str)) {
            if (builtins_disabled_state == 0 || !(builtins_disabled_state & builtins[i].flag)) {
                tty_println("ncsh enable: the builtin '%s' is already enabled", builtins[i].str.value);
                return EXIT_SUCCESS;
            }
            if (builtins_disabled_state & builtins[i].flag) {
                builtins_disabled_state ^= builtins[i].flag;
                tty_println("ncsh enable: enabled builtin %s.", builtins[i].str.value);
                return EXIT_SUCCESS;
            }
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

// [[nodiscard]]
// static int builtins_export(Tokens* restrict toks, size_t* restrict buf_lens)
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

// // TODO: implement declare

// // NOTE: set is not implemented.
// // TODO: finish set/unset implementations
// [[nodiscard]]
// static int builtins_set_e()
// {
//     puts("set e detected");
//     return EXIT_SUCCESS;
// }
//
// #define SET_NOTHING_TO_SET_MESSAGE "ncsh set: nothing to set, please pass in a value to set (i.e. '-e', '-c')"
// #define SET_VALID_OPERATIONS_MESSAGE "ncsh set: valid set operations are in the form '-e', '-c', etc."
// [[nodiscard]]
// static int builtins_set(Tokens* restrict toks, size_t* restrict buf_lens)
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

#define UNSET_NOTHING_TO_UNSET_MESSAGE "ncsh unset: nothing to unset, please pass in a value to unset."
[[nodiscard]]
static int builtins_unset(Str* restrict strs, Env* restrict env)
{
    assert(strs); assert(strs->value); assert(env);

    // skip first position since we know it is 'unset'
    Str* args = strs + 1;
    if (!args || !args->value) {
        if (builtins_writeln(vm_output_fd,
                           UNSET_NOTHING_TO_UNSET_MESSAGE,
                           sizeof(UNSET_NOTHING_TO_UNSET_MESSAGE) - 1) == -1) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    Str* val = env_add_or_get(env, *args);
    if (!val || !val->value) {
        return EXIT_SUCCESS;
    }

    val->value = NULL;
    val->length = 0;
    return EXIT_SUCCESS;
}

/* builtins_check_and_run
 * Checks current command against builtins, and if matches runs the builtin.
 */
[[nodiscard]]
bool builtins_check_and_run(Vm_Data* restrict vm, Shell* restrict shell, Arena* restrict scratch)
{
    if (shell) {
        if (estrcmp(vm->strs[0], Str_New_Literal(Z))) {
            vm->status = builtins_z(&shell->z_db, vm->strs, &shell->arena, scratch);
            return true;
        }

        if (estrcmp(vm->strs[0], Str_New_Literal(NCSH_HISTORY))) {
            if (builtins_disabled_state & BF_HISTORY) {
                return false;
            }
            vm->status = builtins_history(&shell->input.history, vm->strs, &shell->arena, scratch);
            return true;
        }

        if (estrcmp(vm->strs[0], Str_New_Literal(NCSH_ALIAS))) {
            if (builtins_disabled_state & BF_ALIAS) {
                return false;
            }
            vm->status = builtins_alias(vm->strs, &shell->arena);
            return true;
        }

        if (estrcmp(vm->strs[0], Str_New_Literal(NCSH_UNSET))) {
            if (builtins_disabled_state & BF_UNSET) {
                return false;
            }
            vm->status = builtins_unset(vm->strs, shell->env);
            return true;
        }
    }

    for (size_t i = 0; i < builtins_count; ++i) {
        if (estrcmp(vm->strs[0], builtins[i].str)) {
            if (builtins_disabled_state & builtins[i].flag) {
                return false;
            }
            vm->status = (*builtins[i].func)(vm->strs);
            return true;
        }
    }

    return false;
}
