// Copyright (c) z by Alex Eski 2024

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../src/z/z.h"
#include "../lib/arena_test_helper.h"

static Str config_location = {.length = 0, .value = NULL};

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char cwd[PATH_MAX];
    if (!getcwd(cwd, PATH_MAX)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        return EXIT_FAILURE;
    }

    z_Database db = {};
    z_init(&config_location, &db, &arena);
    uint8_t* data = arena_malloc(&arena, Size, uint8_t);
    memcpy(data, Data, Size);
    z((char*)data, Size, cwd, &db, &arena, scratch_arena);
    z_exit(&db);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;

    chdir(cwd);

    return EXIT_SUCCESS;
}
