/* Copyright Alex Eski 2024 */

#include <linux/limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_string.h"
#include "ncsh_types.h"
#include "ncsh_args.h"
#include "ncsh_terminal.h"
#include "ncsh_commands.h"
#include "ncsh_builtin_commands.h"
#include "ncsh_parser.h"
#include "ncsh_io.h"
// #ifndef NDEBUG
// #include "ncsh_debug.h"
// #endif /* ifndef NDEBUG */

enum ncsh_Hotkey ncsh_get_key(char character) {
	switch (character) {
		case UP_ARROW: {
			return UP;
		}
		case DOWN_ARROW: {
			return DOWN;
		}
		case RIGHT_ARROW: {
			return RIGHT;
		}
		case LEFT_ARROW: {
			return LEFT;
		}
		case DELETE_KEY: {
			return DELETE;
		}
		default: {
			return NONE;
		}
	}
}

int main(void) {
	char character;
	char buffer[MAX_INPUT] = {};
	uint_fast8_t buffer_position = 0;

	enum ncsh_Hotkey key;
	bool reprint_prompt = false;
	struct ncsh_Directory prompt_info;
	prompt_info.user = getenv("USER");
	ncsh_print_prompt(prompt_info);

	uint_fast8_t command_result = 0;
	struct ncsh_Args args = ncsh_args_malloc();

	uint_fast32_t history_position = 0;
	struct ncsh_String history;
	ncsh_history_malloc();

	ncsh_terminal_init();
	atexit(ncsh_terminal_reset);

	while (1) {
		if (buffer_position == 0 && reprint_prompt == true) {
			ncsh_print_prompt(prompt_info);
			history_position = 0;
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
			if (buffer_position > 1 && buffer[buffer_position]) {
				--buffer_position;

				ncsh_write(BACKSPACE_AND_SAVE_POSITION_STRING, BACKSPACE_AND_SAVE_POSITION_STRING_LENGTH);
				ncsh_write(ERASE_CURRENT_LINE, ERASE_CURRENT_LINE_LENGTH);

				for (uint_fast8_t i = buffer_position; buffer[i]; i++) {
					buffer[i] = buffer[i + 1];
				}

				while (buffer[buffer_position])
					putchar(buffer[buffer_position++]);

				ncsh_write(RESTORE_SAVED_POSITION_STRING, RESTORE_SAVED_POSITION_STRING_LENGTH);
			}
			else if (buffer_position == 0) {
				reprint_prompt = false;
				continue;
			}
			else {
				reprint_prompt = false;
				ncsh_write(BACKSPACE_STRING, BACKSPACE_STRING_LENGTH);
				--buffer_position;
				buffer[buffer_position] = 0;
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

				if (key == RIGHT) {
					if (buffer_position == MAX_INPUT) {
						reprint_prompt = false;
						continue;
					}

					ncsh_write(MOVE_CURSOR_RIGHT, MOVE_CURSOR_RIGHT_LENGTH);
					++buffer_position;
				}
				else if (key == LEFT) {
					if (buffer_position == 0) {
						reprint_prompt = false;
						continue;
					}

					ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);
					--buffer_position;

					if (buffer_position == 0)
						reprint_prompt = false;
				}
				else if (key == DELETE) {
					if (buffer_position <= 1) {
						reprint_prompt = false;
						if (buffer_position == 0)
							continue;
					}

					ncsh_write(BACKSPACE_STRING, BACKSPACE_STRING_LENGTH);
					--buffer_position;
				}
				else if (key == UP) {
					reprint_prompt = false;

					history = ncsh_history_get(history_position++);
					if (history.length > 0) {
						ncsh_write(ERASE_CURRENT_LINE, ERASE_CURRENT_LINE_LENGTH);
						buffer_position = buffer_position > history.length ? buffer_position : history.length;
						eskilib_string_copy(buffer, history.value, ++buffer_position);
						printf("%s", buffer);
						fflush(stdout);
					}
				}
				else if (key == DOWN) {
					reprint_prompt = false;

					history = ncsh_history_get(history_position--);
					if (history.value != NULL) {
						// --history_position;
						buffer_position = buffer_position > history.length ? buffer_position : history.length;
						eskilib_string_copy(buffer, history.value, ++buffer_position);
						printf("%s", buffer);
						fflush(stdout);
					}
				}
			}
		}
		else if (character == '\n' || character == '\r') {
			putchar('\n');
			fflush(stdout);

			if (buffer_position == 0)
				continue;

			buffer[buffer_position++] = '\0';

			args = ncsh_parse(buffer, buffer_position, args);
			if (!ncsh_args_is_valid(args))
				continue;
			// #ifndef NDEBUG
			// ncsh_debug_args(args);
			// #endif /* ifndef NDEBUG */

			ncsh_history_add(buffer, buffer_position);

			command_result = ncsh_execute(args);

			ncsh_args_free_values(args);

			if (command_result == 0)
				break;

			buffer[0] = '\0';
			buffer_position = 0;
			args.count = 0;
			args.max_line_length = 0;
			args.values[0] = NULL;
		}
		else {
			putchar(character);
			fflush(stdout);
			buffer[buffer_position++] = character;
			if (buffer[buffer_position]) {
				for (uint_fast8_t i = buffer_position; buffer[i]; i++) {
					buffer[i] = buffer[i + 1];
				}
			}
			buffer[buffer_position] = '\0';
		}
	}

	ncsh_args_free(args);
	ncsh_history_free();

	return EXIT_SUCCESS;
}
