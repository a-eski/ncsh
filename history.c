#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "eskilib/eskilib_string.h"

static char** history;
static uint_fast32_t history_position = 0;
static const uint_fast32_t max_history_position = 50;

void ncsh_history_malloc(void) {
	history = malloc(sizeof(char*) * max_history_position);
}

void ncsh_history_add(char* line, uint_fast32_t length) {
	if (history_position < max_history_position) {
		history[history_position] = malloc(sizeof(char) * length);
		eskilib_string_copy(history[history_position++], line, length);
	}
}

char* ncsh_history_get(uint_fast32_t position) {
	if (history_position == 0 || position > history_position || position > max_history_position)
		return "";
	else if (position > history_position)
		return "";
	else if (history_position - position < 0)
		return "";
	else if (position > max_history_position)
		return history[max_history_position];
	else
		return history[history_position - position];
}

uint_fast32_t ncsh_history_command(void) {
	for (uint_fast32_t i = 0; i < history_position; i++) {
		printf("%lu %s\n", i + 1, history[i]);
	}
	return 1;
}

int main(void) {
	return EXIT_SUCCESS;
}
