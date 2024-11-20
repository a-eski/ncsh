// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_config_h
#define ncsh_config_h

#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"

#define NCSH "ncsh"
#define NCSH_LENGTH 5
#define NCSH_CONFIG_LENGTH 8 // length for .config

struct ncsh_Config {
	struct eskilib_String home_location;
	struct eskilib_String xdg_config_home_location;
	struct eskilib_String config_location;
};

enum eskilib_Result ncsh_config_init(struct ncsh_Config* config);

void ncsh_config_free(struct ncsh_Config* config);

#endif // !ncsh_config_h
