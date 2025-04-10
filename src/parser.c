/* Copyright ncsh by Alex Eski 2024 */

#include <assert.h>
#include <glob.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "eskilib/eskilib_result.h"
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

// ops: numeric
#define ADD '+'
#define SUBTRACT '-'
#define MULTIPLY '*'
#define DIVIDE '/'
#define MODULO '%'
#define EXPONENTIATION "**"

// expansions
#define GLOB_STAR '*'
#define GLOB_QUESTION '?'
#define TILDE '~'

// currently unsupported
// #define BANG '!'

/* parser_args_alloc
 * Allocate memory for the parser that lives for the lifetime of the shell.
 * Allocates the memory for string values, bytecodes, and lengths of the string values.
 * This same memory is reused for the lifetime of the shell.
 * Returns: enum eskilib_Result, E_SUCCESS is successful, E_FAILURE_NULL_REFERENCE if *args is null
 */
[[nodiscard]]
enum eskilib_Result parser_args_alloc(struct Args* const restrict args, struct Arena* const arena)
{
    if (!args) {
        return E_FAILURE_NULL_REFERENCE;
    }

    args->count = 0;

    args->values = arena_malloc(arena, PARSER_TOKENS_LIMIT, char*);
    args->ops = arena_malloc(arena, PARSER_TOKENS_LIMIT, uint_fast8_t);
    args->lengths = arena_malloc(arena, PARSER_TOKENS_LIMIT, size_t);

    return E_SUCCESS;
}

/* parser_is_delimiter
 * Internal function used to determine where to split values being parsed.
 * For example, "ls -a" gets split into ["ls", "-a"], and this function determines
 * where that split happens.
 */
[[nodiscard]]
bool parser_is_delimiter(const char ch)
{
    switch (ch) {
    case ' ': {
        return true;
    }
    // case '\t': { return true; }
    case '\r': {
        return true;
    }
    case '\n': {
        return true;
    }
    // case '\a': { return true; }
    // case EOF:  { return true; }
    case '\0': {
        return true;
    }
    default: {
        return false;
    }
    }
}

/* ops_2char_str
 * A constant array that contains all shell operations that are 2 characters long, like ">>", "||", "&&".
 * Size of the array is stored as constant expression in ops_2char_len
 * Bytecodes (opcodes) equivalents are stored in the array of enum Ops, ops_2char
 */
const char* const restrict ops_2char_str[] = {
    STDOUT_REDIRECTION_APPEND, STDERR_REDIRECTION, STDOUT_AND_STDERR_REDIRECTION, AND, OR, EXPONENTIATION};

constexpr size_t ops_2char_len = sizeof(ops_2char_str) / sizeof(char*);

const enum Ops ops_2char[] = {OP_STDOUT_REDIRECTION_APPEND,
                              OP_STDERR_REDIRECTION,
                              OP_STDOUT_AND_STDERR_REDIRECTION,
                              OP_AND,
                              OP_OR,
                              OP_EXPONENTIATION};

/* ops_3char_str
 * A constant array that contain all shell operations that are 3 characters long, like "&>>".
 * Size of the array is stored as constant expression in ops_3char_len
 * Bytecodes (opcodes) equivalents are stored in the array of enum Ops, ops_3char
 */
const char* const restrict ops_3char_str[] = {STDERR_REDIRECTION_APPEND, STDOUT_AND_STDERR_REDIRECTION_APPEND};

constexpr size_t ops_3char_len = sizeof(ops_3char_str) / sizeof(char*);

const enum Ops ops_3char[] = {OP_STDERR_REDIRECTION_APPEND, OP_STDOUT_AND_STDERR_REDIRECTION_APPEND};

/* parser_op_get
 * Internal function used to map the inputted line to a bytecode.
 * Returns: a value from enum Ops, the bytecode relevant to the input
 */
[[nodiscard]]
enum Ops parser_op_get(const char* const restrict line, const size_t length)
{
    switch (length) {
    case 0: {
        return OP_NONE;
    }
    case 1: {
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
        case OPENING_PARAN: {
            return OP_MATH_EXPRESSION_START;
        }
        case CLOSING_PARAN: {
            return OP_MATH_EXPRESSION_END;
        }
        default: {
            return OP_CONSTANT;
        }
        }
    }
    case 2: {
        for (uint_fast32_t i = 0; i < ops_2char_len; ++i) {
            if (!memcmp(line, ops_2char_str[i], 2)) {
                return ops_2char[i];
            }
        }

        return OP_CONSTANT;
    }
    case 3: {
        for (uint_fast32_t i = 0; i < ops_3char_len; ++i) {
            if (!memcmp(line, ops_3char_str[i], 3)) {
                return ops_3char[i];
            }
        }

        return OP_CONSTANT;
    }
    default: {
        return OP_CONSTANT;
    }
    }
}

/* enum Parser_State
 * Flags used by the parser to keep track of state, like whether the current character
 * being processed is inside quotes or a mathematical expression.
 */
enum Parser_State {
    IN_SINGLE_QUOTES = 0x01,
    IN_DOUBLE_QUOTES = 0x02,
    IN_BACKTICK_QUOTES = 0x04,
    IN_MATHEMATICAL_EXPRESSION = 0x08
};

/* parser_parse
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 * Handles expansions like *, ?, and ~
 * Results are stored in struct Args
 */
void parser_parse(const char* const restrict line, const size_t length, struct Args* const restrict args,
                  struct Arena* scratch_arena)
{
    assert(args);
    assert(line);

    if (!line || length < 2 || length > NCSH_MAX_INPUT) {
        args->count = 0;
        return;
    }

    assert(length >= 2);

#ifdef NCSH_DEBUG
    debug_parser_input(line, length);
#endif /* ifdef NCSH_DEBUG */

    char buffer[NCSH_MAX_INPUT];
    size_t buf_pos = 0;
    bool glob_found = false;
    int state = 0;

    for (uint_fast32_t line_position = 0; line_position < length + 1; ++line_position) {
        if (args->count == PARSER_TOKENS_LIMIT - 1 && line_position < length) { // can't parse all of the args
            args->count = 0;
            break;
        }
        else if (line_position == length || line_position >= NCSH_MAX_INPUT - 1 || buf_pos >= NCSH_MAX_INPUT - 1 ||
                 args->count == PARSER_TOKENS_LIMIT - 1) {
            args->values[args->count] = NULL; // set the last value as null to use as a sentinel in the VM
            break;
        }
        else if (parser_is_delimiter(line[line_position]) && (!state || (state & IN_MATHEMATICAL_EXPRESSION))) {
            buffer[buf_pos] = '\0';

            if (glob_found) {
                glob_t glob_buf = {0};
                size_t glob_len;
                glob(buffer, GLOB_DOOFFS, NULL, &glob_buf);

                for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
                    glob_len = strlen(glob_buf.gl_pathv[i]) + 1;
                    if (!glob_len || glob_len >= NCSH_MAX_INPUT) {
                        break;
                    }
                    buf_pos = glob_len;
                    args->values[args->count] = arena_malloc(scratch_arena, buf_pos, char);
                    memcpy(args->values[args->count], glob_buf.gl_pathv[i], glob_len);
                    args->ops[args->count] = OP_CONSTANT;
                    args->lengths[args->count] = buf_pos;
                    ++args->count;
                    if (args->count >= PARSER_TOKENS_LIMIT - 1) {
                        break;
                    }
                }

                globfree(&glob_buf);
                glob_found = false;
            }
            else {
                args->values[args->count] = arena_malloc(scratch_arena, buf_pos + 1, char);
                memcpy(args->values[args->count], buffer, buf_pos + 1);
                args->ops[args->count] = parser_op_get(buffer, buf_pos);
                args->lengths[args->count] = buf_pos + 1; // + 1 for null terminator
                ++args->count;
            }

            buffer[0] = '\0';
            buf_pos = 0;
        }
        else {
            switch (line[line_position]) {
            case DOUBLE_QUOTE_KEY: {
                if (!(state & IN_DOUBLE_QUOTES)) {
                    state |= IN_DOUBLE_QUOTES;
                }
                else {
                    state &= ~IN_DOUBLE_QUOTES;
                }
                break;
            }
            case SINGLE_QUOTE_KEY: {
                if (!(state & IN_SINGLE_QUOTES)) {
                    state |= IN_SINGLE_QUOTES;
                }
                else {
                    state &= ~IN_SINGLE_QUOTES;
                }
                break;
            }
            case BACKTICK_QUOTE_KEY: {
                if (!(state & IN_BACKTICK_QUOTES)) {
                    state |= IN_BACKTICK_QUOTES;
                }
                else {
                    state &= ~IN_BACKTICK_QUOTES;
                }
                break;
            }
            case OPENING_PARAN: {
                if (!(state & IN_MATHEMATICAL_EXPRESSION)) {
                    state |= IN_MATHEMATICAL_EXPRESSION;
                }

                buffer[buf_pos++] = line[line_position];
                break;
            }
            case CLOSING_PARAN: {
                if (state & IN_MATHEMATICAL_EXPRESSION) {
                    state &= ~IN_MATHEMATICAL_EXPRESSION;
                }

                buffer[buf_pos++] = line[line_position];
                break;
            }
            case TILDE: {
                char* home = getenv("HOME");
                size_t home_length = strlen(home);
                if (buf_pos + home_length > NCSH_MAX_INPUT) {
                    // protect from overflow
                    args->count = 0;
                    return;
                }
                memcpy(buffer + buf_pos, home, home_length);
                buf_pos += home_length;
                break;
            }
            case GLOB_STAR: {
                if (!(state & IN_MATHEMATICAL_EXPRESSION)) {
                    glob_found = true;
                }
                buffer[buf_pos++] = line[line_position];
                break;
            }
            case GLOB_QUESTION: {
                glob_found = true;
                buffer[buf_pos++] = line[line_position];
                break;
            }
            default: {
                buffer[buf_pos++] = line[line_position];
                break;
            }
            }
        }
    }

#ifdef NCSH_DEBUG
    debug_args(args);
#endif /* ifdef NCSH_DEBUG */
}

/* parser_parse_noninteractive
 * Parse the command line input into commands, command lengths, and op codes stored in struct Args.
 * Allocates memory that is freed by parser_free_values at the end of each main loop of the shell.
 */
void parser_parse_noninteractive(const char** const restrict inputs, const size_t inputs_count,
                                 struct Args* const restrict args, struct Arena* scratch_arena)
{
    assert(args);
    assert(inputs);
    assert(inputs_count > 0);

    if (!inputs || inputs_count == 0) {
        args->count = 0;
        return;
    }

    for (size_t i = 0; i < inputs_count; ++i) {
        parser_parse(inputs[i], strlen(inputs[i]) + 1, args, scratch_arena);
    }

#ifdef NCSH_DEBUG
    debug_args(args);
#endif /* ifdef NCSH_DEBUG */
}
