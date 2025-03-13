// Copyright (c) z by Alex Eski 2024

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../src/z/z.h"

static const struct eskilib_String config_location = { .length = 0, .value = NULL };

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    struct z_Database db = {};
    z_init(&config_location, &db);
    z((char*)Data, Size, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh/src/z/tests", &db);
    z_exit(&db);
    return 0;
}
