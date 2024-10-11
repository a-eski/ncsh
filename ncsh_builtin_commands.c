/* Copyright Alex Eski 2024 */

#include <stddef.h>
#include <string.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "ncsh_args.h"
#include "eskilib/eskilib_string.h"
#include "ncsh_builtin_commands.h"

bool ncsh_is_exit_command(struct ncsh_Args args) {
	if (eskilib_string_equals(args.values[0], "q", args.max_line_length))
		return true;
	else if (eskilib_string_equals(args.values[0], "exit", args.max_line_length))
		return true;
	else if (eskilib_string_equals(args.values[0], "quit", args.max_line_length))
		return true;
	else
		return false;
}

uint_fast32_t ncsh_echo_command(struct ncsh_Args args) {
	for (uint_fast32_t i = 1; i < args.count; i++)
		printf("%s ", args.values[i]);

	if (args.count > 0)
		putchar('\n');

	return 1;
}

uint_fast32_t ncsh_help_command(void) {
	puts("ncsh by Alex Eski: help\n");

	puts("Builtin Commands {command} {args}");
	puts("q:		To exit, type q, exit, or quit and press enter. You can also use Ctrl+D to exit.");
	puts("cd/z:		You can change directory with cd or z.");
	puts("echo:		You can write things to the screen using echo.");
	puts("history:	You can see your command history using the history command.");

	return 1;
}

bool ncsh_is_cd_command(struct ncsh_Args args) {
	if (eskilib_string_equals(args.values[0], "cd", args.max_line_length))
		return true;
	if (eskilib_string_equals(args.values[0], "z", args.max_line_length))
		return true;

	return false;
}

uint_fast32_t ncsh_cd_command(struct ncsh_Args args) {
	if (args.values[1] == NULL) {
		char* home = getenv("HOME");
		if (home == NULL || chdir(home) != 0)
			fputs("ncsh: could not change directory.\n", stderr);

		return 1;
	}

	if (chdir(args.values[1]) != 0)
		fputs("ncsh: could not change directory.\n", stderr);

	return 1;
}

static struct eskilib_String* history;
static uint_fast32_t history_position = 0;
static const uint_fast32_t max_history_position = 50;
static bool history_loaded = false;

void ncsh_history_malloc(void) {
	history_position = 0;
	history_loaded = false;
	history = malloc(sizeof(char*) * max_history_position);
}

void ncsh_history_save(void) {
	if (history == NULL || !history[0].value)
		return;

	FILE* file = fopen(NCSH_HISTORY_FILE, "a");
	if (file == NULL) {
		fputs("Could not create or open .ncsh_history file.\n", stderr);
		return;
	}

	for (uint_fast32_t i = 0; i < history_position; i ++) {
		if (!fputs(history[i].value, file)) {
			perror("Error writing to file");
			break;
		}
		if (!fputc('\n', file)) {
			perror("Error writing to file");
			break;
		}
	}

	fclose(file);
}

int_fast32_t ncsh_fgets(char* s, int n, FILE* file_pointer) {
	register int c;
	register char* cs;
	cs = s;
	int_fast32_t cs_read = 0;

	while(--n > 0 && (c = getc(file_pointer)) != EOF) {
		++cs_read;
		// put the input char into the current pointer position, then increment it
		// if a newline entered, break
		if((*cs++ = c) == '\n')
			break;
	}

	*cs = '\0';
	return (c == EOF && cs == s) ? cs_read : EOF;
}

void ncsh_history_load() {
	FILE* file = fopen(NCSH_HISTORY_FILE, "r");
	if (file == NULL) {
		file = fopen(NCSH_HISTORY_FILE, "w");
		if (file == NULL)
		{
			perror("Could not load or create history file.");
		}
		return;
	}

	// char buffer[MAX_INPUT];
	// int_fast32_t buffer_length = 0;

	// for (uint_fast8_t i = 0;
	// 	(buffer_length = ncsh_fgets(buffer, sizeof(buffer), file)) != EOF && i < max_history_position;
	// 	i++) {
	// 	if (buffer_length > 0) {
	// 		printf("loaded %s", buffer);
	// 		fflush(stdout);
	// 		history[i].value = malloc(sizeof(char) * buffer_length + 1);
	// 		eskilib_string_copy(history[i].value, buffer, buffer_length + 1);
	// 	}
	// }

	// char buffer[MAX_INPUT];
	// for (uint_fast8_t i = 0; (fgets(buffer, sizeof(buffer), file)) != NULL && i < max_history_position; i++) {
	// 	int buffer_length = strlen(buffer);
	//
	// 	if (buffer_length > 0) {
	// 		printf("loaded %s", buffer);
	// 		// history[i].value = malloc(sizeof(char) * buffer_length);
	// 		// eskilib_string_copy(history[i].value, buffer, buffer_length);
	// 		// ++history_position;
	// 	}
	// }

	fclose(file);

	history_loaded = true;
}

void ncsh_history_free(void) {
	for (uint_fast32_t i = 0; i < history_position; i++) {
		free(history[i].value);
		history[i].value = NULL;
	}

	free(history);
	history = NULL;
	history_position = 0;
}

void ncsh_history_add(char* line, uint_fast32_t length) {
	if (history != NULL && history_position < max_history_position) {
		history[history_position].length = length;
		history[history_position].value = malloc(sizeof(char) * length);
		eskilib_string_copy(history[history_position].value, line, length);
		++history_position;
	}
}

struct eskilib_String ncsh_history_get(uint_fast32_t position) {
	const struct eskilib_String empty_string = { .length = 0, .value = NULL };

	if (!history_loaded)
		return empty_string;
	else if (history_position == 0 && position == 0)
		return empty_string;
	else if (position >= history_position)
		return empty_string;
	else if (position > max_history_position)
		return history[max_history_position];
	else
		return history[history_position - position - 1];
}

uint_fast32_t ncsh_history_command(void) {
	for (uint_fast32_t i = 0; i < history_position; i++) {
		printf("%lu %s\n", i + 1, history[i].value);
	}
	return 1;
}

