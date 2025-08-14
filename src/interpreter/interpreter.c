/* Copyright ncsh (C) by Alex Eski 2025 */
/* interpreter.h: interpreter implementation for ncsh */

#include <stdlib.h>

#include "interpreter.h"
#include "lexemes.h"
#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "vm.h"

[[nodiscard]]
int interpreter_run(Shell* restrict shell, Arena scratch)
{
    Lexemes lexemes = {0};
    lexer_lex(shell->input.buffer, shell->input.pos, &lexemes, &scratch);

    int result = sema_analyze(&lexemes);
    if (result != EXIT_SUCCESS)
        return result;

    Statements statements = {0};
    result = parser_parse(&lexemes, &statements, shell, &scratch);
    if (result != EXIT_SUCCESS) {
        return result;
    }

    return vm_execute(&statements, shell, &scratch);
}

[[nodiscard]]
int interpreter_run_noninteractive(char** restrict argv, size_t argc, Shell* restrict shell)
{
    Lexemes lexemes = {0};
    lexer_lex_noninteractive(argv, argc, &lexemes, &shell->arena);

    int result = sema_analyze(&lexemes);
    if (result != EXIT_SUCCESS) {
        return result;
    }

    Statements statements = {0};
    result = parser_parse(&lexemes, &statements, shell, &shell->arena);
    if (result != EXIT_SUCCESS) {
        return result;
    }

    return vm_execute_noninteractive(&statements, shell);
}
