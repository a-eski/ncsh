#ifndef eskilib_string_h
#define eskilib_string_h

#include <stdint.h>
#include <stdbool.h>

struct eskilib_String {
	uint_fast8_t length;
	char* value;
};

char* eskilib_string_copy(char* dest, char* source, const uint_fast32_t maxBufferSize);

//checks if two strings are equivalent and returns true if they are, false otherwise.
bool eskilib_string_equals(char* stringOne, char* stringTwo, const uint_fast32_t maxStringSize);

//returns values similar to strcmp from string.h, return value == 0 if strings match.
int_fast32_t eskilib_string_compare(char* stringOne, char* stringTwo, const uint_fast32_t maxStringSize);

#endif // !eskilib_string_h

