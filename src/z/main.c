//for testing only
#include <stdio.h>

#include "z_database.h"
#include "z.h"
#include "../eskilib/eskilib_string.h"

int main(void) {
	struct z_Database database;

	z_start(&database);

	const struct eskilib_String test = { .value = "ncsh", .length = 5 };
	struct eskilib_String result = z_process(test, &database);
	printf("result.length: %lu, result.value: %s", result.length, result.value);

	z_finish(&database);

	return 0;
}

