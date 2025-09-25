/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "../eskilib/str.h"

#define LEXER_TOKENS_LIMIT 128

enum Token {
    T_NONE,
    T_CONST,
    T_NUM,
    T_QUOTE,     // '
    T_D_QUOTE,   // "
    T_BACKTICK,  // `
    T_O_PARAN,   // (
    T_C_PARAN,   // )
    T_O_BRACK,   // [
    T_C_BRACK,   // ]
    T_SEMIC,     // ;
    T_GLOB,      // *, ?
    // T_STAR,      // *
    // T_QUESTION,  // ?
    T_BANG,      // !
    T_EQ,        // =
    T_GT,        // >
    T_LT,        // <
    T_DOLLAR,    // $
    T_HOME,      // ~
    T_WS,        // whitespace \t, \r, \n, \0, ' '
    T_PIPE,      // |
    T_AMP,       // &
    T_IF,
    T_FI,
    T_THEN,
    T_ELSE,
    T_ELIF,
    T_DO,
    T_FOR,
    T_WHILE,
    T_DONE,
    T_EQ_A,     // -eq
    T_GT_A,     // -gt
    T_GE_A,     // -ge
    T_LT_A,     // -lt
    T_LE_A,     // -le
    T_TRUE,     // true
    T_FALSE,    // false
};

typedef struct {
    size_t count;
    uint8_t* ops;
    Str* strs;
} Lexemes;

/* lex
 * Turns the inputted line into values, lengths, and bytecodes that can be parsed.
 * Populates lexemes with the output.
 */
void lex(Str line, Lexemes* lexemes, Arena* restrict scratch);

/* lex_noninteractive
 * Turns the inputted line into values, lengths, and bytecodes that can be parsed.
 * Used for noninteractive mode.
 * Populates lexemes with the output.
 */
void lex_noninteractive(char** restrict inputs, size_t inputs_count, Lexemes* lexemes, Arena* restrict scratch);
