#include <string.h>

#include "../src/compiler/lexer.h"
#include "lib/arena_test_helper.h"

void lexer_lex_bench()
{
    SCRATCH_ARENA_TEST_SETUP;

    Tokens* toks;
    toks = lexer_lex("ls", 3, &scratch_arena);
    toks = lexer_lex("ls -l", 6, &scratch_arena);
    toks = lexer_lex("ls ~", 5, &scratch_arena);
    toks = lexer_lex("ls ~/snap", 10, &scratch_arena);
    toks = lexer_lex("ls -l --color", 14, &scratch_arena);
    toks = lexer_lex("ps", 3, &scratch_arena);
    toks = lexer_lex("ss", 3, &scratch_arena);
    toks = lexer_lex("ls > t.txt", 11, &scratch_arena);
    toks = lexer_lex("ls >> t.txt", 12, &scratch_arena);
    toks = lexer_lex("ls &> t.txt", 12, &scratch_arena);
    toks = lexer_lex("ls &>> t.txt", 13, &scratch_arena);
    toks = lexer_lex("ls | sort", 10, &scratch_arena);
    toks = lexer_lex("ls | sort | wc -c", 18, &scratch_arena);
    toks = lexer_lex("ls | sort | wc -c > t.txt", 26, &scratch_arena);
    toks = lexer_lex("longrunningprogram &", 21, &scratch_arena);
    toks = lexer_lex("t.txt < sort", 13, &scratch_arena);
    toks = lexer_lex("STR=hello", 10, &scratch_arena);
    toks = lexer_lex("echo $STR", 10, &scratch_arena);
    toks = lexer_lex("STR2='hello'", 10, &scratch_arena);
    toks = lexer_lex("echo $STR2", 10, &scratch_arena);
    toks = lexer_lex("echo hello", 11, &scratch_arena);
    toks = lexer_lex("echo 'hello'", 13, &scratch_arena);
    toks = lexer_lex("echo \"hello\"", 13, &scratch_arena);
    toks = lexer_lex("echo `hello`", 13, &scratch_arena);
    toks = lexer_lex("git commit -m \"this is a commit message\"",
                     strlen("git commit -m \"this is a commit message\"") + 1, &scratch_arena);
    toks = lexer_lex("( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )", strlen("( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )") + 1, &scratch_arena);
    (void)toks;

    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    lexer_lex_bench();

    return EXIT_SUCCESS;
}
