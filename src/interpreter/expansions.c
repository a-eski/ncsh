/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <glob.h>
#include <stdio.h>

#include "lexemes.h"
#include "vars.h"
#include "../env.h"
#include "ops.h"
// #include "lexer.h"
// #include "parser.h"
// #include "statements.h"
// #include "../types.h"
// #include "../alias.h"

void expansion_home(Lexemes* rst lexemes, size_t pos, Arena* rst scratch)
{
    assert(lexemes);

   Str home = {0};
   env_home_get(&home, scratch);
   assert(home.value && home.length);
   if (home.length == 0) {
        // TODO: return error
        return;
   }
   if (lexemes->lens[pos] == 2) {
        lexemes->vals[pos] = home.value;
        lexemes->ops[pos] = OP_CONSTANT;
        lexemes->lens[pos] = home.length;
        debugf("lexemes->vals[pos] set to %s\n", lexemes->vals[pos]);
        return;
    }

    assert(lexemes->vals[pos]);
    // skip if value is null or home expansion not at beginning of value
    if (!lexemes->vals[pos] || lexemes->vals[pos][0] != '~')
        return;

    size_t len = lexemes->lens[pos] + home.length - 2; // subtract 1, both account for null termination
    char* new_value = arena_malloc(scratch, len, char);
    memcpy(new_value, home.value, home.length - 1);
    memcpy(new_value + home.length - 1, lexemes->vals[pos] + 1, lexemes->lens[pos] - 1);
    debugf("performing home expansion on %s to %s\n", lexemes->vals[pos], new_value);
    lexemes->vals[pos] = new_value;
    lexemes->ops[pos] = OP_CONSTANT;
    lexemes->lens[pos] = len;
}

[[nodiscard]]
int expansion_glob(Lexemes* rst lexemes, size_t pos, Arena* rst scratch)
{
    glob_t glob_buf = {0};
    size_t glob_len;
    glob(lexemes->vals[pos], GLOB_DOOFFS, NULL, &glob_buf);
    if (!glob_buf.gl_pathc) {
        globfree(&glob_buf);
        return 0;
    }

    size_t n = pos;
    for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
        printf("%s\n", glob_buf.gl_pathv[i]);
        if (!lexemes->vals[n]) {
            lexemes_init_n(lexemes, n, scratch);
        }

        glob_len = strlen(glob_buf.gl_pathv[i]) + 1;
        if (glob_len > lexemes->lens[n]) {
            assert(lexemes->vals[n]);
            lexemes->vals[n] = arena_realloc(scratch, glob_len, char, lexemes->vals[n], lexemes->lens[n]);
            memcpy(lexemes->vals[n], glob_buf.gl_pathv[i], glob_len);
        }
        else {
            memcpy(lexemes->vals[n], glob_buf.gl_pathv[i], glob_len);
        }
        lexemes->lens[n] = glob_len;
        lexemes->ops[n] = OP_CONSTANT;
        ++lexemes->count;
        ++n;
    }

    globfree(&glob_buf);
    return n;
}

void expansion_assignment(Lexemes* lexeme, size_t pos, Vars* rst vars, Arena* rst arena)
{
    assert(lexeme);
    assert(vars);
    assert(arena);

    // variable values are stored in vars hashmap.
    // the key is the previous value, which is tagged with OP_VARIABLE.
    // when VM comes in contact with OP_VARIABLE, it looks up value in vars.
    size_t assignment_pos;
    for (assignment_pos = 0; assignment_pos < lexeme->lens[pos]; ++assignment_pos) {
        if (lexeme->vals[pos][assignment_pos] == '=')
            break;
    }

    size_t key_len = assignment_pos + 1;
    char* var_name = arena_malloc(arena, key_len, char);
    memcpy(var_name, lexeme->vals[pos], assignment_pos);
    var_name[assignment_pos] = '\0';
    debugf("saving var_name %s\n", var_name);

    Str* val = arena_malloc(arena, 1, Str);
    val->length = lexeme->lens[pos] - assignment_pos - 1;
    val->value = arena_malloc(arena, val->length, char);
    memcpy(val->value, lexeme->vals[pos] + key_len, val->length);
    debugf("var_name: %s : %s (%zu)\n", var_name, val->value, val->length);

    char* key = arena_malloc(arena, key_len, char);
    memcpy(key, var_name, key_len);
    debugf("key: %s (%zu)\n", key, key_len);

    vars_set(key, val, arena, vars);
}

void parser_tok_update(Lexemes* rst lexeme, size_t pos, Str* rst var, Arena* rst scratch)
{
    lexeme->vals[pos] = arena_realloc(scratch, var->length, char, lexeme->vals[pos], lexeme->lens[pos]);
    memcpy(lexeme->vals[pos], var->value, var->length);
    lexeme->lens[pos] = var->length;
    lexeme->ops[pos] = OP_CONSTANT; // replace OP_VARIABLE to OP_CONSTANT so VM sees it as a regular constant value
    debugf("replaced variable with value %s %zu\n", tok->val, tok->len);
}

/*void expansion_variable(Lexemes* rst lexeme, size_t pos, Vars* rst vars, Arena* rst scratch)
{
    Str var;
    if (estrcmp(lexeme->vals[pos], lexeme->lens[pos], NCSH_PATH_VAR, sizeof(NCSH_PATH_VAR))) {

        debug("replacing variable $PATH\n");
        var = env_path_get();
        if (!var.value || !*var.value) {
            puts("ncsh: could not load path to replace $PATH variable.");
            return;
        }
        parser_tok_update(lexeme, pos, &var, scratch);
        return;
    }
    else if (estrcmp(lexeme->vals[pos], lexeme->lens[pos], NCSH_HOME_VAR, sizeof(NCSH_HOME_VAR))) {

        debug("replacing variable $HOME\n");
        env_home_get(&var, scratch);
        if (!var.value || !*var.value) {
            puts("ncsh: could not load home to replace $HOME variable.");
            return;
        }
        parser_tok_update(lexeme, pos, &var, scratch);
        return;
    }
    else {
        char* key = lexeme->vals[pos] + 1; // skip first value in tok->val (the $)
        debugf("trying to get variable %s\n", key);
        Str* val = vars_get(key, vars);
        if (!val || !val->value || !*val->value) {
            return;
        }
        var = *val;
    }

    char* space = strchr(var.value, ' ');
    if (!space) {
        parser_tok_update(lexeme, pos, &var, scratch);
        return;
    }

    debug("found space");
    Lexemes lexemes = {0};
    lexer_lex(var.value, var.length, &lexemes, scratch);
    Lexemes var_tok = lexemes;
    if (!var_tok.count) {
        return;
    }
    debugf("found value %s\n", var_tok->val);
    Str var_str = {.value = var_tok.vals[0], .length = var_tok.lens[0]};
    parser_tok_update(&lexemes, 0, &var_str, scratch);
    var_tok = *(&var_tok + 1);

    for (size_t i = 0; i < lexemes.count; ++i) {
        if (!var_tok.vals[i])
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

    if (!lexeme)
        return;
    if (lexeme->next)
        lexeme->next = NULL;
}*/

/* expansion_alias
 * Replaces aliases with their aliased commands before executing
 */
/*void expansion_alias(Token* rst tok, Arena* rst scratch)
{
    Str alias = alias_check(lexemes->vals[pos], lexemes->lens[pos]);
    if (!alias.length) {
        return;
    }

    if (alias.length > lexemes->lens[pos]) {
        lexemes->vals[pos] = arena_realloc(scratch, alias.length, char, lexemes->vals[pos], lexemes->lens[pos]);
    }

    memcpy(lexemes->vals[pos], alias.value, alias.length);
    lexemes->lens[pos] = alias.length;
}
}*/

/*[[nodiscard]]
int expansions_process(Lexemes* rst lexemes, Shell* rst shell, Arena* rst scratch)
{
    assert(lexemes);
    assert(scratch);

    Token* tok = lexemes->head->next;
    while (tok) {
        switch (tok->op) {
        case OP_HOME_EXPANSION: {
            expansion_home(tok, shell->config.home_location, scratch);
            break;
        }
        case OP_GLOB_EXPANSION: {
            expansion_glob(tok, &toks->count, scratch);
            break;
        }
        case OP_VARIABLE: {
            expansion_variable(tok, &shell->vars, scratch);
            break;
        }
        default: {
            expansion_alias(tok, scratch);
            break;
        }
        }
        if (tok)
            tok = tok->next;
    }

    return EXIT_SUCCESS;
}*/
