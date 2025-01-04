/* Copyright eskilib by Alex Eski 2024 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#include "eskilib_string.h"

#if __has_c_attribute(nodiscard)
#if __STDC_VERSION__ >= 202000
#define eskilib_nodiscard [[nodiscard]]
#else
#define eskilib_nodiscard
#endif
#else
#define eskilib_nodiscard
#endif

eskilib_nodiscard
char* eskilib_string_copy(char* dest, char* source, const size_t maxStringSize) {
	assert(dest != NULL);
	assert(source != NULL);
	if (maxStringSize == 0)
		return NULL;

	char* originalStringToSave = source;

	for (size_t i = 0;
		i < maxStringSize && (*dest = *source) != '\0';
		i++, ++dest, ++source);

	return (originalStringToSave);
}

eskilib_nodiscard
bool eskilib_string_equals(char* stringOne, char* stringTwo, const size_t maxStringSize) {
	assert(stringOne != NULL);
	assert(stringTwo != NULL);
	if (maxStringSize == 0)
		return false;

	const unsigned char *p1 = (const unsigned char*)stringOne;
	const unsigned char *p2 = (const unsigned char*)stringTwo;

	for (size_t i = 0; i <= maxStringSize && *p1 && *p1 == *p2; i++) {
		if (i == maxStringSize)
			return -1;

		++p1, ++p2;
	}

	return (( *p1 > *p2 ) - ( *p2  > *p1 )) == 0;
}


eskilib_nodiscard
int_fast32_t eskilib_string_compare(char* stringOne, char* stringTwo, const size_t maxStringSize) {
	assert(stringOne != NULL);
	assert(stringTwo != NULL);
	if (maxStringSize == 0)
		return -1;

	const unsigned char *p1 = (const unsigned char*)stringOne;
	const unsigned char *p2 = (const unsigned char*)stringTwo;

	for (size_t i = 0; i <= maxStringSize && *p1 && *p1 == *p2; i++) {
		if (i == maxStringSize)
			return -1;

		++p1, ++p2;
	}

	return ( *p1 > *p2 ) - ( *p2  > *p1 );
}


eskilib_nodiscard
bool eskilib_string_contains_unsafe(const char* string, const char* substring) {
	assert(string != NULL);
	assert(substring != NULL);

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


eskilib_nodiscard
bool eskilib_string_contains(const struct eskilib_String string, const struct eskilib_String substring) {
	assert(string.value != NULL);
	assert(substring.value != NULL);

	if (string.length == 0 || substring.length == 0)
		return false;

	assert(string.value[string.length - 1] == '\0');
	assert(substring.value[substring.length - 1] == '\0');
	if (string.length < substring.length)
		return false;

	char* stringValue = string.value;

	const char* a;
	const char* b;

	b = substring.value;

	if (*b == '\0')
		return true;

	for (size_t i = 0; i < string.length && *stringValue != '\0'; i++, stringValue += 1) {
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


eskilib_nodiscard
bool eskilib_string_contains_s(const char* string, size_t string_length, const struct eskilib_String substring) {
	assert(string != NULL);
	assert(substring.value != NULL);

	if (string_length == 0 || substring.length == 0)
		return false;

	assert(string[string_length - 1] == '\0');
	assert(substring.value[substring.length - 1] == '\0');

	if (string_length < substring.length)
		return false;

	char* stringValue = (char*)string;

	const char* a;
	const char* b;

	b = substring.value;

	if (*b == '\0')
		return true;

	for (size_t i = 0; i < string_length && *stringValue != '\0'; i++, stringValue += 1) {
		if (*stringValue != *b)
			continue;

		a = stringValue;
		for (size_t j = 0; j < substring.length; j++) {
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


eskilib_nodiscard
bool eskilib_string_contains_s2(const char* string, size_t string_length, const char* string_two, size_t string_two_length) {
	assert(string != NULL);
	assert(string_length >= 2);
	assert(string_two != NULL);
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
