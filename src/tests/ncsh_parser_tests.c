#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ncsh_parser.h"
#include "../eskilib/eskilib_string.h"
#include "../eskilib/eskilib_test.h"

void ncsh_parser_ls_test(void) {
	char* line = "ls\0";
	uint_fast8_t length = 3;


	struct ncsh_Args args;
	bool result = ncsh_args_malloc(&args);
	eskilib_assert(result == true);
	args = ncsh_parse(line, length, args);

	eskilib_assert(args.values != NULL);
	eskilib_assert(args.count == 1);
	eskilib_assert(args.max_line_length == 2);

	eskilib_assert(eskilib_string_equals(args.values[0], line, length));
	eskilib_assert(args.ops[0] == OP_CONSTANT);

	ncsh_args_free_values(args);
	ncsh_args_free(args);
}

void ncsh_parser_ls_dash_l_test(void) {
	char* line = "ls -l\0";
	uint_fast8_t length = 6;

	struct ncsh_Args args;
	bool result = ncsh_args_malloc(&args);
	eskilib_assert(result == true);
	args = ncsh_parse(line, length, args);

	eskilib_assert(args.values != NULL);
	eskilib_assert(args.count == 2);
	eskilib_assert(args.max_line_length == 2);
	eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
	eskilib_assert(args.ops[0] == OP_CONSTANT);
	eskilib_assert(eskilib_string_equals(args.values[1], "-l", length));
	eskilib_assert(args.ops[1] == OP_CONSTANT);

	ncsh_args_free_values(args);
	ncsh_args_free(args);
}

void ncsh_parser_pipe_test(void) {
	char* line = "ls | sort\0";
	uint_fast8_t length = 10;

	struct ncsh_Args args;
	bool result = ncsh_args_malloc(&args);
	eskilib_assert(result == true);
	args = ncsh_parse(line, length, args);

	eskilib_assert(args.values != NULL);
	eskilib_assert(args.count == 3);
	eskilib_assert(args.max_line_length == 4);
	eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
	eskilib_assert(args.ops[0] == OP_CONSTANT);
	eskilib_assert(eskilib_string_equals(args.values[1], "|", length));
	eskilib_assert(args.ops[1] == OP_PIPE);
	eskilib_assert(eskilib_string_equals(args.values[2], "sort", length));
	eskilib_assert(args.ops[2] == OP_CONSTANT);

	ncsh_args_free_values(args);
	ncsh_args_free(args);
}

void ncsh_parser_multiple_pipe_test(void) {
	char* line = "ls | sort | table";
	uint_fast8_t length = 18;

	struct ncsh_Args args;
	bool result = ncsh_args_malloc(&args);
	eskilib_assert(result == true);
	args = ncsh_parse(line, length, args);

	eskilib_assert(args.values != NULL);
	eskilib_assert(args.count == 5);
	eskilib_assert(args.max_line_length == 5);

	eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
	eskilib_assert(args.ops[0] == OP_CONSTANT);
	eskilib_assert(eskilib_string_equals(args.values[1], "|", length));
	eskilib_assert(args.ops[1] == OP_PIPE);
	eskilib_assert(eskilib_string_equals(args.values[2], "sort", length));
	eskilib_assert(args.ops[2] == OP_CONSTANT);
	eskilib_assert(eskilib_string_equals(args.values[3], "|", length));
	eskilib_assert(args.ops[3] == OP_PIPE);
	eskilib_assert(eskilib_string_equals(args.values[4], "table", length));
	eskilib_assert(args.ops[4] == OP_CONSTANT);

	ncsh_args_free_values(args);
	ncsh_args_free(args);
}

void ncsh_parser_background_job_test(void) {
	char* line = "longrunningprogram &\0";
	uint_fast8_t length = 21;

	struct ncsh_Args args;
	bool result = ncsh_args_malloc(&args);
	eskilib_assert(result == true);
	args = ncsh_parse(line, length, args);

	eskilib_assert(args.values != NULL);
	eskilib_assert(args.count == 2);
	eskilib_assert(args.max_line_length == 18);

	eskilib_assert(eskilib_string_equals(args.values[0], "longrunningprogram", length));
	eskilib_assert(args.ops[0] == OP_CONSTANT);
	eskilib_assert(eskilib_string_equals(args.values[1], "&", length));
	eskilib_assert(args.ops[1] == OP_BACKGROUND_JOB);

	ncsh_args_free_values(args);
	ncsh_args_free(args);
}

void ncsh_parser_output_redirection_test(void) {
	char* line = "ls > text.txt\0";
	uint_fast8_t length = 14;

	struct ncsh_Args args;
	bool result = ncsh_args_malloc(&args);
	eskilib_assert(result == true);
	args = ncsh_parse(line, length, args);

	eskilib_assert(args.values != NULL);
	eskilib_assert(args.count == 3);
	eskilib_assert(args.max_line_length == 8);

	eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
	eskilib_assert(args.ops[0] == OP_CONSTANT);
	eskilib_assert(eskilib_string_equals(args.values[1], ">", length));
	eskilib_assert(args.ops[1] == OP_OUTPUT_REDIRECTION);
	eskilib_assert(eskilib_string_equals(args.values[2], "text.txt", length));
	eskilib_assert(args.ops[2] == OP_CONSTANT);

	ncsh_args_free_values(args);
	ncsh_args_free(args);
}

void ncsh_parser_output_redirection_append_test(void) {
	char* line = "ls >> text.txt\0";
	uint_fast8_t length = 15;

	struct ncsh_Args args;
	bool result = ncsh_args_malloc(&args);
	eskilib_assert(result == true);
	args = ncsh_parse(line, length, args);

	eskilib_assert(args.values != NULL);
	eskilib_assert(args.count == 3);
	eskilib_assert(args.max_line_length == 8);

	eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
	eskilib_assert(args.ops[0] == OP_CONSTANT);
	eskilib_assert(eskilib_string_equals(args.values[1], ">>", length));
	eskilib_assert(args.ops[1] == OP_OUTPUT_REDIRECTION_APPEND);
	eskilib_assert(eskilib_string_equals(args.values[2], "text.txt", length));
	eskilib_assert(args.ops[2] == OP_CONSTANT);

	ncsh_args_free_values(args);
	ncsh_args_free(args);
}

void ncsh_parser_double_quotes_test(void) {
	char* line = "echo \"hello\"\0";
	uint_fast8_t length = 13;

	struct ncsh_Args args;
	bool result = ncsh_args_malloc(&args);
	eskilib_assert(result == true);
	args = ncsh_parse(line, length, args);

	eskilib_assert(args.values != NULL);
	eskilib_assert(args.count == 2);
	eskilib_assert(args.max_line_length == 5);

	eskilib_assert(eskilib_string_equals(args.values[0], "echo", length));
	eskilib_assert(eskilib_string_equals(args.values[1], "hello", length));

	ncsh_args_free_values(args);
	ncsh_args_free(args);
}

void ncsh_parser_tests(void) {
	eskilib_test_run("ncsh_parser_ls_test", ncsh_parser_ls_test);
	eskilib_test_run("ncsh_parser_ls_dash_l_test", ncsh_parser_ls_dash_l_test);
	eskilib_test_run("ncsh_parser_pipe_test", ncsh_parser_pipe_test);
	eskilib_test_run("ncsh_parser_multiple_pipe_test", ncsh_parser_multiple_pipe_test);
	eskilib_test_run("ncsh_parser_background_job_test", ncsh_parser_background_job_test);
	eskilib_test_run("ncsh_parser_output_redirection_test", ncsh_parser_output_redirection_test);
	eskilib_test_run("ncsh_parser_double_quotes_test", ncsh_parser_double_quotes_test);
	eskilib_test_run("ncsh_parser_output_redirection_append_test", ncsh_parser_output_redirection_append_test);
}

#ifndef ncsh_TEST_ALL
int main(void) {
	ncsh_parser_tests();

	return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
