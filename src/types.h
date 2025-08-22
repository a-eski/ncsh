/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "arena.h"
#include "z/z.h"
#include "io/ac.h"
#include "io/history.h"

#define NCSH_MAX_PROCESSES 100

/* struct Processes
 * Maintains details about background jobs that have been started by the user.
 */
typedef struct {
    size_t job_number;
    __pid_t pids[NCSH_MAX_PROCESSES];
} Processes;

/* Env
 * Stores env variables from envp in hashtable with static size.
 */
constexpr size_t env_exp = 7;
constexpr size_t env_size = 1 << env_exp; // 128
typedef struct
{
    Str keys[env_size];
    Str vals[env_size];
} Env;

/* Config
 * Stores home location, config location, and full path to the config file.
 */
typedef struct {
    Str location;
    Str file;
} Config;

/* struct Input
 * Store information related to reading and processing user input.
 */
typedef struct {
    // values related to prompt
    bool reprint_prompt;
    size_t prompt_len;
    Str* user;

    // values related to the line buffer
    char c;
    size_t start_pos;
    size_t pos;
    size_t max_pos;
    char* buffer;
    char* yank;

    // position relative to start line of prompt
    size_t current_y;
    size_t lines_y;

    // history
    size_t history_position;
    Str history_entry;
    History history;

    // autocompletions
    size_t current_autocompletion_len;
    char* current_autocompletion;
    Autocompletion_Node* autocompletions_tree;
    Arena* scratch;
} Input;

/* struct Shell
 * Store information relevant to the shell.
 * Lives for the shell's lifetime: not freed until exit.
 * Certain parts of Args are freed in the main shell loop,
 * they live for the lifetime of the main shell loop.
 */
typedef struct Shell {
    __pid_t pgid;

    Arena arena;
    Arena scratch;

    Env* env;
    Config config;

    Input input;
    Processes pcs;

    z_Database z_db;
} Shell;
