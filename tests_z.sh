#!/bin/env bash

set -e

cd src
cd z

chmod +x ./z.sh
./z.sh
./ztests

rm ztests _z_database.bin
