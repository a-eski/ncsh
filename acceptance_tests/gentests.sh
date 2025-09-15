#!/bin/env bash
cd ..
make debug DEFINES="-DNCSH_HISTORY_TEST -DZ_TEST" -B
cd acceptance_tests
rake
