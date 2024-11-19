//for testing
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "z.h"
#include "../eskilib/eskilib_string.h"

void z_test_one(struct z_Database* database) {
	char buffer[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL)
		perror("wd error");
	printf("initial dir %s\n", buffer);

	const struct eskilib_String test = { .value = "ncsh", .length = 5 };
	struct eskilib_String result = z_process(test, buffer, database);
	printf("result.length: %lu, result.value: %s\n", result.length, result.value);

	wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL)
		perror("wd error");
	printf("after chdir: %s\n", buffer);
}

void z_test_two(struct z_Database* database) {
	char buffer[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL)
		perror("wd error");
	printf("initial dir %s\n", buffer);

	const struct eskilib_String test = { .value = "nvim", .length = 5 };
	struct eskilib_String result = z_process(test, buffer, database);
	printf("result.length: %lu, result.value: %s\n", result.length, result.value);

	wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL)
		perror("wd error");
	printf("after chdir: %s\n", buffer);
}

void z_test_three(struct z_Database* database) {
	char buffer[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL)
		perror("wd error");
	printf("initial dir %s\n", buffer);

	const struct eskilib_String test = { .value = "Tapcheck", .length = 9 };
	struct eskilib_String result = z_process(test, buffer, database);
	printf("result.length: %lu, result.value: %s\n", result.length, result.value);

	wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL)
		perror("wd error");
	printf("after chdir: %s\n", buffer);
}

void z_test(struct z_Database* database) {
	z_test_one(database);
	z_test_two(database);
	z_test_three(database);
}

int main(void) {
	puts("Starting z tests.");
	const struct eskilib_String config_path = eskilib_String_Empty;
	struct z_Database database;

	enum eskilib_Result result;
	if ((result = z_begin(528, config_path, &database)) != E_SUCCESS) {
		printf("Error starting z with config_path: %s\n", config_path.value);
		printf("z_start result: %d\n", result);
		return 1;
	}

	z_test(&database);

	if ((result = z_end(&database)) != E_SUCCESS) {
		puts("Error ending z");
	}

	return 0;
}

