#include <stdint.h>
#include <stdio.h>

#include "z_database.h"

static struct z_Database database;

uint_fast32_t z_start(void) {
	if (z_database_malloc(&database) != Z_SUCCESS)
		return 1;
	if (z_database_load(&database) != Z_SUCCESS)
		return 1;

	return 0;
}

struct eskilib_String* z_process (const struct eskilib_String* target) {
	printf("z_run: %s\n", target->value);

	//struct eskilib_String* match = z_database_get_match(target, &database);

	return (struct eskilib_String*)target;
}

uint_fast32_t z_finish(void) {
	if (z_database_save(&database) != Z_SUCCESS)
		return 1;
	if (z_database_clean(&database) != Z_SUCCESS)
		return 1;
	z_database_free(&database);

	return 0;
}

