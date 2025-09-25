/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "../ttyio/ttyio.h"
#include "../defines.h"
#include "lex.h"
#include "stmts.h"
#include "ops.h"

[[nodiscard]]
int sema_error_write(char* restrict message, size_t message_length)
{
    tty_dwriteln(STDERR_FILENO, message, message_length);

    return EXIT_SYNTAX_ERROR;
}

#define INVALID_SYNTAX(message) sema_error_write(message, sizeof(message) - 1)

#define INVALID_SYNTAX_PIPE_FIRST_ARG                                                                                  \
    "ncsh: Invalid syntax: found pipe operator ('|') as first argument. Correct usage of pipe operator is 'program1 "  \
    "| program2'."
#define INVALID_SYNTAX_PIPE_LAST_ARG                                                                                   \
    "ncsh: Invalid syntax: found pipe operator ('|') as last argument. Correct usage of pipe operator is 'program1 | " \
    "program2'."

#define INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG                                                                          \
    "ncsh: Invalid syntax: found output redirection operator ('>') as first argument. Correct usage of output "        \
    "redirection operator is 'program > file'."
#define INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG                                                                           \
    "ncsh: Invalid syntax: found no filename after output redirect operator ('>'). Correct usage of output "           \
    "redirection operator is 'program > file'."

#define INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG                                                                   \
    "ncsh: Invalid syntax: found output redirection append operator ('>>') as first argument. Correct usage of "       \
    "output redirection append operator is 'program >> file'."
#define INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG                                                                    \
    "ncsh: Invalid syntax: found no filename after output redirect append operator ('>>'). Correct usage of output "   \
    "redirection operator is 'program >> file'."

#define INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG                                                                           \
    "ncsh: Invalid syntax: found input redirection operator ('<') as first argument. Correct usage of input "          \
    "redirection operator is 'program < file'."
#define INVALID_SYNTAX_STDIN_REDIR_LAST_ARG                                                                            \
    "ncsh: Invalid syntax: found input redirection operator ('<') as last argument. Correct usage of input "           \
    "redirection operator is 'program < file'."

#define INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG                                                                          \
    "ncsh: Invalid syntax: found error redirection operator ('2>') as first argument. Correct usage of error "         \
    "redirection is 'program 2> file'."
#define INVALID_SYNTAX_STDERR_REDIR_LAST_ARG                                                                           \
    "ncsh: Invalid syntax: found error redirection operator ('2>') as last argument. Correct usage of error "          \
    "redirection is 'program 2> file'."

#define INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG                                                                   \
    "ncsh: Invalid syntax: found error redirection append operator ('2>>') as first argument. Correct usage of error " \
    "redirection is 'program 2>> file'."
#define INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG                                                                    \
    "ncsh: Invalid syntax: found error redirection operator ('2>>') as last argument. Correct usage of error "         \
    "redirection is 'program 2>> file'."

#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_FIRST_ARG                                                               \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>') as first argument. Correct usage of "      \
    "output & error redirection is 'program &> file'."
#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_LAST_ARG                                                                \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>') as last argument. Correct usage of "       \
    "output & error redirection is 'program &> file'."

#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_FIRST_ARG                                                        \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>>') as first argument. Correct usage of "     \
    "output & error redirection is 'program &>> file'."
#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_LAST_ARG                                                         \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>>') as last argument. Correct usage of "      \
    "output & error redirection is 'program &>> file'."

#define INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG                                                                        \
    "ncsh: Invalid syntax: found background job operator ('&') as first argument. Correct usage of background job "    \
    "operator is 'program &'."
#define INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG                                                                     \
    "ncsh: Invalid syntax: found background job operator ('&') in position other than last argument. Correct usage "   \
    "of background job operator is 'program &'."

#define INVALID_SYNTAX_AND_IN_LAST_ARG                                                                                 \
    "ncsh: Invalid syntax: found and operator ('&&') as last argument. Correct usage of and operator is "              \
    "'true && true'"
#define INVALID_SYNTAX_AND_IN_FIRST_ARG                                                                                \
    "ncsh: Invalid syntax: found and operator ('&&') as first argument. Correct usage of and operator is "             \
    "'true && true'"

#define INVALID_SYNTAX_OR_IN_LAST_ARG                                                                                  \
    "ncsh: Invalid syntax: found or operator ('||') as last argument. Correct usage of or operator is "                \
    "'false || true'"
#define INVALID_SYNTAX_OR_IN_FIRST_ARG                                                                                 \
    "ncsh: Invalid syntax: found or operator ('||') as first argument. Correct usage of or operator is "               \
    "'false || true'"

/* sema_check_first_arg
 * Simple check to see if something is in first position that shouldn't be
 */
int syntax_validatator_first_arg_check(uint8_t op)
{
    switch (op) {
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
    case OP_AND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_AND_IN_FIRST_ARG);
    }
    case OP_OR: {
        return INVALID_SYNTAX(INVALID_SYNTAX_OR_IN_FIRST_ARG);
    }
    default: {
        return EXIT_SUCCESS;
    }
    }
}

/* sema_check_last_arg
 * Simple check to see if something is in last position that shouldn't be
 */
int sema_last_arg_check(Lexemes* restrict lexemes)
{
    switch (lexemes->ops[lexemes->count - 1]) {
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
    case OP_AND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_AND_IN_LAST_ARG);
    }
    case OP_OR: {
        return INVALID_SYNTAX(INVALID_SYNTAX_OR_IN_LAST_ARG);
    }
    default: {
        return EXIT_SUCCESS;
    }
    }
}

#define INVALID_SYNTAX_NO_ARGS "ncsh: Invalid Syntax: no values passes to syntax validation."

#define INVALID_SYNTAX_IF_NO_NEXT_ARG                                                                                  \
    "ncsh: Invalid syntax: found 'if' with no value after. "                                                           \
    "Correct usage of 'if' is 'if [ (CONDITION) ]; then (STATEMENT); [else [STATEMENT];] fi'."
#define INVALID_SYNTAX_IF_NO_START_CONDITION                                                                           \
    "ncsh: Invalid syntax: found 'if' with no condition after. "                                                          \
    "Correct usage of 'if' is 'if [ (CONDITION) ]; then (STATEMENT); [else [ STATEMENT ];] fi'."

#define INVALID_SYNTAX_CONDITION_START_NO_NEXT_ARG                                                                     \
    "ncsh: Invalid Syntax: expecting expression after 'if ['. "                                                        \
    "Correct usage of 'if' is 'if [ (CONDITION) ]; then (STATEMENT); [else [STATEMENT];] fi'."

#define INVALID_SYNTAX_CONDITION_END_NO_NEXT_ARG                                                                       \
    "ncsh: Invalid Syntax: expecting values after 'if [ (CONDITION) ];'. "                                               \
    "Correct usage of 'if' is 'if [ (CONDITION) ]; then (STATEMENT); [else [STATEMENT];] fi'."
#define INVALID_SYNTAX_CONDITION_END_NO_NEXT_THEN                                                                      \
    "ncsh: Invalid Syntax: expecting 'then' after 'if [ (CONDITION) ];'. "                                               \
    "Correct usage of 'if' is 'if [ (CONDITION) ]; then (STATEMENT); [else [STATEMENT];] fi'."

#define INVALID_SYNTAX_THEN_NO_NEXT_ARG                                                                                \
    "ncsh: Invalid Syntax: expecting some value after 'if [ (CONDITION) ]; then'. "                                      \
    "Correct usage of 'if' is 'if [ (CONDITION) ]; then (STATEMENT); [else [STATEMENT];] fi'."
#define INVALID_SYNTAX_THEN_NO_NEXT_STATEMENT                                                                          \
    "ncsh: Invalid Syntax: expecting some statement after 'if [ (CONDITION) ]; then'. "                                  \
    "Correct usage of 'if' is 'if [ (CONDITION) ]; then (STATEMENT); [else [STATEMENT];] fi'."

#define INVALID_SYNTAX_ELSE_NO_NEXT_ARG                                                                                \
    "ncsh: Invalid Syntax: expecting some value after 'if [ (CONDITION) ]; then (STATEMENT); else'. "                    \
    "Correct usage of 'if' is 'if [ (CONDITION) ]; then (STATEMENT); [else [STATEMENT];] fi'."
#define INVALID_SYNTAX_ELSE_NO_NEXT_STATEMENT                                                                          \
    "ncsh: Invalid Syntax: expecting some statement after 'if [ (CONDITION) ]; then (STATEMENT); else'. "                \
    "Correct usage of 'if' is 'if [ (CONDITION) ]; then (STATEMENT); [else [STATEMENT];] fi'."

int sema_check(Lexemes* restrict lexemes)
{
    if (!lexemes) {
        return INVALID_SYNTAX(INVALID_SYNTAX_NO_ARGS);
    }

    for (size_t i = 0; i < lexemes->count; ++i) {
        switch (lexemes->ops[i]) {

        case OP_BACKGROUND_JOB: {
            if (i < lexemes->count - 1) {
                return INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG);
            }
            break;
        }

        case OP_IF: {
            if (i >= lexemes->count - 1) {
                return INVALID_SYNTAX(INVALID_SYNTAX_IF_NO_NEXT_ARG);
            }
            if (lexemes->ops[i + 1] != OP_CONDITION_START) {
                return INVALID_SYNTAX(INVALID_SYNTAX_IF_NO_START_CONDITION);
            }
            break;
        }

        case OP_CONDITION_START: {
            if (!i)
                break;
            if (i >= lexemes->count - 1) {
                return INVALID_SYNTAX(INVALID_SYNTAX_CONDITION_START_NO_NEXT_ARG);
            }
            // OP_STATEMENT check
            break;
        }

        case OP_CONDITION_END: {
            if (!i)
                break;
            if (i >= lexemes->count - 1) {
                return INVALID_SYNTAX(INVALID_SYNTAX_CONDITION_END_NO_NEXT_ARG);
            }
            if (lexemes->ops[i + 1] != OP_THEN) {
                return INVALID_SYNTAX(INVALID_SYNTAX_CONDITION_END_NO_NEXT_THEN);
            }
            break;
        }

        case OP_THEN: {
            if (!i)
                break;
            if (i >= lexemes->count - 1) {
                return INVALID_SYNTAX(INVALID_SYNTAX_THEN_NO_NEXT_ARG);
            }
            if (lexemes->ops[i + 1] != OP_CONST) { // OP_STATEMENT for logic statements instead of using constant?
                return INVALID_SYNTAX(INVALID_SYNTAX_THEN_NO_NEXT_STATEMENT);
            }
            break;
        }

        case OP_ELSE: {
            if (!i)
                break;
            if (i + 1 == lexemes->count - 1) {
                return INVALID_SYNTAX(INVALID_SYNTAX_ELSE_NO_NEXT_ARG);
            }
            if (lexemes->ops[i + 1] != OP_CONST) { // OP_STATEMENT for logic statements instead of using constant?
                return INVALID_SYNTAX(INVALID_SYNTAX_ELSE_NO_NEXT_STATEMENT);
            }
            break;
        }
        }
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int sema_analyze(Lexemes* restrict lexemes)
{
    assert(lexemes);
    if (!lexemes || !lexemes->count)
        return EXIT_FAILURE_CONTINUE;

    int result = syntax_validatator_first_arg_check(lexemes->ops[0]);
    if (result != EXIT_SUCCESS)
        return result;
    result = sema_last_arg_check(lexemes);
    if (result != EXIT_SUCCESS)
        return result;
    result = sema_check(lexemes);
    return result;
}

[[nodiscard]]
int sema(Statements* restrict stmts)
{
    assert(stmts);
    if (!stmts || !stmts->head)
        return EXIT_FAILURE_CONTINUE;

    return 0;
}
