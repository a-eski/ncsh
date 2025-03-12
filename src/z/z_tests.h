/* Copyright (c) z by Alex Eski 2024 */
/* Used by unit tests for z */

#include <stdint.h>
#include <time.h>

#include "../eskilib/eskilib_string.h"
#include "z.h"

static const struct eskilib_String config_location = { .length = 0, .value = NULL };

double z_score(struct z_Directory* const restrict directory,
               const int fzf_score,
               const time_t now);

struct z_Directory* z_match_find(char* const target,
                                 const size_t target_length,
                                 const char* const cwd,
                                 const size_t cwd_length,
                                 struct z_Database* const restrict db);

enum z_Result z_database_add(const char* const path,
                             const size_t path_length,
                             const char* const cwd,
                             const size_t cwd_length,
                             struct z_Database* const restrict db);

void z_free(struct z_Database* const restrict db);
