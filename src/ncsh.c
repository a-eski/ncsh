// Copyright (c) ncsh by Alex Eski 2024

#include <linux/limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "ncsh_args.h"
#include "ncsh_terminal.h"
#include "ncsh_vm.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "ncsh_io.h"
#include "ncsh.h"
#include "ncsh_autocompletions.h"
#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_string.h"

// #define NCSH_DEBUG
#ifdef NCSH_DEBUG
#include "ncsh_debug.h"
#endif /* ifdef NCSH_DEBUG */

#define NCSH_MAX_INPUT 528
#define NCSH_MAX_MATCHES 32

enum ncsh_Hotkey ncsh_get_key(char character) {
	switch (character) {
		case UP_ARROW: { return UP; }
		case DOWN_ARROW: { return DOWN; }
		case RIGHT_ARROW: { return RIGHT; }
		case LEFT_ARROW: { return LEFT; }
		case DELETE_PREFIX_KEY: { return DELETE_PREFIX; }
		default: { return NONE; }
	}
}

void ncsh_write(char* string, uint_fast32_t length) {
	if (write(STDOUT_FILENO, string, length) == -1) {
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

void ncsh_print_prompt(struct ncsh_Directory prompt_info) {
	char *getcwd_result = getcwd(prompt_info.path, sizeof(prompt_info.path));
	if (getcwd_result == NULL) {
		perror(RED "conch-shell: error when getting current directory" RESET);
		exit(EXIT_FAILURE);
	}

	printf(ncsh_GREEN "%s" WHITE ":" ncsh_CYAN "%s" WHITE "$ ", prompt_info.user, prompt_info.path);
	fflush(stdout);
}

void ncsh_backspace(char* buffer, uint_fast32_t* buf_position, uint_fast32_t* max_buf_position) {
	uint_fast32_t buf_start;

	if (*buf_position == 0) {
		return;
	}
	else {// if (buffer[*buf_position]) {
		--*buf_position;
		--*max_buf_position;

		ncsh_write(BACKSPACE_STRING ERASE_CURRENT_LINE, BACKSPACE_STRING_LENGTH + ERASE_CURRENT_LINE_LENGTH);

		buf_start = *buf_position;
		for (uint_fast32_t i = *buf_position; buffer[i]; ++i) {
			buffer[i] = buffer[i + 1];
		}

		while (buffer[*buf_position]) {
			putchar(buffer[*buf_position]);
			++*buf_position;
		}

		fflush(stdout);

		while (*buf_position > buf_start) {
			if (*buf_position == 0 || !buffer[*buf_position - 1]) {
				break;
			}

			ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);
			--*buf_position;
		}
	}
	/*else {
		ncsh_write(BACKSPACE_STRING, BACKSPACE_STRING_LENGTH);
		--*max_buf_position;
		--*buf_position;
		buffer[*buf_position] = '\0';
	}*/
}

void ncsh_delete(char* buffer, uint_fast32_t* buf_position, uint_fast32_t* max_buf_position) {
	ncsh_write(DELETE_STRING ERASE_CURRENT_LINE, DELETE_STRING_LENGTH + ERASE_CURRENT_LINE_LENGTH);

	uint_fast32_t buf_start = *buf_position;
	for (uint_fast32_t i = *buf_position; i < *max_buf_position && buffer[i]; ++i)
		buffer[i] = buffer[i + 1];

	--*max_buf_position;

	for (uint_fast32_t i = *buf_position; i < *max_buf_position && buffer[*buf_position]; ++i) {
		putchar(buffer[i]);
		++*buf_position;
	}

	fflush(stdout);

	if (*buf_position == 0)
		return;

	while (*buf_position > buf_start && *buf_position != 0 && buffer[*buf_position - 1]) {
		ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);
		--*buf_position;
	}
}

int ncsh(void) {
	clock_t start = clock();

	char character;
	char temp_character;
	char buffer[NCSH_MAX_INPUT] = {0};
	uint_fast32_t buf_start = 0;
	uint_fast32_t buf_position = 0;
	uint_fast32_t max_buf_position = 0;
	enum ncsh_Hotkey key;

	bool reprint_prompt = false;
	struct ncsh_Directory prompt_info;
	prompt_info.user = getenv("USER");

	uint_fast32_t command_result = 0;
	struct ncsh_Args args = {0};
	bool did_malloc_succeed = ncsh_args_malloc(&args);
	if (!did_malloc_succeed)
		return EXIT_FAILURE;

	enum eskilib_Result result; // used to track operation results
	uint_fast32_t history_position = 0; // current position in history for the current loop, reset every loop
	struct eskilib_String history_entry; // used to hold return value when cycling through history
	struct ncsh_History history;
	result = ncsh_history_malloc(&history);
	if (result != E_SUCCESS) {
		perror(RED "Error when allocating memory for history" RESET);
		fflush(stdout);
		ncsh_args_free(args);
		return EXIT_FAILURE;
	}
	// history.history_file_directory = getenv("HOME");
	// getcwd(history.history_file_directory, PATH_MAX);
	result = ncsh_history_load(&history);
	if (result != E_SUCCESS) {
		perror(RED "Error when loading data from history file into memory" RESET);
		fflush(stdout);
		ncsh_args_free(args);
		return EXIT_FAILURE;
	}

	char* current_autocompletion = malloc(sizeof(char) * MAX_INPUT);

	uint_fast32_t autocompletions_matches_count = 0;
	struct ncsh_Autocompletions* autocompletions_tree = ncsh_autocompletions_malloc();
	if (autocompletions_tree == NULL)
	{
		perror(RED "Error when loading data from history file into memory" RESET);
		fflush(stdout);
		ncsh_history_free(&history);
		ncsh_args_free(args);
		return EXIT_FAILURE;
	}
	ncsh_autocompletions_add_multiple(history.entries, history.history_count, autocompletions_tree);

	ncsh_terminal_init();

	// save cursor position so we can reset cursor when loading history entries
	ncsh_write(SAVE_CURSOR_POSITION, SAVE_CURSOR_POSITION_LENGTH);

	clock_t end = clock();
	double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
	printf("ncsh: startup time: %.2f milliseconds\n", elapsed_ms);

	ncsh_print_prompt(prompt_info);

	while (1) {
		if (buf_position == 0 && reprint_prompt == true) {
			ncsh_print_prompt(prompt_info);
			history_position = 0;
			// save cursor position so we can reset cursor when loading history entries
			ncsh_write(SAVE_CURSOR_POSITION, SAVE_CURSOR_POSITION_LENGTH);
		}
		else {
			reprint_prompt = true;
		}

		if (read(STDIN_FILENO, &character, 1) == -1) {
			perror(RED "Error reading from stdin" RESET);
			fflush(stdout);
			exit(EXIT_FAILURE);
		}

		if (character == CTRL_D) {
			putchar('\n');
			fflush(stdout);
			break;
		}

		if (character == BACKSPACE_KEY) {
			reprint_prompt = false;
			if (buf_position == 0) {
				continue;
			}
			ncsh_backspace(buffer, &buf_position, &max_buf_position);
		}
		else if (character == ESCAPE_CHARACTER) {
			if (read(STDIN_FILENO, &character, 1) == -1) {
				perror(RED "Error reading from stdin" RESET);
				fflush(stdout);
				exit(EXIT_FAILURE);
			}

			if (character == '[') {
				if (read(STDIN_FILENO, &character, 1) == -1) {
					perror(RED "Error reading from stdin" RESET);
					fflush(stdout);
					exit(EXIT_FAILURE);
				}
				key = ncsh_get_key(character);

				switch (key) {
					case RIGHT: {
						reprint_prompt = false;

						if (buf_position == max_buf_position) {
							printf("%s", current_autocompletion);
							for (uint_fast32_t i = 0; current_autocompletion[i] != '\0'; i++) {
								buffer[buf_position] = current_autocompletion[i];
								++buf_position;
							}
							buffer[buf_position] = '\0';

							fflush(stdout);
							break;
						}

						if (buf_position == NCSH_MAX_INPUT - 1 || (!buffer[buf_position] && !buffer[buf_position + 1]))
							continue;

						ncsh_write(MOVE_CURSOR_RIGHT, MOVE_CURSOR_RIGHT_LENGTH);
						++buf_position;

						break;
					}
					case LEFT: {
						reprint_prompt = false;

						if (buf_position == 0 || (!buffer[buf_position] && !buffer[buf_position - 1]))
							continue;

						ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);
						--buf_position;

						break;
					}
					case UP: {
						reprint_prompt = false;

						history_entry = ncsh_history_get(history_position, &history);
						if (history_entry.length > 0) {
							++history_position;
							ncsh_write(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE,
								RESTORE_CURSOR_POSITION_LENGTH + ERASE_CURRENT_LINE_LENGTH);
							buf_position = history_entry.length - 1;
							max_buf_position = history_entry.length - 1;
							eskilib_string_copy(buffer, history_entry.value, buf_position);
							ncsh_write(buffer, buf_position);
						}

						break;
					}
					case DOWN: {
						reprint_prompt = false;

						if (history_position == 0)
							break;

						history_entry = ncsh_history_get(history_position - 2, &history);
						ncsh_write(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE,
								RESTORE_CURSOR_POSITION_LENGTH + ERASE_CURRENT_LINE_LENGTH);
						if (history_entry.length > 0) {
							--history_position;
							buf_position = history_entry.length - 1;
							max_buf_position = history_entry.length - 1;
							eskilib_string_copy(buffer, history_entry.value, buf_position);
							ncsh_write(buffer, buf_position);
						}
						else {
							buffer[0] = '\0';
							buf_position = 0;
							max_buf_position = 0;
						}

						break;
					}
					case DELETE_PREFIX: {
						if (read(STDIN_FILENO, &character, 1) == -1) {
							perror(RED "Error reading from stdin" RESET);
							fflush(stdout);
							exit(EXIT_FAILURE);
						}

						if (character != DELETE_KEY)
							continue;

						reprint_prompt = false;

						ncsh_delete(buffer, &buf_position, &max_buf_position);

						break;
					}
					case NONE: {
						continue;
					}
				}
			}
		}
		else if (character == '\n' || character == '\r') {
			if (buf_position == 0 && !buffer[buf_position])
				continue;

			while (buf_position < max_buf_position && buffer[buf_position])
				++buf_position;

			while (buf_position > 1 && buffer[buf_position - 1] == ' ')
				--buf_position;

			buffer[buf_position++] = '\0';

			ncsh_write(ERASE_CURRENT_LINE, ERASE_CURRENT_LINE_LENGTH);

			putchar('\n');
			fflush(stdout);

			#ifdef NCSH_DEBUG
			ncsh_debug_line(buffer, buf_position);
			#endif /* ifdef NCSH_DEBUG */

			args = ncsh_parse(buffer, buf_position, args);
			if (!ncsh_args_is_valid(args))
				continue;

			#ifdef NCSH_DEBUG
			ncsh_debug_args(args);
			#endif /* ifdef NCSH_DEBUG */

			ncsh_history_add(buffer, buf_position, &history);
			ncsh_autocompletions_add(buffer, buf_position, autocompletions_tree);

			command_result = ncsh_vm_execute(args, &history);

			ncsh_args_free_values(args);

			if (command_result == 0)
				break;

			buffer[0] = '\0';
			buf_position = 0;
			max_buf_position = 0;
			args.count = 0;
			args.max_line_length = 0;
			args.values[0] = NULL;
		}
		else {
			if (buf_position == NCSH_MAX_INPUT - 1) {
				fputs(RED "\nHit max input.\n" RESET, stderr);
				buffer[0] = '\0';
				buf_position = 0;
				max_buf_position = 0;
				continue;
			}

			if (buf_position < max_buf_position && buffer[buf_position]) {
				buf_start = buf_position;

				if (buf_position == 0) {
					temp_character = buffer[0];
					buffer[0] = character;
					putchar(character);
					character = temp_character;
					++buf_position;
				}

				for (uint_fast32_t i = buf_position - 1; i < max_buf_position && i < NCSH_MAX_INPUT; ++i) {
					temp_character = character;
					character = buffer[i + 1];
					buffer[i + 1] = temp_character;
					putchar(temp_character);
					++buf_position;
				}

				if (buf_position > max_buf_position)
					max_buf_position = buf_position;

				if (buf_position == max_buf_position)
					buffer[buf_position] = '\0';

				fflush(stdout);

				if (buf_position == 0 || buffer[1] == '\0')
					continue;

				while (buf_position > buf_start + 1) {
					ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);
					--buf_position;
				}
			}
			else {
				putchar(character);
				fflush(stdout);
				buffer[buf_position++] = character;

				if (buf_position > max_buf_position)
					max_buf_position = buf_position;

				if (buf_position == max_buf_position)
					buffer[buf_position] = '\0';

				ncsh_write(ERASE_CURRENT_LINE, ERASE_CURRENT_LINE_LENGTH);
			}

			// autocompletion logic
			if (buffer[0] == '\0')
				continue;

			char* autocompletion_matches[NCSH_MAX_MATCHES] = {0};
			autocompletions_matches_count = ncsh_autocompletions_get(buffer, buf_position + 1, autocompletion_matches, NCSH_MAX_INPUT, autocompletions_tree);

			if (autocompletions_matches_count == 0) {
				current_autocompletion[0] = '\0';
				continue;
			}

			struct ncsh_Coordinates position = ncsh_terminal_position();
			if (position.x == 0 && position.y == 0)
				continue;

			current_autocompletion = autocompletion_matches[0];
			printf(WHITE_DIM "%s" RESET, current_autocompletion);
			ncsh_terminal_move(position.x, position.y);
			fflush(stdout);
		}
	}

	ncsh_args_free(args);

	ncsh_history_save(&history);
	ncsh_history_free(&history);

	free(current_autocompletion);
	ncsh_autocompletions_free(autocompletions_tree);

	ncsh_terminal_reset();

	return EXIT_SUCCESS;
}

