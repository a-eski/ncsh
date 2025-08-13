/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "../types.h"

enum Dir_Type {
    DIR_NORMAL,
    DIR_SHORT,
    DIR_NONE
};

typedef struct {
    bool show_user;
    enum Dir_Type dir_type;
} Prompt_Data;

void prompt_init(bool showUser, enum Dir_Type dir_type);

int prompt_if_needed(Input* restrict input);
