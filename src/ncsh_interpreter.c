// Copyright (c) ncsh by Alex Eski 2025

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_defines.h"
#include "ncsh_builtins.h"
#include "ncsh_config.h"
#include "ncsh_defines.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "ncsh_types.h"
#include "ncsh_vm.h"

char *builtins[] = { "exit", "quit", "q", "echo", "help", "cd", "pwd", "kill", "set" };

int_fast32_t (*builtin_func[]) (struct ncsh_Args *) =
{
    &ncsh_builtins_exit,
    &ncsh_builtins_exit,
    &ncsh_builtins_exit,
    &ncsh_builtins_echo,
    &ncsh_builtins_help,
    &ncsh_builtins_cd,
    &ncsh_builtins_pwd,
    &ncsh_builtins_kill,
    &ncsh_builtins_set
};

eskilib_nodiscard int_fast32_t ncsh_z(struct ncsh_Args *args, struct z_Database *z_db)
{
    assert(z_db);
    assert(args->count > 0);

    if (args->count > 2)
    {
        if (!args->values[1] || !args->values[2])
            return NCSH_COMMAND_EXIT_FAILURE;

        if (eskilib_string_equals(args->values[1], "add", args->lengths[1]))
        {
            size_t length = strlen(args->values[2]) + 1;
            if (z_add(args->values[2], length, z_db) == Z_SUCCESS)
                return NCSH_COMMAND_SUCCESS_CONTINUE;
            else
                return NCSH_COMMAND_EXIT_FAILURE;
        }
        else
        {
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    size_t length = !args->values[1] ? 0 : strlen(args->values[1]) + 1;
    char cwd[PATH_MAX] = {0};
    char *cwd_result = getcwd(cwd, PATH_MAX);
    if (!cwd_result)
    {
        perror(RED "ncsh z: Could not load cwd information" RESET);
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    z(args->values[1], length, cwd, z_db);
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

eskilib_nodiscard int_fast32_t ncsh_interpreter_execute(struct ncsh_Shell *shell)
{
    if (shell->args.count == 0)
        return NCSH_COMMAND_SUCCESS_CONTINUE;

    if (eskilib_string_equals(shell->args.values[0], "z", shell->args.lengths[0]))
        return ncsh_z(&shell->args, &shell->z_db);

    if (eskilib_string_equals(shell->args.values[0], "history", shell->args.lengths[0]))
    {
        if (shell->args.values[1] && shell->args.lengths[1] == 6 &&
            eskilib_string_equals(shell->args.values[1], "count", 6))
        {
            printf("history count: %d\n", shell->history.count);
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
        return ncsh_history_command(&shell->history);
    }

    struct eskilib_String alias = ncsh_config_alias_check(shell->args.values[0], shell->args.lengths[0]);
    if (alias.length)
    {
        shell->args.values[0] = realloc(shell->args.values[0], alias.length);
        memcpy(shell->args.values[0], alias.value, alias.length - 1);
        shell->args.values[0][alias.length - 1] = '\0';
        shell->args.lengths[0] = alias.length;
    }

    for (uint_fast32_t i = 0; i < sizeof(builtins) / sizeof(char *); ++i)
    {
        if (eskilib_string_equals(shell->args.values[0], builtins[i], shell->args.lengths[0]))
        {
            return (*builtin_func[i])(&shell->args);
        }
    }

    return ncsh_vm_execute(&shell->args);
}

eskilib_nodiscard int_fast32_t ncsh_interpreter_execute_noninteractive(struct ncsh_Args *args)
{
    for (uint_fast32_t i = 0; i < sizeof(builtins) / sizeof(char *); ++i)
    {
        if (eskilib_string_equals(args->values[0], builtins[i], args->lengths[0]))
        {
            return (*builtin_func[i])(args);
        }
    }
    return ncsh_vm_execute_noninteractive(args);
}
