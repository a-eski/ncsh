/* Copyright eskilib by Alex Eski 2024 */

#include "efile.h"
#include "edefines.h"

// simple fgets implementation that returns the number of characters read
enodiscard int efgets(char* rst input, size_t len, FILE* rst file)
{
    register int character;
    register char* buffer = input;
    int characters_read = 0;
    size_t characters_left_to_read = len;

    while (--characters_left_to_read > 0 && (character = getc(file)) != EOF) {
        ++characters_read;

        if ((*buffer = (char)character) == '\n')
            break;
        else
            ++buffer;
    }

    *buffer = '\0';
    return characters_read > 0 ? characters_read : EOF;
}

enodiscard int efgets_delim(char* rst input, size_t len, FILE* rst file, char delimiter)
{
    register int character;
    register char* buffer = input;
    int characters_read = 0;
    size_t characters_left_to_read = len;

    while (--characters_left_to_read > 0 && (character = getc(file)) != EOF) {
        ++characters_read;

        if ((*buffer = (char)character) == delimiter)
            break;
        else
            ++buffer;
    }

    *buffer = '\0';
    return characters_read > 0 ? characters_read : EOF;
}
