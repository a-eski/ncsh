#ifndef eskilib_file_h
#define eskilib_file_h

#include <stdint.h>
#include <stdio.h>

int_fast32_t eskilib_fgets(char* input_buffer, size_t size_of_input_buffer, FILE* file_pointer);

int_fast32_t eskilib_fgets_delimited(char* input_buffer, size_t size_of_input_buffer, FILE* file_pointer, char delimiter);

#endif // eskilib_file_h

