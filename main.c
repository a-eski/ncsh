#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#include "shl_types.h"
#include "shl_terminal.h"
#include "shl_output.h"
#include "shl_string.h"
#include "shl_commands.h"

#define SHL_TOKEN_BUFFER_SIZE 64
#define SHL_ARGS_ARRAY_SIZE 20

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

_Bool shl_is_delimiter(char ch)
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

struct shl_Args shl_line_split(char line[], uint_fast32_t length)
{
	const uint_fast32_t buffer_size = SHL_TOKEN_BUFFER_SIZE;
	char buffer[SHL_TOKEN_BUFFER_SIZE];
	uint_fast32_t buffer_position = 0;

	struct shl_Args args = { .count = 0, .lines = NULL };
	args.lines = malloc(sizeof(char*) * SHL_TOKEN_BUFFER_SIZE);
	if (args.lines == NULL)
		exit(-1);

	uint_fast32_t double_quotes_count = 0;

	for (uint_fast32_t line_position = 0; line_position < length + 1; line_position++)
	{
		if (line_position == length || buffer_position == SHL_TOKEN_BUFFER_SIZE - 1)
		{
			args.lines[args.count] = NULL;
			break;
		}
		else if (shl_is_delimiter(line[line_position]) && (double_quotes_count == 0 || double_quotes_count == 2))
		{
			buffer[buffer_position] = '\0';

			args.lines[args.count] = malloc(sizeof(char) * buffer_size);
			shl_string_copy(args.lines[args.count], buffer, buffer_size);
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

_Bool shl_args_is_valid(struct shl_Args args)
{
	if (args.count == 0 || args.lines == NULL)
		return 0;

	if (args.lines[0][0] == '\0')
		return 0;

	return 1;
}

void shl_args_free(struct shl_Args args)
{
	for (uint_fast32_t i = 0; i < args.count; i++)
		free(args.lines[i]);
	free(args.lines);
}

void shl_print_prompt(struct shl_Directory prompt_info)
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

enum shl_Hotkey shl_get_key(char character)
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

char shl_read_char(void)
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

void shl_write(char* string, uint_fast32_t length)
{
	if (write(STDOUT_FILENO, string, length) == -1)
	{
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

void shl_backspace(void)
{
	if (write(STDOUT_FILENO, BACKSPACE_STRING, BACKSPACE_STRING_LENGTH) == -1)
	{
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

void shl_delete_line(void)
{
	if (write(STDOUT_FILENO, ERASE_CURRENT_LINE, ERASE_CURRENT_LINE_LENGTH) == -1)
	{
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

void shl_move_cursor_left(void)
{
	if (write(STDOUT_FILENO, MOVE_CURSOR_LEFT, MOVE_CURSOR_LEFT_LENGTH) == -1)
	{
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

void shl_move_cursor_right(void)
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

	struct shl_Directory prompt_info;
	prompt_info.user = getenv("USER");
	bool reprint_prompt = true;

	struct shl_Args args;
	uint_fast32_t command_result = 0;
	enum shl_Hotkey key;

	shl_terminal_init();
	shl_history_init();

	while (1)
	{
		if (buffer_position == 0 && reprint_prompt == true)
			shl_print_prompt(prompt_info);
		else
			reprint_prompt = true;

		character = shl_read_char();

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

			shl_backspace();
			--buffer_position;
		}
		else if (character == ESCAPE_CHARACTER)
		{
			character = shl_read_char();

			if (character == '[')
			{
				character = shl_read_char();
				key = shl_get_key(character);

				if (key == RIGHT)
				{
					if (buffer_position == MAX_INPUT)
					{
						reprint_prompt = false;
						continue;
					}

					shl_move_cursor_right();
					++buffer_position;
				}
				if (key == LEFT)
				{
					if (buffer_position == 0)
					{
						reprint_prompt = false;
						continue;
					}

					shl_move_cursor_left();
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

					shl_backspace();
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

			args = shl_line_split(buffer, buffer_position);
			if (!shl_args_is_valid(args))
				continue;

			shl_history_add(buffer, buffer_position);

			command_result = shl_execute_command(args);
			shl_args_free(args);
			
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

	shl_terminal_reset();

	return EXIT_SUCCESS;
}
