// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_parser_h
#define ncsh_parser_h

#include "ncsh_args.h"

#include "eskilib/eskilib_result.h"

bool ncsh_args_is_valid(const struct ncsh_Args* args);

enum eskilib_Result ncsh_args_malloc(struct ncsh_Args* args);

void ncsh_args_free(struct ncsh_Args* args);

void ncsh_args_free_values(struct ncsh_Args* args);

void ncsh_parse(char line[], size_t length, struct ncsh_Args* args);

#endif // !ncsh_parser_h

