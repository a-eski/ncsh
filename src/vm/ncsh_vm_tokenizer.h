// Copyright (c) ncsh by Alex Eski 2025

#ifndef NCSH_VM_TOKENIZER_H_
#define NCSH_VM_TOKENIZER_H_

#include <stdint.h>

#include "../ncsh_parser.h"

struct ncsh_Tokens {
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

int_fast32_t ncsh_vm_tokenizer_tokenize(struct ncsh_Args* args, struct ncsh_Tokens* tokens);

#endif // !NCSH_VM_TOKENIZER_H_
