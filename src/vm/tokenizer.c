/* Copyright ncsh (C) by Alex Eski 2025 */
/* tokenizer.c: Preprocessing of parser output to ensure ready for VM to process. */

#include "tokenizer.h"

#include <assert.h>
#include <glob.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../alias.h"
#include "../args.h"
#include "../defines.h"
#include "../env.h"
#include "tokenizer_syntax.h"
#include "vars.h"

void tokenizer_home_expansion_process(Arg* rst arg, Str home_location, Arena* rst scratch_arena)
{
    if (arg->len == 2) {
        size_t len = home_location.length + 1;
        arg->val = arena_malloc(scratch_arena, len, char);
        arg->op = OP_CONSTANT;
        arg->len = len;
        memcpy(arg->val, home_location.value, len);
        return;
    }

    assert(arg->val);
    // skip if value is null or home expansion not at beginning of value
    if (!arg->val || arg->val[0] != '~')
        return;

    size_t len = arg->len + home_location.length; // arg->len accounts for null termination
    char* new_value = arena_malloc(scratch_arena, len, char);
    memcpy(new_value, home_location.value, home_location.length);
    memcpy(new_value + home_location.length, arg->val + 1, arg->len - 1);
    debugf("performing home expansion on %s to %s\n", arg->val, new_value);
    arg->val = new_value;
    arg->op = OP_CONSTANT;
    arg->len = len;
}

void tokenizer_glob_process(Arg* rst arg, size_t* rst args_count, Arena* rst scratch_arena)
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

        Arg* new_arg = arg_alloc(OP_CONSTANT, glob_len, glob_buf.gl_pathv[i], scratch_arena);
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

void tokenizer_assignment_process(Arg* arg, Vars* rst vars, Arena* rst arena)
{
    assert(arg);
    assert(vars);
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
    Str* val = arena_malloc(arena, 1, Str);
    val->length = arg->len - assignment_pos - 1;
    val->value = arena_malloc(arena, val->length, char);
    memcpy(val->value, arg->val + key_len, val->length);
    debugf("var_name: %s : %s (%zu)\n", var_name, val->value, val->length);
    char* key = arena_malloc(arena, key_len, char);
    memcpy(key, var_name, key_len);
    debugf("key: %s (%zu)\n", key, key_len);
    vars_set(key, val, arena, vars);
}

void tokenizer_arg_update(Arg* rst arg, Str* rst var, Arena* rst scratch_arena)
{
    arg->val = arena_realloc(scratch_arena, var->length, char, arg->val, arg->len);
    memcpy(arg->val, var->value, var->length);
    arg->len = var->length;
    arg->op = OP_CONSTANT; // replace OP_VARIABLE to OP_CONSTANT so VM sees it as a regular constant value
    debugf("replaced variable with value %s %zu\n", arg->val, arg->len);
}

void tokenizer_variable_process(Arg* rst arg, Vars* rst vars, Arena* rst scratch_arena)
{
    Str var;
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
            puts("ncsh: could not load home to replace $HOME variable.");
            return;
        }
    }
    else {
        char* key = arg->val + 1; // skip first value in arg->val (the $)
        debugf("trying to get variable %s\n", key);
        Str* val = vars_get(key, vars);
        if (!val || !val->value || !*val->value) {
            // printf("ncsh: variable with name '%s' did not have a value associated with it.", key);
            return;
        }
        var = *val;
    }

    char* space = strchr(var.value, ' ');
    if (!space) {
        tokenizer_arg_update(arg, &var, scratch_arena);
        return;
    }

    debug("found space");
    Args* args = parser_parse(var.value, var.length, scratch_arena);
    Arg* var_arg = args->head->next;
    if (!var_arg) {
        return;
    }
    debugf("found value %s\n", var_arg->val);
    Str var_str = {.value = var_arg->val, .length = var_arg->len};
    tokenizer_arg_update(arg, &var_str, scratch_arena);
    var_arg = var_arg->next;

    for (size_t i = 0; i < args->count; ++i) {
        if (!var_arg)
            break;

        debugf("found next value %s\n", var_arg->val);
        if (!arg) {
            arg = var_arg;
            var_arg = var_arg->next;
            arg = arg->next;
            continue;
        }
        else if (arg->next) {
            arg_set_after(arg, var_arg);
            arg = arg->next;
            var_arg = var_arg->next;
        }
        else {
            arg->next = var_arg;
            arg = arg->next->next;
            var_arg = var_arg->next;
        }
    }

    if (!arg)
        return;
    if (arg->next)
        arg->next = NULL;
}

/* tokenizer_alias_replace
 * Replaces aliases with their aliased commands before executing
 */
void tokenizer_alias_replace(Arg* rst arg, Arena* rst scratch_arena)
{
    Str alias = alias_check(arg->val, arg->len);
    if (alias.length) {
        arg->val = arena_realloc(scratch_arena, alias.length, char, arg->val, arg->len);
        memcpy(arg->val, alias.value, alias.length);
        arg->len = alias.length;
    }
}

enum Logic_Type {
    OP_CODE,
    OP_ARG
};

union Logic_Value {
    int code;
    Arg* arg;
};

typedef struct {
    enum Logic_Type type;
    union Logic_Value val;
} Logic_Result;

Logic_Result tokenizer_logic_process(Arg* rst arg, Arena* rst scratch_arena)
{
    assert(arg);
    assert(scratch_arena);
    (void)scratch_arena;

    if (!arg->next) {
        int code = INVALID_SYNTAX("ncsh: Invalid Syntax: found 'if' at end of command.\n");
        return (Logic_Result){.type = OP_CODE, .val.code = code};
    }

    arg = arg->next;
    if (arg->op != OP_START_EXPRESSION) {
        int code = INVALID_SYNTAX("ncsh: Invalid Syntax: expecting expression start '[' after 'if'.\n");
        return (Logic_Result){.type = OP_CODE, .val.code = code};
    }

    if (!arg->next) {
        int code = INVALID_SYNTAX("ncsh: Invalid Syntax: expecting expression after 'if ['.\n");
        return (Logic_Result){.type = OP_CODE, .val.code = code};
    }
    arg = arg->next;

    char* val = arg->val;
    arg = arg->next;
    enum Ops op = arg->op;
    arg = arg->next;
    char* val2 = arg->val;
    arg = arg->next;

    bool result;
    switch (op) {
    case OP_EQ: {
        result = atoi(val) == atoi(val2);
        break;
    }
    default: {
        result = false;
        break;
    }
    }

    if (!result)
        return (Logic_Result){.type = OP_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};

    assert(arg->op == OP_END_EXPRESSION);
    arg = arg->next;
    assert(arg->op == OP_THEN);
    arg = arg->next;
    assert(arg->op == OP_CONSTANT);

    return (Logic_Result){.type = OP_ARG, .val.arg = arg};
}

int tokenizer_ops_process(Args* rst args, Tokens* rst tokens, Shell* rst shell, Arena* rst scratch_arena)
{
    assert(args && args->head);
    assert(tokens);
    assert(scratch_arena);

    Arg* arg = args->head->next;
    Arg* prev = NULL;
    tokens->is_background_job = false;
    while (arg) {
        switch (arg->op) {
        case OP_STDOUT_REDIRECTION: {
            assert(arg->next && arg->next->val);
            if (!arg->next || !arg->next->val)
                break;
            tokens->stdout_file = arg->next->val;
            tokens->stdout_redirect = arg;
            break;
        }
        case OP_STDOUT_REDIRECTION_APPEND: {
            assert(arg->next && arg->next->val);
            if (!arg->next || !arg->next->val)
                break;
            tokens->stdout_file = arg->next->val;
            tokens->stdout_redirect = arg;
            tokens->output_append = true;
            break;
        }
        case OP_STDIN_REDIRECTION: {
            assert(arg->next && arg->next->val);
            if (!arg->next || !arg->next->val)
                break;
            tokens->stdin_file = arg->next->val;
            tokens->stdin_redirect = arg;
            break;
        }
        case OP_STDERR_REDIRECTION: {
            assert(arg->next && arg->next->val);
            if (!arg->next || !arg->next->val)
                break;
            tokens->stderr_file = arg->next->val;
            tokens->stderr_redirect = arg;
            break;
        }
        case OP_STDERR_REDIRECTION_APPEND: {
            assert(arg->next && arg->next->val);
            if (!arg->next || !arg->next->val)
                break;
            tokens->stderr_file = arg->next->val;
            tokens->stderr_redirect = arg;
            tokens->output_append = true;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION: {
            assert(arg->next && arg->next->val);
            if (!arg->next || !arg->next->val)
                break;
            tokens->stdout_and_stderr_file = arg->next->val;
            tokens->stdout_and_stderr_redirect = arg;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
            assert(arg->next && arg->next->val);
            if (!arg->next || !arg->next->val)
                break;
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
            tokens->is_background_job = true;
            break;
        }
        case OP_HOME_EXPANSION: {
            tokenizer_home_expansion_process(arg, shell->config.home_location, scratch_arena);
            break;
        }
        case OP_GLOB_EXPANSION: {
            tokenizer_glob_process(arg, &args->count, scratch_arena);
            break;
        }
        case OP_ASSIGNMENT: {
            // skip command like arguments that look like assignment.
            // for example "CC=clang" is an assignment, "make CC=clang" is not.
            if (arg != args->head->next) {
                arg->op = OP_CONSTANT;
                break;
            }

            tokenizer_assignment_process(arg, &shell->vars, &shell->arena);
            if (prev && arg && arg->next) {
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
            tokenizer_variable_process(arg, &shell->vars, scratch_arena);
            break;
        }
        /*case OP_IF: {
            Logic_Result result = tokenizer_logic_process(arg, scratch_arena);
            if (result.type == OP_CODE)
                return result.val.code;

            arg = result.val.arg;
            debugf("setting arg to head %s\n", arg->val);
            arg_set_after(!prev ? args->head : prev, arg);
            break;
        }
        case OP_FI: {
            break; // just skip unless OP_IF never found
        }*/
        default: {
            tokenizer_alias_replace(arg, scratch_arena);
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
int tokenizer_tokenize(Args* rst args, Tokens* rst tokens, Shell* rst shell, Arena* rst scratch_arena)
{
    assert(args);
    assert(tokens);
    assert(scratch_arena);
    if (!args || !args->head || !args->count)
        return NCSH_COMMAND_FAILED_CONTINUE;

    int result;
    if ((result = tokenizer_syntax_check(args)) != NCSH_COMMAND_SUCCESS_CONTINUE)
        return result;

    if ((result = tokenizer_ops_process(args, tokens, shell, scratch_arena)) != NCSH_COMMAND_SUCCESS_CONTINUE)
        return result;

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}
