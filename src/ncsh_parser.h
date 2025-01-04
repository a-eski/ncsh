// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_parser_h
#define ncsh_parser_h

#include "ncsh_args.h"

#include "eskilib/eskilib_result.h"
#include <stdint.h>

bool ncsh_args_is_valid(const struct ncsh_Args* args);

enum eskilib_Result ncsh_args_malloc(struct ncsh_Args* args);

enum eskilib_Result ncsh_args_malloc_count(int_fast32_t count, struct ncsh_Args* args);

void ncsh_args_free(struct ncsh_Args* args);

void ncsh_args_free_values(struct ncsh_Args* args);

void ncsh_parser_parse(const char* line, size_t length, struct ncsh_Args* args);

void ncsh_parser_parse_noninteractive(int argc, char** argv, struct ncsh_Args* args);

#endif // !ncsh_parser_h

