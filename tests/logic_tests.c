#include <stdlib.h>

#include "../src/args.h"
#include "../src/eskilib/etest.h"
#include "../src/parser.h"
#include "../src/vm/logic.h"
#include "lib/arena_test_helper.h"

#define COMMAND_VALIDATE(command, index, expected, op)                                                                 \
    eassert(!strcmp(command->vals[index], expected));                                                                  \
    eassert(command->lens[index] == sizeof(expected));                                                                 \
    eassert(command->ops[index] == op)

void logic_preprocess_if_test()
{
    ARENA_TEST_SETUP;

    char* input = "if [ 1 -eq 1 ]; then echo hello; fi";
    size_t len = strlen(input) + 1;
    Args* args = parser_parse(input, len, &arena);
    Token_Data tokens = {0};

    Logic_Result result = logic_preprocess(args->head->next, &tokens, &arena);

    eassert(result.type == LT_IF);
    eassert(result.val.arg);
    eassert(result.val.arg->op == OP_FI);

    eassert(tokens.conditions->count == 3);

    COMMAND_VALIDATE(tokens.conditions, 0, "1", OP_CONSTANT);
    COMMAND_VALIDATE(tokens.conditions, 1, "-eq", OP_EQUALS);
    COMMAND_VALIDATE(tokens.conditions, 2, "1", OP_CONSTANT);
    eassert(tokens.conditions->lens[3] == 0);

    eassert(tokens.if_statements);
    eassert(tokens.if_statements->commands);
    eassert(tokens.if_statements->count == 1);
    eassert(tokens.if_statements->commands->count == 2);
    eassert(tokens.if_statements->commands->vals[0]);

    COMMAND_VALIDATE(tokens.if_statements->commands, 0, "echo", OP_CONSTANT);
    COMMAND_VALIDATE(tokens.if_statements->commands, 1, "hello", OP_CONSTANT);
    eassert(tokens.if_statements->commands->lens[3] == 0);

    eassert(!tokens.else_statements);

    ARENA_TEST_TEARDOWN;
}

void logic_preprocess_if_long_test()
{
    ARENA_TEST_SETUP;

    char* input = "if [ 1 -eq 1 ]; then git commit -m \"this is a very long commit message for testing "
                  "realloc of if statements\"; fi";
    size_t len = strlen(input) + 1;
    Args* args = parser_parse(input, len, &arena);
    Token_Data tokens = {0};

    Logic_Result result = logic_preprocess(args->head->next, &tokens, &arena);

    eassert(result.type == LT_IF);
    eassert(result.val.arg);
    eassert(result.val.arg->op == OP_FI);

    eassert(tokens.conditions->count == 3);

    COMMAND_VALIDATE(tokens.conditions, 0, "1", OP_CONSTANT);
    COMMAND_VALIDATE(tokens.conditions, 1, "-eq", OP_EQUALS);
    COMMAND_VALIDATE(tokens.conditions, 2, "1", OP_CONSTANT);
    eassert(tokens.conditions->lens[3] == 0);

    eassert(tokens.if_statements);
    eassert(tokens.if_statements->commands);
    eassert(tokens.if_statements->count == 1);
    eassert(tokens.if_statements->commands->count == 4);
    eassert(tokens.if_statements->commands->vals[0]);

    COMMAND_VALIDATE(tokens.if_statements->commands, 0, "git", OP_CONSTANT);
    COMMAND_VALIDATE(tokens.if_statements->commands, 1, "commit", OP_CONSTANT);
    COMMAND_VALIDATE(tokens.if_statements->commands, 2, "-m", OP_CONSTANT);
    COMMAND_VALIDATE(tokens.if_statements->commands, 3,
                     "this is a very long commit message for testing realloc of if statements", OP_CONSTANT);
    eassert(tokens.if_statements->commands->lens[4] == 0);

    eassert(!tokens.else_statements);

    ARENA_TEST_TEARDOWN;
}

void logic_preprocess_if_else_test()
{
    ARENA_TEST_SETUP;

    char* input = "if [ 1 -eq 1 ]; then echo hello; else echo hi; fi";
    size_t len = strlen(input) + 1;
    Args* args = parser_parse(input, len, &arena);
    Token_Data tokens = {0};

    Logic_Result result = logic_preprocess(args->head->next, &tokens, &arena);

    eassert(result.type == LT_IF);
    eassert(result.val.arg);
    eassert(result.val.arg->op == OP_FI);

    eassert(tokens.conditions->count == 3);

    COMMAND_VALIDATE(tokens.conditions, 0, "1", OP_CONSTANT);
    COMMAND_VALIDATE(tokens.conditions, 1, "-eq", OP_EQUALS);
    COMMAND_VALIDATE(tokens.conditions, 2, "1", OP_CONSTANT);
    eassert(tokens.conditions->lens[3] == 0);

    eassert(tokens.if_statements);
    eassert(tokens.if_statements->commands);
    eassert(tokens.if_statements->count == 1);
    eassert(tokens.if_statements->commands->count == 2);
    eassert(tokens.if_statements->commands->vals[0]);

    COMMAND_VALIDATE(tokens.if_statements->commands, 0, "echo", OP_CONSTANT);
    COMMAND_VALIDATE(tokens.if_statements->commands, 1, "hello", OP_CONSTANT);
    eassert(tokens.if_statements->commands->lens[3] == 0);

    eassert(tokens.else_statements);
    eassert(tokens.else_statements->commands);
    eassert(tokens.else_statements->count == 1);
    eassert(tokens.else_statements->commands->count == 2);
    eassert(tokens.else_statements->commands->vals[0]);

    COMMAND_VALIDATE(tokens.else_statements->commands, 0, "echo", OP_CONSTANT);
    COMMAND_VALIDATE(tokens.else_statements->commands, 1, "hi", OP_CONSTANT);
    eassert(tokens.else_statements->commands->lens[3] == 0);

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
