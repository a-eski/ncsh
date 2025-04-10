#include <stdlib.h>

#include "../src/config.h"
#include "../src/eskilib/eskilib_test.h"

void config_alias_check_no_alias_test(void)
{
    char buffer[] = "echo hello";
    struct eskilib_String result = config_alias_check(buffer, sizeof(buffer));

    eskilib_assert(!result.length);
    eskilib_assert(!result.value);
}

void config_alias_check_alias_found_test(void)
{
    char buffer[] = "n"; // alias for nvim
    struct eskilib_String result = config_alias_check(buffer, sizeof(buffer));

    char expected_result[] = "nvim";
    eskilib_assert(result.length == sizeof(expected_result));
    eskilib_assert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void config_alias_check_alias_found_multiple_chars_test(void)
{
    char buffer[] = "fd"; // alias for fdfind
    struct eskilib_String result = config_alias_check(buffer, sizeof(buffer));

    char expected_result[] = "fdfind";
    eskilib_assert(result.length == sizeof(expected_result));
    eskilib_assert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void config_tests(void)
{
    eskilib_test_start();

    eskilib_test_run(config_alias_check_no_alias_test);
    eskilib_test_run(config_alias_check_alias_found_test);
    eskilib_test_run(config_alias_check_alias_found_multiple_chars_test);

    eskilib_test_finish();
}

int main(void)
{
    config_tests();

    return EXIT_SUCCESS;
}
