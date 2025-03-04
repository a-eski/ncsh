// Copyright (c) ncsh by Alex Eski 2024

#ifndef NCSH_BUILTINS_H_
#define NCSH_BUILTINS_H_

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
int_fast32_t ncsh_builtins_z(struct z_Database* z_db, struct ncsh_Args* args);

#define NCSH_HISTORY "history" // the base command, displays history
#define NCSH_HISTORY_COUNT "count"
#define NCSH_HISTORY_CLEAN "clean"
#define NCSH_HISTORY_ADD "add"
#define NCSH_HISTORY_RM "rm" // alias for rm
#define NCSH_HISTORY_REMOVE "remove"
int_fast32_t ncsh_builtins_history(struct ncsh_History* history, struct ncsh_Args* args);

#define NCSH_EXIT "exit" // the base command
#define NCSH_QUIT "quit" // alias for exit
#define NCSH_Q "q"       // alias for exit
int_fast32_t ncsh_builtins_exit(struct ncsh_Args* args);

#define NCSH_ECHO "echo"
int_fast32_t ncsh_builtins_echo(struct ncsh_Args* args);

#define NCSH_HELP "help"
int_fast32_t ncsh_builtins_help(struct ncsh_Args* args);

#define NCSH_CD "cd"
int_fast32_t ncsh_builtins_cd(struct ncsh_Args* args);

#define NCSH_PWD "pwd"
int_fast32_t ncsh_builtins_pwd(struct ncsh_Args* args);

#define NCSH_KILL "kill"
int_fast32_t ncsh_builtins_kill(struct ncsh_Args* args);

#define NCSH_SET "set" // not fully implemented
int_fast32_t ncsh_builtins_set(struct ncsh_Args* args);

struct ncsh_Builtin {
    size_t length;
    char* value;
    int_fast32_t (*func)(struct ncsh_Args*);
};

static struct ncsh_Builtin builtins[] = {
    { .length = sizeof(NCSH_EXIT), .value = NCSH_EXIT, .func = &ncsh_builtins_exit },
    { .length = sizeof(NCSH_QUIT), .value = NCSH_QUIT, .func = &ncsh_builtins_exit },
    { .length = sizeof(NCSH_Q), .value = NCSH_Q, .func = &ncsh_builtins_exit },
    { .length = sizeof(NCSH_ECHO), .value = NCSH_ECHO, .func = &ncsh_builtins_echo },
    { .length = sizeof(NCSH_HELP), .value = NCSH_HELP, .func = &ncsh_builtins_help },
    { .length = sizeof(NCSH_CD), .value = NCSH_CD, .func = &ncsh_builtins_cd },
    { .length = sizeof(NCSH_PWD), .value = NCSH_PWD, .func = &ncsh_builtins_pwd },
    { .length = sizeof(NCSH_KILL), .value = NCSH_KILL, .func = &ncsh_builtins_kill },
    { .length = sizeof(NCSH_SET), .value = NCSH_SET, .func = &ncsh_builtins_set },
};

static constexpr size_t builtins_count = sizeof(builtins) / sizeof(struct ncsh_Builtin);

#endif // !NCSH_BUILTINS_H_
