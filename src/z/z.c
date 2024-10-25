#include <stdint.h>
#include <stdio.h>

#include "z_database.h"

enum z_Database_Result z_start(struct z_Database* database) {
	enum z_Database_Result result;
	if ((result = z_database_malloc(database)) != Z_SUCCESS)
		return result;

	if ((result = z_database_load(database)) != Z_SUCCESS)
		return result;

	return Z_SUCCESS;
}

struct eskilib_String* z_process (const struct eskilib_String* target, struct z_Database* database) {
	printf("z_run: %s\n", target->value);

	z_database_add(target, database);

	//struct eskilib_String* match = z_database_get_match(target, &database);

	return (struct eskilib_String*)target;
}

enum z_Database_Result z_finish(struct z_Database* database) {
	enum z_Database_Result result;
	if ((result = z_database_save(database)) != Z_SUCCESS)
		return result;
	if ((result = z_database_clean(database)) != Z_SUCCESS)
		return result;
	if ((result = z_database_free(database)) != Z_SUCCESS)
		return result;

	return Z_SUCCESS;
}
