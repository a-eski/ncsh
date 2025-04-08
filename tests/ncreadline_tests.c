#include <stdlib.h>

#include "../src/eskilib/eskilib_test.h"
#include "../src/readline/ncreadline.h" // NOTE: is used for macro

volatile int sigwinch_caught;

/* ncreadline_prompt_size forward declaration
 * get the prompt size accounting for prompt length, user length, and cwd length
 * Returns: length of the prompt
 */
size_t ncreadline_prompt_size(const size_t user_len, const size_t dir_len);

const char user[] = "alex";
const char dir[] = "/home/alex";

void ncreadline_prompt_size_only_prompt_test()
{
    size_t result = ncreadline_prompt_size(0, 0);
    eskilib_assert(result == NCSH_PROMPT_ENDING_STRING_LENGTH);
}

void ncreadline_prompt_size_only_prompt_and_user_test()
{
    size_t result = ncreadline_prompt_size(sizeof(user), 0);
    eskilib_assert(result == NCSH_PROMPT_ENDING_STRING_LENGTH + sizeof(user) - 1);
}

void ncreadline_prompt_size_only_prompt_and_dir_test()
{
    size_t result = ncreadline_prompt_size(0, sizeof(dir));
    eskilib_assert(result == NCSH_PROMPT_ENDING_STRING_LENGTH + sizeof(dir) - 1);
}

void ncreadline_prompt_size_all_test(void)
{
    size_t result = ncreadline_prompt_size(sizeof(user), sizeof(dir));
    eskilib_assert(result == NCSH_PROMPT_ENDING_STRING_LENGTH + sizeof(user) - 1 + sizeof(dir));
}

void ncreadline_tests(void)
{
    eskilib_test_start();

    eskilib_test_run(ncreadline_prompt_size_only_prompt_test);
    eskilib_test_run(ncreadline_prompt_size_only_prompt_and_user_test);
    eskilib_test_run(ncreadline_prompt_size_only_prompt_and_dir_test);
    eskilib_test_run(ncreadline_prompt_size_all_test);

    eskilib_test_finish();
}

int main(void)
{
    ncreadline_tests();

    return EXIT_SUCCESS;
}
