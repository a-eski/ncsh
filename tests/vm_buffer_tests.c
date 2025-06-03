#include <stdlib.h>

#include "../src/eskilib/etest.h"
#include "../src/vm/vm_types.h"
#include "../src/vm/vm_buffer.h"
// #include "../src/vm/preprocessor.h"

void vm_buffer_args_test()
{
    Token_Data tokens = {0};

}

int main()
{
    etest_start();

    etest_run(vm_buffer_args_test);

    etest_finish();

    return EXIT_SUCCESS;
}
