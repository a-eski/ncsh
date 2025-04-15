/* Copyright eskilib by Alex Eski 2024 */

#pragma once

#include <stdint.h>
#include <stdio.h>

int efgets(char* const buffer, const size_t len, FILE* const restrict file);

int efgets_delim(char* const buffer, const size_t len, FILE* const restrict file, char delimiter);
