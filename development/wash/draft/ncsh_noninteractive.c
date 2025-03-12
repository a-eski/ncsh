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
