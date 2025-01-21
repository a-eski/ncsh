#!/bin/env bash

echo 'setting up for ncsh integration tests'

rm t.txt
rm t2.txt
rm t3.txt
rm t4.txt
make clean
rm _z_database.bin
rm .ncsh_history
rm .ncsh_history_test

set -e
echo "starting ncsh integration tests"

make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wconversion -Wsign-conversion -Wshadow -Wvla -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_SHORT_DIRECTORY -DNCSH_START_TIME -DNDEBUG"
./integration_tests/integration_test.rb
make clean

set +e

rm _z_database.bin
rm .ncsh_history
rm .ncsh_history_test
rm t4.txt
