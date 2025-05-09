/* Copyright ncsh (C) by Alex Eski 2025 */

#include "vm_tokenizer.h"

#include <assert.h>
#include <glob.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../args.h"
#include "../defines.h"
#include "../env.h"
#include "../vars.h"

[[nodiscard]]
int vm_tokenizer_syntax_error(char* restrict message, size_t message_length)
{
    if (write(STDIN_FILENO, message, message_length) == -1) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    return NCSH_COMMAND_SYNTAX_ERROR;
}

#define INVALID_SYNTAX_PIPE_FIRST_ARG                                                                                  \
    "ncsh: Invalid syntax: found pipe operator ('|') as first argument. Correct usage of pipe operator is 'program1 "  \
    "| program2'.\n"
#define INVALID_SYNTAX_PIPE_LAST_ARG                                                                                   \
    "ncsh: Invalid syntax: found pipe operator ('|') as last argument. Correct usage of pipe operator is 'program1 | " \
    "program2'.\n"

#define INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG                                                                          \
    "ncsh: Invalid syntax: found output redirection operator ('>') as first argument. Correct usage of output "        \
    "redirection operator is 'program > file'.\n"
#define INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG                                                                           \
    "ncsh: Invalid syntax: found no filename after output redirect operator ('>'). Correct usage of output "           \
    "redirection operator is 'program > file'.\n"

#define INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG                                                                   \
    "ncsh: Invalid syntax: found output redirection append operator ('>>') as first argument. Correct usage of "       \
    "output redirection append operator is 'program >> file'.\n"
#define INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG                                                                    \
    "ncsh: Invalid syntax: found no filename after output redirect append operator ('>>'). Correct usage of output "   \
    "redirection operator is 'program >> file'.\n"

#define INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG                                                                           \
    "ncsh: Invalid syntax: found input redirection operator ('<') as first argument. Correct usage of input "          \
    "redirection operator is 'program < file'.\n"
#define INVALID_SYNTAX_STDIN_REDIR_LAST_ARG                                                                            \
    "ncsh: Invalid syntax: found input redirection operator ('<') as last argument. Correct usage of input "           \
    "redirection operator is 'program < file'.\n"

#define INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG                                                                          \
    "ncsh: Invalid syntax: found error redirection operator ('2>') as first argument. Correct usage of error "         \
    "redirection is 'program 2> file'.\n"
#define INVALID_SYNTAX_STDERR_REDIR_LAST_ARG                                                                           \
    "ncsh: Invalid syntax: found error redirection operator ('2>') as last argument. Correct usage of error "          \
    "redirection is 'program 2> file'.\n"

#define INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG                                                                   \
    "ncsh: Invalid syntax: found error redirection append operator ('2>>') as first argument. Correct usage of error " \
    "redirection is 'program 2>> file'.\n"
#define INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG                                                                    \
    "ncsh: Invalid syntax: found error redirection operator ('2>>') as last argument. Correct usage of error "         \
    "redirection is 'program 2>> file'.\n"

#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_FIRST_ARG                                                               \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>') as first argument. Correct usage of "      \
    "output & error redirection is 'program &> file'.\n"
#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_LAST_ARG                                                                \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>') as last argument. Correct usage of "       \
    "output & error redirection is 'program &> file'.\n"

#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_FIRST_ARG                                                        \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>>') as first argument. Correct usage of "     \
    "output & error redirection is 'program &>> file'.\n"
#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_LAST_ARG                                                         \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>>') as last argument. Correct usage of "      \
    "output & error redirection is 'program &>> file'.\n"

#define INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG                                                                        \
    "ncsh: Invalid syntax: found background job operator ('&') as first argument. Correct usage of background job "    \
    "operator is 'program &'.\n"
#define INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG                                                                     \
    "ncsh: Invalid syntax: found background job operator ('&') in position other than last argument. Correct usage "   \
    "of background job operator is 'program &'.\n"

#define INVALID_SYNTAX_AND_IN_LAST_POSITION                                                                            \
    "ncsh: Invalid syntax: found and operator ('&&') as last argument. Correct usage of and operator is "              \
    "'true && true'\n"
#define INVALID_SYNTAX_AND_IN_FIRST_POSITION                                                                           \
    "ncsh: Invalid syntax: found and operator ('&&') as first argument. Correct usage of and operator is "             \
    "'true && true'\n"

#define INVALID_SYNTAX_OR_IN_LAST_POSITION                                                                             \
    "ncsh: Invalid syntax: found or operator ('||') as last argument. Correct usage of or operator is "                \
    "'false || true'\n"
#define INVALID_SYNTAX_OR_IN_FIRST_POSITION                                                                            \
    "ncsh: Invalid syntax: found or operator ('||') as first argument. Correct usage of or operator is "               \
    "'false || true'\n"

#define INVALID_SYNTAX(message) vm_tokenizer_syntax_error(message, sizeof(message) - 1)

int vmtok_invalid_syntax_check_res;

/* vm_tokenizer_syntax_check_first_arg
 * Simple check to see if something is in first position that shouldn't be
 */
void vm_tokenizer_syntax_check_first_arg(uint8_t op)
{
    switch (op) {
    case OP_PIPE: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_PIPE_FIRST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG);
        break;
    }
    case OP_STDIN_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_FIRST_ARG);
        break;
    }
    case OP_BACKGROUND_JOB: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG);
        break;
    }
    case OP_AND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_AND_IN_FIRST_POSITION);
        break;
    }
    case OP_OR: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_OR_IN_FIRST_POSITION);
        break;
    }
    }
}

/* vm_tokenizer_syntax_check_last_arg
 * Simple check to see if something is in last position that shouldn't be
 */
void vm_tokenizer_syntax_check_last_arg(struct Args* restrict args)
{
    struct Arg* arg = args->head->next;
    while (arg->next)
        arg = arg->next;

    switch (arg->op) {
    case OP_PIPE: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_PIPE_LAST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG);
        break;
    }
    case OP_STDIN_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDIN_REDIR_LAST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_LAST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_LAST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_LAST_ARG);
        break;
    }
    case OP_AND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_AND_IN_LAST_POSITION);
        break;
    }
    case OP_OR: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_OR_IN_LAST_POSITION);
        break;
    }
    }
}

[[nodiscard]]
int vm_tokenizer_syntax_check(struct Args* restrict args)
{
    assert(args);

    vmtok_invalid_syntax_check_res = NCSH_COMMAND_SUCCESS_CONTINUE;
    vm_tokenizer_syntax_check_first_arg(args->head->next->op);
    if (vmtok_invalid_syntax_check_res != NCSH_COMMAND_SUCCESS_CONTINUE)
        return vmtok_invalid_syntax_check_res;
    vm_tokenizer_syntax_check_last_arg(args);
    return vmtok_invalid_syntax_check_res;
}

void vm_tokenizer_glob_process(struct Arg* restrict arg, size_t* restrict args_count,
                               struct Arena* restrict scratch_arena)
{
    glob_t glob_buf = {0};
    size_t glob_len;
    glob(arg->val, GLOB_DOOFFS, NULL, &glob_buf);
    if (!glob_buf.gl_pathc) {
        globfree(&glob_buf);
        return;
    }

    // handle first arg manually to preserve pointer from previous arg to current arg
    glob_len = strlen(glob_buf.gl_pathv[0]) + 1;
    if (glob_len > arg->len) {
        arg->val = arena_realloc(scratch_arena, glob_len, char, arg->val, arg->len);
        memcpy(arg->val, glob_buf.gl_pathv[0], glob_len);
    }
    else {
        memcpy(arg->val, glob_buf.gl_pathv[0], glob_len);
    }
    arg->len = glob_len;
    arg->op = OP_CONSTANT;

    for (size_t i = 1; i < glob_buf.gl_pathc; ++i) {
        glob_len = strlen(glob_buf.gl_pathv[i]) + 1;
        if (!glob_len || glob_len >= NCSH_MAX_INPUT)
            break;

        struct Arg* new_arg = arg_alloc(OP_CONSTANT, glob_len, glob_buf.gl_pathv[i], scratch_arena);
        if (!arg->next) {
            arg->next = new_arg;
            arg = arg->next;
        }
        else {
            arg_set_after(arg->next, new_arg);
        }
        ++*args_count;
    }

    globfree(&glob_buf);
}

void vm_tokenizer_assignment_process(struct Arg* restrict arg, struct Vars* vars, struct Arena* restrict arena)
{
    assert(arena);
    // variable values are stored in vars hashmap.
    // the key is the previous value, which is tagged with OP_VARIABLE.
    // when VM comes in contact with OP_VARIABLE, it looks up value in vars.
    size_t assignment_pos;
    for (assignment_pos = 0; assignment_pos < arg->len; ++assignment_pos) {
        if (arg->val[assignment_pos] == '=')
            break;
    }
    size_t key_len = assignment_pos + 1;
    char* var_name = arena_malloc(arena, key_len, char);
    memcpy(var_name, arg->val, assignment_pos);
    var_name[assignment_pos] = '\0';
    debugf("saving var_name %s\n", var_name);
    struct Str* val = arena_malloc(arena, 1, struct Str);
    val->length = arg->len - assignment_pos - 1;
    val->value = arena_malloc(arena, val->length, char);
    memcpy(val->value, arg->val + key_len, val->length);
    debugf("var_name: %s : %s (%zu)\n", var_name, val->value, val->length);
    char* key = arena_malloc(arena, key_len, char);
    memcpy(key, var_name, key_len);
    debugf("key: %s (%zu)\n", key, key_len);
    vars_set(key, val, arena, vars);
}

void vm_tokenizer_variable_process(struct Arg* restrict arg, struct Vars* restrict vars,
                                   struct Arena* restrict scratch_arena)
{
    struct Str var;
    if (estrcmp(arg->val, arg->len, NCSH_PATH_VAR, sizeof(NCSH_PATH_VAR))) {

        debug("replacing variable $PATH\n");
        var = env_path_get();
        if (!var.value || !*var.value) {
            puts("ncsh: could not load path to replace $PATH variable.");
            return;
        }
    }
    else if (estrcmp(arg->val, arg->len, NCSH_HOME_VAR, sizeof(NCSH_HOME_VAR))) {

        debug("replacing variable $HOME\n");
        env_home_get(&var, scratch_arena);
        if (!var.value || !*var.value) {
            puts("ncsh: could not load path to replace $PATH variable.");
            return;
        }
    }
    else {
        char* key = arg->val + 1; // skip first value in arg->val (the $)
        debugf("trying to get variable %s\n", key);
        struct Str* val = vars_get(key, vars);
        if (!val || !val->value || !*val->value) {
            printf("ncsh: variable with name '%s' did not have a value associated with it.\n", key);
            return;
        }
        var = *val;
    }

    // replace the variable name with the value of the variable
    arg->val = arena_realloc(scratch_arena, var.length, char, arg->val, arg->len);
    memcpy(arg->val, var.value, var.length);
    arg->len = var.length;
    arg->op = OP_CONSTANT; // replace OP_VARIABLE to OP_CONSTANT so VM sees it as a regular constant value
    debugf("replaced variable with value %s %zu\n", arg->val, arg->len);
}

/* vm_tokenizer_alias_replace
 * Replaces aliases with their aliased commands before executing
 */
void vm_tokenizer_alias_replace(struct Arg* restrict arg, struct Arena* restrict scratch_arena)
{
    struct Str alias = config_alias_check(arg->val, arg->len);
    if (alias.length) {
        arg->val = arena_realloc(scratch_arena, alias.length, char, arg->val, arg->len);
        memcpy(arg->val, alias.value, alias.length);
        arg->len = alias.length;
    }
}

int vm_tokenizer_ops_process(struct Args* restrict args, struct Tokens* restrict tokens, struct Shell* restrict shell,
                             struct Arena* restrict scratch_arena)
{
    assert(args && args->head);
    assert(tokens);
    assert(scratch_arena);

    struct Arg* arg = args->head->next;
    struct Arg* prev = NULL;
    tokens->is_background_job = false;
    while (arg) {
        switch (arg->op) {
        case OP_STDOUT_REDIRECTION: {
            tokens->stdout_file = arg->next->val;
            tokens->stdout_redirect = arg;
            break;
        }
        case OP_STDOUT_REDIRECTION_APPEND: {
            tokens->stdout_file = arg->next->val;
            tokens->stdout_redirect = arg;
            tokens->output_append = true;
            break;
        }
        case OP_STDIN_REDIRECTION: {
            tokens->stdin_file = arg->next->val;
            tokens->stdin_redirect = arg;
            break;
        }
        case OP_STDERR_REDIRECTION: {
            tokens->stderr_file = arg->next->val;
            tokens->stderr_redirect = arg;
            break;
        }
        case OP_STDERR_REDIRECTION_APPEND: {
            tokens->stderr_file = arg->next->val;
            tokens->stderr_redirect = arg;
            tokens->output_append = true;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION: {
            tokens->stdout_and_stderr_file = arg->next->val;
            tokens->stdout_and_stderr_redirect = arg;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
            tokens->stdout_and_stderr_file = arg->next->val;
            tokens->stdout_and_stderr_redirect = arg;
            tokens->output_append = true;
            break;
        }
        case OP_PIPE: {
            ++tokens->number_of_pipe_commands;
            break;
        }
        case OP_BACKGROUND_JOB: {
            if (arg->next) {
                return INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG);
            }
            tokens->is_background_job = true;
            break;
        }
        case OP_HOME_EXPANSION: {
            size_t len = shell->config.home_location.length;
            arg->val = arena_malloc(scratch_arena, len, char);
            memcpy(arg->val, shell->config.home_location.value, len);
            break;
        }
        case OP_GLOB_EXPANSION: {
            vm_tokenizer_glob_process(arg, &args->count, scratch_arena);
            break;
        }
        case OP_ASSIGNMENT: {
            vm_tokenizer_assignment_process(arg, &shell->vars, &shell->arena);
            if (prev && arg) {
                arg_set_after(arg->next, prev);
                prev = arg->next;
                if (arg->next->next) {
                    arg = arg->next->next;
                    continue;
                }
            }
            break;
        }
        case OP_VARIABLE: {
            vm_tokenizer_variable_process(arg, &shell->vars, scratch_arena);
            break;
        }
        default: {
            vm_tokenizer_alias_replace(arg, scratch_arena);
            break;
        }
        }
        prev = arg;
        arg = arg->next;
    }
    ++tokens->number_of_pipe_commands;

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int vm_tokenizer_tokenize(struct Args* restrict args, struct Tokens* restrict tokens, struct Shell* restrict shell,
                          struct Arena* restrict scratch_arena)
{
    assert(args);
    assert(tokens);
    assert(scratch_arena);
    if (!args || !args->head || !args->count)
        return NCSH_COMMAND_FAILED_CONTINUE;

    int result;
    if ((result = vm_tokenizer_syntax_check(args)) != NCSH_COMMAND_SUCCESS_CONTINUE)
        return result;

    if ((result = vm_tokenizer_ops_process(args, tokens, shell, scratch_arena)) != NCSH_COMMAND_SUCCESS_CONTINUE)
        return result;

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}
