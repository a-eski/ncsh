/* Copyright ncsh by Alex Eski 2024 */

#ifndef ncsh_builtins_h
#define ncsh_builtins_h

#include <stdint.h>

#include "ncsh_args.h"
#include "eskilib/eskilib_string.h"

#ifndef NCSH_TEST_HISTORY
#define NCSH_HISTORY_FILE ".ncsh_history"
#else
#define NCSH_HISTORY_FILE ".ncsh_history_test"
#endif /* ifndef NCSH_TEST_HISTORY */

bool ncsh_is_exit_command(struct ncsh_Args args);

uint_fast32_t ncsh_echo_command(struct ncsh_Args args);

uint_fast32_t ncsh_help_command(void);

bool ncsh_is_cd_command(struct ncsh_Args args);

uint_fast32_t ncsh_cd_command(struct ncsh_Args args);

#endif // !ncsh_builtins_h

