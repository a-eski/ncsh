/* Copyright eskilib by Alex Eski 2024 */

#ifndef eskilib_string_h
#define eskilib_string_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define eskilib_String_Empty ((const struct eskilib_String){.value = NULL, .length = 0})

struct eskilib_String
{
    size_t length;
    char *value;
};

bool eskilib_string_compare(char *str, size_t str_len, char *str_two, size_t str_two_len);

bool eskilib_string_contains(const char *string, size_t string_length, const char *string_two, size_t string_two_length);

#endif // !eskilib_string_h
