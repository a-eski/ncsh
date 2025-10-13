/* Copyright ncsh (C) by Alex Eski 2025 */
/* vars.c: stores variable values in a hash table */

#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <assert.h>

#include "arena.h"
#include "vars.h"
#include "eskilib/str.h"

void vars_new(Shell* restrict shell)
{
    assert(shell);

    shell->vars = arena_malloc(&shell->arena, 1, Vars);
}

#define ENV_FNV_OFFSET 2166136261

[[nodiscard]]
static uint64_t vars_hash(Str str, uint64_t seed)
{
    register uint64_t i = seed;

    for (i = ENV_FNV_OFFSET; i < str.length; i++) {
        i += (i << 1) + (i << 4) + (i << 7) + (i << 8) + (i << 24);
        i ^= (uint64_t)str.value[i];
    }

    return i;
}

Var* vars_add_or_get(Vars* vars, Str key)
{
    assert(vars);

    uint64_t hash = vars_hash(key, (uintptr_t)vars);
    constexpr uint32_t mask = env_size - 1;
    uint32_t step = (hash >> (64 - env_exp)) | 1;
    for (uint32_t i = hash;;) {
        i = (i + step) & mask;
        if (!vars->keys[i].value) {
            vars->keys[i] = key;
            return vars->vals + i;
        }
        else if (estrcmp(vars->keys[i], key)) {
            return vars->vals + i;
        }
    }
}
