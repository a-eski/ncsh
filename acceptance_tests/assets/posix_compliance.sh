#!/bin/sh

## TODO: continue work on posix compliance script
## or incorporate into acceptance tests

# Define the shell to test
TEST_SHELL_NAME="ncsh"
TEST_SHELL="../../bin/ncsh"
POSIX_SHELL="/bin/sh"

# Define a set of commands to test
COMMANDS=(
    "echo 'Hello, world!'"
    "pwd"
    "cd /tmp && pwd"
    "export TEST_VAR=123; echo $TEST_VAR"
    "unset TEST_VAR; echo $TEST_VAR"
    "test -f /etc/passwd && echo 'File exists'"
    "[ 1 -eq 1 ] && echo 'True'"
    "echo 'A' | tr 'A' 'B'"
    "for i in 1 2 3; do echo \"Loop $i\"; done"
    "printf 'Formatted %s\\n' 'output'"
)

# Run tests
echo "Testing POSIX compliance of $TEST_SHELL_NAME"
echo "-------------------------------------"
for cmd in "${COMMANDS[@]}"; do
    echo "Testing: $cmd"
    EXPECTED_OUTPUT=$($POSIX_SHELL -c "$cmd")
    ACTUAL_OUTPUT=$($TEST_SHELL -c "$cmd")

    if [ "$EXPECTED_OUTPUT" = "$ACTUAL_OUTPUT" ]; then
        echo "✅ Passed"
    else
        echo "❌ Failed"
        echo "  Expected: $EXPECTED_OUTPUT"
        echo "  Got:      $ACTUAL_OUTPUT"
    fi
    echo
done

echo "Testing complete!"
