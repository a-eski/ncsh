/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "arena.h"
#include "config.h"
#include "eskilib/emap.h"
#include "parser.h"
#include "readline/ncreadline.h"
#include "var.h"
#include "z/z.h"

#define NCSH_MAX_PROCESSES 100

/* struct Processes
 * Maintains details about background jobs that have been started by the user.
 */
struct Processes {
    size_t job_number;
    size_t process_ids[NCSH_MAX_PROCESSES];
};

/* struct Shell
 * Store information relevant to the shell.
 * Lives for the shell's lifetime: not freed until exit.
 * Certain parts of Args are freed in the main shell loop,
 * they live for the lifetime of the main shell loop.
 */
struct Shell {
    struct Arena arena;
    struct Arena scratch_arena;

    struct Config config;

    struct Input input;
    struct Args args;
    struct Processes processes;

    struct z_Database z_db;
};
