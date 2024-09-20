#ifndef ncsh_commands_h
#define ncsh_commands_h

#include <stdint.h>

#define PIPE_KEY '|'
#define OUTPUT_REDIRECTION_KEY '>'
#define OUTPUT_REDIRECTION_APPEND_KEY '>>'
#define INPUT_REDIRECTION_KEY '<'
#define ERROR_REDIRECTION_KEY '2>'
#define ERROR_REDIRECTION_APPEND_KEY '2>>'
#define BACKGROUND_JOB_KEY '&'

enum ncsh_Commands
{
	PIPE = 0,
	OUTPUT_REDIRECTION = 1,
	OUTPUT_REDIRECTION_APPEND = 2,
	INPUT_REDIRECTION = 3,
	ERROR_REDIRECTION = 4,
	ERROR_REDIRECTION_APPEND = 5,
	BACKGROUND_JOB = 6
};

uint_fast32_t ncsh_execute_command(struct ncsh_Args args);

void ncsh_history_init(void);
void ncsh_history_add(char* line, uint_fast32_t length);

#endif // !ncsh_commands_h
