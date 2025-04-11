#include <stdlib.h>

#include "../src/config.h"
#include "../src/eskilib/etest.h"

void config_alias_check_no_alias_test(void)
{
    char buffer[] = "echo hello";
    struct estr result = config_alias_check(buffer, sizeof(buffer));

    eassert(!result.length);
    eassert(!result.value);
}

void config_alias_check_alias_found_test(void)
{
    char buffer[] = "n"; // alias for nvim
    struct estr result = config_alias_check(buffer, sizeof(buffer));

    char expected_result[] = "nvim";
    eassert(result.length == sizeof(expected_result));
    eassert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void config_alias_check_alias_found_multiple_chars_test(void)
{
    char buffer[] = "fd"; // alias for fdfind
    struct estr result = config_alias_check(buffer, sizeof(buffer));

    char expected_result[] = "fdfind";
    eassert(result.length == sizeof(expected_result));
    eassert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void config_tests(void)
{
    etest_start();

    etest_run(config_alias_check_no_alias_test);
    etest_run(config_alias_check_alias_found_test);
    etest_run(config_alias_check_alias_found_multiple_chars_test);

    etest_finish();
}

int main(void)
{
    config_tests();

    return EXIT_SUCCESS;
}
