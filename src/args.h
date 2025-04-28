/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "arena.h"

struct Arg {
    uint8_t op;
    size_t len;
    char* val;
    struct Arg* next;
};

struct Args {
    size_t count;
    struct Arg* args;
};

struct Arg* arg_alloc(uint8_t op, size_t len, char* val, struct Arena* const restrict arena);

enum eresult arg_set_after(struct Arg* currentNode, struct Arg* nodeToSetAfter);
