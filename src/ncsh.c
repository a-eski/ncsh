/* Copyright ncsh by Alex Eski 2024 */

#include <linux/limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_string.h"
#include "ncsh_args.h"
#include "ncsh_terminal.h"
#include "ncsh_vm.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "ncsh_io.h"
#include "ncsh.h"

// #define NCSH_DEBUG
#ifdef NCSH_DEBUG
#include "ncsh_debug.h"
#endif /* ifdef NCSH_DEBUG */

// struct ncsh_Loop {
// 	char character;
// 	char buffer[MAX_INPUT];
// 	uint_fast8_t buf_start;
// 	uint_fast8_t buf_position;
// 	uint_fast8_t max_buf_position;
// 	enum ncsh_Hotkey key;
//
// 	bool reprint_prompt;
// 	struct ncsh_Directory prompt_info;
//
// 	uint_fast8_t command_result;
// 	struct ncsh_Args args;
//
// 	uint_fast32_t history_position;
// 	struct eskilib_String history;
// };

int ncsh(void) {
	clock_t start = clock();

	char character;
	char temp_character;
	char buffer[MAX_INPUT] = {0};
	uint_fast8_t buf_start = 0;
	uint_fast8_t buf_position = 0;
	uint_fast8_t max_buf_position = 0;

	enum ncsh_Hotkey key;
	bool reprint_prompt = false;
	struct ncsh_Directory prompt_info;
	prompt_info.user = getenv("USER");

	uint_fast8_t command_result = 0;
	struct ncsh_Args args = {0};
	bool did_malloc_succeed = ncsh_args_malloc(&args);
	if (!did_malloc_succeed)
		return EXIT_FAILURE;

	uint_fast32_t history_position = 0; // current position in history for the current loop, reset every loop
	struct eskilib_String history_entry; // used to hold return value when cycling through history
	enum ncsh_History_Result result;
	struct ncsh_History history;
	result = ncsh_history_malloc(&history);
	if (result != H_SUCCESS) {
		perror(RED "Error when allocating memory for history" RESET);
		fflush(stdout);
		ncsh_args_free(args);
		return EXIT_FAILURE;
	}
	// history.history_file_directory = getenv("HOME");
	getcwd(history.history_file_directory, PATH_MAX);
	printf("%s\n", history.history_file_directory);
	result = ncsh_history_load(&history);
	if (result != H_SUCCESS) {
		perror(RED "Error when loading data from history file into memory" RESET);
		fflush(stdout);
		ncsh_args_free(args);
		return EXIT_FAILURE;
	}

	ncsh_terminal_init();

	// save cursor position so we can reset cursor when loading history entries
	ncsh_write(SAVE_CURSOR_POSITION, SAVE_CURSOR_POSITION_LENGTH);

	clock_t end = clock();
	double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
	printf("Startup time: %.2f milliseconds\n", elapsed_ms);

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
			else if (buf_position > 0 && buffer[buf_position]) {
				--buf_position;
				--max_buf_position;

				ncsh_write(BACKSPACE_STRING ERASE_CURRENT_LINE, BACKSPACE_STRING_LENGTH + ERASE_CURRENT_LINE_LENGTH);

				buf_start = buf_position;
				for (uint_fast8_t i = buf_position; buffer[i]; i++) {
					buffer[i] = buffer[i + 1];
				}

				while (buffer[buf_position])
					putchar(buffer[buf_position++]);

				fflush(stdout);

				while (buf_position > buf_start) {
					if (buf_position == 0 || !buffer[buf_position - 1]) {
						break;
					}

					ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);
					--buf_position;
				}
			}
			else {
				ncsh_write(BACKSPACE_STRING, BACKSPACE_STRING_LENGTH);
				--buf_position;
				buffer[buf_position] = 0;
			}
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

						if (buf_position == MAX_INPUT - 1 || (!buffer[buf_position] && !buffer[buf_position + 1]))
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

						ncsh_write(DELETE_STRING ERASE_CURRENT_LINE, DELETE_STRING_LENGTH + ERASE_CURRENT_LINE_LENGTH);
						buf_start = buf_position;
						for (uint_fast8_t i = buf_position; buffer[i]; i++)
							buffer[i] = buffer[i + 1];

						while (buffer[buf_position])
							putchar(buffer[buf_position++]);

						fflush(stdout);

						while (buf_position > buf_start) {
							if (buf_position == 0 || !buffer[buf_position - 1]) {
								break;
							}

							ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);
							--buf_position;
						}

						break;
					}
					case NONE: {

						continue;
					}
				}
			}
		}
		else if (character == '\n' || character == '\r') {
			putchar('\n');
			fflush(stdout);

			if (buf_position == 0 && !buffer[buf_position])
				continue;

			while (buf_position < max_buf_position && buffer[buf_position])
				++buf_position;

			while (buf_position > 1 && buffer[buf_position - 1] == ' ')
				--buf_position;

			buffer[buf_position++] = '\0';
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
			if (buf_position == MAX_INPUT - 1) {
				fputs(RED "\nHit max input.\n" RESET, stderr);
				buffer[0] = '\0';
				buf_position = 0;
				max_buf_position = 0;
				continue;
			}

			if (buf_position < max_buf_position && buf_position > 0 && buf_position < MAX_INPUT - 1 && buffer[buf_position + 1]) {
				buf_start = buf_position;

				for (uint_fast8_t i = buf_position - 1; i < max_buf_position; i++) {
					temp_character = character;
					character = buffer[i + 1];
					buffer[i + 1] = temp_character;
					putchar(buffer[i + 1]);
					++buf_position;
				}

				if (buf_position > max_buf_position)
					max_buf_position = buf_position;

				if (buf_position == max_buf_position)
					buffer[buf_position] = '\0';
				fflush(stdout);

				while (buf_position > buf_start + 1)
				{
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
			}
		}
	}

	ncsh_args_free(args);

	ncsh_history_save(&history);
	ncsh_history_free(&history);

	ncsh_terminal_reset();

	return EXIT_SUCCESS;
}

