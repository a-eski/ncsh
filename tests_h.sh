#!/bin/bash

gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST ./src/eskilib/eskilib_string.c ./src/eskilib/eskilib_test.c ./src/eskilib/eskilib_file.c ./src/ncsh_history.c ./src/tests/ncsh_history_tests.c -o ./bin/ncsh_history_tests &&
./bin/ncsh_history_tests

