#!/bin/bash

set -e
chmod +x ./tests.sh
echo "starting tests" &&
./tests.sh

rv=$?
if [[ $rv != 0 ]]
then
	echo "tests failed"
	exit 1
fi

echo "tests passed"
exit 0

