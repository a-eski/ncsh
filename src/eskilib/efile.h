/* Copyright eskilib by Alex Eski 2024 */

#pragma once

#include <stdio.h>

#include "edefines.h"

int efgets(char* rst buffer, size_t len, FILE* rst file);

int efgets_delim(char* rst buffer, size_t len, FILE* rst file, char delimiter);
