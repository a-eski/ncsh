/* Copyright ncsh (C) by Alex Eski 2025 */
/* io.h: read input, handle prompts, history, and autocompletions */

#pragma once

#include <stddef.h>

#include "../conf.h"
#include "ac.h"
#include "history.h"

#define LINE_LIMIT 100

/* struct Input
 * Store information related to reading and processing user input.
 */
typedef struct {
    // values related to prompt
    bool reprint_prompt;
    size_t prompt_len;
    Str user;

    // values related to the line buffer
    char c;
    size_t start_pos;
    size_t pos;
    size_t max_pos;
    char* buffer;

    // position relative to start line of prompt
    size_t current_y;
    size_t lines_y;

    // history
    size_t history_position;
    Str history_entry;
    History history;

    // autocompletions
    size_t current_autocompletion_len;
    char* current_autocompletion;
    Autocompletion_Node* autocompletions_tree;
    Arena* scratch;
} Input;

/* io_init
 * Loads history and autocompletions.
 * input. Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in defines.h (EXIT_...)
 */
int io_init(Config* restrict config, Input* restrict input, Arena* restrict arena);

/* io_readline
 * Read user input while supporting different operations like backspace, delete, history, autocompletions, home/end, and
 * other inputs. Accepts a pointer to the scratch arena, but it passes a copy (by value) to autocompletion logic when it
 * is needed to be used. Returns: exit status( EXIT_SUCCESS, EXIT_FAILURE, or value in defines.h (EXIT_...))
 */
int io_readline(Input* restrict input, Arena* restrict scratch_arena);

/* io_deinit
 * Saves history changes
 */
void io_deinit(Input* restrict input);
