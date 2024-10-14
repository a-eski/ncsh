#include <stdint.h>
#include <stdlib.h>

#include "../ncsh_parser.h"
#include "../ncsh_tokenize.h"
#include "../eskilib/eskilib_string.h"
#include "../eskilib/eskilib_test.h"

void ncsh_tokenize_basic_test(void) {
	char* line = "ls\0";
	uint_fast8_t length = 3;

	struct ncsh_Args args = ncsh_args_malloc();
	args = ncsh_parse_v2(line, length, args);

	struct ncsh_Tokens tokens = ncsh_tokens_malloc();
	tokens = ncsh_tokenize(args, tokens);
	eskilib_assert(tokens.tokens != NULL);
	eskilib_assert(tokens.ops != NULL);
	eskilib_assert(tokens.ops[0] == OP_CONSTANT);
	eskilib_assert(eskilib_string_equals(tokens.tokens[0][0],  "ls\0", 3));
	eskilib_assert(tokens.tokens[0][1] == NULL);

	ncsh_args_free_values(args);
	ncsh_args_free(args);
	/*ncsh_tokens_values_free(tokens);*/
	ncsh_tokens_free(tokens);
}

void ncsh_tokenize_basic_with_args_test(void) {
	char* line = "ls -l\0";
	uint_fast8_t length = 6;

	struct ncsh_Args args = ncsh_args_malloc();
	args = ncsh_parse_v2(line, length, args);

	struct ncsh_Tokens tokens = ncsh_tokens_malloc();
	tokens = ncsh_tokenize(args, tokens);
	eskilib_assert(tokens.tokens != NULL);
	eskilib_assert(tokens.ops != NULL);
	eskilib_assert(tokens.ops[0] == OP_CONSTANT);
	eskilib_assert(eskilib_string_equals(tokens.tokens[0][0],  "ls", 3));
	eskilib_assert(eskilib_string_equals(tokens.tokens[0][1],  "-l", 3));
	eskilib_assert(tokens.tokens[0][2] == NULL);

	ncsh_args_free_values(args);
	ncsh_args_free(args);
	/*ncsh_tokens_values_free(tokens);*/
	ncsh_tokens_free(tokens);
}

void ncsh_tokenizer_tests(void) {
	eskilib_test_run("ncsh_tokenize_basic_test", ncsh_tokenize_basic_test);
	eskilib_test_run("ncsh_tokenize_basic_with_args_test", ncsh_tokenize_basic_with_args_test);
}

#ifndef ncsh_TEST_ALL
int main(void) {
	ncsh_tokenizer_tests();

	return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
