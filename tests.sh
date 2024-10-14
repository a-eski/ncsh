#!/bin/bash

gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/ncsh_parser.c ./src/tests/ncsh_parser_tests.c ./src/ncsh_args.c -o ncsh_parser_tests &&
./ncsh_parser_tests &&

gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/ncsh_builtin_commands.c ./src/tests/ncsh_builtin_commands_tests.c ./src/ncsh_args.c -o ncsh_builtin_commands_tests &&
./ncsh_builtin_commands_tests &&

gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/ncsh_builtin_commands.c ./src/tests/ncsh_history_file_tests.c ./src/ncsh_args.c -o ncsh_history_file_tests &&
./ncsh_history_file_tests
