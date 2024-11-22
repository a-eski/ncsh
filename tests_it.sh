#!/bin/bash

rm t.txt
rm t2.txt
rm t3.txt

set -e
echo "starting integration tests"

make clean &&
make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST" &&
./integration_tests/integration_test.rb

