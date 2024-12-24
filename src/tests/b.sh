gcc -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -DNCSH_HISTORY_TEST -lsqlite3 ncsh_history2_tests.c -o test
