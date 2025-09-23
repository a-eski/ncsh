/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>

#include "../alias.h"
#include "../debug.h"
#include "../env.h"
#include "ops.h"
#include "stmts.h"
#include "../types.h"

static Str* expand_home(Str* restrict s, Env* restrict env, Arena* restrict scratch)
{
    Str* home = env_home_get(env);
    assert(home->value && home->length);
    if (!home || home->length == 0) {
        return NULL;
    }

    if (s->length == 2) {
        return estrdup(home, scratch);
    }

    assert(s->value);
    // only do home expansion when there is a value and home is in first position.
    if (!s->value || s->value[0] != '~') {
        return NULL;
    }

    Str tildeless = {.value = s->value + 1, .length = s->length - 1};
    return estrcat(home, &tildeless, scratch);
}

// static void expansion_glob(char* restrict in, Commands* restrict cmds, Arena* restrict scratch)
// {
//     assert(in); assert(cmds); assert(scratch);
//
//     glob_t glob_buf = {0};
//     glob(in, GLOB_DOOFFS, NULL, &glob_buf);
//     if (!glob_buf.gl_pathc) {
//         globfree(&glob_buf);
//         return;
//     }
//
//     for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
//         debugf("%s\n", glob_buf.gl_pathv[i]);
//         if (cmds->pos >= cmds->cap - 1) {
//           debug("realloc");
//           cmd_realloc(cmds, scratch);
//         }
//
//         estrset(&cmds->strs[cmds->pos], &Str_Get(glob_buf.gl_pathv[i]), scratch);
//         cmds->ops[cmds->pos] = OP_CONST;
//         ++cmds->pos;
//     }
//
//     globfree(&glob_buf);
// }

static void expand_glob(Commands* restrict cmds, size_t pos, Arena* restrict scratch)
{
    assert(cmds); assert(scratch); assert(pos < cmds->count && pos < cmds->cap);

    glob_t glob_buf = {0};
    glob(cmds->strs[pos].value, GLOB_DOOFFS, NULL, &glob_buf);
    if (!glob_buf.gl_pathc) {
        globfree(&glob_buf);
        return;
    }

    if (cmds->count + glob_buf.gl_pathc > cmds->cap) {
        cmd_realloc_exact(cmds, scratch, cmds->count + glob_buf.gl_pathc);
    }

    memmove(cmds->strs + pos + glob_buf.gl_pathc - 1, cmds->strs + pos, glob_buf.gl_pathc - 1);
    memmove(cmds->ops + pos + glob_buf.gl_pathc - 1, cmds->ops + pos, glob_buf.gl_pathc - 1);

    for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
        debugf("%s\n", glob_buf.gl_pathv[i]);

        estrset(&cmds->strs[pos], &Str_Get(glob_buf.gl_pathv[i]), scratch);
        cmds->ops[pos++] = OP_CONST;
    }

    globfree(&glob_buf);
}

// variable values are stored in env hashmap.
// the key is the previous value, which is tagged with OP_VARIABLE.
// when VM comes in contact with OP_VARIABLE, it looks up value in env.
static void expand_assignment(Commands* restrict cmds, Shell* restrict shell)
{
    assert(cmds); assert(shell && shell->env); assert(cmds->op == OP_ASSIGNMENT);

    *env_add_or_get(shell->env, *estrdup(&cmds->strs[0], &shell->arena)) = *estrdup(&cmds->strs[2], &shell->arena);
}

static Str* expand_variable(Str* restrict in, Env* restrict env, Arena* restrict scratch)
{
    assert(in); assert(in->value);
    if (!in || in->length < 2) {
        return NULL;
    }

    Str key;
    if (in->value[0] == '$')
        key = (Str){.value = in->value + 1, .length = in->length - 1};
    else
        key = (Str){.value = in->value, .length = in->length};

    Str* val = env_add_or_get(env, key);
    if (!val || !val->value) {
        return NULL;
    }

    return estrdup(val, scratch);

    // TODO: improve expansion to separate values when not in quotes, expand variables which contain a space
    /*char* space = strchr(var.value, ' ');
    if (!space) {
        cmds->vals[cmds->pos] = arena_malloc(scratch, var.length, char);
        memcpy(cmds->vals[cmds->pos], var.value, var.length - 1);
        cmds->lens[cmds->pos] = var.length;
        cmds->ops[cmds->pos] = OP_CONST;
        ++cmds->pos;
        return;
    }

    debug("found space");
    Lexemes lexemes = {0};
    lex(var.value, var.length, &lexemes, scratch);
    if (!lexemes.count) {
        return;
    }
    int res = parse(&lexemes, stmts, shell, scratch);
    if (res != EXIT_SUCCESS) {
        return;
    }*/
    // cmds->pos = stmts->statements[stmts->pos].commands->count;
}

static void expand_alias(Str* restrict s, Arena* restrict scratch)
{
    Str alias = alias_check(*s);
    if (!alias.length) {
        return;
    }

    char* space = strchr(alias.value, ' ');
    if (!space) {
        if (alias.length > s->length) {
            s->value = arena_realloc(scratch, alias.length, char, s->value, s->length);
        }

        memcpy(s->value, alias.value, alias.length);
        s->length = alias.length;
        return;
    }

    // TODO: alias expansion. Aliases with " " are not expanded into multiple commands.
    debug("found space in alias");
}

static void expand_cmds(Commands* restrict cmds, Env* restrict env, Arena* restrict scratch)
{
    for (size_t i = 0; i < cmds->count; ++i) {
        switch (cmds->ops[i]) {
            case OP_HOME_EXPANSION: {
                Str* out = expand_home(&cmds->strs[i], env, scratch);
                cmds->ops[i] = OP_CONST;
                if (out == NULL)
                    continue;
                cmds->strs[i] = *out;
                break;
            }
            case OP_VARIABLE: {
                Str* out = expand_variable(&cmds->strs[i], env, scratch);
                cmds->ops[i] = OP_CONST;
                if (out == NULL)
                    continue;
                cmds->strs[i] = *out;
                break;
            }
            case OP_GLOB_EXPANSION: {
                expand_glob(cmds, i, scratch);
                break;
            }
            default:
                break;
        }
    }

    if (cmds->next)
        expand_cmds(cmds->next, env, scratch);
}

/* recursively expand each statement and its commands */
static void expand_stmt(Statement* restrict stmt, Shell* restrict shell, Arena* restrict scratch)
{
    if (!stmt || !stmt->commands || !stmt->commands->strs[0].value)
        return;

    if (stmt->commands->strs[0].value && stmt->commands->ops[0] == OP_CONST) {
        expand_alias(&stmt->commands->strs[0], scratch);
    }

    while (stmt->commands && stmt->commands->op == OP_ASSIGNMENT) {
        expand_assignment(stmt->commands, shell);
        if (stmt->commands->next) {// remove assignment so it doesn't get processed by the VM.
            stmt->commands = stmt->commands->next;
        } else {
            stmt->commands = NULL;
            break;
        }
    }

    if (!stmt->commands)
        return;

    expand_cmds(stmt->commands, shell->env, scratch);

    if (stmt->right) {
        expand_stmt(stmt->right, shell, scratch);
    }

    if (stmt->left) {
        expand_stmt(stmt->left, shell, scratch);
    }
}

void expand(Statements* restrict stmts, Shell* restrict shell, Arena* restrict scratch)
{
    if (!stmts->head || !stmts->head->commands)
        return;

    Statement* stmt = stmts->head;
    expand_stmt(stmt, shell, scratch);
}
