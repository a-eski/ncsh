/* Copyright ncsh by Alex Eski 2025 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../config.h"
#include "../eskilib/eresult.h"
#include "history.h"
#include "terminal.h"

#define LINE_LIMIT 100

/* struct Input
 * Store information related to reading and processing user input.
 */
struct Input {
    // values related to prompt
    bool reprint_prompt;
    size_t prompt_len;
    struct estr user;

    // values related to the line buffer
    size_t start_pos;
    size_t pos; // same as max_x, or all of lines_x added together
    size_t max_pos;
    char* buffer;

    // position relative to start line of prompt
    int lines_x[LINE_LIMIT];
    int current_y;
    int lines_y;
    struct Coordinates terminal_size;

    // history
    size_t history_position;
    struct estr history_entry;
    struct History history;

    // autocompletions
    size_t current_autocompletion_len;
    char* current_autocompletion;
    struct Autocompletion_Node* autocompletions_tree;
};

/* readline_init
 * Allocates memory using the arena that lives for the lifetime of the shell and is used by readline to process user
 * input. Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in defines.h (EXIT_...)
 */
int_fast32_t ncreadline_init(struct Config* const restrict config, struct Input* const restrict input,
                             struct Arena* const arena);

/* readline
 * Read user input while supporting different operations like backspace, delete, history, autocompletions, home/end, and
 * other inputs. Accepts a pointer to the scratch arena, but it passes a copy (by value) to autocompletion logic when it
 * is needed to be used. Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in defines.h (EXIT_...)
 */
int_fast32_t ncreadline(struct Input* const restrict input, struct Arena* const scratch_arena);

/* readline_exit
 * Saves history changes and restores the terminal settings from before the shell was started.
 */
void ncreadline_exit(struct Input* const restrict input);
