#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "✅ $input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 3 "
foo() { return 3; }
main() { return foo(); }
"

assert 5 "
f() { return 5; }
main() { return f(); }
"

assert 8 "
sum(a, b) { return a+ b; }
main() { return sum(3, 5); }
"

assert 30 "
mul(a, b) { return a * b; }
main() { return mul(5, 6); }
"

assert 27 "
div(a, b) { return a / b; }
sub(a, b) { return a - b; }
main() { return div(84, 4) + sub(10, 4); }
"

assert 21 "
add6(a, b, c, d, e, f) { return a + b + c + d + e + f; }
main() { return add6(1, 2, 3, 4, 5, 6); }
"

assert 4 "
foo(x) { return x + 1; }
main() { return foo(foo(foo(1))); }
"

assert 6 "
main() {
  a = 1;
  b = 2;
  c = 3;
  d = a + b + c;
  return d;
}
"

assert 15 "
hoge(x) {
    y = x + 2;
    return y * 3;
}
main() {
    return hoge(3);
}
"

assert 21 "
fib(n) {
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}
main() {
    return fib(8);
}
"

assert 55 "
fib(n) {
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}
main() {
    return fib(10);
}
"

echo "✅ all tests passed"