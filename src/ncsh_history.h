// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_history_h
#define ncsh_history_h

#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"

#ifdef NCSH_HISTORY_TEST
	#define NCSH_HISTORY_FILE ".ncsh_history_test"
	#define NCSH_HISTORY_FILE_LENGTH 19
#else
	#define NCSH_HISTORY_FILE "/.ncsh_history"
	#define NCSH_HISTORY_FILE_LENGTH 15
#endif

#define NCSH_MAX_HISTORY_FILE 2000

struct ncsh_History {
	uint_fast32_t count;
	char* file;
	struct eskilib_String* entries;
};

enum eskilib_Result ncsh_history_init(struct eskilib_String config_location, struct ncsh_History* history);

void ncsh_history_exit(struct ncsh_History* history);

enum eskilib_Result ncsh_history_add(char* line, size_t length, struct ncsh_History* history);

struct eskilib_String ncsh_history_get(uint_fast32_t position, struct ncsh_History* history);

uint_fast32_t ncsh_history_command(struct ncsh_History* history);

#endif // !ncsh_history_h

