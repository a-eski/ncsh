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
#include <string.h>
#include <assert.h>

#include "ncsh_terminal.h"
#include "ncsh_vm.h"
#include "ncsh_parser.h"
#include "ncsh_history.h"
#include "ncsh_config.h"
#include "ncsh_autocompletions.h"
#include "ncsh_defines.h"
#include "ncsh_builtins.h"
#include "ncsh.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_defines.h"
#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_string.h"
#include "z/z.h"

#define EXIT_IO_FAILURE -1
// #define EXIT_SUCCESS 0 // From stdlib.h
// #define EXIT_FAILURE 1 // From stdlib.h
#define EXIT_SUCCESS_END 2
#define EXIT_SUCCESS_EXECUTE 3

#define ncsh_write(str, len) if (write(STDOUT_FILENO, str, len) == -1) {	\
		perror(RED NCSH_ERROR_STDOUT RESET);				\
		fflush(stderr);							\
		return EXIT_FAILURE;						\
	}									\

struct ncsh_Input {
	size_t start_pos;
	size_t pos;
	size_t max_pos;
	char* buffer;
};

struct ncsh {
	struct ncsh_Directory prompt_info;
	struct ncsh_Config config;

	struct ncsh_Args args;

	uint_fast32_t history_position;
	struct eskilib_String history_entry;
	struct ncsh_History history;
	char* current_autocompletion;
	struct ncsh_Autocompletion_Node* autocompletions_tree;

	struct z_Database z_db;

	struct ncsh_Coordinates terminal_size;
	struct ncsh_Coordinates terminal_position;
};

#ifdef NCSH_SHORT_DIRECTORY
void ncsh_prompt_directory(char* cwd, char* output) {
	uint_fast32_t i = 1;
	uint_fast32_t last_slash_pos = 0;
	uint_fast32_t second_to_last_slash = 0;

	while (cwd[i] != '\n' && cwd[i] != '\0') {
		if (cwd[i] == '/') {
			second_to_last_slash = last_slash_pos;
			last_slash_pos = i + 1;
		}
		++i;
	}

	memcpy(output, &cwd[second_to_last_slash] - 1, i - second_to_last_slash + 1);
}
#endif /* ifdef NCSH_SHORT_DIRECTORY */

eskilib_nodiscard
int_fast32_t ncsh_prompt(struct ncsh_Directory prompt_info) {
	if (!getcwd(prompt_info.path, sizeof(prompt_info.path))) {
		perror(RED "ncsh: Error when getting current directory" RESET);
		fflush(stderr);
		return EXIT_FAILURE;
	}

	#ifdef NCSH_SHORT_DIRECTORY
	char prompt_directory[PATH_MAX] = {0};
	ncsh_prompt_directory(prompt_info.path, prompt_directory);
	printf(ncsh_GREEN "%s" WHITE " " ncsh_CYAN "%s" WHITE_BRIGHT " \u2771 ", prompt_info.user, prompt_directory);
	#else
	printf(ncsh_GREEN "%s" WHITE " " ncsh_CYAN "%s" WHITE_BRIGHT " \u2771 ", prompt_info.user, prompt_info.path);
	#endif /* ifdef NCSH_SHORT_DIRECTORY */

	fflush(stdout);
	// save cursor position so we can reset cursor when loading history entries
	ncsh_write(SAVE_CURSOR_POSITION, SAVE_CURSOR_POSITION_LENGTH);

	return EXIT_SUCCESS;
}

eskilib_nodiscard
int_fast32_t ncsh_backspace(struct ncsh_Input* input) {
	if (input->pos == 0) {
		return EXIT_SUCCESS;
	}
	else {
		--input->pos;
		if (input->max_pos > 0)
			--input->max_pos;

		ncsh_write(BACKSPACE_STRING ERASE_CURRENT_LINE, BACKSPACE_STRING_LENGTH + ERASE_CURRENT_LINE_LENGTH);

		input->start_pos = input->pos;
		memmove(input->buffer + input->pos, input->buffer + input->pos + 1, input->max_pos);
		input->buffer[input->max_pos] = '\0';

		while (input->buffer[input->pos] != '\0') {
			putchar(input->buffer[input->pos]);
			++input->pos;
		}

		fflush(stdout);

		while (input->pos > input->start_pos) {
			if (input->pos == 0 || !input->buffer[input->pos - 1])
				break;

			ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);

			--input->pos;
		}

		return EXIT_SUCCESS;
	}
}

eskilib_nodiscard
int_fast32_t ncsh_delete(struct ncsh_Input* input) {
	ncsh_write(DELETE_STRING ERASE_CURRENT_LINE, DELETE_STRING_LENGTH + ERASE_CURRENT_LINE_LENGTH);

	input->start_pos = input->pos;
	memmove(input->buffer + input->pos, input->buffer + input->pos + 1, input->max_pos);

	if (input->max_pos > 0)
		--input->max_pos;

	while (input->pos < input->max_pos && input->buffer[input->pos]) {
		putchar(input->buffer[input->pos]);
		++input->pos;
	}
	fflush(stdout);

	if (input->pos == 0)
		return EXIT_SUCCESS;

	while (input->pos > input->start_pos && input->pos != 0 && input->buffer[input->pos - 1]) {
		ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);
		--input->pos;
	}

	if (input->pos > input->max_pos)
		input->max_pos = input->pos;

	return EXIT_SUCCESS;
}

void ncsh_autocomplete(struct ncsh_Input* input, struct ncsh* shell) {
	uint_fast8_t autocompletions_matches_count =
		ncsh_autocompletions_first(input->buffer, input->pos + 1, shell->current_autocompletion, shell->autocompletions_tree);

	if (autocompletions_matches_count == 0) {
		shell->current_autocompletion[0] = '\0';
		return;
	}

	shell->terminal_position = ncsh_terminal_position();

	int length = strlen(shell->current_autocompletion) + 1;
	if (shell->terminal_position.x + length >= shell->terminal_size.x) {
		/*if (write(STDOUT_FILENO, WHITE_DIM, sizeof(WHITE_DIM) - 1) == -1)
			return;

		for (int_fast32_t i = 0; i < shell->terminal_size.x - shell->terminal_position.x; ++i) {
			putchar(shell->current_autocompletion[i]);
		}
		putchar('\n');
		for (int_fast32_t i = shell->terminal_size.x - shell->terminal_position.x; i < length; ++i) {
			putchar(shell->current_autocompletion[i]);
		}

		if (write(STDOUT_FILENO, RESET, sizeof(RESET) - 1) == -1)
			return;
		fflush(stdout);*/
		return;
	}
	printf(WHITE_DIM "%s" RESET, shell->current_autocompletion);
	ncsh_terminal_move(shell->terminal_position.x, shell->terminal_position.y);
	fflush(stdout);
}

char ncsh_read(void) {
	char character = 0;
	if (read(STDIN_FILENO, &character, 1) == 0)  {
		perror(RED NCSH_ERROR_STDIN RESET);
		return EXIT_IO_FAILURE;
	}

	switch (character) {
		case ESCAPE_CHARACTER: {
			if (read(STDIN_FILENO, &character, 1) == -1) {
				perror(RED NCSH_ERROR_STDIN RESET);
				return EXIT_IO_FAILURE;
			}

			if (character == '[') {
				if (read(STDIN_FILENO, &character, 1) == -1) {
					perror(RED NCSH_ERROR_STDIN RESET);
					return EXIT_IO_FAILURE;
				}

				return character;
			}

			break;
		}
		case '\n':
		case '\r': {
			return character;
		}
	}

	return '\0';
}

eskilib_nodiscard
int_fast32_t ncsh_tab_autocomplete(struct ncsh_Input* input, struct ncsh_Autocompletion_Node* autocompletions_tree) {
	struct ncsh_Autocompletion autocompletion_matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
	uint_fast32_t autocompletions_matches_count = ncsh_autocompletions_get(input->buffer, input->pos + 1, autocompletion_matches, autocompletions_tree);

	ncsh_write(ERASE_CURRENT_LINE "\n", ERASE_CURRENT_LINE_LENGTH + 1);

	if (autocompletions_matches_count == 0)
		return EXIT_SUCCESS;

	for (uint_fast8_t i = 0; i < autocompletions_matches_count; ++i)
		printf("%s%s\n", input->buffer, autocompletion_matches[i].value);
	ncsh_terminal_move_up(autocompletions_matches_count);
	fflush(stdout);

	uint_fast32_t position = 0;
	char character;

	int_fast32_t exit = EXIT_SUCCESS;
	bool continue_input = true;
	while (continue_input) {
		if ((character = ncsh_read()) == EXIT_IO_FAILURE) {
			return EXIT_FAILURE;
		}
		switch (character) {
			case UP_ARROW: {
				if (position == 0)
					break;
				ncsh_write(MOVE_CURSOR_UP, MOVE_CURSOR_UP_LENGTH);
				--position;
				break;
			}
			case DOWN_ARROW: {
				if (position == autocompletions_matches_count - 1)
					break;
				ncsh_write(MOVE_CURSOR_DOWN, MOVE_CURSOR_DOWN_LENGTH);
				++position;
				break;
			}
			case '\n':
			case '\r': {
				continue_input = false;
				size_t length = strlen(autocompletion_matches[position].value);
				memcpy(input->buffer + input->pos,
					autocompletion_matches[position].value, length);
				input->pos += length + 1;
				exit = EXIT_SUCCESS_EXECUTE;
				break;
			}
			default : { continue_input = false; break; }
		}
	}

	for (uint_fast8_t i = 0; i < autocompletions_matches_count; ++i) {
		free(autocompletion_matches[i].value);
	}

	ncsh_terminal_move_down(autocompletions_matches_count + 1 - position);
	if (exit == EXIT_SUCCESS_EXECUTE) {
		printf(ncsh_YELLOW "%s\n" RESET, input->buffer);
	}
	fflush(stdout);

	return exit;
}

eskilib_nodiscard
int_fast32_t ncsh_z(struct ncsh_Args* args, struct z_Database* z_db) {
	assert(z_db);
	assert(args->count > 0);

	if (args->count > 2) {
		if (!args->values[1] || !args->values[2])
			return NCSH_COMMAND_EXIT_FAILURE;

		if (eskilib_string_equals(args->values[1], "add", args->lengths[1])) {
			size_t length = strlen(args->values[2]) + 1;
			if (z_add(args->values[2], length, z_db) == Z_SUCCESS)
				return NCSH_COMMAND_SUCCESS_CONTINUE;
			else
				return NCSH_COMMAND_EXIT_FAILURE;
		}
		else {
			return NCSH_COMMAND_SUCCESS_CONTINUE;
		}
	}

	size_t length = !args->values[1] ? 0 : strlen(args->values[1]) + 1;
	char cwd[PATH_MAX] = {0};
	char* cwd_result = getcwd(cwd, PATH_MAX);
	if (!cwd_result) {
		perror("Could not load cwd information");
		return NCSH_COMMAND_EXIT_FAILURE;
	}

	z(args->values[1], length, cwd, z_db);
	return NCSH_COMMAND_SUCCESS_CONTINUE;
}

eskilib_nodiscard
int_fast32_t ncsh_execute(struct ncsh* shell) {
	if (shell->args.count == 0)
		return NCSH_COMMAND_SUCCESS_CONTINUE;

	if (ncsh_is_exit_command(&shell->args))
		return NCSH_COMMAND_EXIT;

	if (eskilib_string_equals(shell->args.values[0], "echo", shell->args.lengths[0])) {
		return ncsh_echo_command(&shell->args);
	}

	if (eskilib_string_equals(shell->args.values[0], "help", shell->args.lengths[0]))
		return ncsh_help_command();

	if (eskilib_string_equals(shell->args.values[0], "cd", shell->args.lengths[0])) {
		return ncsh_cd_command(&shell->args);
	}

	if (eskilib_string_equals(shell->args.values[0], "z", shell->args.lengths[0]))
		return ncsh_z(&shell->args, &shell->z_db);

	if (eskilib_string_equals(shell->args.values[0], "history", shell->args.lengths[0]))
		return ncsh_history_command(&shell->history);

	return ncsh_vm_execute(&shell->args);
}

void ncsh_exit(struct ncsh* shell) {
	ncsh_config_free(&shell->config);

	ncsh_parser_args_free(&shell->args);

	ncsh_history_exit(&shell->history);

	free(shell->current_autocompletion);
	ncsh_autocompletions_free(shell->autocompletions_tree);

	z_exit(&shell->z_db);

	ncsh_terminal_reset();
}

eskilib_nodiscard
int_fast32_t ncsh_init(struct ncsh* shell) {
	shell->prompt_info.user = getenv("USER");

	enum eskilib_Result result;
	if ((result = ncsh_config_init(&shell->config)) != E_SUCCESS) {
		if (result != E_FAILURE_MALLOC)
			ncsh_config_free(&shell->config);
		return EXIT_FAILURE;
	}

	if ((result = ncsh_parser_args_malloc(&shell->args)) != E_SUCCESS) {
		perror(RED "ncsh: Error when allocating memory for parser" RESET);
		fflush(stderr);
		ncsh_config_free(&shell->config);
		if (result != E_FAILURE_MALLOC)
			ncsh_parser_args_free(&shell->args);
		return EXIT_FAILURE;
	}

	if ((result = ncsh_history_init(shell->config.config_location, &shell->history)) != E_SUCCESS) {
		ncsh_config_free(&shell->config);
		ncsh_parser_args_free(&shell->args);
		if (result != E_FAILURE_MALLOC)
			ncsh_history_exit(&shell->history);
		return EXIT_FAILURE;
	}

	shell->current_autocompletion = malloc(NCSH_MAX_INPUT);
	if (!shell->current_autocompletion) {
		perror(RED "ncsh: Error when allocating data for autocompletion results" RESET);
		fflush(stderr);
		ncsh_config_free(&shell->config);
		ncsh_parser_args_free(&shell->args);
		ncsh_history_exit(&shell->history);
	}

	shell->autocompletions_tree = ncsh_autocompletions_malloc();
	if (!shell->autocompletions_tree) {
		perror(RED "ncsh: Error when loading data from history as autocompletions" RESET);
		fflush(stderr);
		ncsh_config_free(&shell->config);
		ncsh_parser_args_free(&shell->args);
		ncsh_history_exit(&shell->history);
		ncsh_autocompletions_free(shell->autocompletions_tree);
		return EXIT_FAILURE;
	}
	ncsh_autocompletions_add_multiple(shell->history.entries, shell->history.count, shell->autocompletions_tree);

	enum z_Result z_result = z_init(shell->config.config_location, &shell->z_db);
	if (z_result != Z_SUCCESS) {
		ncsh_config_free(&shell->config);
		ncsh_parser_args_free(&shell->args);
		ncsh_history_exit(&shell->history);
		free(shell->current_autocompletion);
		ncsh_autocompletions_free(shell->autocompletions_tree);
		if (z_result != Z_MALLOC_ERROR)
			z_exit(&shell->z_db);
		return EXIT_FAILURE;
	}

	ncsh_terminal_init();

	return EXIT_SUCCESS;
}

eskilib_nodiscard
int_fast32_t ncsh_read_input(struct ncsh_Input* input, struct ncsh* shell) {
	char character;
	char temp_character;

	while (1) {
		if (input->pos == 0 && shell->prompt_info.reprint_prompt == true) {
			if (ncsh_prompt(shell->prompt_info) == EXIT_FAILURE) {
				return EXIT_FAILURE;
			}
			shell->history_position = 0;
		}
		else {
			shell->prompt_info.reprint_prompt = true;
		}

		if (read(STDIN_FILENO, &character, 1) == -1) {
			perror(RED NCSH_ERROR_STDIN RESET);
			fflush(stderr);
			return EXIT_FAILURE;
		}

		if (character == CTRL_D) {
			putchar('\n');
			return EXIT_SUCCESS_END;
		}
		else if (character == CTRL_W) { // delete last word
			shell->prompt_info.reprint_prompt = false;
			if (input->pos == 0 && input->max_pos == 0)
				continue;

			ncsh_write(BACKSPACE_STRING ERASE_CURRENT_LINE, BACKSPACE_STRING_LENGTH + ERASE_CURRENT_LINE_LENGTH);
			input->buffer[input->max_pos] = '\0';
			--input->max_pos;

			while (input->max_pos > 0 && input->buffer[input->max_pos] != ' ') {
				ncsh_write(BACKSPACE_STRING, BACKSPACE_STRING_LENGTH);
				input->buffer[input->max_pos] = '\0';
				--input->max_pos;
			}
			fflush(stdout);

			input->pos = 0;
			shell->args.count = 0;
			shell->args.values[0] = NULL;
		}
		else if (character == CTRL_U) { // delete entire line
			shell->prompt_info.reprint_prompt = false;
			if (input->pos == 0 && input->max_pos == 0)
				continue;

			ncsh_write(BACKSPACE_STRING ERASE_CURRENT_LINE, BACKSPACE_STRING_LENGTH + ERASE_CURRENT_LINE_LENGTH);
			input->buffer[input->max_pos] = '\0';
			--input->max_pos;

			while (input->max_pos > 0) {
				ncsh_write(BACKSPACE_STRING, BACKSPACE_STRING_LENGTH);
				input->buffer[input->max_pos] = '\0';
				--input->max_pos;
			}
			fflush(stdout);

			input->pos = 0;
			shell->args.count = 0;
			shell->args.values[0] = NULL;
		}
		else if (character == TAB_KEY) {
			int_fast32_t tab_autocomplete_result = ncsh_tab_autocomplete(input, shell->autocompletions_tree);
			switch (tab_autocomplete_result) {
				case EXIT_FAILURE: { return EXIT_FAILURE; }
				case EXIT_SUCCESS_EXECUTE: { return EXIT_SUCCESS_EXECUTE; }
				case EXIT_SUCCESS : {
					shell->prompt_info.reprint_prompt = true;
					input->buffer[0] = '\0';
					input->pos = 0;
					input->max_pos = 0;
					shell->args.count = 0;
					shell->args.values[0] = NULL;
					break;
				}
			}

			continue;
		}
		else if (character == BACKSPACE_KEY) {
			shell->prompt_info.reprint_prompt = false;
			if (input->pos == 0) {
				continue;
			}
			shell->current_autocompletion[0] = '\0';
			if (ncsh_backspace(input) == EXIT_FAILURE) {
				return EXIT_FAILURE;
			}
		}
		else if (character == ESCAPE_CHARACTER) {
			if (read(STDIN_FILENO, &character, 1) == -1) {
				perror(RED NCSH_ERROR_STDIN RESET);
				fflush(stderr);
				return EXIT_FAILURE;
			}

			if (character == '[') {
				if (read(STDIN_FILENO, &character, 1) == -1) {
					perror(RED NCSH_ERROR_STDIN RESET);
					fflush(stderr);
					return EXIT_FAILURE;
				}

				switch (character) {
					case RIGHT_ARROW: {
						shell->prompt_info.reprint_prompt = false;
						if (input->pos == 0 && input->max_pos == 0)
							continue;

						if (input->pos == input->max_pos && input->buffer[0]) {
							printf("%s", shell->current_autocompletion);
							for (uint_fast32_t i = 0; shell->current_autocompletion[i] != '\0'; i++) {
								input->buffer[input->pos] = shell->current_autocompletion[i];
								++input->pos;
							}
							input->buffer[input->pos] = '\0';
							if (input->pos > input->max_pos)
								input->max_pos = input->pos;

							fflush(stdout);
							shell->current_autocompletion[0] = '\0';
							break;
						}

						if (input->pos == NCSH_MAX_INPUT - 1 || (!input->buffer[input->pos] && !input->buffer[input->pos + 1]))
							continue;

						ncsh_write(MOVE_CURSOR_RIGHT, MOVE_CURSOR_RIGHT_LENGTH);

						++input->pos;

						break;
					}
					case LEFT_ARROW: {
						shell->prompt_info.reprint_prompt = false;

						if (input->pos == 0 || (!input->buffer[input->pos] && !input->buffer[input->pos - 1]))
							continue;

						ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH);

						--input->pos;

						break;
					}
					case UP_ARROW: {
						shell->prompt_info.reprint_prompt = false;

						shell->history_entry = ncsh_history_get(shell->history_position, &shell->history);
						if (shell->history_entry.length > 0) {
							++shell->history_position;
							ncsh_write(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE,
		       						RESTORE_CURSOR_POSITION_LENGTH + ERASE_CURRENT_LINE_LENGTH);

							input->pos = shell->history_entry.length - 1;
							input->max_pos = shell->history_entry.length - 1;
							eskilib_string_copy(input->buffer, shell->history_entry.value, input->pos);

							ncsh_write(input->buffer, input->pos);
						}

						break;
					}
					case DOWN_ARROW: {
						shell->prompt_info.reprint_prompt = false;

						if (shell->history_position == 0)
							break;

						shell->history_entry = ncsh_history_get(shell->history_position - 2, &shell->history);

						ncsh_write(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE,
		       						RESTORE_CURSOR_POSITION_LENGTH + ERASE_CURRENT_LINE_LENGTH);

						if (shell->history_entry.length > 0) {
							--shell->history_position;
							input->pos = shell->history_entry.length - 1;
							input->max_pos = shell->history_entry.length - 1;
							eskilib_string_copy(input->buffer, shell->history_entry.value, input->pos);

							ncsh_write(input->buffer, input->pos);
						}
						else {
							input->buffer[0] = '\0';
							input->pos = 0;
							input->max_pos = 0;
						}

						break;
					}
					case DELETE_PREFIX_KEY: {
						if (read(STDIN_FILENO, &character, 1) == -1) {
							perror(RED NCSH_ERROR_STDIN RESET);
							fflush(stderr);
							return EXIT_FAILURE;
						}

						if (character != DELETE_KEY)
							continue;

						shell->prompt_info.reprint_prompt = false;

						if (ncsh_delete(input) == EXIT_FAILURE)
							return EXIT_FAILURE;

						break;
					}
					case HOME_KEY: {
						shell->prompt_info.reprint_prompt = false;
						if (input->pos == 0) {
							continue;
						}

						ncsh_write(RESTORE_CURSOR_POSITION, RESTORE_CURSOR_POSITION_LENGTH);
						input->pos = 0;

						break;
					}
					case END_KEY: {
						shell->prompt_info.reprint_prompt = false;
						if (input->pos == input->max_pos) {
							continue;
						}

						while (input->buffer[input->pos]) {
							ncsh_write(MOVE_CURSOR_RIGHT, MOVE_CURSOR_RIGHT_LENGTH)
							++input->pos;
						}

						break;
					}
				}
			}
		}
		else if (character == '\n' || character == '\r') {
			if (input->pos == 0 && !input->buffer[input->pos]) {
				putchar('\n');
				fflush(stdout);
				continue;
			}

			while (input->pos < input->max_pos && input->buffer[input->pos])
				++input->pos;

			while (input->pos > 1 && input->buffer[input->pos - 1] == ' ')
				--input->pos;

			input->buffer[input->pos++] = '\0';

			ncsh_write(ERASE_CURRENT_LINE "\n", ERASE_CURRENT_LINE_LENGTH + 1);
			fflush(stdout);

			return EXIT_SUCCESS_EXECUTE;
		}
		else {
			if (input->pos == NCSH_MAX_INPUT - 1) {
				fputs(RED "\nncsh: Hit max input.\n" RESET, stderr);
				input->buffer[0] = '\0';
				input->pos = 0;
				input->max_pos = 0;
				continue;
			}

			// midline insertions
			if (input->pos < input->max_pos && input->buffer[input->pos]) {
				input->start_pos = input->pos;

				if (input->pos == 0) {
					temp_character = input->buffer[0];
					input->buffer[0] = character;
					putchar(character);
					character = temp_character;
					++input->pos;
				}

				for (uint_fast32_t i = input->pos - 1; i < input->max_pos && i < NCSH_MAX_INPUT; ++i) {
					temp_character = character;
					character = input->buffer[i + 1];
					input->buffer[i + 1] = temp_character;
					putchar(temp_character);
					++input->pos;
				}

				if (input->pos > input->max_pos)
					input->max_pos = input->pos;

				if (input->pos == input->max_pos)
					input->buffer[input->pos] = '\0';

				fflush(stdout);

				if (input->pos == 0 || input->buffer[1] == '\0')
					continue;

				while (input->pos > input->start_pos + 1) {
					ncsh_write(MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH)
					--input->pos;
				}
			}
			else { // end of line insertions
				putchar(character);
				fflush(stdout);
				input->buffer[input->pos++] = character;

				if (input->pos > input->max_pos)
					input->max_pos = input->pos;

				if (input->pos == input->max_pos)
					input->buffer[input->pos] = '\0';

				ncsh_write(ERASE_CURRENT_LINE, ERASE_CURRENT_LINE_LENGTH);
			}

			shell->terminal_position = ncsh_terminal_position();
			shell->terminal_size = ncsh_terminal_size();
			if (shell->terminal_position.x >= shell->terminal_size.x) {
				putchar('\n');
				fflush(stdout);
			}

			// autocompletion logic
			if (input->buffer[0] == '\0' || input->buffer[input->pos] != '\0')
				continue;

			ncsh_autocomplete(input, shell);
		}
	}
}

int_fast32_t ncsh(void) {
	#ifdef NCSH_START_TIME
	clock_t start = clock();
	#endif

	int_fast32_t exit_code = EXIT_SUCCESS;
	int_fast32_t input_result = 0;
	int_fast32_t command_result = 0;

	struct ncsh_Input input = {0};
	input.buffer = malloc(NCSH_MAX_INPUT);
	if (!input.buffer) {
		return EXIT_FAILURE;
	}

	struct ncsh shell = {0};
	if (ncsh_init(&shell) == EXIT_FAILURE) {
		free(input.buffer);
		return EXIT_FAILURE;
	}

	#ifdef NCSH_CLEAR_SCREEN_ON_STARTUP
	if (write(STDOUT_FILENO, CLEAR_SCREEN MOVE_CURSOR_HOME, CLEAR_SCREEN_LENGTH + MOVE_CURSOR_HOME_LENGTH) == -1) {
		ncsh_exit(&shell);
		free(input.buffer);
		return EXIT_FAILURE;
	}
	#endif

	#ifdef NCSH_START_TIME
	clock_t end = clock();
	double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
	printf("ncsh: startup time: %.2f milliseconds\n", elapsed_ms);
	#endif

	if (ncsh_prompt(shell.prompt_info) == EXIT_FAILURE) {
		ncsh_exit(&shell);
		free(input.buffer);
		return EXIT_FAILURE;
	}

	while (1) {
		input_result = ncsh_read_input(&input, &shell);
		switch (input_result) {
			case EXIT_FAILURE: {
				exit_code = EXIT_FAILURE;
				break;
			}
			case EXIT_SUCCESS: {
				goto reset;
			}
			case EXIT_SUCCESS_END: {
				goto exit;
			}
		}


		ncsh_parser_parse(input.buffer, input.pos, &shell.args);
		command_result = ncsh_execute(&shell);
		ncsh_parser_args_free_values(&shell.args);

		switch (command_result) {
			case NCSH_COMMAND_EXIT_FAILURE: {
				exit_code = EXIT_FAILURE;
				goto exit;
			}
			case NCSH_COMMAND_EXIT: {
				goto exit;
			}
		}

		if (command_result == NCSH_COMMAND_SUCCESS_CONTINUE) {
			ncsh_history_add(input.buffer, input.pos, &shell.history);
			ncsh_autocompletions_add(input.buffer, input.pos, shell.autocompletions_tree);
		}

		reset:
			memset(input.buffer, '\0', input.max_pos);
			input.pos = 0;
			input.max_pos = 0;
			shell.args.count = 0;
			shell.args.values[0] = NULL;
	}

	exit:
		free(input.buffer);
		ncsh_exit(&shell);

	return exit_code;
}

