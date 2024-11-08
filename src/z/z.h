#ifndef z_main_h
#define z_main_h

#include <stdint.h>
#include <time.h>

#include "../eskilib/eskilib_string.h"

#define Z_DATABASE_FILE "database.z"
#define Z_DATABASE_FILE_LENGTH 10
#define Z_DATABASE_IN_MEMORY_LIMIT 1000

#define Z_SECOND 1
#define Z_MINUTE 60 * Z_SECOND
#define Z_HOUR 60 * Z_MINUTE
#define Z_DAY 24 * Z_HOUR
#define Z_WEEK 7 * Z_DAY
#define Z_MONTH 30 * Z_DAY

struct z_Directory {
	double rank;
	clock_t last_accessed;
	struct eskilib_String path;
};

enum z_Database_Result {
	Z_FAILURE_FILE_OP = -3,
	Z_FAILURE_MALLOC = -2,
	Z_FAILURE_NULL_REFERENCE = -1,
	Z_FAILURE = 0,
	Z_SUCCESS = 1,
};

struct z_Database {
	bool dirty;
	uint_fast32_t count;
	uint_fast32_t start_count;
	// uint_fast8_t* bytes;
	struct z_Directory* directories;
};

enum z_Database_Result z_start (const size_t config_path_max, const struct eskilib_String config_path, struct z_Database* database);

struct eskilib_String z_process (const struct eskilib_String target, struct z_Database* database);

enum z_Database_Result z_finish (struct z_Database* database);

#endif // !z_main_h

