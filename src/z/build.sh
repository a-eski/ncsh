#!/bin/bash

gcc -std=c2x -Wall -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -g -DZ_TEST ../eskilib/eskilib_string.c ../eskilib/eskilib_file.c *.c -o z
echo "Compiled z"
