// Copyright (c) z by Alex Eski 2024

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../z.h"
#include "../z_tests.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    struct z_Database db = {};
    z_init(&config_location, &db);
    z_add((char*)Data, Size, &db);
    z_exit(&db);
    return 0;
}
