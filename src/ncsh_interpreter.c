// Copyright (c) ncsh by Alex Eski 2025

#include <assert.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_defines.h"
#include "eskilib/eskilib_string.h"
#include "ncsh_builtins.h"
#include "ncsh_config.h"
#include "ncsh_defines.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "ncsh_types.h"
#include "ncsh_vm.h"
#include "z/z.h"

#define NCSH_Z "z"
#define NCSH_Z_ADD "add"
#define NCSH_Z_PRINT "print"

#define NCSH_HISTORY "history"

#define NCSH_EXIT "exit" // ncsh_builtins_exit
#define NCSH_QUIT "quit" // ncsh_builtins_exit
#define NCSH_Q "q"       // ncsh_builtins_exit
#define NCSH_ECHO "echo"
#define NCSH_HELP "help"
#define NCSH_CD "cd"
#define NCSH_PWD "pwd"
#define NCSH_KILL "kill"
#define NCSH_SET "set"

char* builtins[] = {NCSH_EXIT, NCSH_QUIT, NCSH_Q, NCSH_ECHO, NCSH_HELP, NCSH_CD, NCSH_PWD, NCSH_KILL, NCSH_SET};
size_t builtins_len[] = {sizeof(NCSH_EXIT), sizeof(NCSH_QUIT), sizeof(NCSH_Q),    sizeof(NCSH_ECHO), sizeof(NCSH_HELP),
                         sizeof(NCSH_CD),   sizeof(NCSH_PWD),  sizeof(NCSH_KILL), sizeof(NCSH_SET)};

int_fast32_t (*builtin_func[])(struct ncsh_Args*) = {&ncsh_builtins_exit, &ncsh_builtins_exit, &ncsh_builtins_exit,
                                                     &ncsh_builtins_echo, &ncsh_builtins_help, &ncsh_builtins_cd,
                                                     &ncsh_builtins_pwd,  &ncsh_builtins_kill, &ncsh_builtins_set};

eskilib_nodiscard int_fast32_t ncsh_interpreter_z(struct ncsh_Args* args, struct z_Database* z_db)
{
    assert(z_db);
    assert(args->count > 0);

    if (args->values[1] &&
        eskilib_string_compare(args->values[1], args->lengths[1], NCSH_Z_PRINT, sizeof(NCSH_Z_PRINT))) {
        z_print(z_db);
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (args->count > 2) {
        if (!args->values[1] || !args->values[2]) {
            return NCSH_COMMAND_FAILED_CONTINUE;
        }

        if (eskilib_string_compare(args->values[1], args->lengths[1], NCSH_Z_ADD, sizeof(NCSH_Z_ADD))) {
            size_t length = strlen(args->values[2]) + 1;
            if (z_add(args->values[2], length, z_db) == Z_SUCCESS) {
                return NCSH_COMMAND_SUCCESS_CONTINUE;
            }
            else {
                return NCSH_COMMAND_EXIT_FAILURE;
            }
        }
        else {
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    size_t length = !args->values[1] ? 0 : strlen(args->values[1]) + 1;
    char cwd[PATH_MAX] = {0};
    char* cwd_result = getcwd(cwd, PATH_MAX);
    if (!cwd_result) {
        perror(RED "ncsh z: Could not load cwd information" RESET);
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    z(args->values[1], length, cwd, z_db);
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

void ncsh_interpreter_alias(struct ncsh_Args* args)
{
    struct eskilib_String alias = ncsh_config_alias_check(args->values[0], args->lengths[0]);
    if (alias.length) {
        args->values[0] = realloc(args->values[0], alias.length);
        memcpy(args->values[0], alias.value, alias.length - 1);
        args->values[0][alias.length - 1] = '\0';
        args->lengths[0] = alias.length;
    }
}

eskilib_nodiscard int_fast32_t ncsh_interpreter_execute(struct ncsh_Shell* shell)
{
    assert(shell);
    assert(&shell->args);

    if (shell->args.count == 0) {
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (eskilib_string_compare(shell->args.values[0], shell->args.lengths[0], NCSH_Z, sizeof(NCSH_Z))) {
        return ncsh_interpreter_z(&shell->args, &shell->z_db);
    }

    if (eskilib_string_compare(shell->args.values[0], shell->args.lengths[0], NCSH_HISTORY, sizeof(NCSH_HISTORY))) {
        return ncsh_history_command(&shell->args, &shell->history);
    }

    ncsh_interpreter_alias(&shell->args);

    for (uint_fast32_t i = 0; i < sizeof(builtins) / sizeof(char*); ++i) {
        if (eskilib_string_compare(shell->args.values[0], shell->args.lengths[0], builtins[i], builtins_len[i])) {
            return (*builtin_func[i])(&shell->args);
        }
    }

    return ncsh_vm_execute(&shell->args);
}

eskilib_nodiscard int_fast32_t ncsh_interpreter_execute_noninteractive(struct ncsh_Args* args)
{
    assert(args);

    ncsh_interpreter_alias(args);

    for (uint_fast32_t i = 0; i < sizeof(builtins) / sizeof(char*); ++i) {
        if (eskilib_string_compare(args->values[0], args->lengths[0], builtins[i], builtins_len[i])) {
            return (*builtin_func[i])(args);
        }
    }

    return ncsh_vm_execute_noninteractive(args);
}
