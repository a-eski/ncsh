#include "../src/parser.h"
#include "lib/arena_test_helper.h"
#include <string.h>

void parser_parse_bench()
{
    SCRATCH_ARENA_TEST_SETUP;

    Args* args;
    args = parser_parse("ls", 3, &scratch_arena);
    args = parser_parse("ls -l", 6, &scratch_arena);
    args = parser_parse("ls ~", 5, &scratch_arena);
    args = parser_parse("ls ~/snap", 10, &scratch_arena);
    args = parser_parse("ls -l --color", 14, &scratch_arena);
    args = parser_parse("ps", 3, &scratch_arena);
    args = parser_parse("ss", 3, &scratch_arena);
    args = parser_parse("ls > t.txt", 11, &scratch_arena);
    args = parser_parse("ls >> t.txt", 12, &scratch_arena);
    args = parser_parse("ls &> t.txt", 12, &scratch_arena);
    args = parser_parse("ls &>> t.txt", 13, &scratch_arena);
    args = parser_parse("ls | sort", 10, &scratch_arena);
    args = parser_parse("ls | sort | wc -c", 18, &scratch_arena);
    args = parser_parse("ls | sort | wc -c > t.txt", 26, &scratch_arena);
    args = parser_parse("longrunningprogram &", 21, &scratch_arena);
    args = parser_parse("t.txt < sort", 13, &scratch_arena);
    args = parser_parse("STR=hello", 10, &scratch_arena);
    args = parser_parse("echo $STR", 10, &scratch_arena);
    args = parser_parse("STR2='hello'", 10, &scratch_arena);
    args = parser_parse("echo $STR2", 10, &scratch_arena);
    args = parser_parse("echo hello", 11, &scratch_arena);
    args = parser_parse("echo 'hello'", 13, &scratch_arena);
    args = parser_parse("echo \"hello\"", 13, &scratch_arena);
    args = parser_parse("echo `hello`", 13, &scratch_arena);
    args = parser_parse("git commit -m \"this is a commit message\"",
                        strlen("git commit -m \"this is a commit message\"") + 1, &scratch_arena);
    args = parser_parse("( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )", strlen("( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )") + 1, &scratch_arena);
    (void)args;

    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    parser_parse_bench();

    return EXIT_SUCCESS;
}
