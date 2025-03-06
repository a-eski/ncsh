#!/bin/env bash

# run z unit tests.

set -e

cd src
cd z

chmod +x ./z.sh
./z.sh
./ztests

rm ztests _z_database.bin
