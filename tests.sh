#!/bin/bash

chmod +x ./tests_h.sh
chmod +x ./tests_p.sh
chmod +x ./tests_ac.sh
chmod +x ./tests_z.sh

./tests_p.sh &&
./tests_h.sh &&
./tests_ac.sh &&
./tests_z.sh
