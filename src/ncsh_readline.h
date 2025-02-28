#ifndef NCSH_READLINE_H_
#define NCSH_READLINE_H_

#include <stddef.h>
#include <stdint.h>

#include "eskilib/eskilib_result.h"
#include "ncsh_config.h"
#include "ncsh_history.h"
#include "ncsh_terminal.h"

#define LINE_LIMIT 100

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

int_fast32_t ncsh_readline_init(struct ncsh_Config* config, struct ncsh_Input *input);

int_fast32_t ncsh_readline(struct ncsh_Input* input);

void ncsh_readline_exit(struct ncsh_Input* input);

#endif // !NCSH_READLINE_H_
