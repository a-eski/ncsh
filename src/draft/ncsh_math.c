#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../eskilib/eskilib_test.h"
#include "../ncsh_parser.h"

struct ncsh_Ast;

struct ncsh_Ast {
    uint_fast8_t op;
    char* constant;
    struct ncsh_Ast* next;
};

struct ncsh_Ast* ncsh_math_tokenize(int pos, struct ncsh_Args* args)
{
    if (args->ops[pos] != OP_MATH_EXPRESSION_START)
        return NULL;

    struct ncsh_Ast* ast = malloc(sizeof(struct ncsh_Ast));
    struct ncsh_Ast* return_value = ast;
    for (uint_fast32_t i = 0; i < args->count && args->ops[i] != OP_MATH_EXPRESSION_END; ++i) {
        ast->op = args->ops[i];
        ast->next = malloc(sizeof(struct ncsh_Ast));
        ast = ast->next;
        /*switch (args->ops[i])
        {
            case OP_CONSTANT: {
                ast->op = OP_CONSTANT;
                break;
            }
        }*/
    }
    ast->op = OP_MATH_EXPRESSION_END;

    return return_value;
}

void ncsh_math_simple_addition_test(void)
{
    struct ncsh_Args args = {0};
    ncsh_parser_args_malloc(&args);
    char* line = "(1 + 1)";
    ncsh_parser_parse(line, (size_t)strlen(line) + 1, &args);

    bool result = ncsh_math_tokenize(0, &args);
    eskilib_assert(result);

    ncsh_parser_args_free(&args);
}

int main(void)
{
    eskilib_test_start();

    eskilib_test_run("ncsh_math_simple_addition_test", ncsh_math_simple_addition_test);

    eskilib_test_finish();
}
