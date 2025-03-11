/* Copyright (c) ncsh by Alex Eski 2025 */

#ifndef NCSH_TYPES_H_
#define NCSH_TYPES_H_

#include <stdint.h>

#include "ncsh_config.h"
#include "ncsh_parser.h"
#include "readline/ncsh_readline.h"
#include "z/z.h"

#define NCSH_MAX_PROCESSES 100

/* struct ncsh_Processes
 * Maintains details about background jobs that have been started by the user.
 */
struct ncsh_Processes {
    uint32_t job_number;
    uint32_t process_ids[NCSH_MAX_PROCESSES];
};

/* struct ncsh_Shell
 * Store information relevant to the shell.
 * Lives for the shell's lifetime: not freed until exit.
 * Certain parts of ncsh_Args are freed in the main shell loop,
 * they live for the lifetime of the main shell loop.
 */
struct ncsh_Shell {
    struct ncsh_Config config;

    struct ncsh_Input input;
    struct ncsh_Args args;
    struct ncsh_Processes processes;

    struct z_Database z_db;
};

#endif // !NCSH_TYPES_H_
