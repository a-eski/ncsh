// Copyright (c) ncsh by Alex Eski 2024

#include <assert.h>
#include <glob.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eskilib/eskilib_defines.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"
#include "ncsh_defines.h"
#include "ncsh_parser.h"

// #define STRCMP_1CHAR(val1, val2) (val1[0] == val2[0])
#define STRCMP_2CHAR(val1, val2) (val1[0] == val2[0] && val1[1] == val2[1])
#define STRCMP_3CHAR(val1, val2) (val1[0] == val2[0] && val1[1] == val2[1] && val1[2] == val2[2])

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
#define BANG '!'

eskilib_nodiscard enum eskilib_Result ncsh_parser_args_malloc(struct ncsh_Args *args)
{
    if (args == NULL)
        return E_FAILURE_NULL_REFERENCE;

    args->count = 0;

    args->values = calloc(NCSH_PARSER_TOKENS, sizeof(char *));
    if (!args->values)
        return E_FAILURE_MALLOC;

    args->ops = calloc(NCSH_PARSER_TOKENS, sizeof(uint_fast8_t));
    if (!args->ops)
    {
        free(args->values);
        return E_FAILURE_MALLOC;
    }

    args->lengths = calloc(NCSH_PARSER_TOKENS, sizeof(size_t));
    if (!args->lengths)
    {
        free(args->values);
        free(args->ops);
        return E_FAILURE_MALLOC;
    }

    return E_SUCCESS;
}

void ncsh_parser_args_free(struct ncsh_Args *args)
{
    if (!args)
        return;

    ncsh_parser_args_free_values(args);
    if (args->values)
    {
        free(args->values);
        args->values = NULL;
    }
    if (args->ops)
    {
        free(args->ops);
        args->ops = NULL;
    }
    if (args->lengths)
    {
        free(args->lengths);
        args->lengths = NULL;
    }
    args->count = 0;
}

void ncsh_parser_args_free_values(struct ncsh_Args *args)
{
    if (args->count == 0)
        return;

    for (uint_fast32_t i = 0; i < args->count; ++i)
    {
        if (args->values[i])
        {
            free(args->values[i]);
            args->values[i] = NULL;
        }
    }
    args->count = 0;
}

eskilib_nodiscard bool ncsh_is_delimiter(char ch)
{
    switch (ch)
    {
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

char *ops_2char_str[] = {
    STDOUT_REDIRECTION_APPEND,
    STDERR_REDIRECTION,
    STDOUT_AND_STDERR_REDIRECTION,
    AND,
    OR,
    EXPONENTIATION
};

enum ncsh_Ops ops_2char[] = {
    OP_STDOUT_REDIRECTION_APPEND,
    OP_STDERR_REDIRECTION,
    OP_STDOUT_AND_STDERR_REDIRECTION,
    OP_AND,
    OP_OR,
    OP_EXPONENTIATION
};

char *ops_3char_str[] = {
    STDERR_REDIRECTION_APPEND,
    STDOUT_AND_STDERR_REDIRECTION_APPEND
};

enum ncsh_Ops ops_3char[] = {
    OP_STDERR_REDIRECTION_APPEND,
    OP_STDOUT_AND_STDERR_REDIRECTION_APPEND
};

eskilib_nodiscard enum ncsh_Ops ncsh_op_get(const char *line, size_t length)
{
    switch (length)
    {
    case 0: {
        return OP_NONE;
    }
    case 1: {
        switch (line[0])
        {
        case PIPE: { return OP_PIPE; }
        case STDOUT_REDIRECTION: { return OP_STDOUT_REDIRECTION; }
        case STDIN_REDIRECTION: { return OP_STDIN_REDIRECTION; }
        case BACKGROUND_JOB: { return OP_BACKGROUND_JOB; }
        case ADD: { return OP_ADD; }
        case SUBTRACT: { return OP_SUBTRACT; }
        case MULTIPLY: { return OP_MULTIPLY; }
        case DIVIDE: { return OP_DIVIDE; }
        case MODULO: { return OP_MODULO; }
        case OPENING_PARAN: { return OP_MATH_EXPRESSION_START; }
        case CLOSING_PARAN: { return OP_MATH_EXPRESSION_END; }
        default: { return OP_CONSTANT; }
        }
    }
    case 2: {
        for (uint_fast32_t i = 0; i < sizeof(ops_2char_str) / sizeof(char *); ++i)
        {
            if (STRCMP_2CHAR(line, ops_2char_str[i]))
                return ops_2char[i];
        }

        return OP_CONSTANT;
    }
    case 3: {
        for (uint_fast32_t i = 0; i < sizeof(ops_3char_str) / sizeof(char *); ++i)
        {
            if (STRCMP_3CHAR(line, ops_3char_str[i]))
                return ops_3char[i];
        }

        return OP_CONSTANT;
    }
    default: {
        return OP_CONSTANT;
    }
    }
}

enum ncsh_Parser_State
{
    IN_SINGLE_QUOTES = 0x01,
    IN_DOUBLE_QUOTES = 0x02,
    IN_BACKTICK_QUOTES = 0x04,
    IN_MATHEMATICAL_EXPRESSION = 0x08
};

void ncsh_parser_parse(const char *line, size_t length, struct ncsh_Args *args)
{
    assert(args);
    assert(line);

    if (!line || length < 2 || length > NCSH_MAX_INPUT)
    {
        args->count = 0;
        return;
    }

    assert(length >= 2);

#ifdef NCSH_DEBUG
    ncsh_debug_parser_input(line, length);
#endif /* ifdef NCSH_DEBUG */

    char buffer[NCSH_MAX_INPUT];
    size_t buf_pos = 0;
    bool glob_found = false;
    int state = 0;

    for (uint_fast32_t line_position = 0; line_position < length + 1; ++line_position)
    {
        if (args->count == NCSH_PARSER_TOKENS - 1 && line_position < length)
        { // can't parse all of the args
            ncsh_parser_args_free_values(args);
            args->count = 0;
            break;
        }
        else if (line_position == length || line_position >= NCSH_MAX_INPUT - 1 || buf_pos >= NCSH_MAX_INPUT - 1 ||
                 args->count == NCSH_PARSER_TOKENS - 1)
        {
            args->values[args->count] = NULL;
            break;
        }
        else if (ncsh_is_delimiter(line[line_position]) && (state == 0 || (state & IN_MATHEMATICAL_EXPRESSION)))
        {
            buffer[buf_pos] = '\0';

            if (glob_found)
            {
                glob_t glob_buf = {0};
                size_t glob_len;
                glob(buffer, GLOB_DOOFFS, NULL, &glob_buf);

                for (size_t i = 0; i < glob_buf.gl_pathc; ++i)
                {
                    glob_len = strlen(glob_buf.gl_pathv[i]) + 1;
                    if (glob_len == 0 || glob_len >= NCSH_MAX_INPUT)
                        break;
                    buf_pos = glob_len;
                    args->values[args->count] = malloc(buf_pos);
                    memcpy(args->values[args->count], glob_buf.gl_pathv[i], glob_len);
                    args->ops[args->count] = OP_CONSTANT;
                    args->lengths[args->count] = buf_pos;
                    ++args->count;
                    if (args->count >= NCSH_PARSER_TOKENS- 1)
                        break;
                }

                globfree(&glob_buf);
                glob_found = false;
            }
            else
            {
                args->values[args->count] = malloc(buf_pos + 1);
                eskilib_string_copy(args->values[args->count], buffer, buf_pos + 1);
                args->ops[args->count] = ncsh_op_get(buffer, buf_pos);
                args->lengths[args->count] = buf_pos + 1; // + 1 for null terminator
                ++args->count;
            }

            buffer[0] = '\0';
            buf_pos = 0;
        }
        else
        {
            switch (line[line_position])
            {
            case DOUBLE_QUOTE_KEY: {
                if (!(state & IN_DOUBLE_QUOTES))
                    state |= IN_DOUBLE_QUOTES;
                else
                    state &= ~IN_DOUBLE_QUOTES;
                break;
            }
            case SINGLE_QUOTE_KEY: {
                if (!(state & IN_SINGLE_QUOTES))
                    state |= IN_SINGLE_QUOTES;
                else
                    state &= ~IN_SINGLE_QUOTES;
                break;
            }
            case BACKTICK_QUOTE_KEY: {
                if (!(state & IN_BACKTICK_QUOTES))
                    state |= IN_BACKTICK_QUOTES;
                else
                    state &= ~IN_BACKTICK_QUOTES;
                break;
            }
	    case OPENING_PARAN: {
		if (!(state & IN_MATHEMATICAL_EXPRESSION))
		    state |= IN_MATHEMATICAL_EXPRESSION;

                buffer[buf_pos++] = line[line_position];
	        break;
	    }
	    case CLOSING_PARAN: {
		if (state & IN_MATHEMATICAL_EXPRESSION)
		    state &= ~IN_MATHEMATICAL_EXPRESSION;

                buffer[buf_pos++] = line[line_position];
		break;
	    }
            case TILDE: {
                char *home = getenv("HOME");
                size_t home_length = strlen(home);
                if (buf_pos + home_length > NCSH_MAX_INPUT)
                {
                    // protect from overflow
                    ncsh_parser_args_free_values(args);
                    args->count = 0;
                    return;
                }
                memcpy(buffer + buf_pos, home, home_length);
                buf_pos += home_length;
                break;
            }
            case GLOB_STAR: {
                if (!(state & IN_MATHEMATICAL_EXPRESSION))
                    glob_found = true;
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
    ncsh_debug_args(args);
#endif /* ifdef NCSH_DEBUG */
}

            /*if (!state)
            {
                if (line_position == length)
                { // missing closing quote or end of mathematical expression

                    ncsh_parser_args_free_values(args);
                    args->count = 0;
                    break;
                }
                // check if quotes not closed, overlap, etc.
                // if (state & IN_DOUBLE_QUOTES)
                // if (state & IN_SINGLE_QUOTES)
                // if (state & IN_BACKTICK_QUOTES)
                // if (state & IN_MATHEMATICAL_EXPRESSION)
            }*/
