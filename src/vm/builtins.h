/* Copyright ncsh (C) by Alex Eski 2024 */
/* builtins.h: shell builtins for ncsh */

#pragma once

#include "../args.h"
#include "../defines.h"
#include "../readline/history.h"
#include "../z/z.h"

struct Builtin_Args {
    char** rst buffer;
    size_t* rst buf_lens;
    int fd;
    Arena* rst arena;
    Arena* rst scratch_arena;
    History* rst history;
    z_Database* rst z_db;
};

#define Z "z" // the base command, changes directory
#define Z_ADD "add"
#define Z_RM "rm"
#define Z_REMOVE "remove" // alias for rm
#define Z_PRINT "print"
#define Z_COUNT "count"
int builtins_z(z_Database* rst z_db, char** rst buffer, size_t* rst buf_lens, Arena* arena,
               Arena* rst scratch_arena);

#define NCSH_HISTORY "history" // the base command, displays history
#define NCSH_HISTORY_COUNT "count"
#define NCSH_HISTORY_CLEAN "clean"
#define NCSH_HISTORY_ADD "add"
#define NCSH_HISTORY_RM "rm" // alias for rm
#define NCSH_HISTORY_REMOVE "remove"
int builtins_history(History* rst history, char** rst buffer, size_t* rst buf_lens, Arena* rst arena,
                     Arena* rst scratch_arena);

#define NCSH_ALIAS "alias"
#define NCSH_ALIAS_PRINT "-p"
#define NCSH_ALIAS_ADD "add"
#define NCSH_ALIAS_RM "rm"
#define NCSH_ALIAS_REMOVE "rm"
#define NCSH_ALIAS_DELETE "delete"
int builtins_alias(char** rst buffer, size_t* rst buf_lens, Arena* rst arena);

#define NCSH_UNALIAS "unalias"
#define NCSH_UNALIAS_DELETE "-a"
#define NCSH_UNALIAS_DELETE_ALIAS "delete"
int builtins_unalias(char** rst buffer, size_t* rst buf_lens);

#define NCSH_EXIT "exit" // the base command
#define NCSH_QUIT "quit" // alias for exit
#define NCSH_Q "q"       // alias for exit
int builtins_exit(char** rst buffer, size_t* rst buf_lens);

#define NCSH_ECHO "echo"
#define NCSH_ECHO_NO_NEWLINE "-n"
int builtins_echo(char** rst buffer, size_t* rst buf_lens);

#define NCSH_HELP "help"
int builtins_help(char** rst buffer, size_t* rst buf_lens);

#define NCSH_CD "cd"
int builtins_cd(char** rst buffer, size_t* rst buf_lens);

#define NCSH_PWD "pwd"
int builtins_pwd(char** rst buffer, size_t* rst buf_lens);

#define NCSH_KILL "kill"
int builtins_kill(char** rst buffer, size_t* rst buf_lens);

#define NCSH_VERSION_CMD "version"
int builtins_version(char** rst buffer, size_t* rst buf_lens);

// TODO: finish implementation
#define NCSH_ENABLE "enable"
int builtins_enable(char** rst buffer, size_t* rst buf_lens);

// TODO: finish implementation
#define NCSH_DISABLE "disable"
int builtins_disable(char** rst buffer, size_t* rst buf_lens);

// TODO: finish implementation
#define NCSH_EXPORT "export"
int builtins_export(Args* rst args, size_t* rst buf_lens);

// TODO: finish implementation
#define NCSH_SET "set"
int builtins_set(Args* rst args, size_t* rst buf_lens);

// TODO: finish implementation
#define NCSH_UNSET "unset"
int builtins_unset(Args* rst args, size_t* rst buf_lens);

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
    int (*func)(char** rst r, size_t* rst l);
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
    // {.flag = BF_ALIAS, .length = sizeof(NCSH_ALIAS), .value = NCSH_ALIAS, .func = &builtins_alias},
    /*{.flag = BF_ENABLE, .length = sizeof(NCSH_ENABLE), .value = NCSH_ENABLE, .func = &builtins_enable},
    {.flag = BF_DISABLE, .length = sizeof(NCSH_DISABLE), .value = NCSH_DISABLE, .func = &builtins_disable},
    {.flag = BF_EXPORT, .length = sizeof(NCSH_EXPORT), .value = NCSH_EXPORT, .func = &builtins_export},
    {.flag = BF_SET, .length = sizeof(NCSH_SET), .value = NCSH_SET, .func = &builtins_set},
    {.flag = BF_UNSET, .length = sizeof(NCSH_UNSET), .value = NCSH_UNSET, .func = &builtins_unset},*/
};

static constexpr size_t builtins_count = sizeof(builtins) / sizeof(Builtin);
