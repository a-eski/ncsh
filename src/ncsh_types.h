// Copyright (c) ncsh by Alex Eski 2025

#ifndef ncsh_types_h
#define ncsh_types_h

#include <stdint.h>

#include "ncsh_config.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "ncsh_terminal.h"
#include "z/z.h"

struct ncsh_Input {
    size_t start_pos;
    size_t pos;
    size_t max_pos;
    char* buffer;
};

// ncsh_Shell: store information relevant to the shell.
// Lives for the shell's lifetime: not freed until exit.
struct ncsh_Shell {
    struct ncsh_Terminal terminal;
    struct ncsh_Config config;

    struct ncsh_Input input;
    struct ncsh_Args args;

    int history_position;
    struct eskilib_String history_entry;
    struct ncsh_History history;
    char* current_autocompletion;
    struct ncsh_Autocompletion_Node* autocompletions_tree;

    struct z_Database z_db;
};

#endif // !ncsh_types_h
