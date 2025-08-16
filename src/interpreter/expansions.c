/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>

#include "../alias.h"
#include "../debug.h"
#include "../env.h"
// #include "../ttyio/ttyio.h"
#include "lexemes.h"
#include "ops.h"
#include "statements.h"
// #include "lexer.h"
// #include "parser.h"
#include "../types.h"

void expansion_home(Shell* shell, Lexemes* restrict lexemes, size_t pos, Arena* scratch)
{
    assert(shell); assert(lexemes); assert(scratch);

    Str* home = env_home_get(shell->env);
    assert(home->value && home->length);
    if (!home || home->length == 0) {
        // TODO: return error
        return;
    }

    if (lexemes->lens[pos] == 2) {
        lexemes->lens[pos] = home->length;
        lexemes->vals[pos] = arena_malloc(scratch, lexemes->lens[pos], char);
        memcpy(lexemes->vals[pos], home->value, lexemes->lens[pos] - 1);
        lexemes->ops[pos] = OP_CONSTANT;
        debugf("lexemes->vals[pos] set to %s\n", lexemes->vals[pos]);
        return;
    }

    assert(lexemes->vals[pos]);
    // only do home expansion when there is a value and home is in first position.
    if (!lexemes->vals[pos] || lexemes->vals[pos][0] != '~')
        return;

    size_t len = lexemes->lens[pos] + home->length - 2; // subtract 1, both account for null termination
    char* new_value = arena_malloc(scratch, len, char);
    memcpy(new_value, home->value, home->length - 1);
    memcpy(new_value + home->length - 1, lexemes->vals[pos] + 1, lexemes->lens[pos] - 1);
    debugf("performing home expansion on %s to %s\n", lexemes->vals[pos], new_value);
    lexemes->vals[pos] = new_value;
    lexemes->ops[pos] = OP_CONSTANT;
    lexemes->lens[pos] = len;
}

void expansion_glob(char* restrict in, Commands* restrict cmds, Arena* restrict scratch)
{
    assert(in); assert(cmds); assert(scratch);

    glob_t glob_buf = {0};
    size_t glob_len;
    glob(in, GLOB_DOOFFS, NULL, &glob_buf);
    if (!glob_buf.gl_pathc) {
        globfree(&glob_buf);
        return;
    }

    for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
        debugf("%s\n", glob_buf.gl_pathv[i]);
        if (cmds->pos >= cmds->cap - 1) {
          debug("realloc");
          command_realloc(cmds, scratch);
        }

        glob_len = strlen(glob_buf.gl_pathv[i]) + 1;
        debugf("cmds->pos: %zu\n", cmds->pos);
        cmds->vals[cmds->pos] = arena_malloc(scratch, glob_len, char);
        memcpy(cmds->vals[cmds->pos], glob_buf.gl_pathv[i], glob_len);
        cmds->lens[cmds->pos] = glob_len;
        cmds->ops[cmds->pos] = OP_CONSTANT;
        ++cmds->pos;
    }

    globfree(&glob_buf);
}

void expansion_assignment(Lexemes* lexeme, size_t pos, Shell* restrict shell)
{
    assert(lexeme); assert(shell);

    // variable values are stored in env hashmap.
    // the key is the previous value, which is tagged with OP_VARIABLE.
    // when VM comes in contact with OP_VARIABLE, it looks up value in env.
    size_t assignment_pos;
    for (assignment_pos = 0; assignment_pos < lexeme->lens[pos]; ++assignment_pos) {
        if (lexeme->vals[pos][assignment_pos] == '=')
            break;
    }

    Str key = {.length = assignment_pos + 1};
    key.value = arena_malloc(&shell->arena, key.length, char);
    memcpy(key.value, lexeme->vals[pos], assignment_pos);

    Str val = {.length = lexeme->lens[pos] - assignment_pos - 1};
    val.value = arena_malloc(&shell->arena, val.length, char);
    memcpy(val.value, lexeme->vals[pos] + key.length, val.length);
    debugf("setting key %s with val %s\n", key.value, val.value);

    *env_add_or_get(shell->env, key) = val;
}

void expansion_variable(char* restrict in, size_t len, Commands* restrict cmds, /*Statements* stmts,*/ Shell* restrict shell, Arena* restrict scratch)
{
    assert(in); assert(cmds); assert(shell); assert(scratch);
    if (!in || len < 2) {
        return;
    }

    char* key = in[0] == '$' ? in + 1 : in; // skip first value if $
    size_t key_len = in[0] == '$' ? len - 1 : len;
    debugf("key %s, key_len %zu\n", key, key_len);
    Str* val = env_add_or_get(shell->env, Str_New(key, key_len));
    if (!val || !val->value) {
        return;
    }

    debugf("var %s %zu\n", val->value, val->length);

    cmds->vals[cmds->pos] = arena_malloc(scratch, val->length, char);
    memcpy(cmds->vals[cmds->pos], val->value, val->length - 1);
    cmds->lens[cmds->pos] = val->length;
    cmds->ops[cmds->pos] = OP_CONSTANT;
    ++cmds->pos;

    // TODO: improve expansion to separate values when not in quotes, expand variables which contain a space
    /*char* space = strchr(var.value, ' ');
    if (!space) {
        cmds->vals[cmds->pos] = arena_malloc(scratch, var.length, char);
        memcpy(cmds->vals[cmds->pos], var.value, var.length - 1);
        cmds->lens[cmds->pos] = var.length;
        cmds->ops[cmds->pos] = OP_CONSTANT;
        ++cmds->pos;
        return;
    }

    debug("found space");
    Lexemes lexemes = {0};
    lexer_lex(var.value, var.length, &lexemes, scratch);
    if (!lexemes.count) {
        return;
    }
    int res = parser_parse(&lexemes, stmts, shell, scratch);
    if (res != EXIT_SUCCESS) {
        return;
    }*/
    // cmds->pos = stmts->statements[stmts->pos].commands->count;
}

/* expansion_alias
 * Replaces aliases with their aliased commands before executing
 */
void expansion_alias(Lexemes* restrict lexemes, size_t n, Arena* restrict scratch)
{
    // TODO: alias expansion. Aliases with " " are not expanded into multiple commands.
    Str alias = alias_check(lexemes->vals[n], lexemes->lens[n]);
    if (!alias.length) {
        return;
    }

    char* space = strchr(alias.value, ' ');
    if (!space) {
        if (alias.length > lexemes->lens[n]) {
            lexemes->vals[n] = arena_realloc(scratch, alias.length, char, lexemes->vals[n], lexemes->lens[n]);
        }

        memcpy(lexemes->vals[n], alias.value, alias.length);
        lexemes->lens[n] = alias.length;
        return;
    }

    debug("found space in alias");
}
