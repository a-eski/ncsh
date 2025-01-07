#include <stdint.h>
#include <unistd.h>

#include "ncsh_parser.h"
#include "ncsh_tokenizer.h"
#include "ncsh_defines.h"

int_fast32_t ncsh_tokenizer_syntax_error(const char* message, size_t message_length) {
	if (write(STDIN_FILENO, message, message_length) == -1)
		return NCSH_COMMAND_EXIT_FAILURE;

	return NCSH_COMMAND_SYNTAX_ERROR;
}

int_fast32_t ncsh_tokenizer_tokenize(struct ncsh_Args* args, struct ncsh_Tokens* tokens) {
	if (args->ops[0] == OP_PIPE)
		return ncsh_tokenizer_syntax_error("ncsh: Invalid syntax: found pipe ('|') as first argument. Correct usage of pipes is 'ls | sort'.\n", 97);
	else if (args->ops[args->count - 1] == OP_PIPE)
		return ncsh_tokenizer_syntax_error("ncsh: Invalid syntax: found pipe ('|') as last argument. Correct usage of pipes is 'ls | sort'.\n", 96);

	for (uint_fast32_t i = 0; i < args->count; ++i) {
		if (args->ops[i] == OP_STDOUT_REDIRECTION) {
			if (i + 1 >= args->count) {
				return ncsh_tokenizer_syntax_error("ncsh: Invalid syntax: found no filename after output redirect symbol '>'.\n", 74);
			}

			tokens->output_file = args->values[i + 1];
			tokens->stdout_redirect_found_index = i;
		}
		else if (args->ops[i] == OP_STDIN_REDIRECTION) {
			if (i == 0 || i + 1 >= args->count) {
				return ncsh_tokenizer_syntax_error("ncsh: Invalid syntax: found no filename before input redirect symbol '<'.\n", 74);
			}

			tokens->input_file = args->values[i + 1];
			tokens->stdin_redirect_found_index = i;
		}
		else if (args->ops[i] == OP_PIPE) {
			++tokens->number_of_pipe_commands;
		}
	}
	++tokens->number_of_pipe_commands;

	if (tokens->stdout_redirect_found_index && tokens->stdin_redirect_found_index) {
		return ncsh_tokenizer_syntax_error("ncsh: Invalid syntax: found both input and output redirects symbols ('<' and '>', respectively).\n", 97);
	}

	return NCSH_COMMAND_SUCCESS_CONTINUE;
}

