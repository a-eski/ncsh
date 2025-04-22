#include "../src/eskilib/etest.h"
#include "../src/parser.h"
#include "lib/arena_test_helper.h"

void parser_parse_bench()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args;
    bool result = parser_init(&args, &arena);
    eassert(result);

    parser_parse("ls", 3, &args, &arena, &scratch_arena);
    parser_parse("ls -l", 6, &args, &arena, &scratch_arena);
    parser_parse("ls ~", 5, &args, &arena, &scratch_arena);
    parser_parse("ls ~/snap", 10, &args, &arena, &scratch_arena);
    parser_parse("ls -l --color", 14, &args, &arena, &scratch_arena);
    parser_parse("ps", 3, &args, &arena, &scratch_arena);
    parser_parse("ss", 3, &args, &arena, &scratch_arena);
    parser_parse("ls > t.txt", 11, &args, &arena, &scratch_arena);
    parser_parse("ls >> t.txt", 12, &args, &arena, &scratch_arena);
    parser_parse("ls &> t.txt", 12, &args, &arena, &scratch_arena);
    parser_parse("ls &>> t.txt", 13, &args, &arena, &scratch_arena);
    parser_parse("ls | sort", 10, &args, &arena, &scratch_arena);
    parser_parse("ls | sort | wc -c", 18, &args, &arena, &scratch_arena);
    parser_parse("ls | sort | wc -c > t.txt", 26, &args, &arena, &scratch_arena);
    parser_parse("longrunningprogram &", 21, &args, &arena, &scratch_arena);
    parser_parse("t.txt < sort", 13, &args, &arena, &scratch_arena);
    parser_parse("STR=hello", 10, &args, &arena, &scratch_arena);
    parser_parse("echo $STR", 10, &args, &arena, &scratch_arena);
    parser_parse("STR2='hello'", 10, &args, &arena, &scratch_arena);
    parser_parse("echo $STR2", 10, &args, &arena, &scratch_arena);
    parser_parse("echo hello", 11, &args, &arena, &scratch_arena);
    parser_parse("echo 'hello'", 13, &args, &arena, &scratch_arena);
    parser_parse("echo \"hello\"", 13, &args, &arena, &scratch_arena);
    parser_parse("echo `hello`", 13, &args, &arena, &scratch_arena);
    parser_parse("git commit -m \"this is a commit message\"", strlen("git commit -m \"this is a commit message\"") + 1,
                 &args, &arena, &scratch_arena);
    parser_parse("( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )", strlen("( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )") + 1, &args, &arena,
                 &scratch_arena);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    parser_parse_bench();

    return EXIT_SUCCESS;
}
