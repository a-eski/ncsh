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
    struct Arg* stdout_redirect;
    struct Arg* stdin_redirect;
    struct Arg* stderr_redirect;
    struct Arg* stdout_and_stderr_redirect;

    char* stdout_file;
    char* stdin_file;
    char* stderr_file;
    char* stdout_and_stderr_file;

    bool output_append;
    bool is_background_job;

    uint8_t number_of_pipe_commands;
};

int vm_tokenizer_tokenize(struct Args* const restrict args, struct Tokens* const restrict tokens, struct Shell* const restrict shell,
                          struct Arena* const restrict scratch_arena);
