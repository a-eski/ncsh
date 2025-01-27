// Copyright (c) ncsh by Alex Eski 2024

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_result.h"
#include "ncsh_defines.h" // used when NCSH_DEBUG defined
#include "ncsh_noninteractive.h"
#include "ncsh_parser.h"
#include "ncsh_vm.h"

/*enum ncsh_Flags
{
    F_COMMAND = 0x01,                    // -c
    F_EXIT_IMMEDIATELY_ON_FAILURE = 0x02 // -e
};

#define FLAG_COMMAND_CHAR 'c'
#define FLAG_EXIT_IMMEDIATELY_ON_FAILURE_CHAR 'e'

int_fast32_t ncsh_flag_error(const char *message, size_t message_length)
{
    if (write(STDIN_FILENO, message, message_length) == -1)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

#define NO_FLAG_COMMAND_MESSAGE                                                                                        \
    "ncsh: Running in noninteractive mode but no command flag ('-c') included. Please use './ncsh -c 'your "           \
    "commands'\n"

#define ALLOW_READ_FROM_STDIN_NONINTERACTIVE // -s
#define RUN_INTERACTIVE // -i
#define DEBUG_EXECUTION // -x
#define DEBUG_INPUT // -v
#define DEBUG_TOKENIZATION // -n*/

/*void ncsh_noninteractive_flags(int argc, char** argv) {
    int_fast32_t i;
    int_fast32_t j = 0;
    int flags = 0;
    for (i = 0; i < argc - 1 && argv[i + 1][0] == '-'; ++i) {
        while (argv[i + 1][j] != '\0') {
            switch (argv[i + 1][j]) {
                case FLAG_COMMAND_CHAR: {
                    flags |= F_COMMAND;
                    break;
                }
                case FLAG_EXIT_IMMEDIATELY_ON_FAILURE_CHAR: {
                    flags |= F_EXIT_IMMEDIATELY_ON_FAILURE;
                }
            }
        }
    }

    printf("argc: %d, i: %zu ", argc, i);
    printf("argv[i]: %s ", argv[i]);
    printf("buffer : %s\n", argv[i + 1]);

    if (!(flags & F_COMMAND)) { // don't try to run binaries for now, force user to pass in -c
        ncsh_flag_error(NO_FLAG_COMMAND_MESSAGE, sizeof(NO_FLAG_COMMAND_MESSAGE) - 1);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS
}*/

int_fast32_t ncsh_noninteractive(int argc, char** argv)
{
#ifdef NCSH_DEBUG
    printf("ncsh running in noninteractive mode.\n");
#endif /* ifdef NCSH_DEBUG */
    if (argc > 2) {
        printf("ncsh currently only supports passing in one command at a time in interactive mode!\n");
    }

    struct ncsh_Args args = {0};
    enum eskilib_Result result;
    if ((result = ncsh_parser_args_malloc(&args)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating memory for parser" RESET);
        fflush(stderr);
        if (result != E_FAILURE_MALLOC)
            ncsh_parser_args_free(&args);
        return EXIT_FAILURE;
    }

    size_t length = strlen(argv[1]) + 1;

#ifdef NCSH_DEBUG
    printf("length : %lu\n", length);
    printf("buffer : %s\n", argv[1]);
#endif /* ifdef NCSH_DEBUG */

    ncsh_parser_parse(argv[1], length, &args);
    int_fast32_t command_result = ncsh_vm_execute_noninteractive(&args);

    int_fast32_t exit_code = EXIT_SUCCESS;
    switch (command_result) {
    case NCSH_COMMAND_EXIT_FAILURE: {
        exit_code = EXIT_FAILURE;
        // goto exit;
    }
        /*case NCSH_COMMAND_EXIT: {
            goto exit;
        }*/
    }

    // exit:
    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);

    return exit_code;
}
