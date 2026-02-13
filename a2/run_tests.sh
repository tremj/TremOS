#!/bin/bash

# -------- CONFIG --------
SRC_DIR="./src"              # path to mysh source
TEST_DIR="./test-cases"      # path to test cases
SHELL_NAME="mysh"
# ------------------------

echo "== Building shell =="
cd "$SRC_DIR"
make clean
make mysh
cd - > /dev/null   # return to root dir

echo
echo "== Running tests =="
echo

FAILURES=0

cd "$TEST_DIR"

for testfile in T_*.txt; do
    # Skip *_result.txt files
    [[ "$testfile" == *_result.txt ]] && continue
    [[ "$testfile" == *_result2.txt ]] && continue

    base=$(basename "$testfile" .txt)
    expected1="${base}_result.txt"
    expected2="${base}_result2.txt"
    output="$(mktemp)"

    if [[ ! -f "$expected1" && ! -f "$expected2" ]]; then
        echo "[SKIP] $base (missing result file)"
        continue
    fi

    # Run the shell from src while inside test-cases
    "../src/$SHELL_NAME" < "$testfile" > "$output"

    PASS=false

    if [[ -f "$expected1" ]] && diff -q "$expected1" "$output" > /dev/null; then
        PASS=true
    fi

    if [[ "$PASS" = false && -f "$expected2" ]] && diff -q "$expected2" "$output" > /dev/null; then
        PASS=true
    fi

    if [[ "$PASS" = true ]]; then
        echo "[PASS] $base"
    else
        echo "[FAIL] $base"
        FAILURES=$((FAILURES + 1))
        FAILED_TESTS+=("$base")
    fi

    rm "$output"
done

echo
echo "== Test Summary =="
echo "Passed: $(( $(ls T_*.txt 2>/dev/null | grep -v '_result' | wc -l) - FAILURES ))"
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
