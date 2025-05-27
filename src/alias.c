/* Copyright ncsh (C) by Alex Eski 2025 */

#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "alias.h"
#include "arena.h"
#include "eskilib/str.h"

#define NCSH_DEFAULT_ALIASES 10
#define NCSH_MAX_ALIASES 100
size_t aliases_count;
size_t aliases_cap;
Alias* aliases;

/* alias_check
 * Checks if the input matches to any of the user defined aliases for commands.
 * Returns: the actual command as a Str, a char* value and a size_t length.
 */
Str alias_check(char* rst buffer, size_t len)
{
    if (!buffer || len < 2) {
        return Str_Empty;
    }

    if (aliases_count) {
        for (size_t i = 0; i < aliases_count; ++i) {
            if (estrcmp(buffer, len, aliases[i].alias->value, aliases[i].alias->length)) {
                return *aliases[i].actual_command;
            }
        }
    }

    return Str_Empty;
}

void alias_add(char* rst val, size_t len, Arena* rst arena)
{
    assert(val);
    assert(len > 0);
    if (!len) {
        return;
    }

    debugf("trying to add alias: %s\n", val);

    size_t i;
    for (i = 0; i < len; ++i) {
        if (val[i] == '=')
            break;
    }

    if (!i || i == len - 1) {
        fprintf(stderr, "ncsh: Could not process alias while reading rc file: %s\n", val);
        return;
    }

    if (!aliases) {
        aliases_count = 0;
        aliases = arena_malloc(arena, NCSH_DEFAULT_ALIASES, Alias);
        aliases_cap = NCSH_DEFAULT_ALIASES;
    }

    if (aliases_count == aliases_cap) {
        if (aliases_cap * 2 > NCSH_MAX_ALIASES)
            return;
        size_t new_cap = aliases_cap * 2;
        aliases = arena_realloc(arena, new_cap, Alias, aliases, aliases_cap);
        aliases_cap = new_cap;
    }

    size_t alias_len = i + 1;
    if (!alias_len)
        return;

    aliases[aliases_count].alias = arena_malloc(arena, 1, Str);

    aliases[aliases_count].alias->length = alias_len;
    aliases[aliases_count].alias->value = arena_malloc(arena, alias_len, char);
    memcpy(aliases[aliases_count].alias->value, val, alias_len - 1);
    aliases[aliases_count].alias->value[alias_len - 1] = '\0';

    size_t ac_len = len - i - 1;
    if (ac_len == 0)
        return;

    aliases[aliases_count].actual_command = arena_malloc(arena, 1, Str);

    aliases[aliases_count].actual_command->length = ac_len;
    aliases[aliases_count].actual_command->value = arena_malloc(arena, ac_len, char);
    memcpy(aliases[aliases_count].actual_command->value, val + i + 1, ac_len - 1);
    aliases[aliases_count].actual_command->value[ac_len - 1] = '\0';

    debugf("added alias %s with actual command %s\n", aliases[aliases_count].alias->value,
           aliases[aliases_count].actual_command->value);

    ++aliases_count;
}

void alias_add_new(char* rst alias, size_t a_len, char* rst command, size_t c_len, Arena* rst arena)
{
    if (!aliases) {
        aliases_count = 0;
        aliases = arena_malloc(arena, NCSH_DEFAULT_ALIASES, Alias);
        aliases_cap = NCSH_DEFAULT_ALIASES;
    }

    if (aliases_count == aliases_cap) {
        if (aliases_cap * 2 > NCSH_MAX_ALIASES)
            return;
        size_t new_cap = aliases_cap * 2;
        aliases = arena_realloc(arena, new_cap, Alias, aliases, aliases_cap);
        aliases_cap = new_cap;
    }

    aliases[aliases_count].alias = arena_malloc(arena, 1, Str);
    aliases[aliases_count].alias->length = a_len;
    aliases[aliases_count].alias->value = arena_malloc(arena, a_len, char);
    memcpy(aliases[aliases_count].alias->value, alias, a_len);

    aliases[aliases_count].actual_command = arena_malloc(arena, 1, Str);
    aliases[aliases_count].actual_command->length = c_len;
    aliases[aliases_count].actual_command->value = arena_malloc(arena, c_len, char);
    memcpy(aliases[aliases_count].actual_command->value, command, c_len);

    debugf("added alias %s with actual command %s\n", aliases[aliases_count].alias->value,
           aliases[aliases_count].actual_command->value);

    ++aliases_count;
}

void alias_remove(char* rst val, size_t len)
{
    assert(val);
    assert(len);
    debugf("removing alias %s %zu\n", val, len);
    Str str = {.value = val, .length = len};
    size_t i;
    bool found = false;
    for (i = 0; i < aliases_count; ++i) {
        if (estrcmp_s(str, *aliases[i].actual_command)) {
            found = true;
            break;
        }
    }

    if (!found)
        return;

    // if at end of array, just remove by decrementing count
    if (i + 1 == aliases_count) {
        aliases[aliases_count].alias = NULL;
        aliases[aliases_count].actual_command = NULL;
        --aliases_count;
        return;
    }

    // else have to do a memmove
    assert((ssize_t)(aliases_count - i - 1) > 0);
    size_t bytes = (aliases_count - i - 1) * sizeof(Alias);
    memmove(aliases + i, aliases + i + 1, bytes);

    aliases[aliases_count].alias = NULL;
    aliases[aliases_count].actual_command = NULL;
    --aliases_count;
}

void alias_delete()
{
    for (size_t i = 0; i < aliases_count; ++i) {
        aliases[i].alias = NULL;
        aliases[i].actual_command = NULL;
    }
    aliases_count = 0;
}

void alias_print(int fd)
{
    for (size_t i = 0; i < aliases_count; ++i) {
        dprintf(fd, "alias %s=%s\n", aliases[i].alias->value, aliases[i].actual_command->value);
    }
}
