/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <glob.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../debug.h"
#include "../defines.h"
#include "lexemes.h"
#include "lexer.h"
#include "lexer_defines.h"
#include "ops.h"

/* ops_2char_str
 * A constant array that contains all shell operations that are 2 characters long, like ">>", "||", "&&".
 * Size of the array is stored as constant expression in ops_2char_len
 * Bytecodes (opcodes) equivalents are stored in the array of enum Ops, ops_2char
 */
static constexpr char ops_2char_str[11][2] = {STDIN_REDIRECTION_APPEND,
                                         STDOUT_REDIRECTION_APPEND,
                                         STDERR_REDIRECTION,
                                         STDOUT_AND_STDERR_REDIRECTION,
                                         AND,
                                         OR,
                                         EXPONENTIATION,
                                         MATH_EXPRESSION_START,
                                         IF,
                                         FI,
                                         CONDITION_END};

static constexpr size_t ops_2char_len = sizeof(ops_2char_str) / sizeof(ops_2char_str[0]);

static constexpr enum Ops ops_2char[] = {OP_STDIN_REDIRECTION_APPEND,
                              OP_STDOUT_REDIRECTION_APPEND,
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
static constexpr char ops_3char_str[5][3] = {STDERR_REDIRECTION_APPEND, STDOUT_AND_STDERR_REDIRECTION_APPEND, EQUALS,
                                         LESS_THAN, GREATER_THAN};

static constexpr size_t ops_3char_len = sizeof(ops_3char_str) / sizeof(ops_3char_str[0]);

static constexpr enum Ops ops_3char[] = {OP_STDERR_REDIRECTION_APPEND, OP_STDOUT_AND_STDERR_REDIRECTION_APPEND, OP_EQUALS,
                              OP_LESS_THAN, OP_GREATER_THAN};

[[nodiscard]]
bool lexer_op_check_var(char* line, size_t len)
{
    return len > 2 && line[0] == VARIABLE;
}

[[nodiscard]]
enum Ops lexer_op_check_len_one(char* restrict line)
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
enum Ops lexer_op_check_len_two(char* restrict line)
{
    for (size_t i = 0; i < ops_2char_len; ++i) {
        if (CMP_2(line, ops_2char_str[i])) {
            return ops_2char[i];
        }
    }

    return OP_CONSTANT;
}

[[nodiscard]]
enum Ops lexer_op_check_len_three(char* restrict line)
{
    for (size_t i = 0; i < ops_3char_len; ++i) {
        if (CMP_3(line, ops_3char_str[i])) {
            return ops_3char[i];
        }
    }

    return OP_CONSTANT;
}

[[nodiscard]]
enum Ops lexer_op_check_len_four(char* restrict line)
{
    switch (line[0]) {
    case 't': {
        if (!memcmp(line, BOOL_TRUE, sizeof(BOOL_TRUE) - 1))
            return OP_TRUE;
        else if (!memcmp(line, THEN, sizeof(THEN) - 1))
            return OP_THEN;
        break;
    }
    case 'e': {
        if (!memcmp(line, ELSE, sizeof(ELSE) - 1))
            return OP_ELSE;
        else if (!memcmp(line, ELIF, sizeof(ELIF) - 1))
            return OP_ELIF;
    }
    }
    return OP_CONSTANT;
}

[[nodiscard]]
enum Ops lexer_op_check_len_five(char* restrict line)
{
    return (STRCMP(line, BOOL_FALSE)) ? OP_FALSE : OP_CONSTANT;
}

/* lexer_op_get
 * Internal function used to map the inputted line to a bytecode.
 * Returns: a value from enum Ops, the bytecode relevant to the input
 */
[[nodiscard]]
enum Ops lexer_op_get(char* restrict line, size_t len)
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
static char* restrict lex_buf;
static size_t lex_state;
static size_t lex_buf_pos;

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

    return lexer_op_get(lex_buf, lex_buf_pos);
}

/* lexer_lex
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 */
void lexer_lex(Str line, Lexemes* lexemes, Arena* restrict scratch)
{
    if (!lexemes) {
        return;
    }
    assert(lexemes);
    lexemes_init(lexemes, scratch);
    assert(line.value); assert(scratch);
    if (!line.value || !*line.value || !line.length) {
        lexemes->count = 0;
        return;
    }
    if (line.length < 2 || line.length > NCSH_MAX_INPUT) {
        lexemes->count = 0;
        return;
    }

    estrtrim(&line);

    debug_lexer_input(line, length);

    lex_buf = arena_malloc(scratch, NCSH_MAX_INPUT, char);
    lex_buf_pos = 0;
    lex_state = 0;
    size_t n = lexemes->count;

    for (register size_t pos = 0; pos < line.length; ++pos) {
        if (lexemes->count == LEXER_TOKENS_LIMIT - 1 && pos < line.length) { // can't lex all of the tokens
            lexemes->count = 0;
            break;
        }
        else if (pos >= NCSH_MAX_INPUT - 1 || lex_buf_pos >= NCSH_MAX_INPUT - 1 ||
                 lexemes->count == LEXER_TOKENS_LIMIT - 1) {
            break;
        }

        switch (line.value[pos]) {
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

            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        case CLOSING_PARAN: {
            if (lex_state & IN_MATHEMATICAL_EXPRESSION)
                lex_state &= ~IN_MATHEMATICAL_EXPRESSION;

            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        case GLOB_STAR: {
            if (!(lex_state & IN_MATHEMATICAL_EXPRESSION) && !(lex_state & IN_GLOB_EXPANSION))
                lex_state |= IN_GLOB_EXPANSION;

            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        case GLOB_QUESTION: {
            if (!(lex_state & IN_GLOB_EXPANSION))
                lex_state |= IN_GLOB_EXPANSION;

            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        case ASSIGNMENT: {
            if (!(lex_state & IN_ASSIGNMENT))
                lex_state |= IN_ASSIGNMENT;

            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        case COMMENT: {
            if (!(lex_state & IN_COMMENT))
                lex_state |= IN_COMMENT;

            continue;
        }
        case DOLLAR_SIGN: {
            // exclude variables from this one
            if (!(lex_state & IN_DOLLAR_SIGN) && pos < line.length - 1 && line.value[pos + 1] == '(')
                lex_state |= IN_DOLLAR_SIGN;

            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        case TILDE: {
            if (!(lex_state & IN_HOME_EXPANSION))
                lex_state |= IN_HOME_EXPANSION;

            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        // delimiter case // NOTE: should \t, \a, or EOF be included?
        case ' ':
        case '\r':
        case '\n':
        case '\0': {
            if (lex_state & IN_COMMENT) {
                if (line.value[pos] != '\n') {
                    continue;
                }
                else {
                    lex_buf[0] = '\0';
                    lex_buf_pos = 0;
                }
            }

            if (lex_state & IN_SINGLE_QUOTES || lex_state & IN_DOUBLE_QUOTES || lex_state & IN_BACKTICK_QUOTES) {
                lex_buf[lex_buf_pos++] = line.value[pos];
                continue;
            }

            // break to code below when delimiter found and no state or certain states found
            if ((!lex_state || (lex_state & IN_MATHEMATICAL_EXPRESSION) || (lex_state & IN_ASSIGNMENT) ||
                 (lex_state & IN_GLOB_EXPANSION) || (lex_state & IN_HOME_EXPANSION))) {
                break;
            }

            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        default: {
            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        }

        lex_buf[lex_buf_pos] = '\0';

        debugf("Current lexer state: %d\n", lex_state);

        lexemes->ops[n] = lexer_op_process();
        lexemes->strs[n].length = lex_buf_pos + 1;
        lexemes->strs[n].value = arena_malloc(scratch, lexemes->strs[n].length, char);
        memcpy(lexemes->strs[n++].value, lex_buf, lex_buf_pos);

        lex_buf[0] = '\0';
        lex_buf_pos = 0;
    }

    lexemes->count = n;
    debug_lexemes(lexemes);
}

/* lexer_lex_noninteractive
 * lex the command line input into commands, command lengths, and op codes then stored in Tokens.
 * Allocates memory that is freed by lexer_free_values at the end of each main loop of the shell.
 * Used for noninteractive mode.
 */
void lexer_lex_noninteractive(char** restrict inputs, size_t inputs_count, Lexemes* lexemes, Arena* restrict scratch)
{
    assert(inputs && inputs_count && scratch);

    if (!inputs || inputs_count == 0) {
        return;
    }

    for (size_t i = 0; i < inputs_count; ++i) {
        lexer_lex(Str_Get(inputs[i]), lexemes, scratch);
    }

    debug_lexemes(lexemes);
}
