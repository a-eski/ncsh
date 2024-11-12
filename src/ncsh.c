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
#include <sys/stat.h>

#include "eskilib/eskilib_result.h"
#include "ncsh_args.h"
#include "ncsh_terminal.h"
#include "ncsh_vm.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "ncsh_io.h"
#include "ncsh.h"
#include "ncsh_autocompletions.h"
#include "ncsh_defines.h"
#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_string.h"

// #define NCSH_DEBUG
#ifdef NCSH_DEBUG
#include "ncsh_debug.h"
#endif /* ifdef NCSH_DEBUG */

#define NCSH "ncsh"
#define NCSH_LENGTH 5
#define NCSH_CONFIG_LENGTH 8 // length for .config
#define NCSH_ERROR_STDOUT "ncsh: Error writing to stdout"
#define NCSH_ERROR_STDIN "ncsh: Error writing to stdin"

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

[[nodiscard]]
int_fast32_t ncsh_prompt(struct ncsh_Directory prompt_info) {
	char* wd_result = getcwd(prompt_info.path, sizeof(prompt_info.path));
	if (wd_result == NULL) {
		perror(RED "ncsh: Error when getting current directory" RESET);
		fflush(stderr);
		return NCSH_EXIT_FAILURE;
	}

	printf(ncsh_GREEN "%s" WHITE ":" ncsh_CYAN "%s" WHITE "$ ", prompt_info.user, prompt_info.path);
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
	uint_fast32_t buf_start;

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

	for (uint_fast32_t i = *buf_position; i < *max_buf_position && buffer[*buf_position]; ++i) {
		putchar(buffer[i]);
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

uint_fast32_t ncsh_config(char* out, size_t max_length) {
	const char *const out_original_ptr = out;
	char *home = getenv("XDG_CONFIG_HOME");
	bool set_config = false;
	if (!home) {
		home = getenv("HOME");
		if (!home) { // neither HOME or XDG_CONFIG_HOME are available
			out[0] = 0;
			return 0;
		}
		set_config = true;
	}

	uint_fast32_t home_length = strlen(home);
	uint_fast32_t length = home_length;

	/* first +1 is "/", second is terminating null */
	if (home_length + 1 + NCSH_CONFIG_LENGTH + NCSH_LENGTH + 1 > max_length) {
		out[0] = 0;
		return 0;
	}

	memcpy(out, home, home_length);
	out += home_length;
	*out = '/';
	++length;
	++out;
	if (set_config) {
		memcpy(out, ".config/", NCSH_CONFIG_LENGTH);
		out += NCSH_CONFIG_LENGTH;
		*out = '\0';
		length += NCSH_CONFIG_LENGTH;
	}
	memcpy(out, NCSH, NCSH_LENGTH);
	out += NCSH_LENGTH;
	*out = '\0';
	length += NCSH_LENGTH;

	#ifdef NCSH_DEBUG
	printf("mkdir %s\n", out_original_ptr);
	#endif /* ifdef NCSH_DEBUG */
	mkdir(out_original_ptr, 0755);

	return length;
}

/*void ncsh_autocomplete(char* buffer, uint_fast32_t buf_position, char* current_autocompletion, struct ncsh_Autocompletion_Node* autocompletions_tree) {
	struct ncsh_Autocompletion autocompletion_matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
	uint_fast32_t autocompletions_matches_count = ncsh_autocompletions_get(buffer, buf_position + 1, autocompletion_matches, autocompletions_tree);

	if (autocompletions_matches_count == 0) {
		current_autocompletion[0] = '\0';
		return;
	}

	putchar('\n');
	for (uint_fast8_t i = 0; i < autocompletions_matches_count; ++i)
		printf("match[%d] (weight: %d) %s\n", i, autocompletion_matches[i].weight, autocompletion_matches[i].value);

	struct ncsh_Coordinates position = ncsh_terminal_position();
	if (position.x == 0 && position.y == 0)
		return;

	eskilib_string_copy(current_autocompletion, autocompletion_matches[0].value, NCSH_MAX_INPUT);

	for (uint_fast32_t i = 0; i < autocompletions_matches_count; ++i)
		free(autocompletion_matches[i].value);

	printf(WHITE_DIM "%s" RESET, current_autocompletion);
	ncsh_terminal_move(position.x, position.y);
	fflush(stdout);
}*/

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

	uint_fast32_t history_position = 0; // current position in history for the current loop, reset every loop
	struct eskilib_String history_entry; // used to hold return value when cycling through history
	struct ncsh_History history;
	if (ncsh_history_malloc(&history) != E_SUCCESS) {
		perror(RED "ncsh: Error when allocating memory for history" RESET);
		fflush(stderr);
		ncsh_args_free(args);
		return EXIT_FAILURE;
	}

	history.config_location.value = malloc(NCSH_MAX_INPUT);
	history.config_location.length = ncsh_config(history.config_location.value, NCSH_MAX_INPUT);
	#ifdef  NCSH_DEBUG
	ncsh_debug_config(history.config_location);
	#endif /* ifdef  NCSH_DEBUG */

	if (ncsh_history_load(&history) != E_SUCCESS) {
		perror(RED "ncsh: Error when loading data from history file" RESET);
		fflush(stderr);
		ncsh_args_free(args);
		ncsh_history_free(&history);
		return EXIT_FAILURE;
	}

	char* current_autocompletion = malloc(NCSH_MAX_INPUT);
	struct ncsh_Autocompletion_Node* autocompletions_tree = ncsh_autocompletions_malloc();
	if (autocompletions_tree == NULL) {
		perror(RED "ncsh: Error when loading data from history as autocompletions" RESET);
		fflush(stderr);
		ncsh_history_free(&history);
		ncsh_args_free(args);
		return EXIT_FAILURE;
	}
	ncsh_autocompletions_add_multiple(history.entries, history.history_count, autocompletions_tree);

	ncsh_terminal_init();

	clock_t end = clock();
	double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
	printf("ncsh: startup time: %.2f milliseconds\n", elapsed_ms);

	int exit = EXIT_SUCCESS;
	if (ncsh_prompt(prompt_info) == NCSH_EXIT_FAILURE) {
		exit = EXIT_FAILURE;
		goto free_all;
	}

	while (1) {
		if (buf_position == 0 && reprint_prompt == true) {
			if (ncsh_prompt(prompt_info) == NCSH_EXIT_FAILURE) {
				exit = EXIT_FAILURE;
				goto free_all;
			}
			history_position = 0;
		}
		else {
			reprint_prompt = true;
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

		if (character == BACKSPACE_KEY) {
			reprint_prompt = false;
			if (buf_position == 0) {
				continue;
			}
			current_autocompletion[0] = '\0';
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
						reprint_prompt = false;

						if (buf_position == max_buf_position && buffer[0]) {
							printf("%s", current_autocompletion);
							for (uint_fast32_t i = 0; current_autocompletion[i] != '\0'; i++) {
								buffer[buf_position] = current_autocompletion[i];
								++buf_position;
							}
							buffer[buf_position] = '\0';
							if (buf_position > max_buf_position)
								max_buf_position = buf_position;

							fflush(stdout);
							current_autocompletion[0] = '\0';
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
						reprint_prompt = false;

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
						reprint_prompt = false;

						history_entry = ncsh_history_get(history_position, &history);
						if (history_entry.length > 0) {
							++history_position;
							if (write(STDOUT_FILENO, RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE,
								RESTORE_CURSOR_POSITION_LENGTH + ERASE_CURRENT_LINE_LENGTH) == -1) {
								perror(RED NCSH_ERROR_STDOUT RESET);
								fflush(stderr);
								exit = EXIT_FAILURE;
								goto free_all;
							}
							buf_position = history_entry.length - 1;
							max_buf_position = history_entry.length - 1;
							eskilib_string_copy(buffer, history_entry.value, buf_position);
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
						reprint_prompt = false;

						if (history_position == 0)
							break;

						history_entry = ncsh_history_get(history_position - 2, &history);
						if (write(STDOUT_FILENO, RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE,
							RESTORE_CURSOR_POSITION_LENGTH + ERASE_CURRENT_LINE_LENGTH) == -1) {
							perror(RED NCSH_ERROR_STDOUT RESET);
							fflush(stderr);
							exit = EXIT_FAILURE;
							goto free_all;
						}
						if (history_entry.length > 0) {
							--history_position;
							buf_position = history_entry.length - 1;
							max_buf_position = history_entry.length - 1;
							eskilib_string_copy(buffer, history_entry.value, buf_position);
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

						reprint_prompt = false;

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

			if (command_result == 0) {
				exit = EXIT_FAILURE;
				goto free_all;
			}

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

			ncsh_autocomplete(buffer, buf_position, current_autocompletion, autocompletions_tree);
		}
	}

	free_all:
		ncsh_args_free(args);

		if (history.history_loaded) {
			ncsh_history_save(&history);
			ncsh_history_free(&history);
		}

		free(current_autocompletion);
		ncsh_autocompletions_free(autocompletions_tree);

		ncsh_terminal_reset();

	return exit;
}

