/* Copyright ncsh (C) by Alex Eski 2025 */
/* interpreter.h: interpreter implementation for ncsh */

#include <stdlib.h>

#include "interpreter.h"
#include "lexemes.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "vars.h"
#include "vm/vm.h"

void interpreter_init(Shell* rst shell)
{
    vars_malloc(&shell->arena, &shell->vars);
}

[[nodiscard]]
int interpreter_run(Shell* rst shell, Arena scratch)
{
    Lexemes lexemes = {0};
    lexemes_init(&lexemes, &scratch);
    lexer_lex(shell->input.buffer, shell->input.pos, &lexemes, &scratch);

    int result = semantic_analyzer_analyze(&lexemes);
    if (result != EXIT_SUCCESS)
        return result;

    Statements statements = {0};
    result = parser_parse(&lexemes, &statements, shell, &shell->arena);
    if (result != EXIT_SUCCESS)
        return result;

    return vm_execute(&statements, shell, &scratch);
}

[[nodiscard]]
int interpreter_run_noninteractive(char** rst argv, size_t argc, Shell* rst shell)
{
    Lexemes lexemes = {0};
    lexemes_init(&lexemes, &shell->arena);
    lexer_lex_noninteractive(argv, argc, &lexemes, &shell->arena);

    int result;
    if ((result = semantic_analyzer_analyze(&lexemes)) != EXIT_SUCCESS)
        return result;

    Statements statements = {0};
    result = parser_parse(&lexemes, &statements, shell, &shell->arena);
    if (result != EXIT_SUCCESS) {
        return result;
    }

    return vm_execute_noninteractive(&statements, shell);
}
