/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.c: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#include <assert.h>
#include <glob.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../alias.h"
#include "../defines.h"
#include "../env.h"
#include "interpreter_types.h"
#include "lexer.h"
#include "logic.h"
#include "parser.h"
#include "tokens.h"
#include "vm/vars.h"

void parser_home_expansion_process(Token* rst tok, Str home, Arena* rst scratch)
{
    if (tok->len == 2) {
        size_t len = home.length + 1;
        tok->val = arena_malloc(scratch, len, char);
        tok->op = OP_CONSTANT;
        tok->len = len;
        memcpy(tok->val, home.value, len);
        return;
    }

    assert(tok->val);
    // skip if value is null or home expansion not at beginning of value
    if (!tok->val || tok->val[0] != '~')
        return;

    size_t len = tok->len + home.length; // tok->len accounts for null termination
    char* new_value = arena_malloc(scratch, len, char);
    memcpy(new_value, home.value, home.length);
    memcpy(new_value + home.length, tok->val + 1, tok->len - 1);
    debugf("performing home expansion on %s to %s\n", tok->val, new_value);
    tok->val = new_value;
    tok->op = OP_CONSTANT;
    tok->len = len;
}

void parser_glob_process(Token* rst tok, size_t* rst args_c, Arena* rst scratch)
{
    glob_t glob_buf = {0};
    size_t glob_len;
    glob(tok->val, GLOB_DOOFFS, NULL, &glob_buf);
    if (!glob_buf.gl_pathc) {
        globfree(&glob_buf);
        return;
    }

    // handle first arg manually to preserve pointer from previous arg to current arg
    glob_len = strlen(glob_buf.gl_pathv[0]) + 1;
    if (glob_len > tok->len) {
        tok->val = arena_realloc(scratch, glob_len, char, tok->val, tok->len);
        memcpy(tok->val, glob_buf.gl_pathv[0], glob_len);
    }
    else {
        memcpy(tok->val, glob_buf.gl_pathv[0], glob_len);
    }
    tok->len = glob_len;
    tok->op = OP_CONSTANT;

    for (size_t i = 1; i < glob_buf.gl_pathc; ++i) {
        glob_len = strlen(glob_buf.gl_pathv[i]) + 1;
        if (!glob_len || glob_len >= NCSH_MAX_INPUT)
            break;

        Token* new_tok = token_alloc(OP_CONSTANT, glob_len, glob_buf.gl_pathv[i], scratch);
        if (!tok->next) {
            tok->next = new_tok;
            tok = tok->next;
        }
        else {
            token_set_after(tok->next, new_tok);
        }
        ++*args_c;
    }

    globfree(&glob_buf);
}

void parser_assignment_process(Token* tok, Vars* rst vars, Arena* rst arena)
{
    assert(tok);
    assert(vars);
    assert(arena);

    // variable values are stored in vars hashmap.
    // the key is the previous value, which is tagged with OP_VARIABLE.
    // when VM comes in contact with OP_VARIABLE, it looks up value in vars.
    size_t assignment_pos;
    for (assignment_pos = 0; assignment_pos < tok->len; ++assignment_pos) {
        if (tok->val[assignment_pos] == '=')
            break;
    }

    size_t key_len = assignment_pos + 1;
    char* var_name = arena_malloc(arena, key_len, char);
    memcpy(var_name, tok->val, assignment_pos);
    var_name[assignment_pos] = '\0';
    debugf("saving var_name %s\n", var_name);

    Str* val = arena_malloc(arena, 1, Str);
    val->length = tok->len - assignment_pos - 1;
    val->value = arena_malloc(arena, val->length, char);
    memcpy(val->value, tok->val + key_len, val->length);
    debugf("var_name: %s : %s (%zu)\n", var_name, val->value, val->length);

    char* key = arena_malloc(arena, key_len, char);
    memcpy(key, var_name, key_len);
    debugf("key: %s (%zu)\n", key, key_len);

    vars_set(key, val, arena, vars);
}

void parser_tok_update(Token* rst tok, Str* rst var, Arena* rst scratch)
{
    tok->val = arena_realloc(scratch, var->length, char, tok->val, tok->len);
    memcpy(tok->val, var->value, var->length);
    tok->len = var->length;
    tok->op = OP_CONSTANT; // replace OP_VARIABLE to OP_CONSTANT so VM sees it as a regular constant value
    debugf("replaced variable with value %s %zu\n", tok->val, tok->len);
}

void parser_variable_process(Token* rst tok, Vars* rst vars, Arena* rst scratch)
{
    Str var;
    if (estrcmp(tok->val, tok->len, NCSH_PATH_VAR, sizeof(NCSH_PATH_VAR))) {

        debug("replacing variable $PATH\n");
        var = env_path_get();
        if (!var.value || !*var.value) {
            puts("ncsh: could not load path to replace $PATH variable.");
            return;
        }
        parser_tok_update(tok, &var, scratch);
        return;
    }
    else if (estrcmp(tok->val, tok->len, NCSH_HOME_VAR, sizeof(NCSH_HOME_VAR))) {

        debug("replacing variable $HOME\n");
        env_home_get(&var, scratch);
        if (!var.value || !*var.value) {
            puts("ncsh: could not load home to replace $HOME variable.");
            return;
        }
        parser_tok_update(tok, &var, scratch);
        return;
    }
    else {
        char* key = tok->val + 1; // skip first value in tok->val (the $)
        debugf("trying to get variable %s\n", key);
        Str* val = vars_get(key, vars);
        if (!val || !val->value || !*val->value) {
            return;
        }
        var = *val;
    }

    char* space = strchr(var.value, ' ');
    if (!space) {
        parser_tok_update(tok, &var, scratch);
        return;
    }

    debug("found space");
    Tokens* toks = lexer_lex(var.value, var.length, scratch);
    Token* var_tok = toks->head->next;
    if (!var_tok) {
        return;
    }
    debugf("found value %s\n", var_tok->val);
    Str var_str = {.value = var_tok->val, .length = var_tok->len};
    parser_tok_update(tok, &var_str, scratch);
    var_tok = var_tok->next;

    for (size_t i = 0; i < toks->count; ++i) {
        if (!var_tok)
            break;

        debugf("found next value %s\n", var_tok->val);
        if (!tok) {
            tok = var_tok;
            var_tok = var_tok->next;
            tok = tok->next;
            continue;
        }
        else if (tok->next) {
            token_set_after(tok, var_tok);
            tok = tok->next;
            var_tok = var_tok->next;
        }
        else {
            tok->next = var_tok;
            tok = tok->next->next;
            var_tok = var_tok->next;
        }
    }

    if (!tok)
        return;
    if (tok->next)
        tok->next = NULL;
}

/* parser_alias_replace
 * Replaces aliases with their aliased commands before executing
 */
void parser_alias_replace(Token* rst tok, Arena* rst scratch)
{
    Str alias = alias_check(tok->val, tok->len);
    if (alias.length) {
        tok->val = arena_realloc(scratch, alias.length, char, tok->val, tok->len);
        memcpy(tok->val, alias.value, alias.length);
        tok->len = alias.length;
    }
}

int parser_expansions_process(Tokens* rst toks, Shell* rst shell, Arena* rst scratch)
{
    assert(toks && toks->head);
    assert(scratch);

    Token* tok = toks->head->next;
    while (tok) {
        switch (tok->op) {
        case OP_HOME_EXPANSION: {
            parser_home_expansion_process(tok, shell->config.home_location, scratch);
            break;
        }
        case OP_GLOB_EXPANSION: {
            parser_glob_process(tok, &toks->count, scratch);
            break;
        }
        case OP_VARIABLE: {
            parser_variable_process(tok, &shell->vars, scratch);
            break;
        }
        default: {
            parser_alias_replace(tok, scratch);
            break;
        }
        }
        if (tok)
            tok = tok->next;
    }

    return EXIT_SUCCESS;
}

#define REDIRECT(tok, str)                                                                                             \
    assert(tok->next && tok->next->val);                                                                               \
    if (!tok->next || !tok->next->val)                                                                                 \
        goto NEXT_INSTRUCTION;                                                                                         \
    str = tok->next->val;                                                                                              \
    tok = NULL;                                                                                                        \
    if (prev)                                                                                                          \
        prev->next = NULL;

int parser_ops_process(Tokens* rst toks, Shell* rst shell, Arena* rst scratch)
{
    assert(toks && toks->head);
    assert(scratch);

    Token* tok = toks->head->next;
    Token* prev = NULL;
    toks->data.is_background_job = false;
    Logic_Result result;

    static void* dispatch_table[] = {
        &&NEXT_INSTRUCTION,                           // OP_NONE = 0
        &&NEXT_INSTRUCTION,                           // OP_CONSTANT = 1
        &&PIPE_LABEL,                                 // OP_PIPE = 2
        &&STDOUT_REDIRECTION_LABEL,                   // OP_STDOUT_REDIRECTION = 3
        &&STDOUT_REDIRECTION_APPEND_LABEL,            // OP_STDOUT_REDIRECTION_APPEND = 4
        &&STDIN_REDIRECTION_LABEL,                    // OP_STDIN_REDIRECTION = 5
        &&STDIN_REDIRECTION_APPEND_LABEL,             // OP_STDIN_REDIRECTION_APPEND = 6
        &&STDERR_REDIRECTION_LABEL,                   // OP_STDERR_REDIRECTION = 7
        &&STDERR_REDIRECTION_APPEND_LABEL,            // OP_STDERR_REDIRECTION_APPEND = 8
        &&STDOUT_AND_STDERR_REDIRECTION_LABEL,        // OP_STDOUT_AND_STDERR_REDIRECTION = 9
        &&STDOUT_AND_STDERR_REDIRECTION_APPEND_LABEL, // OP_STDOUT_AND_STDERR_REDIRECTION_APPEND = 10
        &&BACKGROUND_JOB_LABEL,                       // OP_BACKGROUND_JOB = 11
        &&NEXT_INSTRUCTION,                           // OP_AND = 12
        &&NEXT_INSTRUCTION,                           // OP_OR = 13
        &&NEXT_INSTRUCTION,                           // OP_ADD = 14
        &&NEXT_INSTRUCTION,                           // OP_SUBTRACT = 15
        &&NEXT_INSTRUCTION,                           // OP_MULTIPLY = 16
        &&NEXT_INSTRUCTION,                           // OP_DIVIDE = 17
        &&NEXT_INSTRUCTION,                           // OP_MODULO = 18
        &&NEXT_INSTRUCTION,                           // OP_EXPONENTIATION = 19
        &&NEXT_INSTRUCTION,                           // OP_MATH_EXPRESSION_START = 20
        &&NEXT_INSTRUCTION,                           // OP_MATH_EXPRESSION_END = 21
        &&NEXT_INSTRUCTION,                           // OP_VARIABLE = 22
        &&ASSIGNMENT_LABEL,                           // OP_ASSIGNMENT = 23
        &&NEXT_INSTRUCTION,                           // OP_TRUE = 24
        &&NEXT_INSTRUCTION,                           // OP_FALSE = 25
        &&NEXT_INSTRUCTION,                           // OP_HOME_EXPANSION = 26
        &&NEXT_INSTRUCTION,                           // OP_GLOB_EXPANSION = 27
        &&NEXT_INSTRUCTION,                           // OP_CONDITION_START = 28
        &&NEXT_INSTRUCTION,                           // OP_CONDITION_END = 29
        &&IF_LABEL,                                   // OP_IF = 30
        &&NEXT_INSTRUCTION,                           // OP_ELSE = 31
        &&NEXT_INSTRUCTION,                           // OP_ELIF = 32
        &&NEXT_INSTRUCTION,                           // OP_THEN = 33
        &&FI_LABEL,                                   // OP_FI = 34
        &&NEXT_INSTRUCTION,                           // OP_EQUALS = 35
        &&NEXT_INSTRUCTION,                           // OP_LESS_THAN = 36
        &&NEXT_INSTRUCTION,                           // OP_GREATER_THAN = 37
    };

    while (tok) {
        goto* dispatch_table[tok->op];

    PIPE_LABEL:
        ++toks->data.number_of_pipe_commands;
        goto NEXT_INSTRUCTION;

    STDOUT_REDIRECTION_LABEL:
        REDIRECT(tok, toks->data.stdout_file);
        goto NEXT_INSTRUCTION;

    STDOUT_REDIRECTION_APPEND_LABEL:
        REDIRECT(tok, toks->data.stdout_file);
        toks->data.output_append = true;
        goto NEXT_INSTRUCTION;

    STDIN_REDIRECTION_LABEL:
        REDIRECT(tok, toks->data.stdin_file);
        goto NEXT_INSTRUCTION;

    STDIN_REDIRECTION_APPEND_LABEL:
        REDIRECT(tok, toks->data.stdin_file);
        toks->data.output_append = true;
        goto NEXT_INSTRUCTION;

    STDERR_REDIRECTION_LABEL:
        REDIRECT(tok, toks->data.stderr_file);
        goto NEXT_INSTRUCTION;

    STDERR_REDIRECTION_APPEND_LABEL:
        REDIRECT(tok, toks->data.stderr_file);
        toks->data.output_append = true;
        goto NEXT_INSTRUCTION;

    STDOUT_AND_STDERR_REDIRECTION_LABEL:
        REDIRECT(tok, toks->data.stdout_and_stderr_file);
        goto NEXT_INSTRUCTION;

    STDOUT_AND_STDERR_REDIRECTION_APPEND_LABEL:
        REDIRECT(tok, toks->data.stdout_and_stderr_file);
        toks->data.output_append = true;
        goto NEXT_INSTRUCTION;

    BACKGROUND_JOB_LABEL:
        toks->data.is_background_job = true;
        goto NEXT_INSTRUCTION;

    ASSIGNMENT_LABEL:
        // skip command line arguments that look like assignment.
        // for example "CC=clang" is an assignment, "make CC=clang" is not.
        if (tok != toks->head->next) {
            tok->op = OP_CONSTANT;
            goto NEXT_INSTRUCTION;
        }

        parser_assignment_process(tok, &shell->vars, &shell->arena);
        if (prev && tok && tok->next) {
            token_set_after(tok->next, prev);
            prev = tok->next;
            if (tok->next->next) {
                tok = tok->next->next;
                continue;
            }
        }
        goto NEXT_INSTRUCTION;

    IF_LABEL:
        result = logic_preprocess(tok, &toks->data, scratch);
        if (result.type == LT_CODE) {
            puts("ncsh: error preprocessing logic, could not process 'if' statement.");
            return result.val.code;
        }

        toks->data.logic_type = result.type;
        tok = result.val.tok;
        toks->head->next = tok;
        goto NEXT_INSTRUCTION;

    FI_LABEL:
        result = logic_preprocess(tok, &toks->data, scratch);
        if (result.type == LT_CODE) {
            puts("ncsh: error preprocessing logic, could not process 'if' statement.");
            return result.val.code;
        }

        toks->data.logic_type = result.type;
        tok = result.val.tok;
        toks->head->next = tok;
        goto NEXT_INSTRUCTION;

    NEXT_INSTRUCTION:
        prev = tok;
        if (tok)
            tok = tok->next;
    }
    ++toks->data.number_of_pipe_commands;

    return EXIT_SUCCESS;
}

/*int parser_ops_process(Tokens* rst toks, Shell* rst shell, Arena* rst scratch)
{
    assert(toks && toks->head);
    assert(scratch);

    Token* tok = toks->head->next;
    Token* prev = NULL;
    toks->data.is_background_job = false;
    while (tok) {
        switch (tok->op) {
        case OP_STDOUT_REDIRECTION: {
            assert(tok->next && tok->next->val);
            if (!tok->next || !tok->next->val)
                break;
            toks->data.stdout_file = tok->next->val;
            tok = NULL;
            if (prev)
                prev->next = NULL;
            break;
        }
        case OP_STDOUT_REDIRECTION_APPEND: {
            assert(tok->next && tok->next->val);
            if (!tok->next || !tok->next->val)
                break;
            toks->data.stdout_file = tok->next->val;
            tok = NULL;
            if (prev)
                prev->next = NULL;
            toks->data.output_append = true;
            break;
        }
        case OP_STDIN_REDIRECTION: {
            assert(tok->next && tok->next->val);
            if (!tok->next || !tok->next->val)
                break;
            toks->data.stdin_file = tok->next->val;
            tok = NULL;
            if (prev)
                prev->next = NULL;
            break;
        }
        case OP_STDERR_REDIRECTION: {
            assert(tok->next && tok->next->val);
            if (!tok->next || !tok->next->val)
                break;
            toks->data.stderr_file = tok->next->val;
            tok = NULL;
            if (prev)
                prev->next = NULL;
            break;
        }
        case OP_STDERR_REDIRECTION_APPEND: {
            assert(tok->next && tok->next->val);
            if (!tok->next || !tok->next->val)
                break;
            toks->data.stderr_file = tok->next->val;
            tok = NULL;
            if (prev)
                prev->next = NULL;
            toks->data.output_append = true;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION: {
            assert(tok->next && tok->next->val);
            if (!tok->next || !tok->next->val)
                break;
            toks->data.stdout_and_stderr_file = tok->next->val;
            tok = NULL;
            if (prev)
                prev->next = NULL;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
            assert(tok->next && tok->next->val);
            if (!tok->next || !tok->next->val)
                break;
            toks->data.stdout_and_stderr_file = tok->next->val;
            tok = NULL;
            if (prev)
                prev->next = NULL;
            toks->data.output_append = true;
            break;
        }

        case OP_PIPE: {
            ++toks->data.number_of_pipe_commands;
            break;
        }
        case OP_BACKGROUND_JOB: {
            toks->data.is_background_job = true;
            break;
        }
        case OP_ASSIGNMENT: {
            // skip command line arguments that look like assignment.
            // for example "CC=clang" is an assignment, "make CC=clang" is not.
            if (tok != toks->head->next) {
                tok->op = OP_CONSTANT;
                break;
            }

            parser_assignment_process(tok, &shell->vars, &shell->arena);
            if (prev && tok && tok->next) {
                token_set_after(tok->next, prev);
                prev = tok->next;
                if (tok->next->next) {
                    tok = tok->next->next;
                    continue;
                }
            }
            break;
        }
        case OP_IF: {
            Logic_Result result = logic_preprocess(tok, &toks->data, scratch);
            if (result.type == LT_CODE) {
                puts("ncsh: error preprocessing logic, could not process 'if' statement.");
                return result.val.code;
            }

            toks->data.logic_type = result.type;
            tok = result.val.tok;
            toks->head->next = tok;
            break;
        }
        case OP_FI: {
            // just get rid of OP_FI if we preprocessed if,
            // else set fi to constant
            if (toks->data.logic_type == LT_IF || toks->data.logic_type == LT_IF_ELSE) {
                if (!tok->next)
                    tok = NULL;
                else if (prev)
                    token_set_after(prev, tok->next);
            }
            else
                tok->op = OP_CONSTANT;

            break;
        }
        }
        prev = tok;
        if (tok)
            tok = tok->next;
    }
    ++toks->data.number_of_pipe_commands;

    return EXIT_SUCCESS;
}*/

[[nodiscard]]
int parser_parse(Tokens* rst toks, Shell* rst shell, Arena* rst scratch)
{
    assert(toks);
    assert(scratch);
    if (!toks || !toks->head || !toks->count)
        return EXIT_FAILURE_CONTINUE;

    int result;
    if ((result = parser_expansions_process(toks, shell, scratch)) != EXIT_SUCCESS)
        return result;

    if ((result = parser_ops_process(toks, shell, scratch)) != EXIT_SUCCESS)
        return result;

    return EXIT_SUCCESS;
}
