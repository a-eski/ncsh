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
Str alias_check(char* rst buffer, size_t len);

/* alias_add
 * Add an alias from a line read from .ncshrc config file.
 * val is in form "cmd=command".
 */
void alias_add(char* rst val, size_t len, Arena* rst arena);

/* alias_add_new
 * Add an alias and its command value.
 */
void alias_add_new(char* rst alias, size_t a_len, char* rst command, size_t c_len, Arena* rst arena);

/* alias_remove
 * Remove an alias if it exists
 */
void alias_remove(char* rst val, size_t len);

/* alias_delete
 * Delete all aliases.
 */
void alias_delete();

/* alias_print
 * Print out all current aliases.
 */
void alias_print(int fd);
