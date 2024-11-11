//for testing
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "z.h"
#include "../eskilib/eskilib_string.h"

int main(void) {
	puts("Starting z tests.");
	const struct eskilib_String config_path = eskilib_String_Empty;
	struct z_Database database;

	enum eskilib_Result start_result = z_start(528, config_path, &database);
	if (start_result != E_SUCCESS) {
		printf("Error starting z with config_path: %s\n", config_path.value);
		printf("z_start result: %d\n", start_result);
		return 1;
	}


	char buffer[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL)
		perror("wd error");
	printf("initial dir %s\n", buffer);

	const struct eskilib_String test = { .value = "ncsh", .length = 5 };
	struct eskilib_String result = z_process(test, &database);
	printf("result.length: %lu, result.value: %s\n", result.length, result.value);
	chdir(result.value);

	wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL)
		perror("wd error");
	printf("after chdir: %s\n", buffer);

	z_finish(&database);

	return 0;
}

