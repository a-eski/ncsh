#ifndef ncsh_commands_h
#define ncsh_commands_h

#include <stdint.h>

void ncsh_history_init(void);

void ncsh_history_add(char* line, uint_fast32_t length);

uint_fast32_t ncsh_execute_command(struct ncsh_Args args);

#endif // !ncsh_commands_h
