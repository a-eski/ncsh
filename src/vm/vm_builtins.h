/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../eskilib/eskilib_string.h"
#include "../parser.h"
#include "../readline/history.h"
#include "../z/z.h"

#define Z "z" // the base command, changes directory
#define Z_ADD "add"
#define Z_RM "rm"
#define Z_REMOVE "remove" // alias for rm
#define Z_PRINT "print"
int_fast32_t builtins_z(struct z_Database* const restrict z_db, const struct Args* const restrict args,
                        struct Arena* const arena, struct Arena* const scratch_arena);

#define NCSH_HISTORY "history" // the base command, displays history
#define NCSH_HISTORY_COUNT "count"
#define NCSH_HISTORY_CLEAN "clean"
#define NCSH_HISTORY_ADD "add"
#define NCSH_HISTORY_RM "rm" // alias for rm
#define NCSH_HISTORY_REMOVE "remove"
int_fast32_t builtins_history(struct History* const restrict history, const struct Args* const restrict args,
                              struct Arena* const arena, struct Arena* const scratch_arena);

#define NCSH_EXIT "exit" // the base command
#define NCSH_QUIT "quit" // alias for exit
#define NCSH_Q "q"       // alias for exit
int_fast32_t builtins_exit(const struct Args* const restrict args);

#define NCSH_ECHO "echo"
int_fast32_t builtins_echo(const struct Args* const restrict args);

#define NCSH_HELP "help"
int_fast32_t builtins_help(const struct Args* const restrict args);

#define NCSH_CD "cd"
int_fast32_t builtins_cd(const struct Args* const restrict args);

#define NCSH_PWD "pwd"
int_fast32_t builtins_pwd(const struct Args* const restrict args);

#define NCSH_KILL "kill"
int_fast32_t builtins_kill(const struct Args* const restrict args);

#define NCSH_VERSION_CMD "version"
int_fast32_t builtins_version(const struct Args* const restrict args);

#define NCSH_SET "set" // not fully implemented
int_fast32_t builtins_set(const struct Args* const restrict args);

struct Builtin {
    size_t length;
    char* value;
    int_fast32_t (*func)(const struct Args* const restrict);
};

static const struct Builtin builtins[] = {
    {.length = sizeof(NCSH_EXIT), .value = NCSH_EXIT, .func = &builtins_exit},
    {.length = sizeof(NCSH_QUIT), .value = NCSH_QUIT, .func = &builtins_exit},
    {.length = sizeof(NCSH_Q), .value = NCSH_Q, .func = &builtins_exit},
    {.length = sizeof(NCSH_ECHO), .value = NCSH_ECHO, .func = &builtins_echo},
    {.length = sizeof(NCSH_HELP), .value = NCSH_HELP, .func = &builtins_help},
    {.length = sizeof(NCSH_CD), .value = NCSH_CD, .func = &builtins_cd},
    {.length = sizeof(NCSH_PWD), .value = NCSH_PWD, .func = &builtins_pwd},
    {.length = sizeof(NCSH_KILL), .value = NCSH_KILL, .func = &builtins_kill},
    {.length = sizeof(NCSH_VERSION_CMD), .value = NCSH_VERSION_CMD, .func = &builtins_version},
    {.length = sizeof(NCSH_SET), .value = NCSH_SET, .func = &builtins_set},
};

static constexpr size_t builtins_count = sizeof(builtins) / sizeof(struct Builtin);
