/* Copyright ncsh (C) by Alex Eski 2025 */

#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <assert.h>
#include <string.h>

#include "alias.h"
#include "arena.h"
#include "debug.h"
#include "eskilib/str.h"
#include "ttyio/ttyio.h"

#define NCSH_DEFAULT_ALIASES 10
#define NCSH_MAX_ALIASES 100
static size_t aliases_count;
static size_t aliases_cap;
static Alias* aliases;

/* alias_check
 * Checks if the input matches to any of the user defined aliases for commands.
 * Returns: the actual command as a Str, a char* value and a size_t length.
 */
[[nodiscard]]
Str alias_check(Str alias)
{
    if (!alias.value || alias.length < 2 || !aliases_count) {
        return Str_Empty;
    }

    for (size_t i = 0; i < aliases_count; ++i) {
        if (estrcmp(*aliases[i].alias, alias)) {
            return *aliases[i].actual_command;
        }
    }

    return Str_Empty;
}

void alias_add(Str alias, Arena* restrict arena)
{
    assert(alias.value); assert(alias.length > 0); assert(*alias.value); assert(strlen(alias.value) + 1 == alias.length);

    if (!alias.length) {
        return;
    }

    debugf("trying to add alias: %s\n", val);

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

    size_t split_pos = estridx(&alias, '=');

    if (!split_pos || split_pos == alias.length - 1) {
        tty_fprintln(stderr, "ncsh: Could not process alias while reading rc file: %s", alias.value);
        return;
    }

    size_t alias_len = split_pos + 1;
    if (!alias_len) {
        return;
    }
    aliases[aliases_count].alias = estrdup(&Str_New(alias.value, alias_len), arena);

    size_t ac_len = alias.length - split_pos - 1;
    if (ac_len == 0) {
        return;
    }
    aliases[aliases_count].actual_command = estrdup(&Str_New(alias.value + split_pos + 1, ac_len), arena);

    debugf("added alias %s with actual command %s\n", aliases[aliases_count].alias->value,
           aliases[aliases_count].actual_command->value);
    ++aliases_count;
}

void alias_add_new(Str alias, Str command, Arena* restrict arena)
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

    aliases[aliases_count].alias = estrdup(&alias, arena);
    aliases[aliases_count].actual_command = estrdup(&command, arena);

    debugf("added alias %s with actual command %s\n", aliases[aliases_count].alias->value,
            aliases[aliases_count].actual_command->value);

    ++aliases_count;
}

void alias_remove(Str alias)
{
    debugf("removing alias %s %zu\n", val, len);
    size_t i;
    bool found = false;
    for (i = 0; i < aliases_count; ++i) {
        if (estrcmp(alias, *aliases[i].actual_command)) {
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
    assert((ssize_t)((ssize_t)aliases_count - (ssize_t)i - 1) > 0);
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
        tty_dprintln(fd, "alias %s=%s", aliases[i].alias->value, aliases[i].actual_command->value);
    }
}
