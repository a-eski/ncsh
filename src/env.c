/* Copyright ncsh (C) by Alex Eski 2025 */
/* environment.h: deal with environment variables and other things related to the environment. */

#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <assert.h>

#include "arena.h"
#include "env.h"
#include "eskilib/str.h"

Str* env_add_or_get(Env* env, Str key);

void env_flat_to_hmap(Env* env, char** envp, Arena* restrict arena)
{
    assert(env);
    assert(envp);
    assert(arena);

    while (*envp) {
        Str* strs = estrsplit(Str_Get(*envp), '=', arena);
        *env_add_or_get(env, strs[0]) = strs[1];
        ++envp;
    }
}

void env_new(Shell* restrict shell, char** envp, Arena* restrict arena)
{
    assert(envp);
    assert(arena);

    shell->env = arena_malloc(arena, 1, Env);
    env_flat_to_hmap(shell->env, envp, arena);
}

#define ENV_FNV_OFFSET 2166136261

[[nodiscard]]
uint64_t env_hash(Str str, uint64_t seed)
{
    register uint64_t i = seed;

    for (i = ENV_FNV_OFFSET; i < str.length; i++) {
        i += (i << 1) + (i << 4) + (i << 7) + (i << 8) + (i << 24);
        i ^= (uint64_t)str.value[i];
    }

    return i;
}

Str* env_add_or_get(Env* env, Str key)
{
    assert(env);

    uint64_t hash = env_hash(key, (uintptr_t)env);
    constexpr uint32_t mask = env_size - 1;
    uint32_t step = (hash >> (64 - env_exp)) | 1;
    for (uint32_t i = hash;;) {
        i = (i + step) & mask;
        if (!env->keys[i].value) {
            env->keys[i] = key;
            return env->vals + i;
        }
        else if (estrcmp_s(env->keys[i], key)) {
            return env->vals + i;
        }
    }
}

Str* env_home_get(Env* env)
{
    assert(env);

    Str xdg_config_home_key = Str_New_Literal(NCSH_XDG_CONFIG_HOME_VAL);
    Str home_key = Str_New_Literal(NCSH_HOME_VAL);

    Str* home = env_add_or_get(env, xdg_config_home_key);
    // printf("xdg config? %s\n", home->value);
    if (!home || !home->value) {
        home = env_add_or_get(env, home_key);
        // printf("home? %s\n", home->value);
    }

    return home;
}
