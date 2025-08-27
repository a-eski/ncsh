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

    if (lexemes->strs[pos].length == 2) {
        estrset(&lexemes->strs[pos], home, scratch);
        lexemes->ops[pos] = OP_CONSTANT;
        debugf("lexemes->strs[pos].value set to %s\n", lexemes->strs[pos].value);
        return;
    }

    assert(lexemes->strs[pos].value);
    // only do home expansion when there is a value and home is in first position.
    if (!lexemes->strs[pos].value || lexemes->strs[pos].value[0] != '~') {
        return;
    }

    size_t len = lexemes->strs[pos].length + home->length - 2; // subtract 1, both account for null termination
    char* new_value = arena_malloc(scratch, len, char);
    memcpy(new_value, home->value, home->length - 1);
    memcpy(new_value + home->length - 1, lexemes->strs[pos].value + 1, lexemes->strs[pos].length - 1);
    debugf("performing home expansion on %s to %s\n", lexemes->strs[pos].value, new_value);
    lexemes->strs[pos].value = new_value;
    lexemes->ops[pos] = OP_CONSTANT;
    lexemes->strs[pos].length = len;
}

void expansion_glob(char* restrict in, Commands* restrict cmds, Arena* restrict scratch)
{
    assert(in); assert(cmds); assert(scratch);

    glob_t glob_buf = {0};
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

        estrset(&cmds->strs[cmds->pos], &Str_Get(glob_buf.gl_pathv[i]), scratch);
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
    Str* vals = estrsplit(lexeme->strs[pos], '=', &shell->arena);
    *env_add_or_get(shell->env, vals[0]) = vals[1];

    debugf("setting key %s with val %s\n", key.value, val.value);
}

void expansion_variable(Str* restrict in, Commands* restrict cmds, /*Statements* stmts,*/ Shell* restrict shell, Arena* restrict scratch)
{
    assert(in); assert(in->value); assert(cmds); assert(shell); assert(scratch);
    if (!in || in->length < 2) {
        return;
    }

    Str key;
    if (in->value[0] == '$')
        key = (Str){.value = in->value + 1, .length = in->length - 1};
    else
        key = (Str){.value = in->value, .length = in->length};

    debugf("key %s, key_len %zu\n", key.value, key.length);
    Str* val = env_add_or_get(shell->env, key);
    if (!val || !val->value) {
        return;
    }

    debugf("var %s %zu\n", val->value, val->length);

    estrset(&cmds->strs[cmds->pos], val, scratch);
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
    Str alias = alias_check(lexemes->strs[n]);
    if (!alias.length) {
        return;
    }

    char* space = strchr(alias.value, ' ');
    if (!space) {
        if (alias.length > lexemes->strs[n].length) {
            lexemes->strs[n].value = arena_realloc(scratch, alias.length, char, lexemes->strs[n].value, lexemes->strs[n].length);
        }

        memcpy(lexemes->strs[n].value, alias.value, alias.length);
        lexemes->strs[n].length = alias.length;
        return;
    }

    debug("found space in alias");
}
