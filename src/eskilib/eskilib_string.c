/* Copyright eskilib by Alex Eski 2024 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "eskilib_defines.h"
#include "eskilib_string.h"

eskilib_nodiscard bool eskilib_string_compare(char* str, size_t str_len, char* str_two, size_t str_two_len);
/*{
    return str_len == str_two_len && memcmp(str, str_two, str_len) == 0;
}*/

eskilib_nodiscard bool eskilib_string_contains(const char* string, size_t string_length, const char* string_two,
                                               size_t string_two_length)
{
    assert(string);
    assert(string_length >= 2);
    assert(string_two);
    assert(string_two_length >= 2);

    if (!string || !string_two || string_length < 2 || string_two_length < 2)
        return false;

    assert(string[string_length - 1] == '\0');
    assert(string_two[string_two_length - 1] == '\0');

    if (string_length < string_two_length)
        return false;

    char* stringValue = (char*)string;

    const char* a;
    const char* b;

    b = string_two;

    if (*b == '\0')
        return true;

    for (size_t i = 0; i < string_length && *stringValue != '\0'; i++, stringValue += 1) {
        if (*stringValue != *b)
            continue;

        a = stringValue;
        for (size_t j = 0; j < string_two_length; j++) {
            if (*b == '\0') {
                return true;
            }
            if (*a++ != *b++) {
                break;
            }
        }
        b = string_two;
    }

    return false;
}
