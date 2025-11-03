#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./build/9cc "$input" > build/tmp.s
    cc -o build/tmp build/tmp.s build/lib/*.o

    "./build/tmp" > "build/tpm.out"
    actual_code="$?"
    actual_out=$(cat build/tpm.out)

    if [ "$actual_code" = "$expected" ]; then
        echo "$input => $actual_out ✅"
    else
        echo "$input => $expected expected, but got $actual_code"
        exit 1
    fi
}

for test_file in test/examples/*.c; do
    [ -f "$test_file" ] || continue

    expected=$(sed -n '1, 6p' "$test_file" | sed -nE 's/.*EXPECT:[[:space:]]*([0-9]+).*/\1/p' || true)

    if [ -z "$expected" ]; then
        base=$(basename "$test_file")
        expected="$(echo "$base" | sed -nE 's/([0-9]+)_.*/\1/p')"
    fi

    assert "$expected" "$test_file"
done