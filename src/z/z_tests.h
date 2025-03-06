// Copyright (c) z by Alex Eski 2024
// Used by unit tests for z

#include <stdint.h>
#include <time.h>

#include "../eskilib/eskilib_string.h"
#include "z.h"

double z_score(struct z_Directory* directory, time_t now);

struct z_Directory* z_match_find(char* target, size_t target_length, const char* cwd, size_t cwd_length,
                                 struct z_Database* db);

enum z_Result z_database_add(char* path, size_t path_length, const char* cwd, size_t cwd_length,
                                    struct z_Database* db);

void z_free(struct z_Database* db);
