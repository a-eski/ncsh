/* Copyright ncsh (C) by Alex Eski 2025 */
/* interpreter.h: interpreter implementation for ncsh */

#include <stdlib.h>

#include "../defines.h" // used for macros like EXIT_FAILURE_CONTINUE
#include "interpreter.h"
#include "lex.h"
#include "parse.h"
#include "vm.h"
#include "../ttyio/ttyio.h"

[[nodiscard]]
int interpreter_run(Shell* restrict shell, Arena scratch)
{
    Lexemes lexemes = {0};
    lex(Str(shell->input.buffer, shell->input.pos), &lexemes, &scratch);

    Parser_Output parse_rv = parse(&lexemes, &scratch);
    if (parse_rv.parser_errno) {
        if (parse_rv.parser_errno != PE_NOTHING) {
            tty_fprintln(stderr, "ncsh parser: %s", parse_rv.output.msg);
        }
        return EXIT_FAILURE_CONTINUE;
    }

    return vm_execute(parse_rv.output.stmts, shell, &scratch);
}

[[nodiscard]]
int interpreter_run_noninteractive(char** restrict argv, size_t argc, Shell* restrict shell)
{
    Lexemes lexemes = {0};
    lex_noninteractive(argv, argc, &lexemes, &shell->arena);

    Parser_Output parse_rv = parse(&lexemes, &shell->arena);
    if (parse_rv.parser_errno) {
        tty_fputs(parse_rv.output.msg, stderr);
        return EXIT_FAILURE_CONTINUE;
    }

    return vm_execute_noninteractive(parse_rv.output.stmts, shell);
}
