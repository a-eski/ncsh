/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "../types.h"

enum Dir_Type {
    DIR_NORMAL,
    DIR_SHORT,
    DIR_NONE,
    DIR_FISH
};

typedef struct {
    bool show_user;
    enum Dir_Type dir_type;
} Prompt_Data;


void prompt_init();

Str prompt_get(Input* restrict input, Arena* restrict scratch);

void prompt_set(bool show_user, enum Dir_Type dir_type);
int prompt_dir_type_set(Str dir_type);
int prompt_show_user_set(Str show_user);
