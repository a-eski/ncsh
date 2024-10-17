#ifndef ncsh_autocomplete_h
#define ncsh_autocomplete_h

#include <stdint.h>

struct ncsh_Autocomplete {
	uint_fast32_t count;
	struct eskilib_String* entries;
};

#endif // !ncsh_autocomplete_h

