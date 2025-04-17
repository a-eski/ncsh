/* Copyright ncsh by Alex Eski 2024 */

#include <assert.h>
#include <glob.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "debug.h"
#include "defines.h"
#include "eskilib/eresult.h"
#include "parser.h"
#include "var.h"

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
// ops: variables
#define ASSIGNMENT '='
#define VARIABLE '$'

// expansions
#define GLOB_STAR '*'
#define GLOB_QUESTION '?'
#define TILDE '~'

#define COMMENT '#'

// currently unsupported
// #define BANG '!'

/* parser_init
 * Allocate memory for the parser that lives for the lifetime of the shell.
 * Allocates the memory for string values, bytecodes, and lengths of the string values.
 * This same memory is reused for the lifetime of the shell.
 * Returns: enum eresult, E_SUCCESS is successful, E_FAILURE_NULL_REFERENCE if *args is null
 */
[[nodiscard]]
enum eresult parser_init(struct Args* const restrict args, struct Arena* const arena)
{
    assert(args && arena);
    if (!args)
        return E_FAILURE_NULL_REFERENCE;

    args->count = 0;

    args->values = arena_malloc(arena, PARSER_TOKENS_LIMIT, char*);
    args->ops = arena_malloc(arena, PARSER_TOKENS_LIMIT, uint_fast8_t);
    args->lengths = arena_malloc(arena, PARSER_TOKENS_LIMIT, size_t);
    args->vars = (struct var){0};
    var_malloc(arena, &args->vars);

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
    case ' ':
    // case '\t':
    case '\r':
    case '\n':
    // case '\a':
    // case EOF:
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
        assert(line);
        for (uint_fast32_t i = 0; i < ops_2char_len; ++i) {
            if (CMP_2(line, ops_2char_str[i])) {
                return ops_2char[i];
            }
        }

        return OP_CONSTANT;
    }
    case 3: {
        assert(line);
        for (uint_fast32_t i = 0; i < ops_3char_len; ++i) {
            if (CMP_3(line, ops_3char_str[i])) {
                return ops_3char[i];
            }
        }

        return OP_CONSTANT;
    }
    default: {
        if (line[0] == VARIABLE) {
            return OP_VARIABLE;
        }
        return OP_CONSTANT;
    }
    }
}

/* enum Parser_State
 * Flags used by the parser to keep track of state, like whether the current character
 * being processed is inside quotes or a mathematical expression.
 */
// clang-format off
enum Parser_State {
    IN_NONE =			 0,
    IN_SINGLE_QUOTES =           1 << 0,
    IN_DOUBLE_QUOTES =           1 << 1,
    IN_BACKTICK_QUOTES =         1 << 2,
    IN_MATHEMATICAL_EXPRESSION = 1 << 3,
    IN_ASSIGNMENT =              1 << 4,
    IN_GLOB_EXPANSION =          1 << 5,
    IN_COMMENT =		 1 << 7
};
// clang-format on

char* parser_buffer;
char* var_name;
int parser_state;
size_t parser_buf_pos = 0;
size_t parser_assignment_pos = 0;

/* parser_parse
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 * Handles expansions like *, ?, and ~
 * Results are stored in struct Args, variables values are stored in struct Args.vars.
 */
void parser_parse(const char* const restrict line, const size_t length, struct Args* const restrict args,
                  struct Arena* arena, struct Arena* scratch_arena)
{
    assert(line && args && scratch_arena && arena);
    if (length < 2 || length > NCSH_MAX_INPUT) {
        args->count = 0;
        return;
    }

    assert(length >= 2);

    debug_parser_input(line, length);

    parser_buffer = arena_malloc(scratch_arena, NCSH_MAX_INPUT, char);
    parser_buf_pos = 0;
    parser_assignment_pos = 0;
    var_name = arena_malloc(scratch_arena, NCSH_MAX_INPUT, char);
    parser_state = 0;

    for (uint_fast32_t line_position = 0; line_position < length + 1; ++line_position) {
        if (args->count == PARSER_TOKENS_LIMIT - 1 && line_position < length) { // can't parse all of the args
            args->count = 0;
            break;
        }
        else if (line_position == length || line_position >= NCSH_MAX_INPUT - 1 ||
                 parser_buf_pos >= NCSH_MAX_INPUT - 1 || args->count == PARSER_TOKENS_LIMIT - 1) {
            args->values[args->count] = NULL; // set the last value as null to use as a sentinel in the VM
            break;
        }

        switch (line[line_position]) {
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
            if (!(parser_state & IN_MATHEMATICAL_EXPRESSION))
                parser_state |= IN_MATHEMATICAL_EXPRESSION;

            parser_buffer[parser_buf_pos++] = line[line_position];
            continue;
        }
        case CLOSING_PARAN: {
            if (parser_state & IN_MATHEMATICAL_EXPRESSION)
                parser_state &= ~IN_MATHEMATICAL_EXPRESSION;

            parser_buffer[parser_buf_pos++] = line[line_position];
            continue;
        }
        case TILDE: {
            // TODO: look at performance of tilde expansion
            // TODO: see if it can be moved outside of case statement
            char* home = getenv("HOME");
            size_t home_length = strlen(home);
            if (parser_buf_pos + home_length > NCSH_MAX_INPUT) {
                // protect from overflow
                args->count = 0;
                return;
            }
            memcpy(parser_buffer + parser_buf_pos, home, home_length);
            parser_buf_pos += home_length;
            continue;
        }
        case GLOB_STAR: {
            if (!(parser_state & IN_MATHEMATICAL_EXPRESSION) && !(parser_state & IN_GLOB_EXPANSION))
                parser_state |= IN_GLOB_EXPANSION;

            parser_buffer[parser_buf_pos++] = line[line_position];
            continue;
        }
        case GLOB_QUESTION: {
            if (!(parser_state & IN_GLOB_EXPANSION))
                parser_state |= IN_GLOB_EXPANSION;

            parser_buffer[parser_buf_pos++] = line[line_position];
            continue;
        }
        case ASSIGNMENT: {
            if (!(parser_state & IN_ASSIGNMENT)) {
                parser_state |= IN_ASSIGNMENT;
                parser_assignment_pos = parser_buf_pos + 1;
                memcpy(var_name, parser_buffer, parser_buf_pos);
                var_name[parser_buf_pos] = '\0';
            }
            continue;
        }
        case COMMENT: {
            if (!(parser_state & IN_COMMENT))
                parser_state |= IN_COMMENT;

            continue;
        }
        default: {
            if (parser_state & IN_COMMENT && line[line_position] != '\n')
                continue;

            // break to code below when delimiter found and no state or certain states found
            if (parser_is_delimiter(line[line_position]) &&
                (!parser_state || (parser_state & IN_MATHEMATICAL_EXPRESSION) || (parser_state & IN_ASSIGNMENT) ||
                 (parser_state & IN_GLOB_EXPANSION))) {
                break;
            }
            parser_buffer[parser_buf_pos++] = line[line_position];
            continue;
        }
        }

        parser_buffer[parser_buf_pos] = '\0';

        debugf("Current parser state: %d\n", state);
        if ((parser_state & IN_GLOB_EXPANSION)) {
            glob_t glob_buf = {0};
            size_t glob_len;
            glob(parser_buffer, GLOB_DOOFFS, NULL, &glob_buf);

            for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
                glob_len = strlen(glob_buf.gl_pathv[i]) + 1;
                if (!glob_len || glob_len >= NCSH_MAX_INPUT)
                    break;

                parser_buf_pos = glob_len;
                args->values[args->count] = arena_malloc(scratch_arena, parser_buf_pos, char);
                memcpy(args->values[args->count], glob_buf.gl_pathv[i], glob_len);
                args->ops[args->count] = OP_CONSTANT;
                args->lengths[args->count] = parser_buf_pos;
                ++args->count;
                if (args->count >= PARSER_TOKENS_LIMIT - 1)
                    break;
            }

            globfree(&glob_buf);
            parser_state &= ~IN_GLOB_EXPANSION;
        }
        else if ((parser_state & IN_ASSIGNMENT)) {
            // variable values are stored in vars hashmap.
            // the key is the previous value, which is tagged with OP_VARIABLE.
            // when VM comes in contact with OP_VARIABLE, it looks up value in vars.
            debugf("saving var_name %s\n", var_name);
            struct estr* val = arena_malloc(arena, 1, struct estr);
            val->length = parser_buf_pos - parser_assignment_pos + 2;
            val->value = arena_malloc(arena, val->length, char);
            memcpy(val->value, parser_buffer + parser_assignment_pos - 1, val->length);
            debugf("%s : %s (%zu)\n", var_name, val->value, val->length);
            char* key = arena_malloc(arena, parser_assignment_pos, char);
            memcpy(key, var_name, parser_assignment_pos);
            debugf("key value %s\n", key);
            var_set(key, val, arena, &args->vars);
            parser_state &= ~IN_ASSIGNMENT;
        }
        else if (!(args->count == 0 && CMP_2(parser_buffer, AND))) {
            args->values[args->count] = arena_malloc(scratch_arena, parser_buf_pos + 1, char);
            memcpy(args->values[args->count], parser_buffer, parser_buf_pos + 1);
            args->ops[args->count] = parser_op_get(parser_buffer, parser_buf_pos);
            args->lengths[args->count] = parser_buf_pos + 1; // + 1 for null terminator
            ++args->count;
        }

        parser_buffer[0] = '\0';
        parser_buf_pos = 0;
    }

    debug_args(args);
}

/* parser_parse_noninteractive
 * Parse the command line input into commands, command lengths, and op codes stored in struct Args.
 * Allocates memory that is freed by parser_free_values at the end of each main loop of the shell.
 * Used for noninteractive mode.
 */
void parser_parse_noninteractive(const char** const restrict inputs, const size_t inputs_count,
                                 struct Args* const restrict args, struct Arena* arena, struct Arena* scratch_arena)
{
    assert(inputs && inputs_count && args && arena && scratch_arena);

    if (!inputs || inputs_count == 0) {
        args->count = 0;
        return;
    }

    // parse the split input one parameter at a time.
    for (size_t i = 0; i < inputs_count; ++i) {
        parser_parse(inputs[i], strlen(inputs[i]) + 1, args, arena, scratch_arena);
    }

    debug_args(args);
}
