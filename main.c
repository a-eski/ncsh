#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#include "ncsh_types.h"
#include "ncsh_terminal.h"
#include "ncsh_output.h"
#include "ncsh_string.h"
#include "ncsh_commands.h"

#define ncsh_TOKEN_BUFFER_SIZE 64
#define ncsh_ARGS_ARRAY_SIZE 20

#define ESCAPE_CHARACTER 27
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
#define ERASE_CURRENT_LINE "\033[K"
#define ERASE_CURRENT_LINE_LENGTH 3

_Bool ncsh_is_delimiter(char ch)
{
	switch (ch)
	{
		case ' ':
			return true;
		case '\t':
			return true;
		case '\r':
			return true;
		case '\n':
			return true;
		case '\a':
			return true;
		case EOF:
			return true;
		case '\0':
			return true;
		default:
			return false;
	}
}

struct ncsh_Args ncsh_line_split(char line[], uint_fast32_t length)
{
	const uint_fast32_t buffer_size = ncsh_TOKEN_BUFFER_SIZE;
	char buffer[ncsh_TOKEN_BUFFER_SIZE];
	uint_fast32_t buffer_position = 0;

	struct ncsh_Args args = { .count = 0, .lines = NULL };
	args.lines = malloc(sizeof(char*) * ncsh_TOKEN_BUFFER_SIZE);
	if (args.lines == NULL)
		exit(-1);

	uint_fast32_t double_quotes_count = 0;

	for (uint_fast32_t line_position = 0; line_position < length + 1; line_position++)
	{
		if (line_position == length || buffer_position == ncsh_TOKEN_BUFFER_SIZE - 1)
		{
			args.lines[args.count] = NULL;
			break;
		}
		else if (ncsh_is_delimiter(line[line_position]) && (double_quotes_count == 0 || double_quotes_count == 2))
		{
			buffer[buffer_position] = '\0';

			args.lines[args.count] = malloc(sizeof(char) * buffer_size);
			ncsh_string_copy(args.lines[args.count], buffer, buffer_size);
			args.count++;

			if (args.maxLineSize == 0 || buffer_position > args.maxLineSize)
				args.maxLineSize = buffer_position;

			buffer[0] = '\0';
			buffer_position = 0;
			double_quotes_count = 0;
		}
		else
		{
			if (line[line_position] == '\"')
				double_quotes_count++;
			else	
				buffer[buffer_position++] = line[line_position];
		}
	}

	return args;
}

_Bool ncsh_args_is_valid(struct ncsh_Args args)
{
	if (args.count == 0 || args.lines == NULL)
		return 0;

	if (args.lines[0][0] == '\0')
		return 0;

	return 1;
}

void ncsh_args_free(struct ncsh_Args args)
{
	for (uint_fast32_t i = 0; i < args.count; i++)
		free(args.lines[i]);
	free(args.lines);
}

void ncsh_print_prompt(struct ncsh_Directory prompt_info)
{
	char *getcwd_result = getcwd(prompt_info.path, sizeof(prompt_info.path));
	if (getcwd_result == NULL)
	{
		perror(RED "conch-shell: error when getting current directory" RESET);
		exit(EXIT_FAILURE);
	}

	printf(GREEN "%s" WHITE ":" CYAN "%s" RESET "$ ", prompt_info.user, prompt_info.path);
	fflush(stdout);
}

enum ncsh_Hotkey ncsh_get_key(char character)
{
	switch (character)
	{
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

char ncsh_read_char(void)
{
	char character;
	if (read(STDIN_FILENO, &character, 1) == -1)
	{
		perror(RED "Error reading from stdin" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	return character;
}

void ncsh_write(char* string, uint_fast32_t length)
{
	if (write(STDOUT_FILENO, string, length) == -1)
	{
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

void ncsh_backspace(void)
{
	if (write(STDOUT_FILENO, BACKSPACE_STRING, BACKSPACE_STRING_LENGTH) == -1)
	{
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

void ncsh_delete_line(void)
{
	if (write(STDOUT_FILENO, ERASE_CURRENT_LINE, ERASE_CURRENT_LINE_LENGTH) == -1)
	{
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

void ncsh_move_cursor_left(void)
{
	if (write(STDOUT_FILENO, MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH) == -1)
	{
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

void ncsh_move_cursor_right(void)
{
	if (write(STDOUT_FILENO, MOVE_CURSOR_RIGHT, MOVE_CURSOR_RIGHT_LENGTH) == -1)
	{
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

int main(void)
{
	char character;
	char buffer[MAX_INPUT];
	uint_fast32_t buffer_position = 0;

	struct ncsh_Directory prompt_info;
	prompt_info.user = getenv("USER");
	bool reprint_prompt = true;

	struct ncsh_Args args;
	uint_fast32_t command_result = 0;
	enum ncsh_Hotkey key;

	ncsh_terminal_init();
	ncsh_history_init();

	while (1)
	{
		if (buffer_position == 0 && reprint_prompt == true)
			ncsh_print_prompt(prompt_info);
		else
			reprint_prompt = true;

		character = ncsh_read_char();

		if (character == CTRL_D)
		{
			putchar('\n');
			fflush(stdout);
			break;
		}

		if (character == BACKSPACE_KEY)
		{
			if (buffer_position <= 1)
			{
				reprint_prompt = false;
				if (buffer_position == 0)
					continue;
			}

			ncsh_backspace();
			--buffer_position;
		}
		else if (character == ESCAPE_CHARACTER)
		{
			character = ncsh_read_char();

			if (character == '[')
			{
				character = ncsh_read_char();
				key = ncsh_get_key(character);

				if (key == RIGHT)
				{
					if (buffer_position == MAX_INPUT)
					{
						reprint_prompt = false;
						continue;
					}

					ncsh_move_cursor_right();
					++buffer_position;
				}
				if (key == LEFT)
				{
					if (buffer_position == 0)
					{
						reprint_prompt = false;
						continue;
					}

					ncsh_move_cursor_left();
					--buffer_position;

					if (buffer_position == 0)
						reprint_prompt = false;
				}
			
				if (key == DELETE)
				{
					if (buffer_position <= 1)
					{
						reprint_prompt = false;
						if (buffer_position == 0)
							continue;
					}

					ncsh_backspace();
					--buffer_position;
				}
			}
		}
		else if (character == '\n' || character == '\r')
		{
			putchar('\n');
			fflush(stdout);

			if (buffer_position == 0) continue;

			buffer[buffer_position++] = '\0';

			args = ncsh_line_split(buffer, buffer_position);
			if (!ncsh_args_is_valid(args))
				continue;

			ncsh_history_add(buffer, buffer_position);

			command_result = ncsh_execute_command(args);
			ncsh_args_free(args);
			
			if (command_result == 0)
				break;

			buffer[0] = '\0';
			buffer_position = 0;
		}
		else
		{
			putchar(character);
			fflush(stdout);
			buffer[buffer_position++] = character;
			buffer[buffer_position] = '\0';
		}
	}

	ncsh_terminal_reset();

	return EXIT_SUCCESS;
}
