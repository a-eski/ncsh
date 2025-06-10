/* Copyright ncsh (C) by Alex Eski 2025 */
/* compiler.h: compiler implementation for ncsh */

#include <stdlib.h>

#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "vm/syntax_validator.h"
#include "vm/vars.h"
#include "vm/vm.h"
#include "vm/vm_types.h"

void compiler_init(Shell* rst shell)
{
    vars_malloc(&shell->arena, &shell->vars);
}

int compiler_run(Shell* rst shell, Arena scratch)
{
    Tokens* toks = lexer_lex(shell->input.buffer, shell->input.pos, &scratch);

    int result;
    if ((result = syntax_validator_validate(toks)) != EXIT_SUCCESS)
        return result;

    Token_Data data = {0};
    if ((result = parser_parse(toks, &data, shell, &shell->arena)) != EXIT_SUCCESS)
        return result;

    return vm_execute(toks, &data, shell, &scratch);
}

int compiler_run_noninteractive(char** rst argv, size_t argc, Shell* rst shell)
{
    Tokens* toks = lexer_lex_noninteractive(argv, argc, &shell->arena);

    int result;
    if ((result = syntax_validator_validate(toks)) != EXIT_SUCCESS)
        return result;

    Token_Data data = {0};
    if ((result = parser_parse(toks, &data, shell, &shell->arena)) != EXIT_SUCCESS) {
        return result;
    }

    return vm_execute_noninteractive(toks, &data, shell);
}
