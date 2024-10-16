#ifndef ncsh_history_h
#define ncsh_history_h

#include <stdbool.h>
#include <stdint.h>

#include "eskilib/eskilib_string.h"

#ifndef NCSH_TEST_HISTORY
#define NCSH_HISTORY_FILE ".ncsh_history"
#else
#define NCSH_HISTORY_FILE ".ncsh_history_test"
#endif /* ifndef NCSH_TEST_HISTORY */

#define NCSH_MAX_HISTORY 500

enum ncsh_History_Result {
	H_FAILURE_FILE_OP = -3,
	H_FAILURE_MALLOC = -2,
	H_FAILURE_NULL_REFERENCE = -1,
	H_FAILURE = 0,
	H_SUCCESS = 1,
	H_HISTORY_MAX_REACHED = 2
};

struct ncsh_History {
	uint_fast32_t history_count;
	bool history_loaded;
	struct eskilib_String* entries;
};

enum ncsh_History_Result ncsh_history_malloc(struct ncsh_History* history);

enum ncsh_History_Result ncsh_history_load(struct ncsh_History* history);

enum ncsh_History_Result ncsh_history_save(struct ncsh_History* history);

void ncsh_history_free(struct ncsh_History* history);

enum ncsh_History_Result ncsh_history_add(char* line, uint_fast32_t length, struct ncsh_History* history);

struct eskilib_String ncsh_history_get(uint_fast32_t position, struct ncsh_History* history);

uint_fast32_t ncsh_history_command(struct ncsh_History* history);

#endif // !ncsh_history_h
