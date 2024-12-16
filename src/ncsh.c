// Copyright (c) ncsh by Alex Eski 2024

#ifdef linux
#include <linux/limits.h>
#endif /* ifdef linux */

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

#include "eskilib/eskilib_result.h"
#include "ncsh_args.h"
#include "ncsh_terminal.h"
#include "ncsh_vm.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "ncsh_config.h"
#include "ncsh_autocompletions.h"
#include "ncsh_defines.h"
#include "ncsh_builtins.h"
#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_string.h"
#include "z/z.h"

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

#ifdef NCSH_SHORT_DIRECTORY
void ncsh_prompt_directory(char* cwd, char* output) {
	uint_fast32_t i = 1;
	uint_fast32_t last_slash_pos = 0;
	uint_fast32_t second_to_last_slash = 0;

	for (; cwd[i] != '\n' && cwd[i] != '\0'; ++i) {
		if (cwd[i] == '/') {
			second_to_last_slash = last_slash_pos;
			last_slash_pos = i + 1;
		}
	}

	strncpy(output, &cwd[second_to_last_slash] - 1, i - second_to_last_slash + 1);
}
#endif /* ifdef NCSH_SHORT_DIRECTORY */

[[nodiscard]]
int_fast32_t ncsh_prompt(struct ncsh_Directory prompt_info) {
	char* wd_result = getcwd(prompt_info.path, sizeof(prompt_info.path));
	if (wd_result == NULL) {
		perror(RED "ncsh: Error when getting current directory" RESET);
		fflush(stderr);
		return NCSH_EXIT_FAILURE;
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
	if (write(STDOUT_FILENO, SAVE_CURSOR_POSITION, SAVE_CURSOR_POSITION_LENGTH) == -1) {
		perror(RED NCSH_ERROR_STDOUT RESET);
		fflush(stderr);
		return NCSH_EXIT_FAILURE;
	}

	return NCSH_EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_backspace(char* buffer, uint_fast32_t* buf_position, uint_fast32_t* max_buf_position) {
	if (*buf_position == 0) {
		return NCSH_EXIT_SUCCESS;
	}
	else {
		--*buf_position;
		if (*max_buf_position > 0)
			--*max_buf_position;

		if (write(STDOUT_FILENO, BACKSPACE_STRING ERASE_CURRENT_LINE, BACKSPACE_STRING_LENGTH + ERASE_CURRENT_LINE_LENGTH) == -1) {
			perror(RED NCSH_ERROR_STDOUT RESET);
			fflush(stderr);
			return NCSH_EXIT_FAILURE;
		}

		uint_fast32_t buf_start = *buf_position;
		for (uint_fast32_t i = *buf_position; i < *max_buf_position && buffer[i]; ++i)
			buffer[i] = buffer[i + 1];

		buffer[*max_buf_position] = '\0';

		while (buffer[*buf_position] != '\0') {
			putchar(buffer[*buf_position]);
			++*buf_position;
		}

		fflush(stdout);
		// memmove(&buffer[buf_position - 1], &buffer[buf_position], strlen(buffer) - buf_position + 1);

		while (*buf_position > buf_start) {
			if (*buf_position == 0 || !buffer[*buf_position - 1])
				break;

			if (write(STDOUT_FILENO, MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH) == -1) {
				perror(RED NCSH_ERROR_STDOUT RESET);
				fflush(stderr);
				return NCSH_EXIT_FAILURE;
			}
			--*buf_position;
		}

		return NCSH_EXIT_SUCCESS;
	}
}

[[nodiscard]]
int_fast32_t ncsh_delete(char* buffer, uint_fast32_t* buf_position, uint_fast32_t* max_buf_position) {
	if (write(STDOUT_FILENO, DELETE_STRING ERASE_CURRENT_LINE, DELETE_STRING_LENGTH + ERASE_CURRENT_LINE_LENGTH) == -1) {
		perror(RED NCSH_ERROR_STDOUT RESET);
		fflush(stderr);
		return NCSH_EXIT_FAILURE;
	}

	uint_fast32_t buf_start = *buf_position;
	for (uint_fast32_t i = *buf_position; i < *max_buf_position && buffer[i]; ++i)
		buffer[i] = buffer[i + 1];

	if (*max_buf_position > 0)
		--*max_buf_position;

	while (*buf_position < *max_buf_position && buffer[*buf_position]) {
		putchar(buffer[*buf_position]);
		++*buf_position;
	}

	fflush(stdout);

	if (*buf_position == 0)
		return NCSH_EXIT_SUCCESS;

	while (*buf_position > buf_start && *buf_position != 0 && buffer[*buf_position - 1]) {
		if (write(STDOUT_FILENO, MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH) == -1) {
			perror(RED NCSH_ERROR_STDOUT RESET);
			fflush(stderr);
			return NCSH_EXIT_FAILURE;
		}
		--*buf_position;
	}

	if (*buf_position > *max_buf_position)
		*max_buf_position = *buf_position;

	return NCSH_EXIT_SUCCESS;
}

void ncsh_autocomplete(char* buffer, uint_fast32_t buf_position, char* current_autocompletion, struct ncsh_Autocompletion_Node* autocompletions_tree) {
	uint_fast8_t autocompletions_matches_count = ncsh_autocompletions_first(buffer, buf_position + 1, current_autocompletion, autocompletions_tree);

	if (autocompletions_matches_count == 0) {
		current_autocompletion[0] = '\0';
		return;
	}

	struct ncsh_Coordinates position = ncsh_terminal_position();
	if (position.x == 0 && position.y == 0)
		return;

	printf(WHITE_DIM "%s" RESET, current_autocompletion);
	ncsh_terminal_move(position.x, position.y);
	fflush(stdout);
}

int_fast32_t ncsh_z(struct ncsh_Args* args, struct z_Database* z_db) {
	assert(z_db != NULL);
	assert(args->count > 0);
	if (z_db == NULL)
		return NCSH_COMMAND_EXIT_FAILURE;
	if (args->count == 0)
		return NCSH_COMMAND_CONTINUE;

	if (args->count > 2) {
		if (!args->values[1] || !args->values[2])
			return NCSH_COMMAND_EXIT_FAILURE;

		if (eskilib_string_equals(args->values[1], "add", args->max_line_length > 4 ? args->max_line_length : 4)) {
			size_t length = strlen(args->values[2]) + 1;
			if (z_add(args->values[2], length, z_db) == Z_SUCCESS)
				return NCSH_COMMAND_CONTINUE;
			else
				return NCSH_COMMAND_EXIT_FAILURE;
		}
		else {
			return NCSH_COMMAND_CONTINUE;
		}
	}

	size_t length = args->values[1] == NULL ? 0 : strlen(args->values[1]) + 1;
	char cwd[PATH_MAX] = {0};
	char* cwd_result = getcwd(cwd, PATH_MAX);
	if (!cwd_result) {
		perror("Could not load cwd information");
		return NCSH_COMMAND_EXIT_FAILURE;
	}

	z(args->values[1], length, cwd, z_db);
	return NCSH_COMMAND_CONTINUE;
}

int_fast32_t ncsh_execute(struct ncsh_Args* args,
			  struct ncsh_History* history,
			  struct z_Database* z_db) {
	if (ncsh_is_exit_command(args))
		return 0;

	if (eskilib_string_equals(args->values[0], "echo", args->max_line_length))
		return ncsh_echo_command(args);

	if (eskilib_string_equals(args->values[0], "help", args->max_line_length))
		return ncsh_help_command();

	if (eskilib_string_equals(args->values[0], "cd", args->max_line_length))
		return ncsh_cd_command(args);

	if (eskilib_string_equals(args->values[0], "z", args->max_line_length))
		return ncsh_z(args, z_db);

	if (eskilib_string_equals(args->values[0], "history", args->max_line_length))
		return ncsh_history_command(history);

	return ncsh_vm_execute(args);
}

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
};

void ncsh_exit(struct ncsh* shell) {
	ncsh_config_free(&shell->config);

	ncsh_args_free(&shell->args);

	ncsh_history_exit(&shell->history);

	free(shell->current_autocompletion);
	ncsh_autocompletions_free(shell->autocompletions_tree);

	z_exit(&shell->z_db);

	ncsh_terminal_reset();
}

int ncsh_init(struct ncsh* shell) {
	shell->prompt_info.user = getenv("USER");

	if (ncsh_config_init(&shell->config) != E_SUCCESS) {
		ncsh_config_free(&shell->config);
		return EXIT_FAILURE;
	}

	if (ncsh_args_malloc(&shell->args) != E_SUCCESS) {
		perror(RED "ncsh: Error when allocating memory for parsing" RESET);
		fflush(stderr);
		ncsh_config_free(&shell->config);
		ncsh_args_free(&shell->args);
		return EXIT_FAILURE;
	}

	if (ncsh_history_malloc(&shell->history) != E_SUCCESS) {
		perror(RED "ncsh: Error when allocating memory for history" RESET);
		fflush(stderr);
		ncsh_config_free(&shell->config);
		ncsh_args_free(&shell->args);
		ncsh_history_free(&shell->history);
		return EXIT_FAILURE;
	}

	if (ncsh_history_load(shell->config.config_location, &shell->history) != E_SUCCESS) {
		perror(RED "ncsh: Error when loading data from history file" RESET);
		fflush(stderr);
		ncsh_config_free(&shell->config);
		ncsh_args_free(&shell->args);
		ncsh_history_free(&shell->history);
		return EXIT_FAILURE;
	}

	shell->current_autocompletion = malloc(NCSH_MAX_INPUT);
	shell->autocompletions_tree = ncsh_autocompletions_malloc();
	if (!shell->autocompletions_tree) {
		perror(RED "ncsh: Error when loading data from history as autocompletions" RESET);
		fflush(stderr);
		ncsh_config_free(&shell->config);
		ncsh_args_free(&shell->args);
		ncsh_history_free(&shell->history);
		ncsh_autocompletions_free(shell->autocompletions_tree);
		return EXIT_FAILURE;
	}
	ncsh_autocompletions_add_multiple(shell->history.entries, shell->history.history_count, shell->autocompletions_tree);

	ncsh_terminal_init();

	enum z_Result result = z_init(shell->config.config_location, &shell->z_db);
	if (result != Z_SUCCESS) {
		ncsh_exit(shell);
		return EXIT_FAILURE;
	}

	#ifdef NCSH_CLEAR_SCREEN_ON_STARTUP
	if (write(STDOUT_FILENO,
	   CLEAR_SCREEN MOVE_CURSOR_HOME,
	   CLEAR_SCREEN_LENGTH + MOVE_CURSOR_HOME_LENGTH) == -1) {
		perror(RED NCSH_ERROR_STDOUT RESET);
		fflush(stderr);
		ncsh_exit(shell);
		return EXIT_FAILURE;
	}
	#endif

	return EXIT_SUCCESS;
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
	int_fast32_t command_result = 0;

	struct ncsh shell = {0};
	if (ncsh_init(&shell) == EXIT_FAILURE)
		return EXIT_FAILURE;

	clock_t end = clock();
	double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
	printf("ncsh: startup time: %.2f milliseconds\n", elapsed_ms);

	int exit = EXIT_SUCCESS;
	if (ncsh_prompt(shell.prompt_info) == NCSH_EXIT_FAILURE) {
		exit = EXIT_FAILURE;
		goto free_all;
	}

	while (1) {
		if (buf_position == 0 && shell.prompt_info.reprint_prompt == true) {
			if (ncsh_prompt(shell.prompt_info) == NCSH_EXIT_FAILURE) {
				exit = EXIT_FAILURE;
				goto free_all;
			}
			shell.history_position = 0;
		}
		else {
			shell.prompt_info.reprint_prompt = true;
		}

		if (read(STDIN_FILENO, &character, 1) == -1) {
			exit = EXIT_FAILURE;
			perror(RED NCSH_ERROR_STDIN RESET);
			fflush(stderr);
			goto free_all;
		}

		if (character == CTRL_D) {
			putchar('\n');
			goto free_all;
		}

		if (character == TAB_KEY)
		{
			character = ' ';
		}

		if (character == BACKSPACE_KEY) {
			shell.prompt_info.reprint_prompt = false;
			if (buf_position == 0) {
				continue;
			}
			shell.current_autocompletion[0] = '\0';
			if (ncsh_backspace(buffer, &buf_position, &max_buf_position) == NCSH_EXIT_FAILURE) {
				exit = EXIT_FAILURE;
				goto free_all;
			}
		}
		else if (character == ESCAPE_CHARACTER) {
			if (read(STDIN_FILENO, &character, 1) == -1) {
				perror(RED NCSH_ERROR_STDIN RESET);
				fflush(stderr);
				exit = EXIT_FAILURE;
				goto free_all;
			}

			if (character == '[') {
				if (read(STDIN_FILENO, &character, 1) == -1) {
					perror(RED NCSH_ERROR_STDIN RESET);
					fflush(stderr);
					exit = EXIT_FAILURE;
					goto free_all;
				}

				key = ncsh_get_key(character);

				switch (key) {
					case RIGHT: {
						shell.prompt_info.reprint_prompt = false;

						if (buf_position == max_buf_position && buffer[0]) {
							printf("%s", shell.current_autocompletion);
							for (uint_fast32_t i = 0; shell.current_autocompletion[i] != '\0'; i++) {
								buffer[buf_position] = shell.current_autocompletion[i];
								++buf_position;
							}
							buffer[buf_position] = '\0';
							if (buf_position > max_buf_position)
								max_buf_position = buf_position;

							fflush(stdout);
							shell.current_autocompletion[0] = '\0';
							break;
						}

						if (buf_position == NCSH_MAX_INPUT - 1 || (!buffer[buf_position] && !buffer[buf_position + 1]))
							continue;

						if (write(STDOUT_FILENO, MOVE_CURSOR_RIGHT, MOVE_CURSOR_RIGHT_LENGTH) == -1) {
							perror(RED NCSH_ERROR_STDOUT RESET);
							fflush(stderr);
							exit = EXIT_FAILURE;
							goto free_all;
						}
						++buf_position;

						break;
					}
					case LEFT: {
						shell.prompt_info.reprint_prompt = false;

						if (buf_position == 0 || (!buffer[buf_position] && !buffer[buf_position - 1]))
							continue;

						if (write(STDOUT_FILENO, MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH) == -1) {
							perror(RED NCSH_ERROR_STDOUT RESET);
							fflush(stderr);
							exit = EXIT_FAILURE;
							goto free_all;
						}
						--buf_position;

						break;
					}
					case UP: {
						shell.prompt_info.reprint_prompt = false;

						shell.history_entry = ncsh_history_get(shell.history_position, &shell.history);
						if (shell.history_entry.length > 0) {
							++shell.history_position;
							if (write(STDOUT_FILENO, RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE,
								RESTORE_CURSOR_POSITION_LENGTH + ERASE_CURRENT_LINE_LENGTH) == -1) {
								perror(RED NCSH_ERROR_STDOUT RESET);
								fflush(stderr);
								exit = EXIT_FAILURE;
								goto free_all;
							}
							buf_position = shell.history_entry.length - 1;
							max_buf_position = shell.history_entry.length - 1;
							eskilib_string_copy(buffer, shell.history_entry.value, buf_position);
							if (write(STDOUT_FILENO, buffer, buf_position) == -1) {
								perror(RED NCSH_ERROR_STDOUT RESET);
								fflush(stderr);
								exit = EXIT_FAILURE;
								goto free_all;
							}
						}

						break;
					}
					case DOWN: {
						shell.prompt_info.reprint_prompt = false;

						if (shell.history_position == 0)
							break;

						shell.history_entry = ncsh_history_get(shell.history_position - 2, &shell.history);
						if (write(STDOUT_FILENO, RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE,
							RESTORE_CURSOR_POSITION_LENGTH + ERASE_CURRENT_LINE_LENGTH) == -1) {
							perror(RED NCSH_ERROR_STDOUT RESET);
							fflush(stderr);
							exit = EXIT_FAILURE;
							goto free_all;
						}
						if (shell.history_entry.length > 0) {
							--shell.history_position;
							buf_position = shell.history_entry.length - 1;
							max_buf_position = shell.history_entry.length - 1;
							eskilib_string_copy(buffer, shell.history_entry.value, buf_position);
							if (write(STDOUT_FILENO, buffer, buf_position) == -1) {
								perror(RED NCSH_ERROR_STDOUT RESET);
								fflush(stderr);
								exit = EXIT_FAILURE;
								goto free_all;
							}
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
							perror(RED NCSH_ERROR_STDIN RESET);
							fflush(stderr);
							exit = EXIT_FAILURE;
							goto free_all;
						}

						if (character != DELETE_KEY)
							continue;

						shell.prompt_info.reprint_prompt = false;

						if (ncsh_delete(buffer, &buf_position, &max_buf_position) == NCSH_EXIT_FAILURE) {
							exit = EXIT_FAILURE;
							goto free_all;
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
			if (buf_position == 0 && !buffer[buf_position]) {
				putchar('\n');
				fflush(stdout);
				continue;
			}

			while (buf_position < max_buf_position && buffer[buf_position])
				++buf_position;

			while (buf_position > 1 && buffer[buf_position - 1] == ' ')
				--buf_position;

			buffer[buf_position++] = '\0';

			if (write(STDOUT_FILENO, ERASE_CURRENT_LINE, ERASE_CURRENT_LINE_LENGTH) == -1) {
				perror(RED NCSH_ERROR_STDOUT RESET);
				fflush(stderr);
				exit = EXIT_FAILURE;
				goto free_all;
			}

			putchar('\n');
			fflush(stdout);

			#ifdef NCSH_DEBUG
			ncsh_debug_line(buffer, buf_position, max_buf_position);
			#endif /* ifdef NCSH_DEBUG */

			ncsh_parse(buffer, buf_position, &shell.args);
			if (!ncsh_args_is_valid(&shell.args)) {
				ncsh_args_free_values(&shell.args);
				continue;
			}

			#ifdef NCSH_DEBUG
			ncsh_debug_args(args);
			#endif /* ifdef NCSH_DEBUG */

			command_result = ncsh_execute(&shell.args, &shell.history, &shell.z_db);

			ncsh_args_free_values(&shell.args);

			switch (command_result) {
				case -1: {
					exit = EXIT_FAILURE;
					goto free_all;
				}
				case 0: {
					goto free_all;
				}
			}

			ncsh_history_add(buffer, buf_position, &shell.history);
			ncsh_autocompletions_add(buffer, buf_position, shell.autocompletions_tree);

			buffer[0] = '\0';
			buf_position = 0;
			max_buf_position = 0;
			shell.args.count = 0;
			shell.args.max_line_length = 0;
			shell.args.values[0] = NULL;
		}
		else {
			if (buf_position == NCSH_MAX_INPUT - 1) {
				fputs(RED "\nHit max input.\n" RESET, stderr);
				buffer[0] = '\0';
				buf_position = 0;
				max_buf_position = 0;
				continue;
			}

			// midline insertions
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
					if (write(STDOUT_FILENO, MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH) == -1) {
						perror(RED NCSH_ERROR_STDOUT RESET);
						fflush(stderr);
						exit = EXIT_FAILURE;
						goto free_all;
					}
					--buf_position;
				}
			}
			else { // end of line insertions
				putchar(character);
				fflush(stdout);
				buffer[buf_position++] = character;

				if (buf_position > max_buf_position)
					max_buf_position = buf_position;

				if (buf_position == max_buf_position)
					buffer[buf_position] = '\0';

				if (write(STDOUT_FILENO, ERASE_CURRENT_LINE, ERASE_CURRENT_LINE_LENGTH) == -1) {
					perror(RED NCSH_ERROR_STDOUT RESET);
					fflush(stderr);
					exit = EXIT_FAILURE;
					goto free_all;
				}
			}

			// autocompletion logic
			if (buffer[0] == '\0' || buffer[buf_position] != '\0')
				continue;

			ncsh_autocomplete(buffer, buf_position, shell.current_autocompletion, shell.autocompletions_tree);
		}
	}

	free_all:
		ncsh_exit(&shell);

	return exit;
}

