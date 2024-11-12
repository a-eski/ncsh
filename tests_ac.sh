#!/bin/bash

 gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -g ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/ncsh_autocompletions.c ./src/tests/ncsh_autocompletions_tests.c -o ./bin/ncsh_autocompletions_tests &&
 ./bin/ncsh_autocompletions_tests
