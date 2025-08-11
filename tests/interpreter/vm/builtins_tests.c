#include <stdlib.h>

int builtins_echo(char** restrict buffer, size_t* restrict buf_lens);

void builtins_echo_test()
{

}

void builtins_echo_no_newline_test()
{

}

void builtins_echo_many_spaces_test()
{

}

void builtins_tests()
{

}

#ifndef TEST_ALL
int main()
{
    builtins_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
