/* Copyright ncsh (C) by Alex Eski 2025 */

#include "alias.h"
#include "arena.h"
#include <assert.h>
#include <stdio.h>

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
        aliases = arena_malloc(arena, 10, Alias);
        aliases_cap = 10;
    }

    if (aliases_count == aliases_cap) {
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
