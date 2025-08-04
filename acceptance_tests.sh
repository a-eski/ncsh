#!/bin/env bash

# run ncsh acceptance tests.

echo 'starting ncsh acceptance tests'

rm t.txt t2.txt t3.txt t4.txt
make clean
rm _z_database.bin ncsh_history ncsh_history_test
# run corpus dirs since ls test currently expects those directories to exist
# chmod +x create_corpus_dirs.sh
# ./create_corpus_dirs.sh
set -e

# definitions from ncsh_configurables.h
# Directory Options
# #define NCSH_DIRECTORY_NORMAL 0  // show the current working directory in the prompt line
# #define NCSH_DIRECTORY_SHORT  1  // show up to 2 of the parent directories in the prompt line
# #define NCSH_DIRECTORY_NONE   2  // do not show the current working directory in the prompt line
# User Options
# #define NCSH_SHOW_USER_NORMAL 0
# #define NCSH_SHOW_USER_NONE 1

CFLAGS_BASE="-Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts -D_FORTIFY_SOURCE=3"

echo "COMPILING SHORT DIRECTORY ACCEPTANCE TESTS"
make CFLAGS="-Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts -D_FORTIFY_SOURCE=3 -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=1 -DNCSH_PROMPT_SHOW_USER=0 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_short_acceptance_test_runner.rb
echo "STARING SHORT DIRECTORY ACCEPTANCE TESTS"
./acceptance_tests/directory_short_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin ncsh_history_test
set -e

echo "COMPILING SHORT DIRECTORY NO USER ACCEPTANCE TESTS"
make CFLAGS="-Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts -D_FORTIFY_SOURCE=3 -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=1 -DNCSH_PROMPT_SHOW_USER=1 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_short_no_user_acceptance_test_runner.rb
echo "STARTING SHORT DIRECTORY NO USER ACCEPTANCE TESTS"
./acceptance_tests/directory_short_no_user_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin ncsh_history_test
set -e

echo "COMPILING NORMAL DIRECTORY ACCEPTANCE TESTS"
make CFLAGS="-Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts -D_FORTIFY_SOURCE=3 -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=0 -DNCSH_PROMPT_SHOW_USER=0 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_normal_acceptance_test_runner.rb
echo "STARTING NORMAL DIRECTORY ACCEPTANCE TESTS"
./acceptance_tests/directory_normal_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin ncsh_history_test
set -e

echo "COMPILING NORMAL DIRECTORY NO USER ACCEPTANCE TESTS"
make CFLAGS="-Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts -D_FORTIFY_SOURCE=3 -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=0 -DNCSH_PROMPT_SHOW_USER=1 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_normal_no_user_acceptance_test_runner.rb
echo "STARTING NORMAL DIRECTORY NO USER ACCEPTANCE TESTS"
./acceptance_tests/directory_normal_no_user_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin ncsh_history_test
set -e

echo "COMPILING NO DIRECTORY ACCEPTANCE TESTS"
make CFLAGS="-Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts -D_FORTIFY_SOURCE=3 -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=2 -DNCSH_PROMPT_SHOW_USER=0 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_none_acceptance_test_runner.rb
echo "STARTING NO DIRECTORY ACCEPTANCE TESTS"
./acceptance_tests/directory_none_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin ncsh_history_test

echo "COMPILING NO DIRECTORY NO USER ACCEPTANCE TESTS"
make CFLAGS="-Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts -D_FORTIFY_SOURCE=3 -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=2 -DNCSH_PROMPT_SHOW_USER=1 -DNCSH_START_TIME -DNDEBUG"
chmod +x ./acceptance_tests/directory_none_no_user_acceptance_test_runner.rb
echo "STARTING NO DIRECTORY NO USER ACCEPTANCE TESTS"
./acceptance_tests/directory_none_no_user_acceptance_test_runner.rb
make clean

set +e
rm _z_database.bin ncsh_history_test
set -e

echo "COMPILING CUSTOM PROMPT ACCEPTANCE TESTS"
make CFLAGS="-Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts -D_FORTIFY_SOURCE=3 -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=2 -DNCSH_PROMPT_SHOW_USER=1 -DNCSH_START_TIME -DNDEBUG -DNCSH_PROMPT_ENDING_STRING_TEST"
chmod +x ./acceptance_tests/custom_prompt_test_runner.rb
echo "STARTING CUSTOM PROMPT ACCEPTANCE TESTS"
./acceptance_tests/custom_prompt_test_runner.rb
make clean

set +e
rm _z_database.bin ncsh_history_test
set -e

# echo "COMPILING NONINTERACTIVE ACCEPTANCE TESTS"
# make CFLAGS="-Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts -D_FORTIFY_SOURCE=3 -DNCSH_HISTORY_TEST -DZ_TEST -DNCSH_PROMPT_DIRECTORY=1 -DNCSH_PROMPT_SHOW_USER=0 -DNCSH_START_TIME -DNDEBUG"
# chmod +x ./acceptance_tests/noninteractive_acceptance_test_runner.rb
# echo "STARING NONINTERACTIVE ACCEPTANCE TESTS"
# ./acceptance_tests/noninteractive_acceptance_test_runner.rb
# make clean
#
# set +e
# rm _z_database.bin ncsh_history_test

echo "ALL ACCEPTANCE TESTS PASSED"
