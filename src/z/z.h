#ifndef z_h
#define z_h

#include "time.h"
#include "../eskilib/eskilib_string.h"
#include <stdint.h>

#define Z_DATABASE_FILE "z_database.bin"
#define Z_DATABASE_FILE_LENGTH 10
#define Z_DATABASE_IN_MEMORY_LIMIT 100

#define Z_SECOND 1
#define Z_MINUTE 60 * Z_SECOND
#define Z_HOUR 60 * Z_MINUTE
#define Z_DAY 24 * Z_HOUR
#define Z_WEEK 7 * Z_DAY
#define Z_MONTH 30 * Z_DAY

struct z_Directory {
	double rank;
	time_t last_accessed;
	char* path;
	uint32_t path_length;
};

struct z_Database {
	// bool dirty;
	uint32_t count;
	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT];
};

extern void z_init(struct z_Database* database);

extern void z(const struct eskilib_String target, const char* directory, struct z_Database* database);

extern void z_free(struct z_Database* db);

#endif // !z_h
