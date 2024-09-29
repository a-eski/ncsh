#!/bin/bash

./tests.sh > test_output.txt &&
sed -i -e '$a\' test_output.txt &&
sed -i -e '$a\' expected_test_output.txt &&
ls &&
diff test_output.txt expected_test_output.txt

