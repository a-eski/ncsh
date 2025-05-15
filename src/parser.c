/* Copyright ncsh by Alex Eski 2024 */

#include <assert.h>
#include <glob.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "args.h"
#include "configurables.h"
#include "debug.h"
#include "defines.h"
#include "parser.h"

// supported quotes
#define SINGLE_QUOTE_KEY '\''
#define DOUBLE_QUOTE_KEY '\"'
#define SINGLE_QUOTE_KEY '\''
#define BACKTICK_QUOTE_KEY '`'
#define OPENING_PARAN '('
#define CLOSING_PARAN ')'

// ops
#define PIPE '|'
#define STDIN_REDIRECTION '<'
#define STDOUT_REDIRECTION '>'
#define BACKGROUND_JOB '&'

#define STDOUT_REDIRECTION_APPEND ">>"
#define STDERR_REDIRECTION "2>"
#define STDERR_REDIRECTION_APPEND "2>>"
#define STDOUT_AND_STDERR_REDIRECTION "&>"
#define STDOUT_AND_STDERR_REDIRECTION_APPEND "&>>"
#define AND "&&"
#define OR "||"

// ops: multiple
#define DOLLAR_SIGN '$'
#define VARIABLE '$'
#define MATH_EXPRESSION_START "$("
#define MATH_EXPRESSION_END ')'

// ops: numeric
#define ADD '+'
#define SUBTRACT '-'
#define MULTIPLY '*'
#define DIVIDE '/'
#define MODULO '%'
#define EXPONENTIATION "**"
// ops: variables
#define ASSIGNMENT '='
// ops: boolean
// prefixed with BOOL to avoid any possible conflicts in the future
#define BOOL_TRUE "true"
#define BOOL_FALSE "false"

// expansions
#define GLOB_STAR '*'
#define GLOB_QUESTION '?'
#define TILDE '~'

#define COMMENT '#'

// currently unsupported
// #define BANG '!'

/* enum Parser_State
 * Flags used by the parser to keep track of state, like whether the current character
 * being processed is inside quotes or a mathematical expression.
 */
// clang-format off
enum Parser_State: size_t {
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

/* Parser Internal Variables */
/* These variables used in parser_parse, they are not accessed anywhere else within the shell,
 * and should not be accessed anywhere else in the shell.
 * They were put here because they increased speed of the parser in benchmarks.
 */
char* rst parser_buffer;
size_t parser_state;
size_t parser_buf_pos;

/* ops_2char_str
 * A constant array that contains all shell operations that are 2 characters long, like ">>", "||", "&&".
 * Size of the array is stored as constant expression in ops_2char_len
 * Bytecodes (opcodes) equivalents are stored in the array of enum Ops, ops_2char
 */
const char* const rst ops_2char_str[] = {
    STDOUT_REDIRECTION_APPEND, STDERR_REDIRECTION, STDOUT_AND_STDERR_REDIRECTION, AND, OR, EXPONENTIATION,
    MATH_EXPRESSION_START};

constexpr size_t ops_2char_len = sizeof(ops_2char_str) / sizeof(char*);

const enum Ops ops_2char[] = {OP_STDOUT_REDIRECTION_APPEND,
                              OP_STDERR_REDIRECTION,
                              OP_STDOUT_AND_STDERR_REDIRECTION,
                              OP_AND,
                              OP_OR,
                              OP_EXPONENTIATION,
                              OP_MATH_EXPRESSION_START};

/* ops_3char_str
 * A constant array that contain all shell operations that are 3 characters long, like "&>>".
 * Size of the array is stored as constant expression in ops_3char_len
 * Bytecodes (opcodes) equivalents are stored in the array of enum Ops, ops_3char
 */
const char* const rst ops_3char_str[] = {STDERR_REDIRECTION_APPEND, STDOUT_AND_STDERR_REDIRECTION_APPEND};

constexpr size_t ops_3char_len = sizeof(ops_3char_str) / sizeof(char*);

const enum Ops ops_3char[] = {OP_STDERR_REDIRECTION_APPEND, OP_STDOUT_AND_STDERR_REDIRECTION_APPEND};

/* parser_op_get
 * Internal function used to map the inputted line to a bytecode.
 * Returns: a value from enum Ops, the bytecode relevant to the input
 */
[[nodiscard]]
enum Ops parser_op_get(char* rst line, size_t length)
{
    switch (length) {
    case 0: {
        return OP_NONE;
    }
    case 1: {
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
        default: {
            return OP_CONSTANT;
        }
        }
    }
    case 2: {
        assert(line);
        for (size_t i = 0; i < ops_2char_len; ++i) {
            if (CMP_2(line, ops_2char_str[i])) {
                return ops_2char[i];
            }
        }

        return OP_CONSTANT;
    }
    case 3: {
        assert(line);
        for (size_t i = 0; i < ops_3char_len; ++i) {
            if (CMP_3(line, ops_3char_str[i])) {
                return ops_3char[i];
            }
        }

        return OP_CONSTANT;
    }
    case 4: {
        if (line[0] == 't' && !memcmp(line, BOOL_TRUE, 4)) {
            return OP_TRUE;
        }
        break;
    }
    case 5: {
        if (line[0] == 'f' && !memcmp(line, BOOL_FALSE, 5)) {
            return OP_FALSE;
        }
        break;
    }
    }

    if (line[0] == VARIABLE) {
        return OP_VARIABLE;
    }

    return OP_CONSTANT;
}

uint8_t parser_op_process()
{
    if ((parser_state & IN_HOME_EXPANSION)) {
        parser_state &= ~IN_HOME_EXPANSION;
        return OP_HOME_EXPANSION;
    }

    if ((parser_state & IN_GLOB_EXPANSION)) {
        parser_state &= ~IN_GLOB_EXPANSION;
        return OP_GLOB_EXPANSION;
    }

    if ((parser_state & IN_ASSIGNMENT)) {
        parser_state &= ~IN_ASSIGNMENT;
        return OP_ASSIGNMENT;
    }

    return parser_op_get(parser_buffer, parser_buf_pos);
}

/* parser_parse
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 * Handles expansions like *, ?, and ~
 */
Args* parser_parse(char* rst line, size_t length, Arena* rst scratch_arena)
{
    assert(line && scratch_arena);
    assert(!line[length - 1]);
    Args* args = args_alloc(scratch_arena);
    assert(args);
    if (length < 2 || length > NCSH_MAX_INPUT) {
        args->count = 0;
        return args;
    }

    assert(length >= 2);

    debug_parser_input(line, length);

    parser_buffer = arena_malloc(scratch_arena, NCSH_MAX_INPUT, char);
    parser_buf_pos = 0;
    parser_state = 0;

    Arg* arg = args->head;

    for (register size_t line_pos = 0; line_pos < length + 1; ++line_pos) {
        if (args->count == PARSER_TOKENS_LIMIT - 1 && line_pos < length) { // can't parse all of the args
            args->count = 0;
            break;
        }
        else if (line_pos == length || line_pos >= NCSH_MAX_INPUT - 1 || parser_buf_pos >= NCSH_MAX_INPUT - 1 ||
                 args->count == PARSER_TOKENS_LIMIT - 1) {
            break;
        }

        switch (line[line_pos]) {
        case DOUBLE_QUOTE_KEY: {
            if (!(parser_state & IN_DOUBLE_QUOTES))
                parser_state |= IN_DOUBLE_QUOTES;
            else
                parser_state &= ~IN_DOUBLE_QUOTES;
            continue;
        }
        case SINGLE_QUOTE_KEY: {
            if (!(parser_state & IN_SINGLE_QUOTES))
                parser_state |= IN_SINGLE_QUOTES;
            else
                parser_state &= ~IN_SINGLE_QUOTES;
            continue;
        }
        case BACKTICK_QUOTE_KEY: {
            if (!(parser_state & IN_BACKTICK_QUOTES))
                parser_state |= IN_BACKTICK_QUOTES;
            else
                parser_state &= ~IN_BACKTICK_QUOTES;
            continue;
        }
        case OPENING_PARAN: {
            if ((parser_state & IN_DOLLAR_SIGN) && !(parser_state & IN_MATHEMATICAL_EXPRESSION)) {
                parser_state |= IN_MATHEMATICAL_EXPRESSION;
                parser_state &= ~IN_DOLLAR_SIGN;
            }

            parser_buffer[parser_buf_pos++] = line[line_pos];
            continue;
        }
        case CLOSING_PARAN: {
            if (parser_state & IN_MATHEMATICAL_EXPRESSION)
                parser_state &= ~IN_MATHEMATICAL_EXPRESSION;

            parser_buffer[parser_buf_pos++] = line[line_pos];
            continue;
        }
        case GLOB_STAR: {
            if (!(parser_state & IN_MATHEMATICAL_EXPRESSION) && !(parser_state & IN_GLOB_EXPANSION))
                parser_state |= IN_GLOB_EXPANSION;

            parser_buffer[parser_buf_pos++] = line[line_pos];
            continue;
        }
        case GLOB_QUESTION: {
            if (!(parser_state & IN_GLOB_EXPANSION))
                parser_state |= IN_GLOB_EXPANSION;

            parser_buffer[parser_buf_pos++] = line[line_pos];
            continue;
        }
        case ASSIGNMENT: {
            if (!(parser_state & IN_ASSIGNMENT))
                parser_state |= IN_ASSIGNMENT;

            parser_buffer[parser_buf_pos++] = line[line_pos];
            continue;
        }
        case COMMENT: {
            if (!(parser_state & IN_COMMENT))
                parser_state |= IN_COMMENT;

            continue;
        }
        case DOLLAR_SIGN: {
            // exclude variables from this one
            if (!(parser_state & IN_DOLLAR_SIGN) && line_pos < length - 1 && line[line_pos + 1] == '(')
                parser_state |= IN_DOLLAR_SIGN;

            parser_buffer[parser_buf_pos++] = line[line_pos];
            continue;
        }
        case TILDE: {
            if (!(parser_state & IN_HOME_EXPANSION))
                parser_state |= IN_HOME_EXPANSION;

            parser_buffer[parser_buf_pos++] = line[line_pos];
            continue;
        }
        // delimiter case // NOTE: should \t, \a, or EOF be included?
        case ' ':
        case '\r':
        case '\n':
        case '\0': {
            if (parser_state & IN_COMMENT && line[line_pos] != '\n')
                continue;

            // break to code below when delimiter found and no state or certain states found
            if ((!parser_state || (parser_state & IN_MATHEMATICAL_EXPRESSION) || (parser_state & IN_ASSIGNMENT) ||
                 (parser_state & IN_GLOB_EXPANSION) || (parser_state & IN_HOME_EXPANSION))) {
                break;
            }

            parser_buffer[parser_buf_pos++] = line[line_pos];
            continue;
        }
        default: {
            parser_buffer[parser_buf_pos++] = line[line_pos];
            continue;
        }
        }

        parser_buffer[parser_buf_pos] = '\0';

        debugf("Current parser state: %d\n", parser_state);

        uint8_t op = parser_op_process();
        arg->next = arg_alloc(op, parser_buf_pos + 1, parser_buffer, scratch_arena);
        arg = arg->next;
        ++args->count;

        parser_buffer[0] = '\0';
        parser_buf_pos = 0;
    }

    debug_args(args);
    return args;
}

/* parser_parse_noninteractive
 * Parse the command line input into commands, command lengths, and op codes stored in Args.
 * Allocates memory that is freed by parser_free_values at the end of each main loop of the shell.
 * Used for noninteractive mode.
 */
Args* parser_parse_noninteractive(char** rst inputs, size_t inputs_count, Arena* rst scratch_arena)
{
    assert(inputs && inputs_count && scratch_arena);

    if (!inputs || inputs_count == 0) {
        return NULL;
    }

    Args* args = parser_parse(inputs[0], strlen(inputs[0]) + 1, scratch_arena);
    if (inputs_count == 1)
        return args;

    // parse the split input one parameter at a time.
    for (size_t i = 1; i < inputs_count; ++i) {
        Args* next_args = parser_parse(inputs[i], strlen(inputs[i]) + 1, scratch_arena);
        if (next_args->count > 0) {
            arg_set_last(args, next_args->head->next); // skip head
            args->count += next_args->count;
        }
    }

    debug_args(args);
    return args;
}
