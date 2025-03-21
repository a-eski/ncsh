/* Copyright ncsh by Alex Eski 2025 */

#include "ncsh_vm_tokenizer.h"

#include <assert.h>
#include <unistd.h>

#include "../ncsh_defines.h"
#include "../ncsh_parser.h"

[[nodiscard]]
int_fast32_t ncsh_vm_tokenizer_syntax_error(const char* const message, const size_t message_length)
{
    if (write(STDIN_FILENO, message, message_length) == -1) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    return NCSH_COMMAND_SYNTAX_ERROR;
}

#define INVALID_SYNTAX_PIPE_FIRST_ARG                                                                                  \
    "ncsh: Invalid syntax: found pipe operator ('|') as first argument. Correct usage of pipe operator is 'program1 "  \
    "| program2'.\n"
#define INVALID_SYNTAX_PIPE_LAST_ARG                                                                                   \
    "ncsh: Invalid syntax: found pipe operator ('|') as last argument. Correct usage of pipe operator is 'program1 | " \
    "program2'.\n"

#define INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG                                                                          \
    "ncsh: Invalid syntax: found output redirection operator ('>') as first argument. Correct usage of output "        \
    "redirection operator is 'program > file'.\n"
#define INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG                                                                           \
    "ncsh: Invalid syntax: found no filename after output redirect operator ('>'). Correct usage of output "           \
    "redirection operator is 'program > file'.\n"

#define INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG                                                                   \
    "ncsh: Invalid syntax: found output redirection append operator ('>>') as first argument. Correct usage of "       \
    "output redirection append operator is 'program >> file'.\n"
#define INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG                                                                    \
    "ncsh: Invalid syntax: found no filename after output redirect append operator ('>>'). Correct usage of output "   \
    "redirection operator is 'program >> file'.\n"

#define INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG                                                                           \
    "ncsh: Invalid syntax: found input redirection operator ('<') as first argument. Correct usage of input "          \
    "redirection operator is 'program < file'.\n"
#define INVALID_SYNTAX_STDIN_REDIR_LAST_ARG                                                                            \
    "ncsh: Invalid syntax: found input redirection operator ('<') as last argument. Correct usage of input "           \
    "redirection operator is 'program < file'.\n"

#define INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG                                                                          \
    "ncsh: Invalid syntax: found error redirection operator ('2>') as first argument. Correct usage of error "         \
    "redirection is 'program 2> file'.\n"
#define INVALID_SYNTAX_STDERR_REDIR_LAST_ARG                                                                           \
    "ncsh: Invalid syntax: found error redirection operator ('2>') as last argument. Correct usage of error "          \
    "redirection is 'program 2> file'.\n"

#define INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG                                                                   \
    "ncsh: Invalid syntax: found error redirection append operator ('2>>') as first argument. Correct usage of error " \
    "redirection is 'program 2>> file'.\n"
#define INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG                                                                    \
    "ncsh: Invalid syntax: found error redirection operator ('2>>') as last argument. Correct usage of error "         \
    "redirection is 'program 2>> file'.\n"

#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_FIRST_ARG                                                               \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>') as first argument. Correct usage of "      \
    "output & error redirection is 'program &> file'.\n"
#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_LAST_ARG                                                                \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>') as last argument. Correct usage of "       \
    "output & error redirection is 'program &> file'.\n"

#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_FIRST_ARG                                                        \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>>') as first argument. Correct usage of "     \
    "output & error redirection is 'program &>> file'.\n"
#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_LAST_ARG                                                         \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>>') as last argument. Correct usage of "      \
    "output & error redirection is 'program &>> file'.\n"

#define INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG                                                                        \
    "ncsh: Invalid syntax: found background job operator ('&') as first argument. Correct usage of background job "    \
    "operator is 'program &'.\n"
#define INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG                                                                     \
    "ncsh: Invalid syntax: found background job operator ('&') in position other than last argument. Correct usage "   \
    "of background job operator is 'program &'.\n"

#define INVALID_SYNTAX(message) ncsh_vm_tokenizer_syntax_error(message, sizeof(message) - 1)

[[nodiscard]]
int_fast32_t ncsh_vm_args_syntax_check(const struct ncsh_Args* const restrict args)
{
    assert(args);

    switch (args->ops[0]) {
    case OP_PIPE: {
        return INVALID_SYNTAX(INVALID_SYNTAX_PIPE_FIRST_ARG);
    }
    case OP_STDOUT_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG);
    }
    case OP_STDOUT_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG);
    }
    case OP_STDIN_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG);
    }
    case OP_STDERR_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG);
    }
    case OP_STDERR_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG);
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_FIRST_ARG);
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_FIRST_ARG);
    }
    case OP_BACKGROUND_JOB: {
        return INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG);
    }
    }

    switch (args->ops[args->count - 1]) {
    case OP_PIPE: {
        return INVALID_SYNTAX(INVALID_SYNTAX_PIPE_LAST_ARG);
    }
    case OP_STDOUT_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG);
    }
    case OP_STDOUT_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG);
    }
    case OP_STDIN_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDIN_REDIR_LAST_ARG);
    }
    case OP_STDERR_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_LAST_ARG);
    }
    case OP_STDERR_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG);
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_LAST_ARG);
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_LAST_ARG);
    }
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int_fast32_t ncsh_vm_tokenizer_tokenize(const struct ncsh_Args* const restrict args,
                                        struct ncsh_Tokens* const restrict tokens)
{
    assert(args);
    assert(tokens);

    int_fast32_t syntax_check;
    if ((syntax_check = ncsh_vm_args_syntax_check(args)) != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return syntax_check;
    }

    tokens->is_background_job = false;
    for (uint_fast32_t i = 0; i < args->count; ++i) {
        switch (args->ops[i]) {
        case OP_STDOUT_REDIRECTION: {
            tokens->stdout_file = args->values[i + 1];
            tokens->stdout_redirect_index = i;
            break;
        }
        case OP_STDOUT_REDIRECTION_APPEND: {
            tokens->stdout_file = args->values[i + 1];
            tokens->stdout_redirect_index = i;
            tokens->output_append = true;
            break;
        }
        case OP_STDIN_REDIRECTION: {
            tokens->stdin_file = args->values[i + 1];
            tokens->stdin_redirect_index = i;
            break;
        }
        case OP_STDERR_REDIRECTION: {
            tokens->stderr_file = args->values[i + 1];
            tokens->stderr_redirect_index = i;
            break;
        }
        case OP_STDERR_REDIRECTION_APPEND: {
            tokens->stderr_file = args->values[i + 1];
            tokens->stderr_redirect_index = i;
            tokens->output_append = true;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION: {
            tokens->stdout_and_stderr_file = args->values[i + 1];
            tokens->stdout_and_stderr_redirect_index = i;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
            tokens->stdout_and_stderr_file = args->values[i + 1];
            tokens->stdout_and_stderr_redirect_index = i;
            tokens->output_append = true;
            break;
        }
        case OP_PIPE: {
            ++tokens->number_of_pipe_commands;
            break;
        }
        case OP_BACKGROUND_JOB: {
            if (i != args->count - 1) {
                return INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG);
            }
            tokens->is_background_job = true;
        }
        }
    }
    ++tokens->number_of_pipe_commands;

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}
