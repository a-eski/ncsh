//for testing only


#include "z_database.h"

int main(void) {
	struct z_Database database;

	z_database_malloc(&database);
	z_database_load(&database);

	z_database_free(&database);

	return 0;
}

