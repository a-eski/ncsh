/* Copyright ncsh (C) by Alex Eski 2025 */

#include "vm_tokenizer.h"

#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "../defines.h"
#include "../parser.h"
#include "../types.h"

[[nodiscard]]
int vm_tokenizer_syntax_error(const char* const restrict message, const size_t message_length)
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

#define INVALID_SYNTAX_AND_IN_LAST_POSITION                                                                            \
    "ncsh: Invalid syntax: found and operator ('&&') as last argument. Correct usage of and operator is "              \
    "'true && true'\n"
#define INVALID_SYNTAX_AND_IN_FIRST_POSITION                                                                           \
    "ncsh: Invalid syntax: found and operator ('&&') as first argument. Correct usage of and operator is "             \
    "'true && true'\n"

#define INVALID_SYNTAX_OR_IN_LAST_POSITION                                                                             \
    "ncsh: Invalid syntax: found or operator ('||') as last argument. Correct usage of or operator is "                \
    "'false || true'\n"
#define INVALID_SYNTAX_OR_IN_FIRST_POSITION                                                                            \
    "ncsh: Invalid syntax: found or operator ('||') as first argument. Correct usage of or operator is "               \
    "'false || true'\n"

#define INVALID_SYNTAX(message) vm_tokenizer_syntax_error(message, sizeof(message) - 1)

int vmtok_invalid_syntax_check_res;

/* vm_tokenizer_syntax_check_first_arg
 * Simple check to see if something is in first position that shouldn't be
 */
void vm_tokenizer_syntax_check_first_arg(uint8_t op)
{
    switch (op) {
    case OP_PIPE: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_PIPE_FIRST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG);
        break;
    }
    case OP_STDIN_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_FIRST_ARG);
        break;
    }
    case OP_BACKGROUND_JOB: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG);
        break;
    }
    case OP_AND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_AND_IN_FIRST_POSITION);
        break;
    }
    case OP_OR: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_OR_IN_FIRST_POSITION);
        break;
    }
    }
}

/* vm_tokenizer_syntax_check_last_arg
 * Simple check to see if something is in last position that shouldn't be
 */
void vm_tokenizer_syntax_check_last_arg(uint8_t op)
{
    switch (op) {
    case OP_PIPE: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_PIPE_LAST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG);
        break;
    }
    case OP_STDIN_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDIN_REDIR_LAST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_LAST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_LAST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_LAST_ARG);
        break;
    }
    case OP_AND: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_AND_IN_LAST_POSITION);
        break;
    }
    case OP_OR: {
        vmtok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_OR_IN_LAST_POSITION);
        break;
    }
    }
}

[[nodiscard]]
int vm_tokenizer_syntax_check(const struct Args* const restrict args)
{
    assert(args);

    vmtok_invalid_syntax_check_res = NCSH_COMMAND_SUCCESS_CONTINUE;
    vm_tokenizer_syntax_check_first_arg(args->ops[0]);
    if (vmtok_invalid_syntax_check_res != NCSH_COMMAND_SUCCESS_CONTINUE)
        return vmtok_invalid_syntax_check_res;
    vm_tokenizer_syntax_check_last_arg(args->ops[args->count - 1]);
    return vmtok_invalid_syntax_check_res;
}

int vm_tokenizer_ops_process(struct Shell* const restrict shell, struct Tokens* const restrict tokens,
                           struct Arena* const restrict scratch_arena)
{
    assert(shell);
    struct Args args = shell->args;
    assert(tokens);
    assert(scratch_arena);

    tokens->is_background_job = false;
    for (uint8_t i = 0; i < args.count; ++i) {
        switch (args.ops[i]){
        case OP_STDOUT_REDIRECTION: {
            tokens->stdout_file = args.values[i + 1];
            tokens->stdout_redirect_index = i;
            break;
        }
        case OP_STDOUT_REDIRECTION_APPEND: {
            tokens->stdout_file = args.values[i + 1];
            tokens->stdout_redirect_index = i;
            tokens->output_append = true;
            break;
        }
        case OP_STDIN_REDIRECTION: {
            tokens->stdin_file = args.values[i + 1];
            tokens->stdin_redirect_index = i;
            break;
        }
        case OP_STDERR_REDIRECTION: {
            tokens->stderr_file = args.values[i + 1];
            tokens->stderr_redirect_index = i;
            break;
        }
        case OP_STDERR_REDIRECTION_APPEND: {
            tokens->stderr_file = args.values[i + 1];
            tokens->stderr_redirect_index = i;
            tokens->output_append = true;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION: {
            tokens->stdout_and_stderr_file = args.values[i + 1];
            tokens->stdout_and_stderr_redirect_index = i;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
            tokens->stdout_and_stderr_file = args.values[i + 1];
            tokens->stdout_and_stderr_redirect_index = i;
            tokens->output_append = true;
            break;
        }
        case OP_PIPE: {
            ++tokens->number_of_pipe_commands;
            break;
        }
        case OP_BACKGROUND_JOB: {
            if (i != args.count - 1) {
                return INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG);
            }
            tokens->is_background_job = true;
            break;
        }
        case OP_HOME_EXPANSION: {
            size_t len = shell->config.home_location.length;
            args.values[i] = arena_malloc(scratch_arena, len, char);
            memcpy(args.values[i], shell->config.home_location.value, len);
        }
        }
    }
    ++tokens->number_of_pipe_commands;

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int vm_tokenizer_tokenize(struct Shell* const restrict shell, struct Tokens* const restrict tokens,
                          struct Arena* const restrict scratch_arena)
{
    assert(shell);
    assert(tokens);
    assert(scratch_arena);

    int result;
    if ((result = vm_tokenizer_syntax_check(&shell->args)) != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return result;
    }

    if ((result = vm_tokenizer_ops_process(shell, tokens, scratch_arena)) != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return result;
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}
