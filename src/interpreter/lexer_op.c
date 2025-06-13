#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "../defines.h"
#include "interpreter_types.h"
#include "lexer_defines.h"

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
