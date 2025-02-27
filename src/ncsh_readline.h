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
    bool reprint_prompt; // prompt
    size_t prompt_len;
    struct eskilib_String user;

    size_t start_pos; // buffer
    size_t pos; // same as max_x, or all of lines_x
    size_t max_pos;
    char* buffer;

    int lines_x[LINE_LIMIT]; //terminal pos
    int current_y;
    int lines_y;
    struct ncsh_Coordinates terminal_size;

    int history_position; // history & autocomplete
    struct eskilib_String history_entry;
    struct ncsh_History history;
    char* current_autocompletion;
    struct ncsh_Autocompletion_Node* autocompletions_tree;
};

int_fast32_t ncsh_readline_init(struct ncsh_Config* config, struct ncsh_Input *input);

int_fast32_t ncsh_readline(struct ncsh_Input* input);

void ncsh_readline_exit(struct ncsh_Input* input);

#endif // !NCSH_READLINE_H_
