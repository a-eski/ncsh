#!/bin/bash

set -e
gcc -std=c2x -Wall -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -g ../eskilib/eskilib_string.c ../eskilib/eskilib_test.c z.c tests/z_tests.c -o z_tests
echo "Compiled z tests"
