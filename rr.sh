#!/bin/bash

make clean
make RELEASE=1 CLANG=1 &&
./ncsh
