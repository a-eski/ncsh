// Copyright (c) eskilib by Alex Eski 2024

#include "eskilib_file.h"
#include "eskilib_defines.h"

// simple fgets implementation that returns the number of characters read
eskilib_nodiscard int eskilib_fgets(char* const input_buffer,
                                    const size_t size_of_input_buffer,
                                    FILE* const restrict file_pointer)
{
    register int character;
    register char* buffer = input_buffer;
    int characters_read = 0;
    size_t characters_left_to_read = size_of_input_buffer;

    while (--characters_left_to_read > 0 && (character = getc(file_pointer)) != EOF) {
        ++characters_read;

        if ((*buffer = (char)character) == '\n')
            break;
        else
            ++buffer;
    }

    *buffer = '\0';
    return characters_read > 0 ? characters_read : EOF;
}

eskilib_nodiscard int eskilib_fgets_delimited(char* const input_buffer,
                                              const size_t size_of_input_buffer,
                                              FILE* const restrict file_pointer,
                                              const char delimiter)
{
    register int character;
    register char* buffer = input_buffer;
    int characters_read = 0;
    size_t characters_left_to_read = size_of_input_buffer;

    while (--characters_left_to_read > 0 && (character = getc(file_pointer)) != EOF) {
        ++characters_read;

        if ((*buffer = (char)character) == delimiter)
            break;
        else
            ++buffer;
    }

    *buffer = '\0';
    return characters_read > 0 ? characters_read : EOF;
}
