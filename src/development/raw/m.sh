#!/bin/env bash

gcc -std=c2x -Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wformat=2 -Wshadow -Wvla -fstack-protector-all -fsanitize=address,undefined,leak -g min.c ../../ncsh_terminal.c -o msh
