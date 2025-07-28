/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>

#include "../alias.h"
#include "../debug.h"
#include "../env.h"
#include "../ttyio/ttyio.h"
#include "lexemes.h"
#include "ops.h"
#include "statements.h"
#include "vars.h"
// #include "lexer.h"
// #include "parser.h"
#include "../shell.h"

void expansion_home(Lexemes* restrict lexemes, size_t pos, Arena* restrict scratch)
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

void expansion_glob(char* restrict in, Commands* restrict cmds, Arena* restrict scratch)
{
    assert(in);
    assert(cmds);
    assert(scratch);

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

void expansion_assignment(Lexemes* lexeme, size_t pos, Vars* restrict vars, Arena* restrict arena)
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
    debugf("set key %s with val %s\n", key, val->value);
}

void expansion_variable(char* restrict in, size_t len, Commands* restrict cmds, /*Statements* stmts,*/ Shell* restrict shell, Arena* restrict scratch)
{
    Str var;
    // TODO: store a hashtable of environment vars that we can do lookups on instead of hardcoding each env val.
    if (estrcmp(in, len, NCSH_PATH_VAR, sizeof(NCSH_PATH_VAR))) {
        debug("replacing variable $PATH\n");
        var = env_path_get();
        if (!var.value || !*var.value) {
            tty_puts("ncsh: could not load path to replace $PATH variable.");
            return;
        }

        cmds->vals[cmds->pos] = arena_malloc(scratch, var.length, char);
        memcpy(cmds->vals[cmds->pos], var.value, var.length - 1);
        cmds->lens[cmds->pos] = var.length;
        cmds->ops[cmds->pos] = OP_CONSTANT;
        ++cmds->pos;
        return;
    }
    else if (estrcmp(in, len, NCSH_HOME_VAR, sizeof(NCSH_HOME_VAR))) {
        debug("replacing variable $HOME\n");
        env_home_get(&var, scratch);
        if (!var.value || !*var.value) {
            tty_puts("ncsh: could not load home to replace $HOME variable.");
            return;
        }
        cmds->vals[cmds->pos] = arena_malloc(scratch, var.length, char);
        memcpy(cmds->vals[cmds->pos], var.value, var.length - 1);
        cmds->lens[cmds->pos] = var.length;
        cmds->ops[cmds->pos] = OP_CONSTANT;
        ++cmds->pos;
        return;
    }
    else {
        char* key = in + 1; // skip first value in tok->val (the $)
        debugf("trying to get variable %s\n", key);
        Str* val = vars_get(key, &shell->vars);
        if (!val || !val->value || !*val->value) {
            return;
        }
        var = *val;

        // TODO: improve expansion to separate values when not in quotes
        debugf("cmds->pos %zu\n", cmds->pos);
        cmds->vals[cmds->pos] = arena_malloc(scratch, var.length, char);
        memcpy(cmds->vals[cmds->pos], var.value, var.length - 1);
        cmds->lens[cmds->pos] = var.length;
        cmds->ops[cmds->pos] = OP_CONSTANT;
        ++cmds->pos;
    }

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
    // TODO: alias expansion. Aliases with " " are not expaneded into multiple commands.
    Str alias = alias_check(lexemes->vals[n], lexemes->lens[n]);
    if (!alias.length) {
        return;
    }

    if (alias.length > lexemes->lens[n]) {
        lexemes->vals[n] = arena_realloc(scratch, alias.length, char, lexemes->vals[n], lexemes->lens[n]);
    }

    memcpy(lexemes->vals[n], alias.value, alias.length);
    lexemes->lens[n] = alias.length;
}
