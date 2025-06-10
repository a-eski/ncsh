/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "tokens.h"

/* lexer_lex
 * Turns the inputted line into values, lengths, and bytecodes that can be parsed
 * Returns: The args ready to be processed by the parser, allocated with the scratch arena.
 */
Tokens* lexer_lex(char* rst line, size_t length, Arena* rst scratch);

/* lexer_lex_noninteractive
 * Turns the inputted line into values, lengths, and bytecodes that can be parsed.
 * Used for noninteractive mode.
 * Returns: The args ready to be processed by the parser, allocated with the scratch arena.
 */
Tokens* lexer_lex_noninteractive(char** rst inputs, size_t inputs_count, Arena* rst scratch);
