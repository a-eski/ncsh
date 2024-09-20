#include "shl_string.h"

char* shl_string_copy(char* dest, char* source, const uint_fast32_t maxBufferSize)
{
	char* originalStringToSave = source;

	for (uint_fast32_t i = 0;
		i < maxBufferSize && (*dest = *source) != '\0';
		i++, ++dest, ++source);

	return (originalStringToSave);
}

int_fast32_t shl_string_compare(char* stringOne, char* stringTwo, const uint_fast32_t maxBufferSize)
{
	const unsigned char *p1 = ( const unsigned char * )stringOne;
	const unsigned char *p2 = ( const unsigned char * )stringTwo;

	for (uint_fast32_t i = 0; i <= maxBufferSize && *p1 && *p1 == *p2; i++)
	{
		if (i == maxBufferSize)
			return -1;
		
		++p1, ++p2;
	}

	return ( *p1 > *p2 ) - ( *p2  > *p1 );
}

