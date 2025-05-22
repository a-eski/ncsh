/* Copyright ncsh (C) by Alex Eski 2025 */
/* tokenizer.h: Preprocessing of parser output to ensure ready for VM to process. */

#pragma once

#include "../parser.h"
#include "../types.h"

/* Tokens
 * Stores information related to tokens from Args,
 * like position of redirect operations, counts of pipe commands, and file names to use to create file descriptors.
 * Output append directs output redirections to append to the file instead of writing over it.
 */
typedef struct {
    Arg* stdout_redirect;
    Arg* stdin_redirect;
    Arg* stderr_redirect;
    Arg* stdout_and_stderr_redirect;

    char* stdout_file;
    char* stdin_file;
    char* stderr_file;
    char* stdout_and_stderr_file;

    bool output_append;
    bool is_background_job;

    uint8_t number_of_pipe_commands;
} Tokens;

int tokenizer_tokenize(Args* rst args, Tokens* rst tokens, Shell* rst shell, Arena* rst scratch_arena);
