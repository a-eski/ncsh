#ifndef shl_string_h
#define shl_string_h

#include <stdint.h>

char* shl_string_copy(char* dest, char* source, const uint_fast32_t maxBufferSize);

int_fast32_t shl_string_compare(char* stringOne, char* stringTwo, const uint_fast32_t maxBufferSize);

#endif // !shl_string_h

