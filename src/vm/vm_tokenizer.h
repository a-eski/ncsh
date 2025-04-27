/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "../parser.h"
#include "../types.h"

/* struct Tokens
 * Stores information related to tokens from struct Args,
 * like position of redirect operations, counts of pipe commands, and file names to use to create file descriptors.
 * Output append directs output redirections to append to the file instead of writing over it.
 */
struct Tokens {
    uint8_t stdout_redirect_index;
    uint8_t stdin_redirect_index;
    uint8_t stderr_redirect_index;
    uint8_t stdout_and_stderr_redirect_index;

    uint8_t number_of_pipe_commands;

    char* stdout_file;
    char* stdin_file;
    char* stderr_file;
    char* stdout_and_stderr_file;

    bool output_append;
    bool is_background_job;
};

int vm_tokenizer_tokenize(struct Shell* const restrict shell, struct Tokens* const restrict tokens,
                          struct Arena* const restrict scratch_arena);
