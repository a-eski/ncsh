#!/bin/env bash

# clang-format -style=file -i src/*.c src/*.h
# clang-format -style=file -i src/eskilib/*.c src/eskilib/*.h
# clang-format -style=file -i src/z/*.c src/z/*.h

find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;
