/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "arena.h"
#include "eskilib/str.h"

typedef struct {
    Str* alias;
    Str* actual_command;
} Alias;

/* alias_check
 * Return the actual command for input if it is an alias.
 */
Str alias_check(Str alias);

/* alias_add
 * Add an alias from a line read from .ncshrc config file.
 * val is in form "cmd=command".
 */
void alias_add(Str alias, Arena* restrict arena);

/* alias_add_new
 * Add an alias and its command value.
 */
void alias_add_new(Str alias, Str command, Arena* restrict arena);

/* alias_remove
 * Remove an alias if it exists
 */
void alias_remove(Str alias);

/* alias_delete
 * Delete all aliases.
 */
void alias_delete();

/* alias_print
 * Print out all current aliases.
 */
void alias_print(int fd);
