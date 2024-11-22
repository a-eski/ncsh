#!/bin/bash

gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/ncsh_parser.c ./src/tests/ncsh_parser_tests.c ./src/ncsh_args.c -o ./bin/ncsh_parser_tests &&
./bin/ncsh_parser_tests
