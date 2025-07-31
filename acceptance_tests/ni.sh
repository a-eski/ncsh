#!/bin/env bash

rm t.txt t2.txt t3.txt t4.txt
rm _z_database.bin ncsh_history_test
set -e

echo "COMPILING NONINTERACTIVE ACCEPTANCE TESTS"
make CFLAGS="-Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts -D_FORTIFY_SOURCE=3 -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=1 -DNCSH_PROMPT_SHOW_USER=0 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/noninteractive_acceptance_test_runner.rb
echo "STARING NONINTERACTIVE ACCEPTANCE TESTS"
./acceptance_tests/noninteractive_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin ncsh_history_test
