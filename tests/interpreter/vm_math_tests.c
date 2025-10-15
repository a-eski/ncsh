/* vm_math_test.c: tests for vm_math engine. */

#include "../../src/interpreter/vm_math.h"
#include "vm_test_helper.h"
#include <stdlib.h>

void vm_math_add_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("$(1 + 1)");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    vm.next_cmds = vm_next(&vm);

    auto res = vm_math_expr(&vm);

    eassert(vm.status == EXIT_SUCCESS);
    eassert(res.value[0] == '2');
    eassert(res.length == 2);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_math_add_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("$(1 + 2 + 3)");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    vm.next_cmds = vm_next(&vm);

    auto res = vm_math_expr(&vm);

    eassert(vm.status == EXIT_SUCCESS);
    eassert(res.value[0] == '6');
    eassert(res.length == 2);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_math_add_more_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("$(1 + 2 + 3 + 4 + 5 + 6 + 7 + 8)");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    vm.next_cmds = vm_next(&vm);

    auto res = vm_math_expr(&vm);

    eassert(vm.status == EXIT_SUCCESS);
    eassert(!memcmp(res.value, "36", 2));
    eassert(res.length == 3);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_math_subtract_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("$(2 - 1)");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    vm.next_cmds = vm_next(&vm);

    auto res = vm_math_expr(&vm);

    eassert(vm.status == EXIT_SUCCESS);
    eassert(res.value[0] == '1');
    eassert(res.length == 2);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_math_subtract_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("$(2 - 3)");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    vm.next_cmds = vm_next(&vm);

    auto res = vm_math_expr(&vm);

    eassert(vm.status == EXIT_SUCCESS);
    eassert(!memcmp(res.value, "-1", 2));
    eassert(res.length == 3);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_math_operator_precedence_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    vm.next_cmds = vm_next(&vm);

    auto res = vm_math_expr(&vm);

    eassert(vm.status == EXIT_SUCCESS);
    eassert(res.value[0] == '1');
    eassert(res.length == 2);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_math_assignment_operator_precedence_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("count=$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto rv = parse(&lexemes, &scratch_arena);
    eassert(!rv.parser_errno);
    eassert(rv.output.stmts->type == ST_NORMAL);

    // simulate setup the VM does
    Vm_Data vm;
    vm_setup(&vm, rv, &s);

    vm.next_cmds = vm_next(&vm);

    auto res = vm_math_expr(&vm);

    eassert(vm.status == EXIT_SUCCESS);
    printf("%s\n", res.value);
    eassert(res.value[0] == '1');
    eassert(res.length == 2);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_math_tests()
{
    etest_start();

    etest_run(vm_math_add_test);
    etest_run(vm_math_add_multiple_test);
    etest_run(vm_math_add_more_multiple_test);

    etest_run(vm_math_subtract_test);
    etest_run(vm_math_subtract_multiple_test);

    etest_run(vm_math_operator_precedence_test);
    // etest_run(vm_math_assignment_operator_precedence_test);

    etest_finish();
}

#ifndef TEST_ALL
int main()
{
    vm_math_tests();
    return 0;
}
#endif /* ifndef TEST_ALL */
