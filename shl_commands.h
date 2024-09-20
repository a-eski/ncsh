#ifndef shl_commands_h
#define shl_commands_h

#include <stdint.h>

#define PIPE_KEY '|'
#define OUTPUT_REDIRECTION_KEY '>'
#define OUTPUT_REDIRECTION_APPEND_KEY '>>'
#define INPUT_REDIRECTION_KEY '<'
#define ERROR_REDIRECTION_KEY '2>'
#define ERROR_REDIRECTION_APPEND_KEY '2>>'
#define BACKGROUND_JOB_KEY '&'

enum shl_Commands
{
	PIPE = 0,
	OUTPUT_REDIRECTION = 1,
	OUTPUT_REDIRECTION_APPEND = 2,
	INPUT_REDIRECTION = 3,
	ERROR_REDIRECTION = 4,
	ERROR_REDIRECTION_APPEND = 5,
	BACKGROUND_JOB = 6
};

uint_fast32_t shl_execute_command(struct shl_Args args);

void shl_history_init(void);
void shl_history_add(char* line, uint_fast32_t length);

#endif // !shl_commands_h
