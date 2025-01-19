// Copyright (c) eskilib by Alex Eski 2024

#ifndef eskilib_file_h
#define eskilib_file_h

#include <stdint.h>
#include <stdio.h>

int eskilib_fgets(char *input_buffer, size_t size_of_input_buffer, FILE *file_pointer);

int eskilib_fgets_delimited(char *input_buffer, size_t size_of_input_buffer, FILE *file_pointer,
                                     char delimiter);

#endif // eskilib_file_h
