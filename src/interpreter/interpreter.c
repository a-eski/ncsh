/* Copyright ncsh (C) by Alex Eski 2025 */
/* interpreter.h: interpreter implementation for ncsh */

#include <stdlib.h>

#include "interpreter.h"
#include "lexemes.h"
#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "vm.h"
#include "../ttyio/ttyio.h"

[[nodiscard]]
int interpreter_run(Shell* restrict shell, Arena scratch)
{
    Lexemes lexemes = {0};
    lexer_lex(Str_New(shell->input.buffer, shell->input.pos), &lexemes, &scratch);

    int rv = sema_analyze(&lexemes);
    if (rv != EXIT_SUCCESS)
        return rv;

    Parser_Output parse_rv = parser_parse(&lexemes, shell, &scratch);
    if (parse_rv.parser_errno) {
        tty_puts(parse_rv.output.msg);
        return rv;
    }

    return vm_execute(parse_rv.output.stmts, shell, &scratch);
}

[[nodiscard]]
int interpreter_run_noninteractive(char** restrict argv, size_t argc, Shell* restrict shell)
{
    Lexemes lexemes = {0};
    lexer_lex_noninteractive(argv, argc, &lexemes, &shell->arena);

    int rv = sema_analyze(&lexemes);
    if (rv != EXIT_SUCCESS)
        return rv;

    Parser_Output parse_rv = parser_parse(&lexemes, shell, &shell->arena);
    if (parse_rv.parser_errno) {
        tty_puts(parse_rv.output.msg);
        return rv;
    }

    return vm_execute_noninteractive(parse_rv.output.stmts, shell);
}
