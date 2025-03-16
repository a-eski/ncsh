// Copyright (c) z by Alex Eski 2024

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/ncsh_arena_test_helper.h"
#include "../src/z/z.h"

static const struct eskilib_String config_location = { .length = 0, .value = NULL };

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    NCSH_ARENA_TEST_SETUP;

    struct z_Database db = {};
    z_init(&config_location, &db, &arena);
    z_add((char*)Data, Size, &db, &arena);
    z_exit(&db);

    NCSH_ARENA_TEST_TEARDOWN;
    return 0;
}
