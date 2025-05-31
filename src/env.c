/* Copyright ncsh (C) by Alex Eski 2025 */
/* environment.h: deal with environment variables and other things related to the environment. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdlib.h>

#include "env.h"
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
    if (!path.value)
        return Str_Empty;
    path.length = strlen(path.value) + 1;
    return path;
}
