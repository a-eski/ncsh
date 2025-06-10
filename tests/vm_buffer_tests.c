#include <stdlib.h>
#include <string.h>

#include "../src/compiler/lexer.h"
#include "../src/compiler/parser.h"
#include "../src/compiler/vm/vm_buffer.h"
#include "../src/compiler/vm/vm_types.h"
#include "../src/eskilib/etest.h"
#include "lib/arena_test_helper.h"

void vm_buffer_token_test()
{
    ARENA_TEST_SETUP;

    char* input = "ls";
    size_t len = strlen(input) + 1;
    Tokens* tokens = lexer_lex(input, len, &arena);
    Token_Data data = {0};
    parser_parse(tokens, &data, NULL, &arena);
    eassert(data.logic_type == LT_NONE);
    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);

    Token* tok = vm_buffer_set(tokens->head->next, &data, &vm);
    eassert(!strcmp(vm.buffer[0], "ls"));
    eassert(vm.buffer_lens[0] == sizeof("ls"));
    eassert(vm.op_current == OP_NONE);

    eassert(!tok->next);
    eassert(!vm.buffer[1]);
    eassert(vm.tokens_end);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_tokens_test()
{
    ARENA_TEST_SETUP;

    char* input = "git commit -m \"this is a commit message\"";
    size_t len = strlen(input) + 1;
    Tokens* tokens = lexer_lex(input, len, &arena);
    Token_Data data = {0};
    parser_parse(tokens, &data, NULL, &arena);
    eassert(data.logic_type == LT_NONE);
    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);

    Token* tok = vm_buffer_set(tokens->head->next, &data, &vm);
    eassert(!strcmp(vm.buffer[0], "git"));
    eassert(vm.buffer_lens[0] == sizeof("git"));

    eassert(!strcmp(vm.buffer[1], "commit"));
    eassert(vm.buffer_lens[1] == sizeof("commit"));

    eassert(!strcmp(vm.buffer[2], "-m"));
    eassert(vm.buffer_lens[2] == sizeof("-m"));

    eassert(!strcmp(vm.buffer[3], "this is a commit message"));
    eassert(vm.buffer_lens[3] == sizeof("this is a commit message"));

    eassert(!vm.buffer[4]);
    eassert(vm.tokens_end);
    eassert(vm.op_current == OP_NONE);
    eassert(!tok->next);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_tokens_and_test()
{
    ARENA_TEST_SETUP;

    char* input = "false && true";
    size_t len = strlen(input) + 1;
    Tokens* tokens = lexer_lex(input, len, &arena);
    Token_Data data = {0};
    parser_parse(tokens, &data, NULL, &arena);
    eassert(data.logic_type == LT_NONE);
    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);

    Token* tok = vm_buffer_set(tokens->head->next, &data, &vm);
    eassert(vm.buffer[0]);
    eassert(!strcmp(vm.buffer[0], "false"));
    eassert(vm.buffer_lens[0] == sizeof("false"));
    eassert(vm.op_current == OP_AND);
    eassert(!vm.buffer[1]);
    eassert(!tok->next);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_tokens_piped_test()
{
    ARENA_TEST_SETUP;

    char* input = "ls | sort | wc -c";
    size_t len = strlen(input) + 1;
    Tokens* tokens = lexer_lex(input, len, &arena);
    Token_Data data = {0};
    parser_parse(tokens, &data, NULL, &arena);
    eassert(data.logic_type == LT_NONE);
    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);
    Token* tok = vm_buffer_set(tokens->head->next, &data, &vm);

    eassert(!strcmp(vm.buffer[0], "ls"));
    eassert(vm.buffer_lens[0] == sizeof("ls"));
    eassert(vm.op_current == OP_PIPE);
    eassert(!vm.buffer[1]);
    eassert(!vm.tokens_end);

    tok = vm_buffer_set(tok, &data, &vm);
    eassert(!strcmp(vm.buffer[0], "sort"));
    eassert(vm.buffer_lens[0] == sizeof("sort"));
    eassert(vm.op_current == OP_PIPE);
    eassert(!vm.buffer[1]);
    eassert(!vm.tokens_end);

    tok = vm_buffer_set(tok, &data, &vm);
    eassert(!strcmp(vm.buffer[0], "wc"));
    eassert(vm.buffer_lens[0] == sizeof("wc"));

    eassert(!strcmp(vm.buffer[1], "-c"));
    eassert(vm.buffer_lens[1] == sizeof("-c"));
    eassert(vm.op_current == OP_PIPE);

    eassert(!vm.buffer[2]);
    eassert(vm.tokens_end);
    eassert(!tok->next);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_tokens_redirected_test()
{
    ARENA_TEST_SETUP;

    char* input = "ls > t.txt";
    size_t len = strlen(input) + 1;
    Tokens* tokens = lexer_lex(input, len, &arena);
    Token_Data data = {0};
    parser_parse(tokens, &data, NULL, &arena);
    eassert(data.stdout_file);
    eassert(!strcmp(data.stdout_file, "t.txt"));
    eassert(data.logic_type == LT_NONE);

    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);
    Token* tok = vm_buffer_set(tokens->head->next, &data, &vm);

    eassert(!strcmp(vm.buffer[0], "ls"));
    eassert(vm.buffer_lens[0] == sizeof("ls"));
    eassert(!vm.buffer[1]);
    eassert(vm.tokens_end);
    eassert(!tok->next);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_if_test()
{
    ARENA_TEST_SETUP;

    char* input = "if [ true ]; then echo hello; fi";
    size_t len = strlen(input) + 1;
    Tokens* tokens = lexer_lex(input, len, &arena);
    Token_Data data = {0};
    parser_parse(tokens, &data, NULL, &arena);
    eassert(data.logic_type == LT_IF);
    eassert(data.conditions);
    eassert(data.if_statements);

    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);

    Token* tok = vm_buffer_set(tokens->head->next, &data, &vm);
    eassert(!strcmp(vm.buffer[0], "true"));
    eassert(vm.buffer_lens[0] == sizeof("true"));
    eassert(vm.op_current == OP_NONE);
    eassert(!vm.buffer[1]);
    eassert(!tok);

    tok = vm_buffer_set(tok, &data, &vm);
    eassert(!tok);
    eassert(!strcmp(vm.buffer[0], "echo"));
    eassert(vm.buffer_lens[0] == sizeof("echo"));
    eassert(!strcmp(vm.buffer[1], "hello"));
    eassert(vm.buffer_lens[1] == sizeof("hello"));
    eassert(vm.op_current == OP_NONE);
    eassert(!vm.buffer[2]);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_if_multiple_condition_test()
{
    ARENA_TEST_SETUP;

    char* input = "if [ true && false ]; then echo hello; fi";
    size_t len = strlen(input) + 1;
    Tokens* tokens = lexer_lex(input, len, &arena);
    Token_Data data = {0};
    parser_parse(tokens, &data, NULL, &arena);
    eassert(data.logic_type == LT_IF);
    eassert(data.conditions);
    eassert(data.if_statements);

    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);

    Token* tok = vm_buffer_set(tokens->head->next, &data, &vm);
    eassert(!strcmp(vm.buffer[0], "true"));
    eassert(vm.buffer_lens[0] == sizeof("true"));
    eassert(!strcmp(vm.buffer[1], "&&"));
    eassert(vm.buffer_lens[1] == sizeof("&&"));
    eassert(!strcmp(vm.buffer[2], "false"));
    eassert(vm.buffer_lens[2] == sizeof("false"));
    eassert(vm.op_current == OP_NONE);
    eassert(!vm.buffer[3]);
    eassert(!tok);

    tok = vm_buffer_set(tok, &data, &vm);
    eassert(!tok);
    eassert(!strcmp(vm.buffer[0], "echo"));
    eassert(vm.buffer_lens[0] == sizeof("echo"));
    eassert(!strcmp(vm.buffer[1], "hello"));
    eassert(vm.buffer_lens[1] == sizeof("hello"));
    eassert(vm.op_current == OP_NONE);
    eassert(!vm.buffer[2]);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_if_else_true_test()
{
    ARENA_TEST_SETUP;

    char* input = "if [ true ]; then echo hello; else echo hi; fi";
    size_t len = strlen(input) + 1;
    Tokens* tokens = lexer_lex(input, len, &arena);
    Token_Data data = {0};
    parser_parse(tokens, &data, NULL, &arena);
    eassert(data.logic_type == LT_IF_ELSE);
    eassert(data.conditions);
    eassert(data.if_statements);
    eassert(data.else_statements);

    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);

    Token* tok = vm_buffer_set(tokens->head->next, &data, &vm);
    eassert(!tok);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(!strcmp(vm.buffer[0], "true"));
    eassert(vm.buffer_lens[0] == sizeof("true"));
    eassert(vm.op_current == OP_NONE);
    eassert(!vm.buffer[1]);

    tok = vm_buffer_set(tok, &data, &vm);
    eassert(!tok);
    eassert(vm.state == VS_IN_IF_STATEMENTS);
    eassert(!strcmp(vm.buffer[0], "echo"));
    eassert(vm.buffer_lens[0] == sizeof("echo"));
    eassert(!strcmp(vm.buffer[1], "hello"));
    eassert(vm.buffer_lens[1] == sizeof("hello"));
    eassert(vm.op_current == OP_NONE);
    eassert(!vm.buffer[2]);

    ARENA_TEST_TEARDOWN;
}

void vm_buffer_if_else_false_test()
{
    ARENA_TEST_SETUP;

    char* input = "if [ false ]; then echo hello; else echo hi; fi";
    size_t len = strlen(input) + 1;
    Tokens* tokens = lexer_lex(input, len, &arena);
    Token_Data data = {0};
    parser_parse(tokens, &data, NULL, &arena);
    eassert(data.logic_type == LT_IF_ELSE);
    eassert(data.conditions);
    eassert(data.if_statements);
    eassert(data.else_statements);

    Vm_Data vm = {0};
    vm.buffer = arena_malloc(&arena, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(&arena, VM_MAX_INPUT, size_t);

    Token* tok = vm_buffer_set(tokens->head->next, &data, &vm);
    eassert(!tok);
    eassert(vm.state == VS_IN_CONDITIONS);
    eassert(!strcmp(vm.buffer[0], "false"));
    eassert(vm.buffer_lens[0] == sizeof("false"));
    eassert(vm.op_current == OP_NONE);
    eassert(!vm.buffer[1]);

    vm.status = EXIT_FAILURE; // simulate failure to have else case set
    tok = vm_buffer_set(tok, &data, &vm);
    eassert(!tok);
    eassert(vm.state == VS_IN_ELSE_STATEMENTS);
    eassert(!strcmp(vm.buffer[0], "echo"));
    eassert(vm.buffer_lens[0] == sizeof("echo"));
    eassert(!strcmp(vm.buffer[1], "hi"));
    eassert(vm.buffer_lens[1] == sizeof("hi"));
    eassert(vm.op_current == OP_NONE);
    eassert(!vm.buffer[2]);

    ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(vm_buffer_token_test);
    etest_run(vm_buffer_tokens_test);
    etest_run(vm_buffer_tokens_and_test);
    etest_run(vm_buffer_tokens_piped_test);
    etest_run(vm_buffer_tokens_redirected_test);
    etest_run(vm_buffer_if_test);
    etest_run(vm_buffer_if_multiple_condition_test);
    etest_run(vm_buffer_if_else_true_test);
    etest_run(vm_buffer_if_else_false_test);

    etest_finish();

    return EXIT_SUCCESS;
}
