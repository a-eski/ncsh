#!/bin/bash

echo 'setting up for integration tests'

rm t.txt
rm t2.txt
rm t3.txt
make clean
rm _z_database.bin

set -e
echo "starting integration tests"

make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -DZ_TEST"
./integration_tests/integration_test.rb
make clean

rm _z_database.bin

