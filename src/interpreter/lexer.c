/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <glob.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../configurables.h"
#include "../debug.h"
#include "../defines.h"
#include "interpreter_types.h"
#include "lexer.h"
#include "lexer_defines.h"
#include "lexer_op.h"

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
size_t lex_buf_pos;
size_t lexeme_pos;

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

void lexer_lex(char* rst line, size_t length, Lexemes* lexemes, Arena* rst scratch)
{
    assert(line && scratch);
    assert(!line[length - 1] && (length <= 2 || line[length - 2]));
    assert(lexemes);
    if (length < 2 || length > NCSH_MAX_INPUT) {
        lexemes->count = 0;
        return;
    }

    assert(length >= 2);

    debug_lexer_input(line, length);

    lex_buf = arena_malloc(scratch, NCSH_MAX_INPUT, char);
    lex_buf_pos = 0;
    lex_state = 0;
    size_t n = lexemes->count;

    for (register size_t pos = 0; pos < length; ++pos) {
        if (lexemes->count == LEXER_TOKENS_LIMIT - 1 && pos < length) { // can't lex all of the tokens
            lexemes->count = 0;
            break;
        }
        else if (pos >= NCSH_MAX_INPUT - 1 || lex_buf_pos >= NCSH_MAX_INPUT - 1 ||
                 lexemes->count == LEXER_TOKENS_LIMIT - 1) {
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

            lex_buf[lex_buf_pos++] = line[pos];
            continue;
        }
        case CLOSING_PARAN: {
            if (lex_state & IN_MATHEMATICAL_EXPRESSION)
                lex_state &= ~IN_MATHEMATICAL_EXPRESSION;

            lex_buf[lex_buf_pos++] = line[pos];
            continue;
        }
        case GLOB_STAR: {
            if (!(lex_state & IN_MATHEMATICAL_EXPRESSION) && !(lex_state & IN_GLOB_EXPANSION))
                lex_state |= IN_GLOB_EXPANSION;

            lex_buf[lex_buf_pos++] = line[pos];
            continue;
        }
        case GLOB_QUESTION: {
            if (!(lex_state & IN_GLOB_EXPANSION))
                lex_state |= IN_GLOB_EXPANSION;

            lex_buf[lex_buf_pos++] = line[pos];
            continue;
        }
        case ASSIGNMENT: {
            if (!(lex_state & IN_ASSIGNMENT))
                lex_state |= IN_ASSIGNMENT;

            lex_buf[lex_buf_pos++] = line[pos];
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

            lex_buf[lex_buf_pos++] = line[pos];
            continue;
        }
        case TILDE: {
            if (!(lex_state & IN_HOME_EXPANSION))
                lex_state |= IN_HOME_EXPANSION;

            lex_buf[lex_buf_pos++] = line[pos];
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
                    lex_buf_pos = 0;
                }
            }

            if (lex_state & IN_SINGLE_QUOTES || lex_state & IN_DOUBLE_QUOTES || lex_state & IN_BACKTICK_QUOTES) {

                lex_buf[lex_buf_pos++] = line[pos];
                continue;
            }

            // break to code below when delimiter found and no state or certain states found
            if ((!lex_state || (lex_state & IN_MATHEMATICAL_EXPRESSION) || (lex_state & IN_ASSIGNMENT) ||
                 (lex_state & IN_GLOB_EXPANSION) || (lex_state & IN_HOME_EXPANSION))) {
                break;
            }

            lex_buf[lex_buf_pos++] = line[pos];
            continue;
        }
        default: {
            lex_buf[lex_buf_pos++] = line[pos];
            continue;
        }
        }

        lex_buf[lex_buf_pos] = '\0';

        debugf("Current lexer state: %d\n", lex_state);

        uint8_t op = lexer_op_process();
        lexemes->ops[n] = lexer_op_process();
        lexemes->lens[n] = lex_buf_pos + 1;
        lexemes->vals[n] = arena_malloc(scratch, lexemes->lens[n], char);
        memcpy(lexemes->vals[n], lex_buf, lex_buf_pos);
        ++n;

        lex_buf[0] = '\0';
        lex_buf_pos = 0;
    }

    lexemes->count = n;
    debug_tokens(toks);
}

/* lexer_lex
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 * Handles expansions like *, ?, and ~
 */
/*[[nodiscard]]
Tokens* lexer_lex(char* rst line, size_t length, Arena* rst scratch)
{
    assert(line && scratch);
    assert(!line[length - 1] && (length <= 2 || line[length - 2]));
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
}*/

/* lexer_lex_noninteractive
 * lex the command line input into commands, command lengths, and op codes then stored in Tokens.
 * Allocates memory that is freed by lexer_free_values at the end of each main loop of the shell.
 * Used for noninteractive mode.
 */
void lexer_lex_noninteractive(char** rst inputs, size_t inputs_count, Lexemes* lexemes, Arena* rst scratch)
{
    assert(inputs && inputs_count && scratch);

    if (!inputs || inputs_count == 0) {
        return;
    }

    for (size_t i = 0; i < inputs_count; ++i) {
        lexer_lex(inputs[i], strlen(inputs[i]) + 1, lexemes, scratch);
    }

    debug_tokens(toks);
}
