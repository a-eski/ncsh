#pragma once

// #include "../ncsh_arena.h"
#include "../../src/ncsh_arena.h"

static const int8_t* capabilities;

void ncsh_terminfo_capabilities_set(struct ncsh_Arena* const scratch_arena);
