int_fast32_t ncsh_fgets(char* input_buffer, int size_of_input_buffer, FILE* file_pointer) {
	register int character;
	register char* buffer;
	buffer = input_buffer;
	int_fast32_t characters_read = 0;

	while(--size_of_input_buffer > 0 && (character = getc(file_pointer)) != EOF) {
		// putchar(c);
		// fflush(stdout);
		++characters_read;
		// put the input char into the current pointer position, then increment it
		// if a newline entered, break
		*buffer = character;
		++buffer;
		if(character == '\n') {
			// puts("found next line");
			// fflush(stdout);
			break;
		}
	}

	printf("cs %s\n", buffer);
	*buffer = '\0';
	printf("cs_read %ld", characters_read);
	return (character == EOF && buffer == input_buffer) ? characters_read : EOF;
}
