// Copyright (c) ncsh by Alex Eski 2024

#ifndef NCSH_BUILTINS_H_
#define NCSH_BUILTINS_H_

#include <stdbool.h>
#include <stdint.h>

#include "eskilib/eskilib_string.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "z/z.h"

#define NCSH_Z "z"
#define NCSH_Z_ADD "add"
#define NCSH_Z_PRINT "print"
// Handles z, z {directory}, z add {directory}, z print
int_fast32_t ncsh_builtins_z(struct z_Database* z_db, struct ncsh_Args* args);

#define NCSH_HISTORY "history"
#define NCSH_HISTORY_COUNT "count"
#define NCSH_HISTORY_CLEAN "clean"
// Handles history, history count, history clean
int_fast32_t ncsh_builtins_history(struct ncsh_History* history, struct ncsh_Args* args);

#define NCSH_EXIT "exit" // ncsh_builtins_exit
#define NCSH_QUIT "quit" // ncsh_builtins_exit
#define NCSH_Q "q"       // ncsh_builtins_exit
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

#endif // !NCSH_BUILTINS_H_
