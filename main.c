#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#include "eskilib/eskilib_colors.h"
#include "ncsh_types.h"
#include "ncsh_args.h"
#include "ncsh_terminal.h"
#include "ncsh_commands.h"
#include "ncsh_builtin_commands.h"
#include "ncsh_parser.h"
// #ifndef NDEBUG
// #include "ncsh_debug.h"
// #endif /* ifndef NDEBUG */

#define ESCAPE_CHARACTER 27
#define DOUBLE_QUOTE_KEY '\"'
#define CTRL_D '\004'
#define BACKSPACE_KEY 127
#define UP_ARROW 'A'
#define DOWN_ARROW 'B'
#define RIGHT_ARROW 'C'
#define LEFT_ARROW 'D'
#define DELETE_KEY '~'

#define MOVE_CURSOR_RIGHT "\033[1C"
#define MOVE_CURSOR_RIGHT_LENGTH 4
#define MOVE_CURSOR_LEFT "\033[1D"
#define MOVE_CURSOR_LEFT_LENGTH 4
#define BACKSPACE_STRING "\b \b"
#define BACKSPACE_STRING_LENGTH 3
#define BACKSPACE_AND_SAVE_POSITION_STRING "\b\0337"
#define BACKSPACE_AND_SAVE_POSITION_STRING_LENGTH 3
#define RESTORE_SAVED_POSITION_STRING "\0338"
#define RESTORE_SAVED_POSITION_STRING_LENGTH 2
#define ERASE_CURRENT_LINE "\033[K"
#define ERASE_CURRENT_LINE_LENGTH 3

char ncsh_read_char(void) {
	char character;
	if (read(STDIN_FILENO, &character, 1) == -1) {
		perror(RED "Error reading from stdin" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	return character;
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

enum ncsh_Hotkey ncsh_get_key(char character) {
	switch (character) {
		case UP_ARROW:
			return UP;
		case DOWN_ARROW:
			return DOWN;
		case RIGHT_ARROW:
			return RIGHT;
		case LEFT_ARROW:
			return LEFT;
		case DELETE_KEY:
			return DELETE;
		default:
			return NONE;
	}
}

int main(void) {
	char character;
	char buffer[MAX_INPUT] = {};
	uint_fast8_t buffer_position = 0;

	struct ncsh_Directory prompt_info;
	prompt_info.user = getenv("USER");
	bool reprint_prompt = false;
	ncsh_print_prompt(prompt_info);

	struct ncsh_Args args;
	uint_fast32_t command_result = 0;
	enum ncsh_Hotkey key;

	ncsh_terminal_init();
	ncsh_history_init();
	// setenv("shell", prompt_info.path, 1);
	
	while (1) {
		if (buffer_position == 0 && reprint_prompt == true)
			ncsh_print_prompt(prompt_info);
		else
			reprint_prompt = true;

		character = ncsh_read_char();

		if (character == CTRL_D) {
			putchar('\n');
			fflush(stdout);
			break;
		}

		if (character == BACKSPACE_KEY) {
			if (buffer_position <= 1)
			{
				reprint_prompt = false;
				if (buffer_position == 0)
					continue;
			}

			// if (buffer_position > 1 && buffer[buffer_position]) {
			// 	--buffer_position;
			// 
			// 	ncsh_write(BACKSPACE_AND_SAVE_POSITION_STRING, BACKSPACE_AND_SAVE_POSITION_STRING_LENGTH);
			// 	ncsh_write(ERASE_CURRENT_LINE, ERASE_CURRENT_LINE_LENGTH);
			// 
			// 	for(uint_fast8_t i = buffer_position; buffer[buffer_position] != 0; i++)
			// 		buffer[i] = buffer[i + 1];
			// 
			// 	printf("%s", buffer);
			// 	ncsh_write(RESTORE_SAVED_POSITION_STRING, RESTORE_SAVED_POSITION_STRING_LENGTH);
			// }
			// else {
				ncsh_write(BACKSPACE_STRING, BACKSPACE_STRING_LENGTH);
				--buffer_position;
				buffer[buffer_position] = 0;
			// }
		}
		else if (character == ESCAPE_CHARACTER) {
			character = ncsh_read_char();

			if (character == '[') {
				character = ncsh_read_char();
				key = ncsh_get_key(character);

				if (key == RIGHT) {
					if (buffer_position == MAX_INPUT) {
						reprint_prompt = false;
						continue;
					}

					ncsh_write(MOVE_CURSOR_RIGHT, MOVE_CURSOR_RIGHT_LENGTH);
					++buffer_position;
				}
				if (key == LEFT) {
					if (buffer_position == 0) {
						reprint_prompt = false;
						continue;
					}

					ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);
					--buffer_position;

					if (buffer_position == 0)
						reprint_prompt = false;
				}
			
				if (key == DELETE) {
					if (buffer_position <= 1) {
						reprint_prompt = false;
						if (buffer_position == 0)
							continue;
					}

					ncsh_write(BACKSPACE_STRING, BACKSPACE_STRING_LENGTH);
					--buffer_position;
				}
			}
		}
		else if (character == '\n' || character == '\r') {
			putchar('\n');
			fflush(stdout);

			if (buffer_position == 0) continue;

			buffer[buffer_position++] = '\0';

			args = ncsh_parse(buffer, buffer_position);
			if (!ncsh_args_is_valid(args))
				continue;
			// #ifndef NDEBUG
			// ncsh_debug_args(args);
			// #endif /* ifndef NDEBUG */

			ncsh_history_add(buffer, buffer_position);

			command_result = ncsh_execute(args);

			ncsh_args_free(args);
			
			if (command_result == 0)
				break;

			buffer[0] = '\0';
			buffer_position = 0;
		}
		else {
			putchar(character);
			fflush(stdout);
			buffer[buffer_position++] = character;
			buffer[buffer_position] = '\0';
		}
	}

	ncsh_terminal_reset();

	return EXIT_SUCCESS;
}
