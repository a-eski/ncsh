#include "../lib/arena_test_helper.h"
#include "../../src/eskilib/str.h"

void estrtrim_bench()
{
    ARENA_TEST_SETUP;

    Str data;
    estrset(&data, &Str_New_Literal("input   "), &a);

    estrtrim(&data);

    ARENA_TEST_TEARDOWN;
}

int main()
{
    estrtrim_bench();
}
