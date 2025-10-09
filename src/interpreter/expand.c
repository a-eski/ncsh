/* Copyright ncsh (C) by Alex Eski 2025 */

#include "expand.h"
#include <assert.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../alias.h"
#include "../debug.h"
#include "../env.h"
#include "../types.h"
#include "stmts.h"
#include "vm_types.h"

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

    for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
        debugf("%s\n", glob_buf.gl_pathv[i]);

        estrset(&cmds->strs[pos], &Str_Get(glob_buf.gl_pathv[i]), scratch);
        cmds->ops[pos++] = OP_CONST;
    }
    cmds->count += pos - 1;

    globfree(&glob_buf);
}

static Str* expand_variable(Str* restrict in, Env* restrict env, Arena* restrict scratch);

void expand_expr_variables(Commands* restrict cmds, size_t p, Env* restrict env, Arena* restrict scratch)
{
    for (size_t j = p; j < cmds->count; j++) {
        if (cmds->ops[j] == OP_MATH_EXPR_END)
            break;
        if (cmds->ops[j] == OP_CONST) {
            Str* out = expand_variable(&cmds->strs[j], env, scratch);
            if (out == NULL)
                continue;
            if (estrisnum(*out)) {
                cmds->ops[j] = OP_NUM;
            }
            cmds->strs[j] = *out;
        }
    }
}

// variable values are stored in env hashmap.
// the key is the previous value, which is tagged with OP_VARIABLE.
// when VM comes in contact with OP_VARIABLE, it looks up value in env.
void expand_assignment(Commands* restrict cmds, Shell* restrict shell)
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

void expand(Vm_Data* restrict vm, Arena* restrict scratch)
{
    Commands* cmds = vm->cmds;
    if (cmds->strs[0].value && cmds->ops[0] == OP_CONST) {
        expand_alias(&cmds->strs[0], scratch);
    }

    for (size_t i = 0; i < cmds->count; ++i) {
        switch (cmds->ops[i]) {
            case OP_HOME_EXPANSION: {
                Str* out = expand_home(&cmds->strs[i], vm->sh->env, scratch);
                cmds->ops[i] = OP_CONST;
                if (out == NULL)
                    continue;
                cmds->strs[i] = *out;
                break;
            }
            case OP_VARIABLE: {
                Str* out = expand_variable(&cmds->strs[i], vm->sh->env, scratch);
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
            case OP_MATH_EXPR_START: {
                expand_expr_variables(cmds, i, vm->sh->env, scratch);
                break;
            }
            default:
                break;
        }
    }
}
