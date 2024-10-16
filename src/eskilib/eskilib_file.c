#include <stdint.h>
#include <stdio.h>

//simple fgets implementation that returns the number of characters read
int_fast32_t eskilib_fgets(char* input_buffer, int size_of_input_buffer, FILE* file_pointer) {
	register int character;
	register char* buffer = input_buffer;
	int_fast32_t characters_read = 0;

	while(--size_of_input_buffer > 0 && (character = getc(file_pointer)) != EOF) {
		++characters_read;

		if((*buffer = character) == '\n')
			break;
		else
			++buffer;
	}

	*buffer = '\0';
	return characters_read;
}
