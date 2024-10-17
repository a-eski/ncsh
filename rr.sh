#!/bin/bash

make clean
make RELEASE=1 &&
./bin/ncsh
