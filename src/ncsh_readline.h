#ifndef NCSH_READLINE_H_
#define NCSH_READLINE_H_

#include <stddef.h>
#include <stdint.h>

#include "eskilib/eskilib_result.h"
#include "ncsh_history.h"
#include "ncsh_terminal.h"

struct ncsh_Input {
    size_t start_pos;
    size_t pos;
    size_t max_pos;
    char* buffer;

    struct ncsh_Terminal terminal;

    int history_position;
    struct eskilib_String history_entry;
    struct ncsh_History history;
    char* current_autocompletion;
    struct ncsh_Autocompletion_Node* autocompletions_tree;
};

int_fast32_t ncsh_readline_init(struct ncsh_Config* config, struct ncsh_Input *input);

int_fast32_t ncsh_readline(struct ncsh_Input* input);

void ncsh_readline_exit(struct ncsh_Input* input);

#endif // !NCSH_READLINE_H_
