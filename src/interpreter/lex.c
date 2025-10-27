/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <glob.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../debug.h"
#include "../defines.h" // used for macros like NCSH_MAX_INPUT
#include "lex.h"
#include "symbols.h"

/* Lexer Internal Variables */
static char* restrict lex_buf;
static size_t lex_state;
static size_t lex_buf_pos;

[[nodiscard]]
static inline enum Token get_const_type(Str s)
{
    return estrisnum(s) ? T_NUM : T_CONST;
}

// clang-format off
enum Lexer_State: size_t {
    IN_NONE =                    0,
    IN_COMMENT =                 1 << 0,
};
// clang-format on

/* ops_2char_str
 * A constant array that contains all shell operations that are 2 characters long, like ">>", "||", "&&".
 * Size of the array is stored as constant expression in ops_2char_len
 * Bytecodes (opcodes) equivalents are stored in the array of enum Ops, ops_2char
 */
static constexpr char ops_2char_str[4][2] = {IF, FI, DO, IN};

static constexpr size_t ops_2char_len = sizeof(ops_2char_str) / sizeof(ops_2char_str[0]);

static constexpr enum Token ops_2char[] = {T_IF, T_FI, T_DO, T_IN};

/* ops_3char_str
 * A constant array that contain all shell operations that are 3 characters long, like "&>>".
 * Size of the array is stored as constant expression in ops_3char_len
 * Bytecodes (opcodes) equivalents are stored in the array of enum Ops, ops_3char
 */
static constexpr char ops_3char_str[6][3] = {EQUALS,
                                             LESS_THAN,
                                             LESS_THAN_OR_EQUALS,
                                             GREATER_THAN,
                                             GREATER_THAN_OR_EQUALS,
                                             FOR};

static constexpr size_t ops_3char_len = sizeof(ops_3char_str) / sizeof(ops_3char_str[0]);

static constexpr enum Token ops_3char[] = {T_EQ_A,
                                           T_LT_A,
                                           T_LE_A,
                                           T_GT_A,
                                           T_GE_A,
                                           T_FOR};

[[nodiscard]]
static inline enum Token tok_check_len_one(Str s)
{
    if (s.value[0] == MINUS)
        return T_MINUS;
    if (s.value[0] == FSLASH)
        return T_FSLASH;

    return get_const_type(s);
}

[[nodiscard]]
static inline enum Token tok_check_len_two(Str s)
{
    for (size_t i = 0; i < ops_2char_len; ++i) {
        if (CMP_2(s.value, ops_2char_str[i])) {
            return ops_2char[i];
        }
    }

    return get_const_type(s);
}

[[nodiscard]]
static inline enum Token tok_check_len_three(Str s)
{
    for (size_t i = 0; i < ops_3char_len; ++i) {
        if (CMP_3(s.value, ops_3char_str[i])) {
            return ops_3char[i];
        }
    }

    return get_const_type(s);
}

[[nodiscard]]
static inline enum Token tok_check_len_four(Str s)
{
    switch (s.value[0]) {
    case 't': {
        if (!memcmp(s.value, BOOL_TRUE, sizeof(BOOL_TRUE) - 1))
            return T_TRUE;
        else if (!memcmp(s.value, THEN, sizeof(THEN) - 1))
            return T_THEN;
        break;
    }
    case 'e': {
        if (!memcmp(s.value, ELSE, sizeof(ELSE) - 1))
            return T_ELSE;
        else if (!memcmp(s.value, ELIF, sizeof(ELIF) - 1))
            return T_ELIF;
        break;
    }
    case 'd': {
        if (!memcmp(s.value, DONE, sizeof(DONE) - 1))
            return T_DONE;
        break;
    }
    }
    return get_const_type(s);
}

[[nodiscard]]
static inline enum Token tok_check_len_five(Str s)
{
    switch (s.value[0]) {
    case 'f': {
        if (!memcmp(s.value, BOOL_FALSE, sizeof(BOOL_FALSE) - 1))
            return T_FALSE;
        break;
    }
    case 'w': {
        if (!memcmp(s.value, WHILE, sizeof(WHILE) - 1))
            return T_WHILE;
        break;
    }
    }
    return get_const_type(s);
}

/* tok_get
 * Internal function used to map the inputted line to a bytecode.
 * Returns: a value from enum Ops, the bytecode relevant to the input
 */
[[nodiscard]]
enum Token tok_get(Str s)
{
    assert(s.value);

    switch (s.length - 1) {
    case 0: {
        return T_NONE;
    }

    case 1: {
        return tok_check_len_one(s);
    }

    case 2: {
        return tok_check_len_two(s);
    }

    case 3: {
        return tok_check_len_three(s);
    }

    case 4: {
        return tok_check_len_four(s);
    }

    case 5: {
        return tok_check_len_five(s);
    }

    default: {
        return get_const_type(s);
    }
    }
}

void lexemes_init(Lexemes* restrict lexemes, Arena* restrict scratch)
{
    assert(lexemes);

    lexemes->ops = arena_malloc(scratch, LEXER_TOKENS_LIMIT, uint8_t);
    lexemes->strs = arena_malloc(scratch, LEXER_TOKENS_LIMIT, Str);
}

void lexeme_add(Lexemes* restrict lexemes, size_t* n, char c, enum Token tok, Arena* restrict scratch) {
    if (lex_buf_pos > 0 && *lex_buf) {
        lexemes->strs[*n].length = lex_buf_pos + 1;
        lexemes->strs[*n].value = arena_malloc(scratch, lexemes->strs[*n].length, char);
        memcpy(lexemes->strs[*n].value, lex_buf, lex_buf_pos);
        lexemes->ops[*n] = get_const_type(lexemes->strs[*n]);
        lex_buf_pos = 0;
        lex_buf[0] = 0;
        *n += 1;
    }

    lexemes->ops[*n] = tok;
    lexemes->strs[*n].length = 2;
    lexemes->strs[*n].value = arena_malloc(scratch, 2, char);
    lexemes->strs[*n].value[0] = c;
    lexemes->strs[*n].value[1] = 0;
    *n += 1;
}

static inline bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/* lex
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 */
void lex(Str line, Lexemes* lexemes, Arena* restrict scratch)
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

    debug_lexer_input(line);

    lex_buf = arena_malloc(scratch, NCSH_MAX_INPUT, char);
    lex_buf_pos = 0;
    lex_state = 0;
    size_t n = lexemes->count;
    enum Token t = T_NONE;

    for (size_t pos = 0; pos < line.length; ++pos) {
        if (lexemes->count == LEXER_TOKENS_LIMIT - 1 && pos < line.length) { // can't lex all of the tokens
            lexemes->count = 0;
            break;
        }
        else if (pos >= NCSH_MAX_INPUT - 1 || lex_buf_pos >= NCSH_MAX_INPUT - 1 ||
                 lexemes->count == LEXER_TOKENS_LIMIT - 1) {
            break;
        }

        switch (line.value[pos]) {
        case QUESTION: {
            t = T_GLOB;
            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        case STAR: {
            if (lex_buf_pos == 0 && pos + 1 < line.length) {
                if (is_whitespace(line.value[pos + 1]) || line.value[pos + 1] == STAR) {
                    lexeme_add(lexemes, &n, line.value[pos], T_STAR, scratch);
                    continue;
                }
            }
            t = T_GLOB;
            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        case TILDE: {
            if (lex_buf_pos == 0) {
                t = T_HOME;
            }
            goto lex_default;
        }
        case DOUBLE_QUOTE: {
            lexeme_add(lexemes, &n, line.value[pos], T_D_QUOTE, scratch);
            continue;
        }
        case SINGLE_QUOTE: {
            lexeme_add(lexemes, &n, line.value[pos], T_QUOTE, scratch);
            continue;
        }
        case BACKTICK_QUOTE: {
            lexeme_add(lexemes, &n, line.value[pos], T_BACKTICK, scratch);
            continue;
        }
        case O_PARAN: {
            lexeme_add(lexemes, &n, line.value[pos], T_O_PARAN, scratch);
            continue;
        }
        case C_PARAN: {
            lexeme_add(lexemes, &n, line.value[pos], T_C_PARAN, scratch);
            continue;
        }
        case EQ: {
            lexeme_add(lexemes, &n, line.value[pos], T_EQ, scratch);
            continue;
        }
        case DOLLAR: {
            lexeme_add(lexemes, &n, line.value[pos], T_DOLLAR, scratch);
            continue;
        }
        case PIPE: {
            lexeme_add(lexemes, &n, line.value[pos], T_PIPE, scratch);
            continue;
        }
        case AMP: {
            lexeme_add(lexemes, &n, line.value[pos], T_AMP, scratch);
            continue;
        }
        case GT: {
            lexeme_add(lexemes, &n, line.value[pos], T_GT, scratch);
            continue;
        }
        case LT: {
            lexeme_add(lexemes, &n, line.value[pos], T_LT, scratch);
            continue;
        }
        case O_BRACKET: {
            lexeme_add(lexemes, &n, line.value[pos], T_O_BRACK, scratch);
            continue;
        }
        case C_BRACKET: {
            lexeme_add(lexemes, &n, line.value[pos], T_C_BRACK, scratch);
            continue;
        }
        case SEMICOLON: {
            lexeme_add(lexemes, &n, line.value[pos], T_SEMIC, scratch);
            continue;
        }
        case PLUS: {
            lexeme_add(lexemes, &n, line.value[pos], T_PLUS, scratch);
            continue;
        }
        case MOD: {
            lexeme_add(lexemes, &n, line.value[pos], T_MOD, scratch);
            continue;
        }
        case COMMENT: {
            if (!(lex_state & IN_COMMENT))
                lex_state |= IN_COMMENT;

            continue;
        }
        // whitespace/delimiter case // NOTE: should \t, \a, or EOF be included?
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
                    continue;
                }
            }

            if (lex_buf_pos == 0) {
                continue;
            }

            break;
        }
        default: {
        lex_default:
            lex_buf[lex_buf_pos++] = line.value[pos];
            continue;
        }
        }

        lex_buf[lex_buf_pos] = '\0';

        debugf("Current lexer state: %d\n", lex_state);

        lexemes->strs[n].length = lex_buf_pos + 1;
        lexemes->strs[n].value = arena_malloc(scratch, lexemes->strs[n].length, char);
        memcpy(lexemes->strs[n].value, lex_buf, lex_buf_pos);
        lexemes->ops[n] = t == T_NONE ? tok_get(lexemes->strs[n]) : t;
        ++n;

        lex_buf[0] = '\0';
        lex_buf_pos = 0;
        t = T_NONE;
    }

    lexemes->count = n;
    debug_lexemes(lexemes);
}

/* lex_noninteractive
 * lex the command line input into commands, command lengths, and op codes then stored in Tokens.
 * Used for noninteractive mode.
 */
void lex_noninteractive(char** restrict inputs, size_t inputs_count, Lexemes* lexemes, Arena* restrict scratch)
{
    assert(inputs && inputs_count && scratch);

    if (!inputs || inputs_count == 0) {
        return;
    }

    for (size_t i = 0; i < inputs_count; ++i) {
        lex(Str_Get(inputs[i]), lexemes, scratch);
    }

    debug_lexemes(lexemes);
}
