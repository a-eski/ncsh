/* Copyright ncsh (C) by Alex Eski 2024 */

#pragma once

#include "../parser.h"
#include "../readline/history.h"
#include "../z/z.h"

#define Z "z" // the base command, changes directory
#define Z_ADD "add"
#define Z_RM "rm"
#define Z_REMOVE "remove" // alias for rm
#define Z_PRINT "print"
int builtins_z(struct z_Database* restrict z_db, struct Args* restrict args, struct Arena* restrict arena,
               struct Arena* restrict scratch_arena);

#define NCSH_HISTORY "history" // the base command, displays history
#define NCSH_HISTORY_COUNT "count"
#define NCSH_HISTORY_CLEAN "clean"
#define NCSH_HISTORY_ADD "add"
#define NCSH_HISTORY_RM "rm" // alias for rm
#define NCSH_HISTORY_REMOVE "remove"
int builtins_history(struct History* restrict history, struct Args* restrict args, struct Arena* restrict arena,
                     struct Arena* restrict scratch_arena);

#define NCSH_EXIT "exit" // the base command
#define NCSH_QUIT "quit" // alias for exit
#define NCSH_Q "q"       // alias for exit
int builtins_exit(struct Args* restrict args);

#define NCSH_ECHO "echo"
int builtins_echo(struct Args* restrict args);

#define NCSH_HELP "help"
int builtins_help(struct Args* restrict args);

#define NCSH_CD "cd"
int builtins_cd(struct Args* restrict args);

#define NCSH_PWD "pwd"
int builtins_pwd(struct Args* restrict args);

#define NCSH_KILL "kill"
int builtins_kill(struct Args* restrict args);

#define NCSH_VERSION_CMD "version"
int builtins_version(struct Args* restrict args);

#define NCSH_ENABLE "enable"
int builtins_enable(struct Args* restrict args);

#define NCSH_DISABLE "disable"
int builtins_disable(struct Args* restrict args);

#define NCSH_SET "set" // not implemented
int builtins_set(struct Args* restrict args);

#define NCSH_UNSET "unset" // not implemented
int builtins_unset(struct Args* restrict args);

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
    BF_ENABLE =      1 << 7,
    BF_DISABLE =     1 << 8,
    BF_SET =         1 << 9,
    BF_UNSET =       1 << 10,
};
// clang-format on

struct Builtin {
    enum Builtins_Disabled flag;
    size_t length;
    char* value;
    int (*func)(struct Args* restrict);
};

static const struct Builtin builtins[] = {
    {.flag = BF_EXIT, .length = sizeof(NCSH_EXIT), .value = NCSH_EXIT, .func = &builtins_exit},
    {.flag = BF_EXIT, .length = sizeof(NCSH_QUIT), .value = NCSH_QUIT, .func = &builtins_exit},
    {.flag = BF_EXIT, .length = sizeof(NCSH_Q), .value = NCSH_Q, .func = &builtins_exit},
    {.flag = BF_ECHO, .length = sizeof(NCSH_ECHO), .value = NCSH_ECHO, .func = &builtins_echo},
    {.flag = BF_HELP, .length = sizeof(NCSH_HELP), .value = NCSH_HELP, .func = &builtins_help},
    {.flag = BF_CD, .length = sizeof(NCSH_CD), .value = NCSH_CD, .func = &builtins_cd},
    {.flag = BF_PWD, .length = sizeof(NCSH_PWD), .value = NCSH_PWD, .func = &builtins_pwd},
    {.flag = BF_KILL, .length = sizeof(NCSH_KILL), .value = NCSH_KILL, .func = &builtins_kill},
    {.flag = BF_VERSION, .length = sizeof(NCSH_VERSION_CMD), .value = NCSH_VERSION_CMD, .func = &builtins_version},
    {.flag = BF_ENABLE, .length = sizeof(NCSH_ENABLE), .value = NCSH_ENABLE, .func = &builtins_enable},
    {.flag = BF_DISABLE, .length = sizeof(NCSH_DISABLE), .value = NCSH_DISABLE, .func = &builtins_disable},
    {.flag = BF_SET, .length = sizeof(NCSH_SET), .value = NCSH_SET, .func = &builtins_set},
    {.flag = BF_UNSET, .length = sizeof(NCSH_UNSET), .value = NCSH_UNSET, .func = &builtins_unset},
};

static constexpr size_t builtins_count = sizeof(builtins) / sizeof(struct Builtin);
