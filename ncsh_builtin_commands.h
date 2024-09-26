#ifndef ncsh_builtin_commands_h
#define ncsh_builtin_commands_h

#include <stdint.h>

#include "ncsh_args.h"

bool ncsh_is_exit_command(struct ncsh_Args args);

uint_fast32_t ncsh_echo_command(struct ncsh_Args args);

uint_fast32_t ncsh_help_command(void);

uint_fast32_t ncsh_cd_command(struct ncsh_Args args);

void ncsh_history_malloc(void);
void ncsh_history_add(char* line, uint_fast32_t length);
struct ncsh_String ncsh_history_get(uint_fast32_t position);
uint_fast32_t ncsh_history_command(void);

#endif // !ncsh_builtin_commands_h

