/* Copyright (c) eskilib by Alex Eski 2024 */

#ifndef ESKILIB_FILE_H_
#define ESKILIB_FILE_H_

#include <stdint.h>
#include <stdio.h>

int eskilib_fgets(char* const input_buffer, const size_t size_of_input_buffer, FILE* const restrict file_pointer);

int eskilib_fgets_delimited(char* const input_buffer, const size_t size_of_input_buffer, FILE* const restrict file_pointer, char delimiter);

#endif // ESKILIB_FILE_H_
