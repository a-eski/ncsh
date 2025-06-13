/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "lexemes.h"


/* lexer_lex
 * Turns the inputted line into values, lengths, and bytecodes that can be parsed.
 * Populates lexemes with the output.
 */
void lexer_lex(char* rst line, size_t length, Lexemes* lexemes, Arena* rst scratch);

/* lexer_lex_noninteractive
 * Turns the inputted line into values, lengths, and bytecodes that can be parsed.
 * Used for noninteractive mode.
 * Populates lexemes with the output.
 */
void lexer_lex_noninteractive(char** rst inputs, size_t inputs_count, Lexemes* lexemes, Arena* rst scratch);
