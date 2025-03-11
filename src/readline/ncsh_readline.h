// Copyright (c) ncsh by Alex Eski 2025

#ifndef NCSH_READLINE_H_
#define NCSH_READLINE_H_

#include <stddef.h>
#include <stdint.h>

#include "../eskilib/eskilib_result.h"
#include "../ncsh_config.h"
#include "ncsh_history.h"
#include "ncsh_terminal.h"

#define LINE_LIMIT 100

/* struct ncsh_Input
 * Store information related to reading and processing user input.
 */
struct ncsh_Input {
    // values related to prompt
    bool reprint_prompt;
    size_t prompt_len;
    struct eskilib_String user;

    // values related to the line buffer
    size_t start_pos;
    size_t pos; // same as max_x, or all of lines_x added together
    size_t max_pos;
    char* buffer;

    // position relative to start line of prompt
    int lines_x[LINE_LIMIT];
    int current_y;
    int lines_y;
    struct ncsh_Coordinates terminal_size;

    // history
    int history_position;
    struct eskilib_String history_entry;
    struct ncsh_History history;

    // autocompletions
    size_t current_autocompletion_len;
    char* current_autocompletion;
    struct ncsh_Autocompletion_Node* autocompletions_tree;
};

/* ncsh_readline_init
 * Allocates memory that lives for the lifetime of the shell and is used by readline to process user input.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in ncsh_defines.h (EXIT_...)
 */
int_fast32_t ncsh_readline_init(struct ncsh_Config* const restrict config, struct ncsh_Input* const restrict input);

/* ncsh_readline
 * Read user input while supporting different operations like backspace, delete, history, autocompletions, home/end, and other inputs.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in ncsh_defines.h (EXIT_...)
 */
int_fast32_t ncsh_readline(struct ncsh_Input* const restrict input);

/* ncsh_readline_history_and_autocompletion_add
 * Add user input that was able to be processed and executed by the VM to readline's history and autocompletion data stores.
 */
void ncsh_readline_history_and_autocompletion_add(struct ncsh_Input* const restrict input);

/* ncsh_readline_exit
 * Releases memory at the end of the shell's lifetime related to readline functionility around processing and manipulating user input.
 */
void ncsh_readline_exit(struct ncsh_Input* const restrict input);

#endif // !NCSH_READLINE_H_
