/* Copyright eskilib by Alex Eski 2024 */

#ifndef ESKILIB_STRING_H_
#define ESKILIB_STRING_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "eskilib_defines.h"

#define eskilib_String_Empty ((const struct eskilib_String){.value = NULL, .length = 0})

struct eskilib_String {
    size_t length;
    char* value;
};

// A simple wrapper for memcmp that checks if lengths match before calling memcmp.
eskilib_nodiscard
static inline
bool eskilib_string_compare(char* str, size_t str_len, char* str_two, size_t str_two_len)
{
    return str_len == str_two_len && memcmp(str, str_two, str_len) == 0;
}

bool eskilib_string_contains(const char* string, size_t string_length, const char* string_two,
                             size_t string_two_length);

#endif // !ESKILIB_STRING_H_
