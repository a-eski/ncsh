// Copyright (c) ncsh by Alex Eski 2024

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "ncsh_parser.h"
#include "ncsh_noninteractive.h"
#include "ncsh_defines.h" // used when NCSH_DEBUG defined
#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_result.h"

int_fast32_t ncsh_noninteractive(int argc, char** argv) {
	#ifdef NCSH_DEBUG
	printf("ncsh running in non-interactive mode.\n");
	printf("argc: %d\n", argc);
	for (int_fast32_t i = 0; i < argc; ++i)
		printf("argv[%lu]: %s\n", i, argv[i]);

	puts("input to process:");
	printf("argc: %d\n", argc - 1);
	for (int_fast32_t i = 0; i < argc - 1; ++i)
		printf("argv[%lu]: %s\n", i, argv[i + 1]);
	#endif /* ifdef NCSH_DEBUG */

	struct ncsh_Args args = {0};

	enum eskilib_Result result;
	if ((result = ncsh_parser_args_malloc_count(argc - 1, &args)) != E_SUCCESS) {
		perror(RED "ncsh: Error when allocating memory for parser" RESET);
		fflush(stderr);
		if (result != E_FAILURE_MALLOC)
			ncsh_parser_args_free(&args);
		return EXIT_FAILURE;
	}

	ncsh_parser_parse_noninteractive(argc - 1, &argv[1], &args);
	#ifdef NCSH_DEBUG
	ncsh_debug_args(args);
	#endif /* ifdef NCSH_DEBUG */

	ncsh_parser_args_free(&args);

	return EXIT_SUCCESS;
}
