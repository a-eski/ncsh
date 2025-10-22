/* vm_next_tests.c: tests for vm.c function vm_next. */

#include <stdlib.h>

#include "../../src/interpreter/lex.h"
#include "../../src/interpreter/parse.h"
#include "../../src/interpreter/vm_types.h"
#include "vm_test_helper.h"

void vm_next_simple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    // conditions
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_NORMAL);
    eassert(vm.op_current == OP_NONE);
    eassert(!memcmp(vm.cmds->strs[0].value, "ls", 2));
    eassert(vm.cmds->strs[0].length == 3);

    eassert(!vm.cmds->strs[1].value);
    eassert(!vm.next_cmds);
    eassert(vm.end);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    // conditions
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_NORMAL);
    eassert(vm.op_current == OP_NONE);
    eassert(!memcmp(vm.cmds->strs[0].value, "ls", 2));
    eassert(vm.cmds->strs[0].length == 3);

    eassert(!vm.cmds->strs[1].value);
    eassert(!vm.next_cmds);
    eassert(vm.end);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_IF);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    // conditions
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQ_A);
    eassert(!memcmp(vm.cmds->strs[0].value, "1", 1));
    eassert(!memcmp(vm.cmds->strs[1].value, "-eq", 1));
    eassert(!memcmp(vm.cmds->strs[2].value, "1", 1));
    eassert(!vm.cmds->strs[3].value);

    vm.state = EXIT_SUCCESS;

    // if statements
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!memcmp(vm.cmds->strs[0].value, "echo", 4));
    eassert(!memcmp(vm.cmds->strs[1].value, "hi", 2));
    eassert(!vm.cmds->strs[2].value);

    eassert(vm.end);
    eassert(!vm.next_cmds);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_multiple_conditions_true_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ true && true ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_IF);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    // conditions
    // first condition
    vm.next_cmds = vm_next(&vm);

    eassert(vm.next_cmds);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(!memcmp(vm.cmds->strs[0].value, "true", 4));
    eassert(!vm.cmds->strs[1].value);

    // assume first condition succeeds
    vm.state = EXIT_SUCCESS;

    // second condition
    vm.next_cmds = vm_next(&vm);

    eassert(vm.next_cmds);
    eassert(vm.state == VS_IN_CONDITIONS);

    eassert(vm.op_current == OP_AND);
    eassert(!memcmp(vm.cmds->strs[0].value, "true", 4));

    eassert(!vm.cmds->strs[1].value);

    // if statements
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!memcmp(vm.cmds->strs[0].value, "echo", 4));
    eassert(!memcmp(vm.cmds->strs[1].value, "hi", 2));

    eassert(vm.end);
    eassert(!vm.next_cmds);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_multiple_conditions_false_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ false && true ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_IF);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    // conditions
    // first condition
    vm.next_cmds = vm_next(&vm);

    eassert(vm.next_cmds);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(!memcmp(vm.cmds->strs[0].value, "false", 4));

    eassert(!vm.cmds->strs[1].value);

    // VM will short ciruit the and and stop evaluation

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_multiple_conditions_true_or_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ true || true ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_IF);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    // conditions
    // first condition
    vm.next_cmds = vm_next(&vm);

    eassert(vm.next_cmds);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(!memcmp(vm.cmds->strs[0].value, "true", 4));
    eassert(!vm.cmds->strs[1].value);

    // assume first condition succeeds
    vm.state = EXIT_SUCCESS;

    // second condition
    vm.next_cmds = vm_next(&vm);

    eassert(vm.next_cmds);
    eassert(vm.state == VS_IN_CONDITIONS);

    eassert(vm.op_current == OP_OR);
    eassert(!memcmp(vm.cmds->strs[0].value, "true", 4));

    eassert(!vm.cmds->strs[1].value);

    // if statements
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!memcmp(vm.cmds->strs[0].value, "echo", 4));
    eassert(!memcmp(vm.cmds->strs[1].value, "hi", 2));

    eassert(vm.end);
    eassert(!vm.next_cmds);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_else_true_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; else echo 'hello'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_IF_ELSE);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    // conditions
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQ_A);
    eassert(!memcmp(vm.cmds->strs[0].value, "1", 1));
    eassert(vm.cmds->strs[0].length == 2);
    eassert(!memcmp(vm.cmds->strs[1].value, "-eq", 1));
    eassert(vm.cmds->strs[1].length == 4);
    eassert(!memcmp(vm.cmds->strs[2].value, "1", 1));
    eassert(vm.cmds->strs[2].length == 2);

    // simulate vm status (condition result)
    vm.status = EXIT_SUCCESS;

    // if statements
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!memcmp(vm.cmds->strs[0].value, "echo", 4));
    eassert(!memcmp(vm.cmds->strs[1].value, "hi", 2));

    eassert(vm.end);
    eassert(!vm.next_cmds);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_else_false_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 2 ]; then echo 'hi'; else echo 'hello'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_IF_ELSE);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    // conditions
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQ_A);
    eassert(!memcmp(vm.cmds->strs[0].value, "1", 1));
    eassert(!memcmp(vm.cmds->strs[1].value, "-eq", 1));
    eassert(!memcmp(vm.cmds->strs[2].value, "2", 1));
    eassert(!vm.cmds->strs[3].value);

    // simulate VM status (condition result)
    vm.status = EXIT_FAILURE;

    // else statements
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_ELSE_STATEMENTS);
    eassert(!memcmp(vm.cmds->strs[0].value, "echo", 4));
    eassert(!memcmp(vm.cmds->strs[1].value, "hello", 5));

    eassert(!vm.cmds->strs[2].value);
    eassert(!vm.next_cmds);
    eassert(vm.end);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_next_if_elif_else_if_true_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; elif [ 2 -eq 1 ]; then echo hey; else echo 'hello'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_IF_ELIF_ELSE);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    // conditions
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQ_A);
    eassert(!memcmp(vm.cmds->strs[0].value, "1", 1));
    eassert(vm.cmds->strs[0].length == 2);
    eassert(!memcmp(vm.cmds->strs[1].value, "-eq", 1));
    eassert(vm.cmds->strs[1].length == 4);
    eassert(!memcmp(vm.cmds->strs[2].value, "1", 1));
    eassert(vm.cmds->strs[2].length == 2);

    // simulate vm status (condition result)
    vm.status = EXIT_SUCCESS;

    // if statements
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!memcmp(vm.cmds->strs[0].value, "echo", 4));
    eassert(!memcmp(vm.cmds->strs[1].value, "hi", 2));

    eassert(vm.end);
    eassert(!vm.next_cmds);
}

void vm_next_if_elif_else_elif_true_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 2 -eq 1 ]; then echo 'hi'; elif [ 1 -eq 1 ]; then echo hey; else echo 'hello'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_IF_ELIF_ELSE);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    // if conditions
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQ_A);
    eassert(!memcmp(vm.cmds->strs[0].value, "2", 1));
    eassert(vm.cmds->strs[0].length == 2);
    eassert(!memcmp(vm.cmds->strs[1].value, "-eq", 1));
    eassert(vm.cmds->strs[1].length == 4);
    eassert(!memcmp(vm.cmds->strs[2].value, "1", 1));
    eassert(vm.cmds->strs[2].length == 2);

    // simulate vm status (condition result)
    vm.status = EXIT_FAILURE;

    // elif conditions
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(vm.op_current == OP_EQ_A);
    eassert(!memcmp(vm.cmds->strs[0].value, "1", 1));
    eassert(vm.cmds->strs[0].length == 2);
    eassert(!memcmp(vm.cmds->strs[1].value, "-eq", 1));
    eassert(vm.cmds->strs[1].length == 4);
    eassert(!memcmp(vm.cmds->strs[2].value, "1", 1));
    eassert(vm.cmds->strs[2].length == 2);

    // simulate vm status (condition result)
    vm.status = EXIT_SUCCESS;

    // elif statements
    vm.next_cmds = vm_next(&vm);

    eassert(vm.state == VS_IN_ELIF_STATEMENTS);
    eassert(!memcmp(vm.cmds->strs[0].value, "echo", 4));
    eassert(!memcmp(vm.cmds->strs[1].value, "hey", 2));

    eassert(vm.end);
    eassert(!vm.next_cmds);
}

void vm_next_tests()
{
    etest_start();

    etest_run(vm_next_simple_test);
    etest_run(vm_next_if_test);
    etest_run(vm_next_if_multiple_conditions_true_and_test);
    // etest_run(vm_next_if_multiple_conditions_false_and_test);
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
