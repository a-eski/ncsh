/* Copyright ncsh (C) by Alex Eski 2025 */
/* environment.h: deal with environment variables and other things related to the environment. */

#include <stdio.h>
#define _POSIX_C_SOURCE 200809L
#include "env.h"
#include <assert.h>
#include <stdlib.h>

// #include "debug.h"
#include "eskilib/str.h"

void env_home_get(Str* output, Arena* rst arena)
{
    char* home = getenv(NCSH_XDG_CONFIG_HOME_VAL);
    if (!home) {
        home = getenv(NCSH_HOME_VAL);
    }
    if (!home) {
        output = NULL;
        return;
    }

    output->length = strlen(home) + 1;
    output->value = arena_malloc(arena, output->length, char);
    memcpy(output->value, home, output->length);
}

Str env_path_get()
{
    Str path;
    path.value = getenv(NCSH_PATH_VAL);
    if (!path.value) {
        puts("PATH not found.");
        return Str_Empty;
    }
    path.length = strlen(path.value) + 1;
    return path;
}

/*void env_path_append(char* rst val, size_t len, Arena* rst scratch_arena)
{
    putenv(val);
}

void env_path_save(Str path)
{
    setenv(NCSH_PATH_VAL, path.value, false);
}

void env_path_append_and_save(char* rst val, size_t len, Arena* rst scratch_arena)
{
    assert(val);
    assert(len > 0);
    if (len <= 0) {
        return;
    }

    assert(!val[len - 1]);
    debugf("trying to add %s to path\n", val);

    Str path = env_path_get();
    if (!path.value) {
        debug("could not load path value");
        return;
    }

    char* new_path = arena_malloc(scratch_arena, path.length + (size_t)len, char);
    memcpy(new_path, path.value, path.length - 1);
    new_path[path.length - 1] = ':';
    memcpy(new_path + path.length, val, (size_t)len);
    debugf("Got new path to set %s\n", new_path);
    setenv(NCSH_PATH_VAL, new_path, true);
}*/
