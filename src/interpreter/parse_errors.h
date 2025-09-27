#pragma once

#include <stddef.h>
#include "../defines.h"
#include "../ttyio/ttyio.h"

[[nodiscard]]
static inline int sema_error_write(char* restrict message, size_t message_length)
{
    tty_dwriteln(STDERR_FILENO, message, message_length);

    return EXIT_SYNTAX_ERROR;
}

#define INVALID_SYNTAX(message) sema_error_write(message, sizeof(message) - 1)

#define INVALID_SYNTAX_PIPE_FIRST_ARG                                                                                  \
    "found pipe operator ('|') as first argument. Correct usage of pipe operator is 'program1 "  \
    "| program2'."
#define INVALID_SYNTAX_PIPE_LAST_ARG                                                                                   \
    "found pipe operator ('|') as last argument. Correct usage of pipe operator is 'program1 | " \
    "program2'."

#define INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG                                                                          \
    "found output redirection operator ('>') as first argument. Correct usage of output "        \
    "redirection operator is 'program > file'."
#define INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG                                                                           \
    "found no filename after output redirect operator ('>'). Correct usage of output "           \
    "redirection operator is 'program > file'."

#define INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG                                                                   \
    "found output redirection append operator ('>>') as first argument. Correct usage of "       \
    "output redirection append operator is 'program >> file'."
#define INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG                                                                    \
    "found no filename after output redirect append operator ('>>'). Correct usage of output "   \
    "redirection operator is 'program >> file'."

#define INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG                                                                           \
    "found input redirection operator ('<') as first argument. Correct usage of input "          \
    "redirection operator is 'program < file'."
#define INVALID_SYNTAX_STDIN_REDIR_LAST_ARG                                                                            \
    "found input redirection operator ('<') as last argument. Correct usage of input "           \
    "redirection operator is 'program < file'."

#define INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG                                                                          \
    "found error redirection operator ('2>') as first argument. Correct usage of error "         \
    "redirection is 'program 2> file'."
#define INVALID_SYNTAX_STDERR_REDIR_LAST_ARG                                                                           \
    "found error redirection operator ('2>') as last argument. Correct usage of error "          \
    "redirection is 'program 2> file'."

#define INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG                                                                   \
    "found error redirection append operator ('2>>') as first argument. Correct usage of error " \
    "redirection is 'program 2>> file'."
#define INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG                                                                    \
    "found error redirection operator ('2>>') as last argument. Correct usage of error "         \
    "redirection is 'program 2>> file'."

#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_FIRST_ARG                                                               \
    "found output & error redirection operator ('&>') as first argument. Correct usage of "      \
    "output & error redirection is 'program &> file'."
#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_LAST_ARG                                                                \
    "found output & error redirection operator ('&>') as last argument. Correct usage of "       \
    "output & error redirection is 'program &> file'."

#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_FIRST_ARG                                                        \
    "found output & error redirection operator ('&>>') as first argument. Correct usage of "     \
    "output & error redirection is 'program &>> file'."
#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_LAST_ARG                                                         \
    "found output & error redirection operator ('&>>') as last argument. Correct usage of "      \
    "output & error redirection is 'program &>> file'."

#define INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG                                                                        \
    "found background job operator ('&') as first argument. Correct usage of background job "    \
    "operator is 'program &'."
#define INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG                                                                     \
    "found background job operator ('&') in position other than last argument. Correct usage "   \
    "of background job operator is 'program &'."

#define INVALID_SYNTAX_AND_IN_LAST_ARG                                                                                 \
    "found and operator ('&&') as last argument. Correct usage of and operator is "              \
    "'true && true'"
#define INVALID_SYNTAX_AND_IN_FIRST_ARG                                                                                \
    "found and operator ('&&') as first argument. Correct usage of and operator is "             \
    "'true && true'"

#define INVALID_SYNTAX_OR_IN_LAST_ARG                                                                                  \
    "found or operator ('||') as last argument. Correct usage of or operator is "                \
    "'false || true'"
#define INVALID_SYNTAX_OR_IN_FIRST_ARG                                                                                 \
    "found or operator ('||') as first argument. Correct usage of or operator is "               \
    "'false || true'"
