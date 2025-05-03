/* Copyright eskilib by Alex Eski 2024 */

#pragma once

#include <stdio.h>

int efgets(char* restrict buffer, size_t len, FILE* restrict file);

int efgets_delim(char* restrict buffer, size_t len, FILE* restrict file, char delimiter);
