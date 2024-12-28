#!/bin/bash

set -e

make check
./tests_z.sh
./tests_it.sh
