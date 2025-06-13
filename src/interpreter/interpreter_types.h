/* Copyright ncsh (C) by Alex Eski 2025 */
/* interpreter_types.h: interpreter types for ncsh */

#pragma once

#include <stdint.h>
#include <unistd.h>

#include "tokens.h"
#include "ops.h"

/* Token
 * Holds op code, length, and value of the parsed token.
 * Includes a pointer to next element in the linked list.
 */
// typedef struct Token_ {
//     uint8_t op; // enum Ops
//     size_t len;
//     char* val;
//     struct Token_* next;
// } Token;

/*** PREPROCSSING AND LOGIC TYPES ***/
typedef struct {
    size_t count;
    size_t cap;
    enum Ops* ops;
    size_t* lens;
    char** vals;
} Commands;

typedef struct {
    size_t count;
    size_t cap;
    Commands* commands;
} Statements;
