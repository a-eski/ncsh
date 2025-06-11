#include <stdlib.h>

#include "../src/eskilib/etest.h"
#include "../src/interpreter/lexer.h"
#include "../src/interpreter/logic.h"
#include "../src/interpreter/tokens.h"
#include "lib/arena_test_helper.h"

#define COMMAND_VALIDATE(command, index, expected, op)                                                                 \
    eassert(!strcmp((command->vals[index]), expected));                                                                \
    eassert((command->lens[index]) == sizeof(expected));                                                               \
    eassert((command->ops[index]) == op)

void logic_preprocess_if_test()
{
    ARENA_TEST_SETUP;

    char* input = "if [ 1 -eq 1 ]; then echo hello; fi";
    size_t len = strlen(input) + 1;
    Tokens* toks = lexer_lex(input, len, &arena);
    Token_Data data = {0};

    Logic_Result result = logic_preprocess(toks->head->next, &data, &arena);

    eassert(result.type == LT_IF);
    eassert(result.val.tok);
    eassert(result.val.tok->op == OP_FI);

    eassert(data.conditions);
    eassert(data.conditions->count == 1);
    eassert(data.conditions->commands->count == 3);

    COMMAND_VALIDATE(data.conditions->commands, 0, "1", OP_CONSTANT);
    COMMAND_VALIDATE(data.conditions->commands, 1, "-eq", OP_EQUALS);
    COMMAND_VALIDATE(data.conditions->commands, 2, "1", OP_CONSTANT);
    eassert(data.conditions->commands[0].lens[3] == 0);

    eassert(data.if_statements);
    eassert(data.if_statements->commands);
    eassert(data.if_statements->count == 1);
    eassert(data.if_statements->commands->count == 2);
    eassert(data.if_statements->commands->vals[0]);

    COMMAND_VALIDATE(data.if_statements->commands, 0, "echo", OP_CONSTANT);
    COMMAND_VALIDATE(data.if_statements->commands, 1, "hello", OP_CONSTANT);
    eassert(data.if_statements->commands->lens[3] == 0);

    eassert(!data.else_statements);

    ARENA_TEST_TEARDOWN;
}

void logic_preprocess_if_long_test()
{
    ARENA_TEST_SETUP;

    char* input = "if [ 1 -eq 1 ]; then git commit -m \"this is a very long commit message for testing "
                  "realloc of if statements\"; fi";
    size_t len = strlen(input) + 1;
    Tokens* toks = lexer_lex(input, len, &arena);
    Token_Data data = {0};

    Logic_Result result = logic_preprocess(toks->head->next, &data, &arena);

    eassert(result.type == LT_IF);
    eassert(result.val.tok);
    eassert(result.val.tok->op == OP_FI);

    eassert(data.conditions);
    eassert(data.conditions->count == 1);
    eassert(data.conditions->commands->count == 3);

    COMMAND_VALIDATE(data.conditions->commands, 0, "1", OP_CONSTANT);
    COMMAND_VALIDATE(data.conditions->commands, 1, "-eq", OP_EQUALS);
    COMMAND_VALIDATE(data.conditions->commands, 2, "1", OP_CONSTANT);
    eassert(data.conditions->commands->lens[3] == 0);

    eassert(data.if_statements);
    eassert(data.if_statements->commands);
    eassert(data.if_statements->count == 1);
    eassert(data.if_statements->commands->count == 4);
    eassert(data.if_statements->commands->vals[0]);

    COMMAND_VALIDATE(data.if_statements->commands, 0, "git", OP_CONSTANT);
    COMMAND_VALIDATE(data.if_statements->commands, 1, "commit", OP_CONSTANT);
    COMMAND_VALIDATE(data.if_statements->commands, 2, "-m", OP_CONSTANT);
    COMMAND_VALIDATE(data.if_statements->commands, 3,
                     "this is a very long commit message for testing realloc of if statements", OP_CONSTANT);
    eassert(data.if_statements->commands->lens[4] == 0);

    eassert(!data.else_statements);

    ARENA_TEST_TEARDOWN;
}

void logic_preprocess_if_else_test()
{
    ARENA_TEST_SETUP;

    char* input = "if [ 1 -eq 1 ]; then echo hello; else echo hi; fi";
    size_t len = strlen(input) + 1;
    Tokens* toks = lexer_lex(input, len, &arena);
    Token_Data data = {0};

    Logic_Result result = logic_preprocess(toks->head->next, &data, &arena);

    eassert(result.type == LT_IF_ELSE);
    eassert(result.val.tok);
    eassert(result.val.tok->op == OP_FI);

    eassert(data.conditions);
    eassert(data.conditions->count == 1);
    eassert(data.conditions->commands->count == 3);
    COMMAND_VALIDATE(data.conditions->commands, 0, "1", OP_CONSTANT);
    COMMAND_VALIDATE(data.conditions->commands, 1, "-eq", OP_EQUALS);
    COMMAND_VALIDATE(data.conditions->commands, 2, "1", OP_CONSTANT);
    eassert(data.conditions->commands->lens[3] == 0);

    eassert(data.if_statements);
    eassert(data.if_statements->commands);
    eassert(data.if_statements->count == 1);
    eassert(data.if_statements->commands->count == 2);
    eassert(data.if_statements->commands->vals[0]);

    COMMAND_VALIDATE(data.if_statements->commands, 0, "echo", OP_CONSTANT);
    COMMAND_VALIDATE(data.if_statements->commands, 1, "hello", OP_CONSTANT);
    eassert(data.if_statements->commands->lens[3] == 0);

    eassert(data.else_statements);
    eassert(data.else_statements->commands);
    eassert(data.else_statements->count == 1);
    eassert(data.else_statements->commands->count == 2);
    eassert(data.else_statements->commands->vals[0]);

    COMMAND_VALIDATE(data.else_statements->commands, 0, "echo", OP_CONSTANT);
    COMMAND_VALIDATE(data.else_statements->commands, 1, "hi", OP_CONSTANT);
    eassert(data.else_statements->commands->lens[3] == 0);

    ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(logic_preprocess_if_test);
    etest_run(logic_preprocess_if_long_test);
    etest_run(logic_preprocess_if_else_test);

    etest_finish();

    return EXIT_SUCCESS;
}
