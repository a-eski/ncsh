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
#include "lexemes.h"
#include "lexer.h"
#include "logic.h"
#include "parser.h"
#include "tokens.h"
#include "vars.h"

void parser_home_expansion_process(Token* rst tok, Str home, Arena* rst scratch)
{
   if (tok->lens[tok->pos] == 2) {
        size_t len = home.length + 1;
        tok->vals[tok->pos] = arena_malloc(scratch, len, char);
        tok->op = OP_CONSTANT;
        tok->lens[tok->pos] = len;
        memcpy(tok->vals[tok->pos], home.value, len);
        return;
    }

    assert(tok->vals[tok->pos]);
    // skip if value is null or home expansion not at beginning of value
    if (!tok->vals[tok->pos] || tok->vals[tok->count][0] != '~')
        return;

    size_t len = tok->lens[tok->pos] + home.length; // tok->len accounts for null termination
    char* new_value = arena_malloc(scratch, len, char);
    memcpy(new_value, home.value, home.length);
    memcpy(new_value + home.length, tok->vals[tok->pos] + 1, tok->lens[tok->count] - 1);
    debugf("performing home expansion on %s to %s\n", tok->val, new_value);
    tok->vals[tok->pos] = new_value;
    tok->op = OP_CONSTANT;
    tok->lens[tok->pos] = len;
}

void parser_glob_process(Token* rst tok, size_t* rst args_c, Arena* rst scratch)
{
    glob_t glob_buf = {0};
    size_t glob_len;
    glob(tok->vals[tok->pos], GLOB_DOOFFS, NULL, &glob_buf);
    if (!glob_buf.gl_pathc) {
        globfree(&glob_buf);
        return;
    }

    // handle first arg manually to preserve pointer from previous arg to current arg
    glob_len = strlen(glob_buf.gl_pathv[0]) + 1;
    if (glob_len > tok->lens[tok->pos]) {
        tok->vals[tok->pos] = arena_realloc(scratch, glob_len, char, tok->vals[tok->pos], tok->lens[tok->pos]);
        memcpy(tok->vals[tok->pos], glob_buf.gl_pathv[0], glob_len);
    }
    else {
        memcpy(tok->vals[tok->pos], glob_buf.gl_pathv[0], glob_len);
    }
    tok->lens[tok->pos] = glob_len;
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
    for (assignment_pos = 0; assignment_pos < tok->lens[tok->pos]; ++assignment_pos) {
        if (tok->vals[tok->pos][assignment_pos] == '=')
            break;
    }

    size_t key_len = assignment_pos + 1;
    char* var_name = arena_malloc(arena, key_len, char);
    memcpy(var_name, tok->vals[tok->pos], assignment_pos);
    var_name[assignment_pos] = '\0';
    debugf("saving var_name %s\n", var_name);

    Str* val = arena_malloc(arena, 1, Str);
    val->length = tok->lens[tok->pos] - assignment_pos - 1;
    val->value = arena_malloc(arena, val->length, char);
    memcpy(val->value, tok->vals[tok->pos] + key_len, val->length);
    debugf("var_name: %s : %s (%zu)\n", var_name, val->value, val->length);

    char* key = arena_malloc(arena, key_len, char);
    memcpy(key, var_name, key_len);
    debugf("key: %s (%zu)\n", key, key_len);

    vars_set(key, val, arena, vars);
}

void parser_tok_update(Token* rst tok, Str* rst var, Arena* rst scratch)
{
    tok->vals[tok->pos] = arena_realloc(scratch, var->length, char, tok->vals[tok->pos], tok->lens[tok->pos]);
    memcpy(tok->vals[tok->pos], var->value, var->length);
    tok->lens[tok->pos] = var->length;
    tok->op = OP_CONSTANT; // replace OP_VARIABLE to OP_CONSTANT so VM sees it as a regular constant value
    debugf("replaced variable with value %s %zu\n", tok->val, tok->len);
}

void parser_variable_process(Token* rst tok, Vars* rst vars, Arena* rst scratch)
{
    Str var;
    if (estrcmp(tok->vals[tok->pos], tok->lens[tok->pos], NCSH_PATH_VAR, sizeof(NCSH_PATH_VAR))) {

        debug("replacing variable $PATH\n");
        var = env_path_get();
        if (!var.value || !*var.value) {
            puts("ncsh: could not load path to replace $PATH variable.");
            return;
        }
        parser_tok_update(tok, &var, scratch);
        return;
    }
    else if (estrcmp(tok->vals[tok->pos], tok->lens[tok->pos], NCSH_HOME_VAR, sizeof(NCSH_HOME_VAR))) {

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
        char* key = tok->vals[tok->pos] + 1; // skip first value in tok->val (the $)
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
    Lexemes lexemes = {0};
    lexemes_init(&lexemes, scratch);
    lexer_lex(var.value, var.length, &lexemes, scratch);
    Tokens toks = {0};
    parser_parse(&lexemes, &toks, NULL, scratch);
    Token* var_tok = toks.head->next;
    if (!var_tok) {
        return;
    }
    debugf("found value %s\n", var_tok->val);
    Str var_str = {.value = var_tok->vals[0], .length = var_tok->lens[0]};
    parser_tok_update(tok, &var_str, scratch);
    var_tok = var_tok->next;

    for (size_t i = 0; i < toks.count; ++i) {
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
    Str alias = alias_check(tok->vals[tok->pos], tok->lens[tok->pos]);
    if (alias.length) {
        tok->vals[tok->pos] = arena_realloc(scratch, alias.length, char, tok->vals[tok->pos], tok->lens[tok->pos]);
        memcpy(tok->vals[tok->pos], alias.value, alias.length);
        tok->lens[tok->pos] = alias.length;
    }
}

[[nodiscard]]
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

[[nodiscard]]
int parser_ops_process(Tokens* rst toks, Shell* rst shell, Arena* rst scratch)
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
}

[[nodiscard]]
int parser_parse(Lexemes* lexemes, Tokens* rst toks, Shell* rst shell, Arena* rst scratch)
{
    (void)lexemes;
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
