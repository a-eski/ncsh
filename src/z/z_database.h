#ifndef z_database_h
#define z_database_h

#include <stdint.h>

#include "z_directory.h"

#define Z_DATABASE_FILE "database.z"
#define Z_DATABASE_IN_MEMORY_LIMIT 1000

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

enum z_Database_Result z_database_malloc(struct z_Database* database);

enum z_Database_Result z_database_load(struct z_Database* database);

enum z_Database_Result z_database_save(struct z_Database* database);

enum z_Database_Result z_database_clean(struct z_Database* database);

void z_database_free(struct z_Database* database);

enum z_Database_Result z_database_add(struct z_Database* database);

struct eskilib_String* z_database_get_match(const struct eskilib_String* target, struct z_Database* database);

#endif // !z_database_h

