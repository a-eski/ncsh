#include <stdlib.h>
#include <string.h>

#include "lib/arena_test_helper.h"
#include "../src/eskilib/etest.h"
#include "../src/vm/vm_types.h"
#include "../src/vm/vm_buffer.h"
#include "../src/vm/preprocessor.h"

void vm_buffer_arg_test()
{
    ARENA_TEST_SETUP;

    char* input = "ls";
    size_t len = strlen(input) + 1;
    Args* args = parser_parse(input, len, &arena);
    Token_Data tokens = {0};
    preprocessor_preprocess(args, &tokens, NULL, &arena);
    eassert(tokens.logic_type == LT_NONE);
    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);

    Arg* arg = vm_buffer_set(args->head->next, &tokens, &vm);
    eassert(!strcmp(vm.buffer[0], "ls"));
    eassert(vm.buffer_lens[0] == sizeof("ls"));
    eassert(vm.op_current == OP_NONE);

    eassert(!arg->next);
    eassert(!vm.buffer[1]);
    eassert(vm.args_end);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_args_test()
{
    ARENA_TEST_SETUP;

    char* input = "git commit -m \"this is a commit message\"";
    size_t len = strlen(input) + 1;
    Args* args = parser_parse(input, len, &arena);
    Token_Data tokens = {0};
    preprocessor_preprocess(args, &tokens, NULL, &arena);
    eassert(tokens.logic_type == LT_NONE);
    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);

    Arg* arg = vm_buffer_set(args->head->next, &tokens, &vm);
    eassert(!strcmp(vm.buffer[0], "git"));
    eassert(vm.buffer_lens[0] == sizeof("git"));

    eassert(!strcmp(vm.buffer[1], "commit"));
    eassert(vm.buffer_lens[1] == sizeof("commit"));

    eassert(!strcmp(vm.buffer[2], "-m"));
    eassert(vm.buffer_lens[2] == sizeof("-m"));

    eassert(!strcmp(vm.buffer[3], "this is a commit message"));
    eassert(vm.buffer_lens[3] == sizeof("this is a commit message"));

    eassert(!vm.buffer[4]);
    eassert(vm.args_end);
    eassert(vm.op_current == OP_NONE);
    eassert(!arg->next);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_args_piped_test()
{
    ARENA_TEST_SETUP;

    char* input = "ls | sort | wc -c";
    size_t len = strlen(input) + 1;
    Args* args = parser_parse(input, len, &arena);
    Token_Data tokens = {0};
    preprocessor_preprocess(args, &tokens, NULL, &arena);
    eassert(tokens.logic_type == LT_NONE);
    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);
    Arg* arg = vm_buffer_set(args->head->next, &tokens, &vm);

    eassert(!strcmp(vm.buffer[0], "ls"));
    eassert(vm.buffer_lens[0] == sizeof("ls"));
    eassert(vm.op_current == OP_PIPE);
    eassert(!vm.buffer[1]);
    eassert(!vm.args_end);

    arg = vm_buffer_set(arg, &tokens, &vm);
    eassert(!strcmp(vm.buffer[0], "sort"));
    eassert(vm.buffer_lens[0] == sizeof("sort"));
    eassert(vm.op_current == OP_PIPE);
    eassert(!vm.buffer[1]);
    eassert(!vm.args_end);

    arg = vm_buffer_set(arg, &tokens, &vm);
    eassert(!strcmp(vm.buffer[0], "wc"));
    eassert(vm.buffer_lens[0] == sizeof("wc"));

    eassert(!strcmp(vm.buffer[1], "-c"));
    eassert(vm.buffer_lens[1] == sizeof("-c"));
    eassert(vm.op_current == OP_PIPE);

    eassert(!vm.buffer[2]);
    eassert(vm.args_end);
    eassert(!arg->next);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_args_redirected_test()
{
    ARENA_TEST_SETUP;

    char* input = "ls > t.txt";
    size_t len = strlen(input) + 1;
    Args* args = parser_parse(input, len, &arena);
    Token_Data tokens = {0};
    preprocessor_preprocess(args, &tokens, NULL, &arena);
    eassert(tokens.stdout_file);
    eassert(!strcmp(tokens.stdout_file, "t.txt"));
    eassert(tokens.logic_type == LT_NONE);
    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);
    Arg* arg = vm_buffer_set(args->head->next, &tokens, &vm);
    (void)arg;

    eassert(!strcmp(vm.buffer[0], "ls"));
    eassert(vm.buffer_lens[0] == sizeof("ls"));
    // eassert(vm.op_current == OP_STDOUT_REDIRECTION);
    eassert(!vm.buffer[1]);
    eassert(vm.args_end);
    eassert(!arg->next);

    ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(vm_buffer_arg_test);
    etest_run(vm_buffer_args_test);
    etest_run(vm_buffer_args_piped_test);
    etest_run(vm_buffer_args_redirected_test);

    etest_finish();

    return EXIT_SUCCESS;
}
