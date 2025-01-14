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

char *eskilib_string_copy(char *dest, char *source, const size_t maxStringSize);

// checks if two strings are equivalent and returns true if they are, false otherwise.
bool eskilib_string_equals(char *stringOne, char *stringTwo, const size_t maxStringSize);

// returns values similar to strcmp from string.h, return value == 0 if strings match.
int_fast32_t eskilib_string_compare(char *stringOne, char *stringTwo, const size_t maxStringSize);

bool eskilib_string_contains_unsafe(const char *string, const char *substring);
bool eskilib_string_contains(const struct eskilib_String string, const struct eskilib_String substring);
bool eskilib_string_contains_s(const char *string, size_t string_length, const struct eskilib_String substring);
bool eskilib_string_contains_s2(const char *string, size_t string_length, const char *string_two,
                                size_t string_two_length);

#endif // !eskilib_string_h
