#!/bin/bash

set -e

clang -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,fuzzer -O3 -DNDEBUG -DZ_TEST ./tests/z_add_fuzzing.c ./z.c ../eskilib/eskilib_string.c
./a.out Z_ADD_CORPUS/ -detect_leaks=0 -rss_limit_mb=8192

