#!/bin/bash

gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak ./eskilib/eskilib_string.c ./eskilib/eskilib_test.c ncsh_parser.c ./tests/ncsh_parser_tests.c ncsh_args.c -o ncsh_parser_tests &&
./ncsh_parser_tests &&
gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak ./eskilib/eskilib_string.c ./eskilib/eskilib_test.c ncsh_builtin_commands.c ./tests/ncsh_builtin_commands_tests.c ncsh_args.c -o ncsh_builtin_commands_tests &&
./ncsh_builtin_commands_tests &&
gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak ./eskilib/eskilib_string.c ./eskilib/eskilib_test.c ncsh_builtin_commands.c ./tests/ncsh_history_file_tests.c ncsh_args.c -o ncsh_history_file_tests &&
./ncsh_history_file_tests
