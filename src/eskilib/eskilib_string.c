/* Copyright eskilib by Alex Eski 2024 */

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

