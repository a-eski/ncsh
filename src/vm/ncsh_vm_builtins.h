/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../eskilib/eskilib_string.h"
#include "../readline/ncsh_history.h"
#include "../ncsh_parser.h"
#include "../z/z.h"

#define NCSH_Z "z" // the base command, changes directory
#define NCSH_Z_ADD "add"
#define NCSH_Z_RM "rm"
#define NCSH_Z_REMOVE "remove" // alias for rm
#define NCSH_Z_PRINT "print"
int_fast32_t ncsh_builtins_z(struct z_Database* const restrict z_db,
                             const struct ncsh_Args* const restrict args,
                             struct ncsh_Arena* const arena,
                             struct ncsh_Arena* const scratch_arena);

#define NCSH_HISTORY "history" // the base command, displays history
#define NCSH_HISTORY_COUNT "count"
#define NCSH_HISTORY_CLEAN "clean"
#define NCSH_HISTORY_ADD "add"
#define NCSH_HISTORY_RM "rm" // alias for rm
#define NCSH_HISTORY_REMOVE "remove"
int_fast32_t ncsh_builtins_history(struct ncsh_History* const restrict history,
                                   const struct ncsh_Args* const restrict args,
                                   struct ncsh_Arena* const arena,
                                   struct ncsh_Arena* const scratch_arena);

#define NCSH_EXIT "exit" // the base command
#define NCSH_QUIT "quit" // alias for exit
#define NCSH_Q "q"       // alias for exit
int_fast32_t ncsh_builtins_exit(const struct ncsh_Args* const restrict args);

#define NCSH_ECHO "echo"
int_fast32_t ncsh_builtins_echo(const struct ncsh_Args* const restrict args);

#define NCSH_HELP "help"
int_fast32_t ncsh_builtins_help(const struct ncsh_Args* const restrict args);

#define NCSH_CD "cd"
int_fast32_t ncsh_builtins_cd(const struct ncsh_Args* const restrict args);

#define NCSH_PWD "pwd"
int_fast32_t ncsh_builtins_pwd(const struct ncsh_Args* const restrict args);

#define NCSH_KILL "kill"
int_fast32_t ncsh_builtins_kill(const struct ncsh_Args* const restrict args);

#define NCSH_VERSION_CMD "version"
int_fast32_t ncsh_builtins_version(const struct ncsh_Args* const restrict args);

#define NCSH_SET "set" // not fully implemented
int_fast32_t ncsh_builtins_set(const struct ncsh_Args* const restrict args);

struct ncsh_Builtin {
    size_t length;
    char* value;
    int_fast32_t (*func)(const struct ncsh_Args* const restrict);
};

static const struct ncsh_Builtin builtins[] = {
    { .length = sizeof(NCSH_EXIT), .value = NCSH_EXIT, .func = &ncsh_builtins_exit },
    { .length = sizeof(NCSH_QUIT), .value = NCSH_QUIT, .func = &ncsh_builtins_exit },
    { .length = sizeof(NCSH_Q), .value = NCSH_Q, .func = &ncsh_builtins_exit },
    { .length = sizeof(NCSH_ECHO), .value = NCSH_ECHO, .func = &ncsh_builtins_echo },
    { .length = sizeof(NCSH_HELP), .value = NCSH_HELP, .func = &ncsh_builtins_help },
    { .length = sizeof(NCSH_CD), .value = NCSH_CD, .func = &ncsh_builtins_cd },
    { .length = sizeof(NCSH_PWD), .value = NCSH_PWD, .func = &ncsh_builtins_pwd },
    { .length = sizeof(NCSH_KILL), .value = NCSH_KILL, .func = &ncsh_builtins_kill },
    { .length = sizeof(NCSH_VERSION_CMD), .value = NCSH_VERSION_CMD, .func = &ncsh_builtins_version },
    { .length = sizeof(NCSH_SET), .value = NCSH_SET, .func = &ncsh_builtins_set },
};

static constexpr size_t builtins_count = sizeof(builtins) / sizeof(struct ncsh_Builtin);
