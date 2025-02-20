// Copyright (c) ncsh by Alex Eski 2025

#ifndef NCSH_TYPES_H_
#define NCSH_TYPES_H_

#include <stdint.h>

#include "ncsh_config.h"
#include "ncsh_parser.h"
#include "ncsh_readline.h"
#include "z/z.h"

#define NCSH_MAX_PROCESSES 100

struct ncsh_Processes {
    uint32_t job_number;
    uint32_t process_ids[NCSH_MAX_PROCESSES];
};

// ncsh_Shell: store information relevant to the shell.
// Lives for the shell's lifetime: not freed until exit.
struct ncsh_Shell {
    struct ncsh_Config config;

    struct ncsh_Input input;
    struct ncsh_Args args;
    struct ncsh_Processes processes;

    struct z_Database z_db;
};

#endif // !NCSH_TYPES_H_
