//for testing only

#include <stdio.h>

#include "z_database.h"
#include "z_main.h"
#include "../eskilib/eskilib_string.h"

int main(void) {
	struct z_Database database;

	enum z_Database_Result result;
	if ((result = z_start(&database)) != Z_SUCCESS)
	{
		printf("Failed to start z: %d\n", result);
		return 1;
	}

	struct eskilib_String input = { .value = "ncsh", .length = 5 };
	struct eskilib_String* output = z_process(&input, &database);

	if ((result = z_finish(&database)) != Z_SUCCESS)
	{
		printf("Failed to finish z: %d\n", result);
		return 1;
	}

	return 0;
}

