// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_config_h
#define ncsh_config_h

#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"

#define NCSH "ncsh"
#define DOT_CONFIG ".config"
#define NCSH_RC ".ncshrc"

struct ncsh_Config
{
    struct eskilib_String home_location;
    struct eskilib_String config_location;
    struct eskilib_String config_file;
};

enum eskilib_Result ncsh_config_init(struct ncsh_Config *config);

void ncsh_config_free(struct ncsh_Config *config);

struct eskilib_String ncsh_config_alias_check(char *buffer, size_t buf_len);

#endif // !ncsh_config_h
