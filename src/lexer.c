#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arena.h"
#include "defines.h"
#include "lexer.h"

void lexer_init(Lexer* lex, char* r line, size_t len, Arena* r temp)
{
    assert(len <= NCSH_MAX_INPUT);

    lex->line = line;
    lex->len = len;
    lex->pos = 0;
    lex->tok = arena_malloc(temp, NCSH_MAX_INPUT, char);
    lex->tok_type = None;
}

int lexer_lex(Lexer* lex)
{
    if (lex->pos == lex->len)
        return EOF;

    char* p = lex->line + lex->pos;
    if (!p || !*p)
        return EOF;

    memset(lex->tok, 0, lex->pos);
    char* tok = lex->tok;
    lex->tok_type = None;

    while (lex->pos < lex->len) {
        if (!*p)
            break;
        if (*p == ' ') {
            ++lex->pos;
            break;
        }

        if (*p >= 0 && *p <= 9) {
            if (lex->tok_type > IntLit)
                break;
            lex->tok_type = IntLit;
        }

        if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) {
            if (lex->tok_type != None && lex->tok_type != StrLit)
                break;
            lex->tok_type = StrLit;
        }

        printf("%c", *p);
        *tok++ = *p++;
        ++lex->pos;
    }

    printf(" ");
    *tok = 0;
    return 0;
}
