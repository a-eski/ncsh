#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "../args.h"
#include "../defines.h"

[[nodiscard]]
int tokenizer_syntax_error(char* rst message, size_t message_length)
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

#define INVALID_SYNTAX(message) tokenizer_syntax_error(message, sizeof(message) - 1)

int tok_invalid_syntax_check_res;

/* tokenizer_syntax_check_first_arg
 * Simple check to see if something is in first position that shouldn't be
 */
void tokenizer_syntax_check_first_arg(uint8_t op)
{
    switch (op) {
    case OP_PIPE: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_PIPE_FIRST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION_APPEND: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG);
        break;
    }
    case OP_STDIN_REDIRECTION: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION_APPEND: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_FIRST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_FIRST_ARG);
        break;
    }
    case OP_BACKGROUND_JOB: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG);
        break;
    }
    case OP_AND: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_AND_IN_FIRST_POSITION);
        break;
    }
    case OP_OR: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_OR_IN_FIRST_POSITION);
        break;
    }
    }
}

/* tokenizer_syntax_check_last_arg
 * Simple check to see if something is in last position that shouldn't be
 */
void tokenizer_syntax_check_last_arg(Args* rst args)
{
    Arg* arg = args->head->next;
    while (arg->next)
        arg = arg->next;

    switch (arg->op) {
    case OP_PIPE: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_PIPE_LAST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG);
        break;
    }
    case OP_STDOUT_REDIRECTION_APPEND: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG);
        break;
    }
    case OP_STDIN_REDIRECTION: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDIN_REDIR_LAST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_LAST_ARG);
        break;
    }
    case OP_STDERR_REDIRECTION_APPEND: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_LAST_ARG);
        break;
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_LAST_ARG);
        break;
    }
    case OP_AND: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_AND_IN_LAST_POSITION);
        break;
    }
    case OP_OR: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_OR_IN_LAST_POSITION);
        break;
    }
    }
}

void tokenizer_syntax_checks(Args* rst args)
{
    Arg* arg = args->head->next->next;
    for (size_t i = 1; i < args->count; ++i) {
        if (arg->op == OP_BACKGROUND_JOB && arg->next) {
            tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG);
            break;
        }
    }
}

[[nodiscard]]
int tokenizer_syntax_check(Args* rst args)
{
    assert(args);

    tok_invalid_syntax_check_res = NCSH_COMMAND_SUCCESS_CONTINUE;
    tokenizer_syntax_check_first_arg(args->head->next->op);
    if (tok_invalid_syntax_check_res != NCSH_COMMAND_SUCCESS_CONTINUE)
        return tok_invalid_syntax_check_res;
    tokenizer_syntax_check_last_arg(args);
    if (tok_invalid_syntax_check_res != NCSH_COMMAND_SUCCESS_CONTINUE)
        return tok_invalid_syntax_check_res;
    tokenizer_syntax_checks(args);
    return tok_invalid_syntax_check_res;
}
