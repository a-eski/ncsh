/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "../defines.h"
#include "../parser/args.h"

int tok_invalid_syntax_check_res;

[[nodiscard]]
int syntax_validator_error_write(char* rst message, size_t message_length)
{
    if (write(STDIN_FILENO, message, message_length) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SYNTAX_ERROR;
}

#define INVALID_SYNTAX(message) syntax_validator_error_write(message, sizeof(message) - 1)

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

#define INVALID_SYNTAX_AND_IN_LAST_ARG                                                                                 \
    "ncsh: Invalid syntax: found and operator ('&&') as last argument. Correct usage of and operator is "              \
    "'true && true'\n"
#define INVALID_SYNTAX_AND_IN_FIRST_ARG                                                                                \
    "ncsh: Invalid syntax: found and operator ('&&') as first argument. Correct usage of and operator is "             \
    "'true && true'\n"

#define INVALID_SYNTAX_OR_IN_LAST_ARG                                                                                  \
    "ncsh: Invalid syntax: found or operator ('||') as last argument. Correct usage of or operator is "                \
    "'false || true'\n"
#define INVALID_SYNTAX_OR_IN_FIRST_ARG                                                                                 \
    "ncsh: Invalid syntax: found or operator ('||') as first argument. Correct usage of or operator is "               \
    "'false || true'\n"

/* syntax_validator_check_first_arg
 * Simple check to see if something is in first position that shouldn't be
 */
void syntax_validatator_first_arg_check(uint8_t op)
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
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_AND_IN_FIRST_ARG);
        break;
    }
    case OP_OR: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_OR_IN_FIRST_ARG);
        break;
    }
    }
}

/* syntax_validator_check_last_arg
 * Simple check to see if something is in last position that shouldn't be
 */
void syntax_validator_last_arg_check(Args* rst args)
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
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_AND_IN_LAST_ARG);
        break;
    }
    case OP_OR: {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_OR_IN_LAST_ARG);
        break;
    }
    }
}

#define INVALID_SYNTAX_NO_ARGS "ncsh: Invalid Syntax: no values passes to syntax validation.\n"

#define INVALID_SYNTAX_IF_NO_NEXT_ARG                                                                                  \
    "ncsh: Invalid syntax: found 'if' with no value after. "                                                           \
    "Correct usage of 'if' is 'if [(CONDITION)]; then [STATEMENT]; [else [STATEMENT];] fi'.\n"
#define INVALID_SYNTAX_IF_NO_START_CONDITION                                                                           \
    "ncsh: Invalid syntax: found 'if' with condition after. "                                                          \
    "Correct usage of 'if' is 'if [(CONDITION)]; then [STATEMENT]; [else [STATEMENT];] fi'.\n"

#define INVALID_SYNTAX_CONDITION_START_NO_NEXT_ARG                                                                     \
    "ncsh: Invalid Syntax: expecting expression after 'if ['. "                                                        \
    "Correct usage of 'if' is 'if [(CONDITION)]; then [STATEMENT]; [else [STATEMENT];] fi'.\n"

#define INVALID_SYNTAX_CONDITION_END_NO_NEXT_ARG                                                                       \
    "ncsh: Invalid Syntax: expecting values after 'if [(CONDITION)];'. "                                               \
    "Correct usage of 'if' is 'if [(CONDITION)]; then [STATEMENT]; [else [STATEMENT];] fi'.\n"
#define INVALID_SYNTAX_CONDITION_END_NO_NEXT_THEN                                                                      \
    "ncsh: Invalid Syntax: expecting 'then' after 'if [(CONDITION)];'. "                                               \
    "Correct usage of 'if' is 'if [(CONDITION)]; then [STATEMENT]; [else [STATEMENT];] fi'.\n"

#define INVALID_SYNTAX_THEN_NO_NEXT_ARG                                                                                \
    "ncsh: Invalid Syntax: expecting some value after 'if [(CONDITION)]; then'. "                                      \
    "Correct usage of 'if' is 'if [(CONDITION)]; then [STATEMENT]; [else [STATEMENT];] fi'.\n"
#define INVALID_SYNTAX_THEN_NO_NEXT_STATEMENT                                                                          \
    "ncsh: Invalid Syntax: expecting some statement after 'if [(CONDITION)]; then'. "                                  \
    "Correct usage of 'if' is 'if [(CONDITION)]; then [STATEMENT]; [else [STATEMENT];] fi'.\n"

#define INVALID_SYNTAX_ELSE_NO_NEXT_ARG                                                                                \
    "ncsh: Invalid Syntax: expecting some value after 'if [(CONDITION)]; then (STATEMENT); else'. "                    \
    "Correct usage of 'if' is 'if [(CONDITION)]; then [STATEMENT]; [else [STATEMENT];] fi'.\n"
#define INVALID_SYNTAX_ELSE_NO_NEXT_STATEMENT                                                                          \
    "ncsh: Invalid Syntax: expecting some statement after 'if [(CONDITION)]; then (STATEMENT); else'. "                \
    "Correct usage of 'if' is 'if [(CONDITION)]; then [STATEMENT]; [else [STATEMENT];] fi'.\n"

void syntax_validator_check(Args* rst args)
{
    Arg* arg = args->head->next;
    if (!arg) {
        tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_NO_ARGS);
        return;
    }
    Arg* prev = NULL;

    for (size_t i = 0; i < args->count; ++i) {
        switch (arg->op) {

        case OP_BACKGROUND_JOB: {
            if (arg->next) {
                tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_NOT_LAST_ARG);
                return;
            }
            break;
        }

        case OP_IF: {
            if (!arg->next) {
                tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_IF_NO_NEXT_ARG);
                return;
            }
            if (arg->next->op != OP_CONDITION_START) {
                tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_IF_NO_START_CONDITION);
            }
            break;
        }

        case OP_CONDITION_START: {
            if (!prev)
                break;
            if (!arg->next) {
                tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_CONDITION_START_NO_NEXT_ARG);
                return;
            }
            // OP_STATEMENT check
            break;
        }

        case OP_CONDITION_END: {
            if (!prev)
                break;
            if (!arg->next) {
                tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_CONDITION_END_NO_NEXT_ARG);
                return;
            }
            if (arg->next->op != OP_THEN) {
                tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_CONDITION_END_NO_NEXT_THEN);
                return;
            }
            break;
        }

        case OP_THEN: {
            if (!prev)
                break;
            if (!arg->next) {
                tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_THEN_NO_NEXT_ARG);
                return;
            }
            if (arg->next->op != OP_CONSTANT) { // OP_STATEMENT for logic statements instead of using constant?
                tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_THEN_NO_NEXT_STATEMENT);
                return;
            }
            break;
        }

        case OP_ELSE: {
            if (!prev)
                break;
            if (!arg->next) {
                tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_ELSE_NO_NEXT_ARG);
                return;
            }
            if (arg->next->op != OP_CONSTANT) { // OP_STATEMENT for logic statements instead of using constant?
                tok_invalid_syntax_check_res = INVALID_SYNTAX(INVALID_SYNTAX_ELSE_NO_NEXT_STATEMENT);
                return;
            }
            break;
        }
        }
        prev = arg;
        arg = arg->next;
        if (!arg)
            break;
    }
}

[[nodiscard]]
int syntax_validator_validate(Args* rst args)
{
    assert(args);

    tok_invalid_syntax_check_res = EXIT_SUCCESS;
    syntax_validatator_first_arg_check(args->head->next->op);
    if (tok_invalid_syntax_check_res != EXIT_SUCCESS)
        return tok_invalid_syntax_check_res;
    syntax_validator_last_arg_check(args);
    if (tok_invalid_syntax_check_res != EXIT_SUCCESS)
        return tok_invalid_syntax_check_res;
    syntax_validator_check(args);
    return tok_invalid_syntax_check_res;
}
