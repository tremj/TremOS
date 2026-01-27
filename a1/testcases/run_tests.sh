#!/bin/bash

# -------- CONFIG --------
SRC_DIR="../project/src"  # path to mysh source
TEST_DIR="."                 # current dir is A1/test-cases
SHELL_NAME="mysh"
# Directories/files to clean after all tests
CLEAN_DIRS=("test" "testdir" "toto")
# ------------------------

echo "== Building shell =="
cd "$SRC_DIR"
make clean
make mysh
cd - > /dev/null   # return to test-cases dir

# Copy mysh into test directory
cp "$SRC_DIR/$SHELL_NAME" "$TEST_DIR/$SHELL_NAME"

echo
echo "== Running tests =="
echo

FAILURES=0

for testfile in "$TEST_DIR"/*.txt; do
    # Skip *_result.txt files
    [[ "$testfile" == *_result.txt ]] && continue

    base=$(basename "$testfile" .txt)
    expected="$TEST_DIR/${base}_result.txt"
    output="$(mktemp)"

    if [[ ! -f "$expected" ]]; then
        echo "[SKIP] $base (missing result file)"
        continue
    fi

    # Run the shell with the test case
    "./$SHELL_NAME" < "$testfile" > "$output"

    if diff -u "$expected" "$output" > /dev/null; then
        echo "[PASS] $base"
    else
        echo "[FAIL] $base"
        echo "------ diff ------"
        diff -u "$expected" "$output"
        echo "------------------"
        FAILURES=$((FAILURES + 1))
    fi

    rm "$output"
done

# Clean up copied shell
rm "$SHELL_NAME"

# Clean up all directories/files created during tests
echo
echo "== Cleaning up test artifacts =="
for dir in "${CLEAN_DIRS[@]}"; do
    if [ -e "$dir" ]; then
        rm -rf "$dir"
        echo "Removed $dir"
    fi
done

echo
if [[ $FAILURES -eq 0 ]]; then
    echo "All tests passed 🎉"
else
    echo "$FAILURES test(s) failed ❌"
    exit 1
fi
