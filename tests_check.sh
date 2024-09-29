#!/bin/bash

echo "starting tests" &&
./tests.sh > test_output.txt &&
echo "made test_ouput" &&
ls -l &&
sed -i -e '$a\' ./test_output.txt &&
echo "appended newline to test_output.txt" &&
sed -i -e '$a\' ./expected_test_output.txt &&
echo "appended newline to expected_test_output.txt" &&
diff ./test_output.txt ./expected_test_output.txt &&
echo "tests passed"

