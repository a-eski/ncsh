#!/bin/env bash

echo 'starting ncsh acceptance tests'

rm t.txt t2.txt t3.txt t4.txt
make clean
rm _z_database.bin .ncsh_history .ncsh_history_test

# definitions from ncsh_configurables.h
# Directory Options
# #define NCSH_DIRECTORY_NORMAL 0  // show the current working directory in the prompt line
# #define NCSH_DIRECTORY_SHORT  1  // show up to 2 of the parent directories in the prompt line
# #define NCSH_DIRECTORY_NONE   2  // do not show the current working directory in the prompt line
# User Options
# #define NCSH_SHOW_USER_NORMAL 0
# #define NCSH_SHOW_USER_NONE 1

set -e
echo "starting short directory acceptance tests"

make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wsign-conversion -Wshadow -Wvla -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=1 -DNCSH_PROMPT_SHOW_USER=0 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_short_acceptance_test_runner.rb
./acceptance_tests/directory_short_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin .ncsh_history_test
set -e

echo "starting short directory no user acceptance tests"
make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wsign-conversion -Wshadow -Wvla -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=1 -DNCSH_PROMPT_SHOW_USER=1 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_short_no_user_acceptance_test_runner.rb
./acceptance_tests/directory_short_no_user_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin .ncsh_history_test
set -e

echo "starting normal directory acceptance tests"
make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wsign-conversion -Wshadow -Wvla -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=0 -DNCSH_PROMPT_SHOW_USER=0 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_normal_acceptance_test_runner.rb
./acceptance_tests/directory_normal_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin .ncsh_history_test
set -e

echo "starting normal directory no user acceptance tests"
make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wsign-conversion -Wshadow -Wvla -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=0 -DNCSH_PROMPT_SHOW_USER=1 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_normal_no_user_acceptance_test_runner.rb
./acceptance_tests/directory_normal_no_user_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin .ncsh_history_test
set -e

echo "starting no directory acceptance tests"
make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wsign-conversion -Wshadow -Wvla -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=2 -DNCSH_PROMPT_SHOW_USER=0 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_none_acceptance_test_runner.rb
./acceptance_tests/directory_none_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin .ncsh_history_test

echo "starting no directory no user acceptance tests"
make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wsign-conversion -Wshadow -Wvla -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=2 -DNCSH_PROMPT_SHOW_USER=1 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_none_no_user_acceptance_test_runner.rb
./acceptance_tests/directory_none_no_user_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin .ncsh_history_test
set -e

echo "starting custom prompt acceptance tests"
make CFLAGS="-Wall -Wextra -Werror -pedantic-errors -Wformat=2 -Wsign-conversion -Wshadow -Wvla -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=2 -DNCSH_PROMPT_SHOW_USER=1 -DNCSH_START_TIME -DNDEBUG -DNCSH_PROMPT_ENDING_STRING_TEST"
chmod +x ./acceptance_tests/custom_prompt_test_runner.rb
./acceptance_tests/custom_prompt_test_runner.rb
make clean

set +e
rm _z_database.bin .ncsh_history_test

echo "ALL ACCEPTANCE TESTS PASSED"
