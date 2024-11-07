//for testing
#include <stdio.h>

#include "z.h"
#include "../eskilib/eskilib_string.h"

int main(void) {
	puts("Starting z tests.");
	struct eskilib_String config_path = eskilib_String_Empty;
	struct z_Database database;

	z_start(528, config_path, &database);

	const struct eskilib_String test = { .value = "ncsh", .length = 5 };
	struct eskilib_String result = z_process(test, &database);
	printf("result.length: %lu, result.value: %s\n", result.length, result.value);

	z_finish(&database);

	return 0;
}

