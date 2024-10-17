#include "../ncsh_parser.c"
#include "../ncsh_args.c"
#include "../eskilib/eskilib_string.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

__AFL_FUZZ_INIT();

int main(void) {
	__AFL_INIT();
	char* source = 0;
	unsigned char* buffer = __AFL_FUZZ_TESTCASE_BUF;
	struct ncsh_Args args = ncsh_args_malloc();

	while (__AFL_LOOP(10000)) {
		int length = __AFL_FUZZ_TESTCASE_LEN;
		source = realloc(source, length + 1);
		memcpy(source, buffer, length);
		source[length] = 0;

		args = ncsh_parse_v2(source, length, args);
	}
}
