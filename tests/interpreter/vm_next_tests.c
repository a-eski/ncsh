/* vm_next_tests.c: tests for vm.c function vm_next. */

#include <stdlib.h>
#include <setjmp.h>

#include "../../src/interpreter/lexemes.h"
#include "../../src/interpreter/lexer.h"
#include "../../src/interpreter/parser.h"
#include "../../src/interpreter/statements.h"
#include "../../src/interpreter/vm_types.h"
#include "../etest.h"
#include "../lib/arena_test_helper.h"

__sig_atomic_t vm_child_pid;
jmp_buf env_jmp_buf;

Commands* vm_next(Statements* restrict stmts, Commands* cmds, Vm_Data* restrict vm);

void vm_next_simple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &scratch_arena);
    eassert(!res);
    eassert(stmts.type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm = {0};
    stmts.pos = 0;
    Commands* cmds = stmts.statements[0].commands;
    cmds->pos = 0;

    // conditions
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 1);
    eassert(vm.state == VS_NORMAL);
    eassert(vm.op_current == OP_NONE);
    eassert(!memcmp(vm.strs[0].value, "ls", 2));
    eassert(vm.strs[0].length == 3);

    eassert(!vm.strs[1].value);
    eassert(!cmds);
    eassert(vm.end);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &scratch_arena);
    eassert(!res);
    eassert(stmts.type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm = {0};
    stmts.pos = 0;
    Commands* cmds = stmts.statements[0].commands;
    cmds->pos = 0;

    // conditions
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 1);
    eassert(vm.state == VS_NORMAL);
    eassert(vm.op_current == OP_NONE);
    eassert(!memcmp(vm.strs[0].value, "ls", 2));
    eassert(vm.strs[0].length == 3);

    eassert(!vm.strs[1].value);
    eassert(!cmds);
    eassert(vm.end);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &scratch_arena);
    eassert(!res);
    eassert(stmts.type == ST_IF);

    // simulate setup the VM does
    Vm_Data vm = {0};
    stmts.pos = 0;
    Commands* cmds = stmts.statements[0].commands;
    cmds->pos = 0;

    // conditions
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 1);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQUALS);
    eassert(!memcmp(vm.strs[0].value, "1", 1));
    eassert(!memcmp(vm.strs[1].value, "-eq", 1));
    eassert(!memcmp(vm.strs[2].value, "1", 1));
    eassert(!vm.strs[3].value);

    vm.state = EXIT_SUCCESS;

    // if statements
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 2);
    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!memcmp(vm.strs[0].value, "echo", 4));
    eassert(!memcmp(vm.strs[1].value, "hi", 2));
    eassert(!vm.strs[2].value);

    eassert(vm.end);
    eassert(!cmds);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_multiple_conditions_true_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ true && true ]; then echo 'hi'; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &scratch_arena);
    eassert(!res);
    eassert(stmts.type == ST_IF);

    // simulate setup the VM does
    Vm_Data vm = {0};
    stmts.pos = 0;
    Commands* cmds = stmts.statements[0].commands;
    cmds->pos = 0;

    // conditions
    // first condition
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(cmds);
    eassert(stmts.pos == 0);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(!memcmp(vm.strs[0].value, "true", 4));
    eassert(!vm.strs[1].value);

    // assume first condition succeeds
    vm.state = EXIT_SUCCESS;

    // second condition
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(cmds);
    eassert(stmts.pos == 1);
    eassert(vm.state == VS_IN_CONDITIONS);

    eassert(vm.op_current == OP_AND);
    eassert(!memcmp(vm.strs[0].value, "true", 4));

    eassert(!vm.strs[1].value);

    // if statements
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 2);
    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!memcmp(vm.strs[0].value, "echo", 4));
    eassert(!memcmp(vm.strs[1].value, "hi", 2));

    eassert(vm.end);
    eassert(!cmds);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_multiple_conditions_false_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ false && true ]; then echo 'hi'; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &scratch_arena);
    eassert(!res);
    eassert(stmts.type == ST_IF);

    // simulate setup the VM does
    Vm_Data vm = {0};
    stmts.pos = 0;
    Commands* cmds = stmts.statements[0].commands;
    cmds->pos = 0;

    // conditions
    // first condition
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(cmds);
    eassert(stmts.pos == 0);

    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(!memcmp(vm.strs[0].value, "false", 4));

    eassert(!vm.strs[1].value);

    // VM will short ciruit the and and stop evaluation

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_multiple_conditions_true_or_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ true || true ]; then echo 'hi'; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &scratch_arena);
    eassert(!res);
    eassert(stmts.type == ST_IF);

    // simulate setup the VM does
    Vm_Data vm = {0};
    stmts.pos = 0;
    Commands* cmds = stmts.statements[0].commands;
    cmds->pos = 0;

    // conditions
    // first condition
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(cmds);
    eassert(stmts.pos == 0);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(!memcmp(vm.strs[0].value, "true", 4));
    eassert(!vm.strs[1].value);

    // assume first condition succeeds
    vm.state = EXIT_SUCCESS;

    // second condition
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(cmds);
    eassert(stmts.pos == 1);
    eassert(vm.state == VS_IN_CONDITIONS);

    eassert(vm.op_current == OP_OR);
    eassert(!memcmp(vm.strs[0].value, "true", 4));

    eassert(!vm.strs[1].value);

    // if statements
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 2);
    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!memcmp(vm.strs[0].value, "echo", 4));
    eassert(!memcmp(vm.strs[1].value, "hi", 2));

    eassert(vm.end);
    eassert(!cmds);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_else_true_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; else echo 'hello'; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &scratch_arena);
    eassert(!res);
    eassert(stmts.type == ST_IF_ELSE);
    eassert(stmts.count == 3);

    // simulate setup the VM does
    Vm_Data vm = {0};
    stmts.pos = 0;
    Commands* cmds = stmts.statements[0].commands;
    cmds->pos = 0;

    // conditions
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 1);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQUALS);
    eassert(!memcmp(vm.strs[0].value, "1", 1));
    eassert(vm.strs[0].length == 2);
    eassert(!memcmp(vm.strs[1].value, "-eq", 1));
    eassert(vm.strs[1].length == 4);
    eassert(!memcmp(vm.strs[2].value, "1", 1));
    eassert(vm.strs[2].length == 2);

    // simulate vm status (condition result)
    vm.status = EXIT_SUCCESS;

    // if statements
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 2);
    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!memcmp(vm.strs[0].value, "echo", 4));
    eassert(!memcmp(vm.strs[1].value, "hi", 2));

    eassert(vm.end);
    eassert(!cmds);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_else_false_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 2 ]; then echo 'hi'; else echo 'hello'; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &scratch_arena);
    eassert(!res);
    eassert(stmts.type == ST_IF_ELSE);
    eassert(stmts.count == 3);

    // simulate setup the VM does
    Vm_Data vm = {0};
    stmts.pos = 0;
    Commands* cmds = stmts.statements[0].commands;
    cmds->pos = 0;

    // conditions
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQUALS);
    eassert(!memcmp(vm.strs[0].value, "1", 1));
    eassert(!memcmp(vm.strs[1].value, "-eq", 1));
    eassert(!memcmp(vm.strs[2].value, "2", 1));
    eassert(!vm.strs[3].value);

    // simulate VM status (condition result)
    vm.status = EXIT_FAILURE;

    // else statements
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(vm.state == VS_IN_ELSE_STATEMENTS);
    eassert(!memcmp(vm.strs[0].value, "echo", 4));
    eassert(!memcmp(vm.strs[1].value, "hello", 5));

    eassert(!vm.strs[2].value);
    eassert(!cmds);
    eassert(vm.end);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_elif_else_if_true_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; elif [ 2 -eq 1 ]; then echo hey; else echo 'hello'; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &scratch_arena);
    eassert(!res);
    eassert(stmts.type == ST_IF_ELIF_ELSE);
    eassert(stmts.count == 5);

    // simulate setup the VM does
    Vm_Data vm = {0};
    stmts.pos = 0;
    Commands* cmds = stmts.statements[0].commands;
    cmds->pos = 0;

    // conditions
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 1);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQUALS);
    eassert(!memcmp(vm.strs[0].value, "1", 1));
    eassert(vm.strs[0].length == 2);
    eassert(!memcmp(vm.strs[1].value, "-eq", 1));
    eassert(vm.strs[1].length == 4);
    eassert(!memcmp(vm.strs[2].value, "1", 1));
    eassert(vm.strs[2].length == 2);

    // simulate vm status (condition result)
    vm.status = EXIT_SUCCESS;

    // if statements
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 2);
    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!memcmp(vm.strs[0].value, "echo", 4));
    eassert(!memcmp(vm.strs[1].value, "hi", 2));

    eassert(vm.end);
    eassert(!cmds);
}

void vm_next_if_elif_else_elif_true_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 2 -eq 1 ]; then echo 'hi'; elif [ 1 -eq 1 ]; then echo hey; else echo 'hello'; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &scratch_arena);
    eassert(!res);
    eassert(stmts.type == ST_IF_ELIF_ELSE);
    eassert(stmts.count == 5);

    // simulate setup the VM does
    Vm_Data vm = {0};
    stmts.pos = 0;
    Commands* cmds = stmts.statements[0].commands;
    cmds->pos = 0;

    // if conditions
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 1);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQUALS);
    eassert(!memcmp(vm.strs[0].value, "2", 1));
    eassert(vm.strs[0].length == 2);
    eassert(!memcmp(vm.strs[1].value, "-eq", 1));
    eassert(vm.strs[1].length == 4);
    eassert(!memcmp(vm.strs[2].value, "1", 1));
    eassert(vm.strs[2].length == 2);

    // simulate vm status (condition result)
    vm.status = EXIT_FAILURE;

    // elif conditions
    cmds = vm_next(&stmts, cmds, &vm);

    eassert(stmts.pos == 3);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQUALS);
    eassert(!memcmp(vm.strs[0].value, "1", 1));
    eassert(vm.strs[0].length == 2);
    eassert(!memcmp(vm.strs[1].value, "-eq", 1));
    eassert(vm.strs[1].length == 4);
    eassert(!memcmp(vm.strs[2].value, "1", 1));
    eassert(vm.strs[2].length == 2);

    // simulate vm status (condition result)
    vm.status = EXIT_SUCCESS;

    // elif statements
    printf("%u\n", stmts.type);
    cmds = vm_next(&stmts, cmds, &vm);

    printf("%zu\n", stmts.pos);
    eassert(stmts.pos == 4);
    eassert(vm.state == VS_IN_ELIF_STATEMENTS);
    eassert(!memcmp(vm.strs[0].value, "echo", 4));
    eassert(!memcmp(vm.strs[1].value, "hey", 2));

    eassert(vm.end);
    eassert(!cmds);
}

void vm_next_tests()
{
    etest_start();

    etest_run(vm_next_simple_test);
    etest_run(vm_next_if_test);
    etest_run(vm_next_if_multiple_conditions_true_and_test);
    etest_run(vm_next_if_multiple_conditions_false_and_test);
    etest_run(vm_next_if_multiple_conditions_true_or_test);
    etest_run(vm_next_if_else_true_test);
    etest_run(vm_next_if_else_false_test);
    etest_run(vm_next_if_elif_else_if_true_test);
    // etest_run(vm_next_if_elif_else_elif_true_test);

    etest_finish();
}

#ifndef TEST_ALL
int main()
{
    vm_next_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
