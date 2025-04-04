// Copyright (c) z by Alex Eski 2024

#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/ncsh_arena_test_helper.h"
#include "../src/z/z.h"

static const struct eskilib_String config_location = { .length = 0, .value = NULL };

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    char cwd[PATH_MAX];
    if (!getcwd(cwd, PATH_MAX)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        return EXIT_FAILURE;
    }

    struct z_Database db = {};
    z_init(&config_location, &db, &arena);
    uint8_t* data = malloc(Size);
    memcpy(data, Data, Size);
    z((char*)data, Size, cwd, &db, &arena, scratch_arena);
    z_exit(&db);

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;

    chdir(cwd);

    return EXIT_SUCCESS;
}
