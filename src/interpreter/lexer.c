/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <glob.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../arena.h"
#include "../configurables.h"
#include "../debug.h"
#include "../defines.h"
#include "interpreter_types.h"
#include "lexer_defines.h"
#include "tokens.h"

#define STRCMP(line, expected_lit) line[0] == expected_lit[0] && !memcmp(line, expected_lit, sizeof(expected_lit) - 1)

#define LEXER_TOKENS_LIMIT 128

/* enum Lexer_State
 * Flags used by the lexer to keep track of state, like whether the current character
 * being processed is inside quotes or a mathematical expression.
 */
// clang-format off
enum Lexer_State: size_t {
    IN_NONE =                    0,
    IN_SINGLE_QUOTES =           1 << 0,
    IN_DOUBLE_QUOTES =           1 << 1,
    IN_BACKTICK_QUOTES =         1 << 2,
    IN_MATHEMATICAL_EXPRESSION = 1 << 3,
    IN_ASSIGNMENT =              1 << 4,
    IN_HOME_EXPANSION =          1 << 5,
    IN_GLOB_EXPANSION =          1 << 6,
    IN_COMMENT =                 1 << 7,
    IN_DOLLAR_SIGN =             1 << 8
};
// clang-format on

/* Lexer Internal Variables */
/* These variables used in lexer_lex, they are not accessed anywhere else within the shell,
 * and should not be accessed anywhere else in the shell.
 * They were put here because they increased speed of the lexer in benchmarks.
 */
char* rst lex_buf;
size_t lex_state;
size_t lex_pos;

/* ops_2char_str
 * A constant array that contains all shell operations that are 2 characters long, like ">>", "||", "&&".
 * Size of the array is stored as constant expression in ops_2char_len
 * Bytecodes (opcodes) equivalents are stored in the array of enum Ops, ops_2char
 */
const char* const rst ops_2char_str[] = {STDOUT_REDIRECTION_APPEND,
                                         STDERR_REDIRECTION,
                                         STDOUT_AND_STDERR_REDIRECTION,
                                         AND,
                                         OR,
                                         EXPONENTIATION,
                                         MATH_EXPRESSION_START,
                                         IF,
                                         FI,
                                         CONDITION_END};

constexpr size_t ops_2char_len = sizeof(ops_2char_str) / sizeof(char*);

const enum Ops ops_2char[] = {OP_STDOUT_REDIRECTION_APPEND,
                              OP_STDERR_REDIRECTION,
                              OP_STDOUT_AND_STDERR_REDIRECTION,
                              OP_AND,
                              OP_OR,
                              OP_EXPONENTIATION,
                              OP_MATH_EXPRESSION_START,
                              OP_IF,
                              OP_FI,
                              OP_CONDITION_END};

/* ops_3char_str
 * A constant array that contain all shell operations that are 3 characters long, like "&>>".
 * Size of the array is stored as constant expression in ops_3char_len
 * Bytecodes (opcodes) equivalents are stored in the array of enum Ops, ops_3char
 */
const char* const rst ops_3char_str[] = {STDERR_REDIRECTION_APPEND, STDOUT_AND_STDERR_REDIRECTION_APPEND, EQUALS,
                                         LESS_THAN, GREATER_THAN};

constexpr size_t ops_3char_len = sizeof(ops_3char_str) / sizeof(char*);

const enum Ops ops_3char[] = {OP_STDERR_REDIRECTION_APPEND, OP_STDOUT_AND_STDERR_REDIRECTION_APPEND, OP_EQUALS,
                              OP_LESS_THAN, OP_GREATER_THAN};

[[nodiscard]]
bool lexer_op_check_var(char* line, size_t len)
{
    return len > 2 && line[0] == VARIABLE;
}

[[nodiscard]]
enum Ops lexer_op_check_len_one(char* rst line)
{
    assert(line);
    switch (line[0]) {
    case PIPE: {
        return OP_PIPE;
    }
    case STDOUT_REDIRECTION: {
        return OP_STDOUT_REDIRECTION;
    }
    case STDIN_REDIRECTION: {
        return OP_STDIN_REDIRECTION;
    }
    case BACKGROUND_JOB: {
        return OP_BACKGROUND_JOB;
    }
    case ADD: {
        return OP_ADD;
    }
    case SUBTRACT: {
        return OP_SUBTRACT;
    }
    case MULTIPLY: {
        return OP_MULTIPLY;
    }
    case DIVIDE: {
        return OP_DIVIDE;
    }
    case MODULO: {
        return OP_MODULO;
    }
    case CLOSING_PARAN: {
        return OP_MATH_EXPRESSION_END;
    }
    case TILDE: {
        return OP_HOME_EXPANSION;
    }
    case CONDITION_START: {
        return OP_CONDITION_START;
    }
    default: {
        return OP_CONSTANT;
    }
    }
}

[[nodiscard]]
enum Ops lexer_op_check_len_two(char* rst line)
{
    for (size_t i = 0; i < ops_2char_len; ++i) {
        if (CMP_2(line, ops_2char_str[i])) {
            return ops_2char[i];
        }
    }

    return OP_CONSTANT;
}

[[nodiscard]]
enum Ops lexer_op_check_len_three(char* rst line)
{
    for (size_t i = 0; i < ops_3char_len; ++i) {
        if (CMP_3(line, ops_3char_str[i])) {
            return ops_3char[i];
        }
    }

    return OP_CONSTANT;
}

[[nodiscard]]
enum Ops lexer_op_check_len_four(char* rst line)
{
    switch (line[0]) {
    case 't': {
        if (!memcmp(line, BOOL_TRUE, sizeof(BOOL_TRUE) - 1))
            return OP_TRUE;
        else if (!memcmp(line, THEN, sizeof(THEN) - 1))
            return OP_THEN;
        else
            return OP_CONSTANT;
    }
    case 'e': {
        if (!memcmp(line, ELSE, sizeof(ELSE) - 1))
            return OP_ELSE;
        else if (!memcmp(line, ELIF, sizeof(ELIF) - 1))
            return OP_ELIF;

        // FALLTHROUGH
    }
    }
    return OP_CONSTANT;
}

[[nodiscard]]
enum Ops lexer_op_check_len_five(char* rst line)
{
    return (STRCMP(line, BOOL_FALSE)) ? OP_FALSE : OP_CONSTANT;
}

/* lexer_op_get
 * Internal function used to map the inputted line to a bytecode.
 * Returns: a value from enum Ops, the bytecode relevant to the input
 */
[[nodiscard]]
enum Ops lexer_op_get(char* rst line, size_t len)
{
    assert(line);

    if (lexer_op_check_var(line, len))
        return OP_VARIABLE;

    switch (len) {
    case 0: {
        return OP_NONE;
    }

    case 1: {
        return lexer_op_check_len_one(line);
    }

    case 2: {
        return lexer_op_check_len_two(line);
    }

    case 3: {
        return lexer_op_check_len_three(line);
    }

    case 4: {
        return lexer_op_check_len_four(line);
    }

    case 5: {
        return lexer_op_check_len_five(line);
    }

    default: {
        return OP_CONSTANT;
    }
    }
}

[[nodiscard]]
uint8_t lexer_op_process()
{
    if ((lex_state & IN_HOME_EXPANSION)) {
        lex_state &= ~IN_HOME_EXPANSION;
        return OP_HOME_EXPANSION;
    }

    if ((lex_state & IN_GLOB_EXPANSION)) {
        lex_state &= ~IN_GLOB_EXPANSION;
        return OP_GLOB_EXPANSION;
    }

    if ((lex_state & IN_ASSIGNMENT)) {
        lex_state &= ~IN_ASSIGNMENT;
        return OP_ASSIGNMENT;
    }

    return lexer_op_get(lex_buf, lex_pos);
}

/* lexer_lex
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 * Handles expansions like *, ?, and ~
 */
[[nodiscard]]
Tokens* lexer_lex(char* rst line, size_t length, Arena* rst scratch)
{
    assert(line && scratch);
    // assert(!line[length - 1] && line[length - 2]);
    Tokens* toks = tokens_alloc(scratch);
    assert(toks);
    if (length < 2 || length > NCSH_MAX_INPUT) {
        toks->count = 0;
        return toks;
    }

    assert(length >= 2);

    debug_lexer_input(line, length);

    lex_buf = arena_malloc(scratch, NCSH_MAX_INPUT, char);
    lex_pos = 0;
    lex_state = 0;

    Token* tok = toks->head;

    for (register size_t pos = 0; pos < length; ++pos) {
        if (toks->count == LEXER_TOKENS_LIMIT - 1 && pos < length) { // can't lex all of the tokens
            toks->count = 0;
            break;
        }
        else if (pos >= NCSH_MAX_INPUT - 1 || lex_pos >= NCSH_MAX_INPUT - 1 || toks->count == LEXER_TOKENS_LIMIT - 1) {
            break;
        }

        switch (line[pos]) {
        case DOUBLE_QUOTE_KEY: {
            if (!(lex_state & IN_DOUBLE_QUOTES))
                lex_state |= IN_DOUBLE_QUOTES;
            else
                lex_state &= ~IN_DOUBLE_QUOTES;
            continue;
        }
        case SINGLE_QUOTE_KEY: {
            if (!(lex_state & IN_SINGLE_QUOTES))
                lex_state |= IN_SINGLE_QUOTES;
            else
                lex_state &= ~IN_SINGLE_QUOTES;
            continue;
        }
        case BACKTICK_QUOTE_KEY: {
            if (!(lex_state & IN_BACKTICK_QUOTES))
                lex_state |= IN_BACKTICK_QUOTES;
            else
                lex_state &= ~IN_BACKTICK_QUOTES;
            continue;
        }
        case OPENING_PARAN: {
            if ((lex_state & IN_DOLLAR_SIGN) && !(lex_state & IN_MATHEMATICAL_EXPRESSION)) {
                lex_state |= IN_MATHEMATICAL_EXPRESSION;
                lex_state &= ~IN_DOLLAR_SIGN;
            }

            lex_buf[lex_pos++] = line[pos];
            continue;
        }
        case CLOSING_PARAN: {
            if (lex_state & IN_MATHEMATICAL_EXPRESSION)
                lex_state &= ~IN_MATHEMATICAL_EXPRESSION;

            lex_buf[lex_pos++] = line[pos];
            continue;
        }
        case GLOB_STAR: {
            if (!(lex_state & IN_MATHEMATICAL_EXPRESSION) && !(lex_state & IN_GLOB_EXPANSION))
                lex_state |= IN_GLOB_EXPANSION;

            lex_buf[lex_pos++] = line[pos];
            continue;
        }
        case GLOB_QUESTION: {
            if (!(lex_state & IN_GLOB_EXPANSION))
                lex_state |= IN_GLOB_EXPANSION;

            lex_buf[lex_pos++] = line[pos];
            continue;
        }
        case ASSIGNMENT: {
            if (!(lex_state & IN_ASSIGNMENT))
                lex_state |= IN_ASSIGNMENT;

            lex_buf[lex_pos++] = line[pos];
            continue;
        }
        case COMMENT: {
            if (!(lex_state & IN_COMMENT))
                lex_state |= IN_COMMENT;

            continue;
        }
        case DOLLAR_SIGN: {
            // exclude variables from this one
            if (!(lex_state & IN_DOLLAR_SIGN) && pos < length - 1 && line[pos + 1] == '(')
                lex_state |= IN_DOLLAR_SIGN;

            lex_buf[lex_pos++] = line[pos];
            continue;
        }
        case TILDE: {
            if (!(lex_state & IN_HOME_EXPANSION))
                lex_state |= IN_HOME_EXPANSION;

            lex_buf[lex_pos++] = line[pos];
            continue;
        }
        // delimiter case // NOTE: should \t, \a, or EOF be included?
        case ' ':
        case '\r':
        case '\n':
        case '\0': {
            if (lex_state & IN_COMMENT) {
                if (line[pos] != '\n') {
                    continue;
                }
                else {
                    lex_buf[0] = '\0';
                    lex_pos = 0;
                }
            }

            if (lex_state & IN_SINGLE_QUOTES || lex_state & IN_DOUBLE_QUOTES || lex_state & IN_BACKTICK_QUOTES) {

                lex_buf[lex_pos++] = line[pos];
                continue;
            }

            // break to code below when delimiter found and no state or certain states found
            if ((!lex_state || (lex_state & IN_MATHEMATICAL_EXPRESSION) || (lex_state & IN_ASSIGNMENT) ||
                 (lex_state & IN_GLOB_EXPANSION) || (lex_state & IN_HOME_EXPANSION))) {
                break;
            }

            lex_buf[lex_pos++] = line[pos];
            continue;
        }
        default: {
            lex_buf[lex_pos++] = line[pos];
            continue;
        }
        }

        lex_buf[lex_pos] = '\0';

        debugf("Current lexer state: %d\n", lex_state);

        uint8_t op = lexer_op_process();
        tok->next = token_alloc(op, lex_pos + 1, lex_buf, scratch);
        tok = tok->next;
        ++toks->count;

        lex_buf[0] = '\0';
        lex_pos = 0;
    }

    debug_tokens(toks);
    return toks;
}

/* lexer_lex_noninteractive
 * lex the command line input into commands, command lengths, and op codes then stored in Tokens.
 * Allocates memory that is freed by lexer_free_values at the end of each main loop of the shell.
 * Used for noninteractive mode.
 */
[[nodiscard]]
Tokens* lexer_lex_noninteractive(char** rst inputs, size_t inputs_count, Arena* rst scratch)
{
    assert(inputs && inputs_count && scratch);

    if (!inputs || inputs_count == 0) {
        return NULL;
    }

    Tokens* toks = lexer_lex(inputs[0], strlen(inputs[0]) + 1, scratch);
    if (inputs_count == 1)
        return toks;

    // lex the split input one parameter at a time.
    for (size_t i = 1; i < inputs_count; ++i) {
        Tokens* next_toks = lexer_lex(inputs[i], strlen(inputs[i]) + 1, scratch);
        if (next_toks->count > 0) {
            token_set_last(toks, next_toks->head->next); // skip head
            toks->count += next_toks->count;
        }
    }

    debug_tokens(toks);
    return toks;
}
