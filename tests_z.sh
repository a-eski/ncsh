#!/bin/bash

set -e

cd src
cd z

chmod +x ./z.sh

set -e
./z.sh &&
./ztests

