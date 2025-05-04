/* Copyright ncsh (C) by Alex Eski 2025 */
/* environment.h: deal with environment variables and other things related to the environment. */

#include "env.h"
#include <stdlib.h>

#include "eskilib/str.h"

void env_home_get(struct Str* output, struct Arena* restrict arena) {
    char* home = getenv(NCSH_XDG_CONFIG_HOME_VAL);
    if (!home) {
        home = getenv(NCSH_HOME_VAL);
    }
    if (!home) {
        output = NULL;
        return;
    }

    output->length = strlen(home);
    output->value = arena_malloc(arena, output->length + 1, char);
    memcpy(output->value, home, output->length + 1);
}

struct Str env_path_get() {
    struct Str path;
    path.value = getenv(NCSH_PATH_VAL);
    path.length = strlen(path.value) + 1;
    return path;
}

void env_path_add(char* restrict val, size_t len) {
    struct Str path = env_path_get();
    (void)path;
    (void)val;
    (void)len;
}
