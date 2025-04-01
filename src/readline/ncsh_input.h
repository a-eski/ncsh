/* Copyright ncsh by Alex Eski 2025 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../eskilib/eskilib_result.h"
#include "../ncsh_config.h"
#include "ncsh_history.h"
#include "ncsh_terminal.h"

#define LINE_LIMIT 100

/*enum ncsh_PromptDirectoryStyle: uint_fast8_t {
    NORMAL = 0,
    SHORT = 1,
    NONE = 2
};*/

/* struct ncsh_Input
 * Store information related to reading and processing user input.
 */
struct ncsh_Input {
    struct ncsh_String user;
    // bool prompt_show_user;
    // enum ncsh_PromptDirectoryStyle prompt_directory_style;
    // char* prompt_ending_str; // i.e. "$", " > ",  "> ", etc.

    size_t pos;
    char* buffer;

    // history
    size_t history_position;
    struct ncsh_String history_entry;
    struct ncsh_History history;

    // autocompletions
    size_t current_autocompletion_len;
    char* current_autocompletion;
    struct ncsh_Autocompletion_Node* autocompletions_tree;
};

/* ncsh_input_init
 * Allocates memory using the arena that lives for the lifetime of the shell and is used by readline to process user input.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in ncsh_defines.h (EXIT_...)
 */
int_fast32_t ncsh_input_init(struct ncsh_Config* const restrict config,
                                struct ncsh_Input* const restrict input,
                                struct ncsh_Arena* const arena);

/* ncsh_input
 * Read user input while supporting different operations like backspace, delete, history, autocompletions, home/end, and other inputs.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in ncsh_defines.h (EXIT_...)
 */
int_fast32_t ncsh_input(struct ncsh_Input* const restrict input,
                           struct ncsh_Arena scratch_arena);
