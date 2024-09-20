#ifndef ncsh_string_h
#define ncsh_string_h

#include <stdint.h>

char* ncsh_string_copy(char* dest, char* source, const uint_fast32_t maxBufferSize);

int_fast32_t ncsh_string_compare(char* stringOne, char* stringTwo, const uint_fast32_t maxBufferSize);

#endif // !ncsh_string_h

