#!/bin/bash

# -------- CONFIG --------
SRC_DIR="./src"
TEST_DIR="./a3-test-cases"
SHELL_NAME="mysh"
# ------------------------

echo "== Running tests =="
echo

FAILURES=0
FAILED_TESTS=()

for i in {1..5}; do
    testfile="tc${i}.txt"
    expected="tc${i}_result.txt"

    if [[ ! -f "$TEST_DIR/$testfile" || ! -f "$TEST_DIR/$expected" ]]; then
        echo "[SKIP] tc${i} (missing files)"
        FAILURES=$((FAILURES + 1))
        FAILED_TESTS+=("tc${i}")
        continue
    fi

    # ---------------------------------------
    # Extract frame/variable store sizes
    # ---------------------------------------
    first_line=$(head -n 1 "$TEST_DIR/$expected")

    # Extract numbers using regex
    framesize=$(echo "$first_line" | sed -n 's/.*Frame Store Size = \([0-9]*\).*/\1/p')
    varmemsize=$(echo "$first_line" | sed -n 's/.*Variable Store Size = \([0-9]*\).*/\1/p')

    if [[ -z "$framesize" || -z "$varmemsize" ]]; then
        echo "[FAIL] tc${i} (could not parse sizes)"
        FAILURES=$((FAILURES + 1))
        FAILED_TESTS+=("tc${i}")
        continue
    fi

    # ---------------------------------------
    # Build with extracted parameters
    # ---------------------------------------
    echo "== Building for tc${i} (F=$framesize, V=$varmemsize) =="

    cd "$SRC_DIR"
    make clean > /dev/null
    make mysh framesize="$framesize" varmemsize="$varmemsize" > /dev/null

    if [[ $? -ne 0 ]]; then
        echo "[FAIL] tc${i} (build failed)"
        cd - > /dev/null
        FAILURES=$((FAILURES + 1))
        FAILED_TESTS+=("tc${i}")
        continue
    fi

    cd - > /dev/null

    # ---------------------------------------
    # Run test
    # ---------------------------------------
    output="$(mktemp)"

    cd $TEST_DIR

    "./../src/$SHELL_NAME" < "$testfile" > "$output"

#    if diff -q "$expected" "$output" > /dev/null; then
    if diff -u "$expected" "$output"; then
        echo "[PASS] tc${i}"
    else
        echo "[FAIL] tc${i}"
        FAILURES=$((FAILURES + 1))
        FAILED_TESTS+=("tc${i}")
    fi

    rm "$output"
    cd ..
done

# ---------------------------------------
# Cleanup
# ---------------------------------------
echo
echo "== Cleaning up Shell =="
cd "$SRC_DIR"
make clean > /dev/null
cd - > /dev/null

# ---------------------------------------
# Summary
# ---------------------------------------
echo
echo "== Test Summary =="

TOTAL=5
PASSED=$((TOTAL - FAILURES))

echo "Passed: $PASSED"
echo "Failed: $FAILURES"

if [[ $FAILURES -ne 0 ]]; then
    echo
    echo "Failed test cases:"
    for t in "${FAILED_TESTS[@]}"; do
        echo " - $t"
    done
    exit 1
fi

echo "All tests passed 🎉"