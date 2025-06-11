/* Copyright ncsh (C) by Alex Eski 2025 */
/* interpreter.h: interpreter implementation for ncsh */

#include <stdlib.h>

#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "vm/vars.h"
#include "vm/vm.h"
#include "vm/vm_types.h"

void interpreter_init(Shell* rst shell)
{
    vars_malloc(&shell->arena, &shell->vars);
}

int interpreter_run(Shell* rst shell, Arena scratch)
{
    Tokens* toks = lexer_lex(shell->input.buffer, shell->input.pos, &scratch);

    int result;
    if ((result = semantic_analyzer_analyze(toks)) != EXIT_SUCCESS)
        return result;

    Token_Data data = {0};
    if ((result = parser_parse(toks, &data, shell, &shell->arena)) != EXIT_SUCCESS)
        return result;

    return vm_execute(toks, &data, shell, &scratch);
}

int interpreter_run_noninteractive(char** rst argv, size_t argc, Shell* rst shell)
{
    Tokens* toks = lexer_lex_noninteractive(argv, argc, &shell->arena);

    int result;
    if ((result = semantic_analyzer_analyze(toks)) != EXIT_SUCCESS)
        return result;

    Token_Data data = {0};
    if ((result = parser_parse(toks, &data, shell, &shell->arena)) != EXIT_SUCCESS) {
        return result;
    }

    return vm_execute_noninteractive(toks, &data, shell);
}
