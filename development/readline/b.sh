#!/bin/env bash

gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -fsanitize=address,undefined,leak -g main.c -lreadline -lncsh_readline -o readline
