#!/bin/bash

cd src
cd z
rm _z_database.bin
rm ./ztests

set -e
./z.sh &&
./ztests

rm _z_database.bin
rm ./ztests
