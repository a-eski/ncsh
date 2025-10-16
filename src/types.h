/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "arena.h"
#include "z/z.h"
#include "io/ac.h"

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

typedef struct {
    enum {
        V_EMPTY = 0,
        V_STR,
        V_NUM
    } type;
    union {
        Str s;
        Num n;
    } val;
} Var;

constexpr size_t var_exp = 7;
constexpr size_t var_size = 1 << var_exp; // 128
typedef struct
{
    Str keys[var_size];
    Var vals[var_size];
} Vars;

/* Config
 * Stores home location, config location, and full path to the config file.
 */
typedef struct {
    Str location;
    Str file;
    Str history_file;
} Config;

/* struct Input
 * Store information related to reading and processing user input.
 */
typedef struct {
    // values related to prompt
    Str* user;

    // values related to the line buffer
    size_t pos;
    char* buffer;

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
    Vars* vars;
    Config config;

    Input input;
    Processes pcs;

    z_Database z_db;
} Shell;
