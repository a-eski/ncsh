// Copyright (c) ncsh by Alex Eski 2025

#ifndef ncsh_types_h
#define ncsh_types_h

#include "ncsh_config.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "ncsh_terminal.h"
#include "z/z.h"
#include <stdint.h>

struct ncsh_Shell
{
    struct ncsh_Directory prompt_info;
    struct ncsh_Config config;

    struct ncsh_Args args;

    int history_position;
    struct eskilib_String history_entry;
    struct ncsh_History history;
    char *current_autocompletion;
    struct ncsh_Autocompletion_Node *autocompletions_tree;

    struct z_Database z_db;

    struct ncsh_Coordinates terminal_size;
    struct ncsh_Coordinates terminal_position;
};

#endif // !ncsh_types_h
