#!/bin/env bash

echo 'setting up for ncsh acceptance tests'

rm t.txt t2.txt t3.txt t4.txt
make clean
rm _z_database.bin .ncsh_history .ncsh_history_test

set -e
echo "starting ncsh acceptance tests"

make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wsign-conversion -Wshadow -Wvla -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_SHORT_DIRECTORY -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/acceptance_tests.rb
./acceptance_tests/acceptance_tests.rb
make clean

set +e

rm _z_database.bin .ncsh_history .ncsh_history_test t4.txt
