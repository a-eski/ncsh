/* Copyright ncsh by Alex Eski 2025 */

#pragma once

#include <stdint.h>

#include "../parser.h"

/* struct Tokens
 * Stores information related to tokens from struct Args,
 * like position of redirect operations, counts of pipe commands, and file names to use to create file descriptors.
 * Output append directs output redirections to append to the file instead of writing over it.
 */
struct Tokens {
    uint_fast32_t stdout_redirect_index;
    uint_fast32_t stdin_redirect_index;
    uint_fast32_t stderr_redirect_index;
    uint_fast32_t stdout_and_stderr_redirect_index;

    uint_fast32_t number_of_pipe_commands;

    char* stdout_file;
    char* stdin_file;
    char* stderr_file;
    char* stdout_and_stderr_file;

    bool output_append;
    bool is_background_job;
};

int_fast32_t vm_tokenizer_tokenize(const struct Args* const restrict args, struct Tokens* const restrict tokens);
