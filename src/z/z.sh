#!/bin/bash

set -e
gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -DZ_TEST ../eskilib/eskilib_string.c ../eskilib/eskilib_test.c z.c tests/z_tests.c -o ztests
echo 'Compiled z'

