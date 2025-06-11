/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "arena.h"
#include "config.h"
#include "readline/ncreadline.h"
#include "interpreter/vm/vars.h"
#include "z/z.h"

#define NCSH_MAX_PROCESSES 100

/* struct Processes
 * Maintains details about background jobs that have been started by the user.
 */
/*typedef struct {
    size_t job_number;
    __pid_t pids[NCSH_MAX_PROCESSES];
} Processes;*/

/* struct Shell
 * Store information relevant to the shell.
 * Lives for the shell's lifetime: not freed until exit.
 * Certain parts of Args are freed in the main shell loop,
 * they live for the lifetime of the main shell loop.
 */
typedef struct {
    Arena arena;
    Arena scratch_arena;

    Config config;

    Input input;
    // Processes processes;

    z_Database z_db;

    Vars vars;
} Shell;
