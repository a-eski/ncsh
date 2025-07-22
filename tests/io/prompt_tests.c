#include <stdlib.h>

#include "../../src/eskilib/etest.h"
#include "../../src/io/io.h" // NOTE: is used for macro

volatile int sigwinch_caught;

/* prompt_size forward declaration
 * get the prompt size accounting for prompt length, user length, and cwd length
 * Returns: length of the prompt
 */
size_t prompt_size(const size_t user_len, const size_t dir_len);

const char user[] = "alex";
const char dir[] = "/home/alex";

void prompt_size_only_prompt_test()
{
    size_t result = prompt_size(0, 0);
    eassert(result == NCSH_PROMPT_ENDING_STRING_LENGTH);
}

void prompt_size_only_prompt_and_user_test()
{
    size_t result = prompt_size(sizeof(user), 0);
    eassert(result == NCSH_PROMPT_ENDING_STRING_LENGTH + sizeof(user) - 1);
}

void prompt_size_only_prompt_and_dir_test()
{
    size_t result = prompt_size(0, sizeof(dir));
    eassert(result == NCSH_PROMPT_ENDING_STRING_LENGTH + sizeof(dir) - 1);
}

void prompt_size_all_test()
{
    size_t result = prompt_size(sizeof(user), sizeof(dir));
    eassert(result == NCSH_PROMPT_ENDING_STRING_LENGTH + sizeof(user) - 1 + sizeof(dir));
}

void prompt_tests()
{
    etest_start();

    etest_run(prompt_size_only_prompt_test);
    etest_run(prompt_size_only_prompt_and_user_test);
    etest_run(prompt_size_only_prompt_and_dir_test);
    etest_run(prompt_size_all_test);

    etest_finish();
}

#ifndef TEST_ALL
int main()
{
    prompt_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
