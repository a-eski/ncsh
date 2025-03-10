#include <stdlib.h>

#include "../eskilib/eskilib_test.h"
#include "../readline/ncsh_readline.h"

static const char user[] = "alex";
static const char dir[] = "/home/alex";

void ncsh_readline_prompt_size_only_prompt_test()
{
    size_t result = ncsh_readline_prompt_size(0, 0);
    eskilib_assert(result == NCSH_PROMPT_ENDING_STRING_LENGTH);
}

void ncsh_readline_prompt_size_only_prompt_and_user_test()
{
    size_t result = ncsh_readline_prompt_size(sizeof(user), 0);
    eskilib_assert(result == NCSH_PROMPT_ENDING_STRING_LENGTH + sizeof(user) - 1);
}

void ncsh_readline_prompt_size_only_prompt_and_dir_test()
{
    size_t result = ncsh_readline_prompt_size(0, sizeof(dir));
    eskilib_assert(result == NCSH_PROMPT_ENDING_STRING_LENGTH + sizeof(dir) - 1);
}

void ncsh_readline_prompt_size_all_test(void)
{
    size_t result = ncsh_readline_prompt_size(sizeof(user), sizeof(dir));
    eskilib_assert(result == NCSH_PROMPT_ENDING_STRING_LENGTH + sizeof(user) - 1 + sizeof(dir));
}

void ncsh_readline_tests(void)
{
    eskilib_test_start();

    eskilib_test_run("ncsh_readline_prompt_size_only_prompt_test", ncsh_readline_prompt_size_only_prompt_test);
    eskilib_test_run("ncsh_readline_prompt_size_only_prompt_and_user_test", ncsh_readline_prompt_size_only_prompt_and_user_test);
    eskilib_test_run("ncsh_readline_prompt_size_only_prompt_and_dir_test", ncsh_readline_prompt_size_only_prompt_and_dir_test);
    eskilib_test_run("ncsh_readline_prompt_size_all_test", ncsh_readline_prompt_size_all_test);

    eskilib_test_finish();
}

#ifndef ncsh_TEST_ALL
int main(void)
{
    ncsh_readline_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
