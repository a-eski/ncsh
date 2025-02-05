#!/bin/env bash

gcc -std=c2x -Wall -Wextra -Werror -pedantic-errors -Wformat=2 -O3 -DNDEBUG main.c -lncurses -o cash

