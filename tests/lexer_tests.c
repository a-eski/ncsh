#include <stdlib.h>
#include <string.h>

#include "lib/arena_test_helper.h"
#include "../src/lexer.h"
#include "../src/eskilib/etest.h"

void lexer_str_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Lexer lex;
    char* line = "this is a test";
    size_t len = strlen(line) + 1;
    lexer_init(&lex, line, len, &scratch_arena);
    eassert(!memcmp(lex.line, line, len));
    eassert(lex.len == len);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "this", 5));
    eassert(lex.tok_type == StrLit);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "is", 3));
    eassert(lex.tok_type == StrLit);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "a", 2));
    eassert(lex.tok_type == StrLit);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "test", 5));
    eassert(lex.tok_type == StrLit);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_num_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Lexer lex;
    char* line = "12 34 56 88889999";
    size_t len = strlen(line) + 1;
    lexer_init(&lex, line, len, &scratch_arena);
    eassert(!memcmp(lex.line, line, len));
    eassert(lex.len == len);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "12", 3));
    // eassert(lex.tok_type == IntLit);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "34", 3));
    // eassert(lex.tok_type == IntLit);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "56", 3));
    // eassert(lex.tok_type == IntLit);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "88889999", 9));
    // eassert(lex.tok_type == IntLit);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_str_and_num_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Lexer lex;
    char* line = "this 12 a 333";
    size_t len = strlen(line) + 1;
    lexer_init(&lex, line, len, &scratch_arena);
    eassert(!memcmp(lex.line, line, len));
    eassert(lex.len == len);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "this", 5));
    eassert(lex.tok_type == StrLit);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "12", 3));
    // eassert(lex.tok_type == IntLit);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "a", 2));
    eassert(lex.tok_type == StrLit);

    lexer_lex(&lex);
    eassert(!memcmp(lex.tok, "333", 4));
    // eassert(lex.tok_type == IntLit);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(lexer_str_test);
    etest_run(lexer_num_test);
    etest_run(lexer_str_and_num_test);

    etest_finish();

    return EXIT_SUCCESS;
}
