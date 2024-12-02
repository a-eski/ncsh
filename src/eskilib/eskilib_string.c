/* Copyright eskilib by Alex Eski 2024 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "eskilib_string.h"

char* eskilib_string_copy(char* dest, char* source, const uint_fast32_t maxBufferSize) {
	char* originalStringToSave = source;

	for (uint_fast32_t i = 0;
		i < maxBufferSize && (*dest = *source) != '\0';
		i++, ++dest, ++source);

	return (originalStringToSave);
}

[[nodiscard]]
bool eskilib_string_equals(char* stringOne, char* stringTwo, const uint_fast32_t maxStringSize) {
	const unsigned char *p1 = (const unsigned char*)stringOne;
	const unsigned char *p2 = (const unsigned char*)stringTwo;

	for (uint_fast32_t i = 0; i <= maxStringSize && *p1 && *p1 == *p2; i++) {
		if (i == maxStringSize)
			return -1;

		++p1, ++p2;
	}

	return (( *p1 > *p2 ) - ( *p2  > *p1 )) == 0;
}

[[nodiscard]]
int_fast32_t eskilib_string_compare(char* stringOne, char* stringTwo, const uint_fast32_t maxStringSize) {
	const unsigned char *p1 = (const unsigned char*)stringOne;
	const unsigned char *p2 = (const unsigned char*)stringTwo;

	for (uint_fast32_t i = 0; i <= maxStringSize && *p1 && *p1 == *p2; i++) {
		if (i == maxStringSize)
			return -1;

		++p1, ++p2;
	}

	return ( *p1 > *p2 ) - ( *p2  > *p1 );
}

bool eskilib_string_contains_unsafe(const char* string, const char* substring) {
	const char* a;
	const char* b;

	b = substring;

	if (*b == '\0')
		return true;

	for (; *string != '\0'; string += 1) {
		if (*string != *b)
			continue;

		a = string;
		while (1) {
			if (*b == 0) {
				return (char*)string;
			}
			if (*a++ != *b++) {
				break;
			}
		}
		b = substring;
	}

	return false;
}

bool eskilib_string_contains(const struct eskilib_String string, const struct eskilib_String substring) {
	if (string.length < substring.length)
		return false;

	char* stringValue = string.value;

	const char* a;
	const char* b;

	b = substring.value;

	if (*b == '\0')
		return true;

	for (uint_fast32_t i = 0; i < string.length && *stringValue != '\0'; i++, stringValue += 1) {
		if (*stringValue != *b)
			continue;

		a = stringValue;
		for (uint_fast32_t j = 0; j < substring.length; j++) {
			if (*b == '\0') {
				return true;
			}
			if (*a++ != *b++) {
				break;
			}
		}
		b = substring.value;
	}

	return false;
}

bool eskilib_string_contains_s(const char* string, uint32_t string_length, const struct eskilib_String substring) {
	if (string_length < substring.length)
		return false;

	char* stringValue = (char*)string;

	const char* a;
	const char* b;

	b = substring.value;

	if (*b == '\0')
		return true;

	for (uint_fast32_t i = 0; i < string_length && *stringValue != '\0'; i++, stringValue += 1) {
		if (*stringValue != *b)
			continue;

		a = stringValue;
		for (uint_fast32_t j = 0; j < substring.length; j++) {
			if (*b == '\0') {
				return true;
			}
			if (*a++ != *b++) {
				break;
			}
		}
		b = substring.value;
	}

	return false;
}

