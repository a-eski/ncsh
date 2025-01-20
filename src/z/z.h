// Copyright (c) z by Alex Eski 2024

#ifndef z_h
#define z_h

#include <stdint.h>
#include <time.h>

#include "../eskilib/eskilib_string.h"

#define Z_DATABASE_FILE "_z_database.bin"
#define Z_DATABASE_IN_MEMORY_LIMIT 100

#define Z_SECOND 1
#define Z_MINUTE 60 * Z_SECOND
#define Z_HOUR 60 * Z_MINUTE
#define Z_DAY 24 * Z_HOUR
#define Z_WEEK 7 * Z_DAY
#define Z_MONTH 30 * Z_DAY

struct z_Directory
{
    double rank;
    time_t last_accessed;
    char *path;
    size_t path_length;
};

struct z_Database
{
    // bool dirty;
    size_t count;
    char *database_file;
    struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT];
};

enum z_Result
{
    Z_BAD_STRING = -8,
    Z_FILE_LENGTH_TOO_LARGE = -7,
    Z_MATCH_NOT_FOUND = -6,
    Z_NULL_REFERENCE = -5,
    Z_STDIO_ERROR = -4,
    Z_MALLOC_ERROR = -3,
    Z_ZERO_BYTES_READ = -2,
    Z_FILE_ERROR = -1,
    Z_FAILURE = 0,
    Z_SUCCESS = 1
};

enum z_Result z_init(struct eskilib_String config_location, struct z_Database *database);

void z(char *target, size_t target_length, const char *cwd, struct z_Database *db);

enum z_Result z_add(char *path, size_t path_length, struct z_Database *db);

enum z_Result z_exit(struct z_Database *db);

#endif // !z_h
