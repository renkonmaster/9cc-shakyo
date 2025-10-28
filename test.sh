#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s func.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual ✅"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 3 "
int foo() { return 3; }
int main() { return foo(); }
"

assert 5 "
int f() { return 5; }
int main() { return f(); }
"

assert 8 "
int sum(int a, int b) { return a + b; }
int main() { return sum(3, 5); }
"

assert 30 "
int mul(int a, int b) { return a * b; }
int main() { return mul(5, 6); }
"

assert 27 "
int div(int a, int b) { return a / b; }
int sub(int a, int b) { return a - b; }
int main() { return div(84, 4) + sub(10, 4); }
"

assert 21 "
int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }
int main() { return add6(1, 2, 3, 4, 5, 6); }
"

assert 4 "
int foo(int x) { return x + 1; }
int main() { return foo(foo(foo(1))); }
"

assert 1 "
int main() {
    int a;
    a = 1;
    return a;
}
"

assert 6 "
int main() {
  int a;
  a = 1;
  int b;
  b = 2;
  int c;
  c = 3;
  return a + b + c;
}
"

assert 15 "
int hoge(int x) {
    int y;
    y = x + 2;
    return y * 3;
}
int main() {
    return hoge(3);
}
"

assert 21 "
int fib(int n) {
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}
int main() {
    return fib(8);
}
"

assert 55 "
int fib(int n) {
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}
int main() {
    return fib(10);
}
"

assert 3 "
int main() {
    int x;
    x = 3;
    int y;
    y = &x;
    return *y;
}
"

assert 3 "
int main() {
    int x;
    int y;
    x = 3;
    y = &x;
    return *y;
}
"

assert 9 "
int main() {
    int x;
    int y;
    int p;
    int q;
    x = 3;
    y = 4;
    p = &x;
    q = &y;
    return *p + *q + 2;
}"

assert 3 "
int main() {
    int x;
    int *p;
    p = &x;
    *p = 3;
    return x;
}"

assert 8 "
int main() {
    int x;
    int y;
    int *p;
    int *q;
    p = &x;
    q = &y;
    *p = 3;
    *q = 5;
    return x + y;
}"

assert 4 "
int main() {
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    int *q;
    q = p + 2;
    return *q;
}"

assert 8 "
int main() {
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    int *q;
    q = p + 3;
    return *q;
}"

assert 2 "
int main() {
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    int *q;
    q = p + 3;
    q = q - 2;
    return *q;
}"

assert 3 "
int sum(int *p, int *q) {
    return *p + *q;
}
int main() {
    int x;
    x = 1;
    int y;
    y = 2;
    return sum(&x, &y);
}"

assert 4 "
int main() {
    int x;
    x = 3;
    int y;
    y = sizeof(x);
    return y;
}"

assert 8 "
int main() {
    int x;
    x = 3;
    int y;
    y = sizeof(&x);
    return y;
}"

assert 4 "
int main() {
    int x;
    return sizeof(sizeof(sizeof(x)));
}"

assert 3 "
int main() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p;
    p = a;
    return *p + *(p + 1);
}"

assert 8 "
int main() {
    int arr[3];
    int *p;
    p = arr;
    *p = 3; 
    *(p + 1) = 5; 
    *(p + 2) = 0;
    return *arr + *(arr + 1) + *(arr + 2);
}"

assert 15 "
int sum(int *arr) { 
    return *arr + *(arr + 1) + *(arr + 2);
}

int main() {
    int data[3];
    
    *data = 10;
    *(data + 1) = 5;
    *(data + 2) = 0;
    
    return sum(data);
}"

assert 3 "
int main() {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    return a[0] + a[1];
}"

assert 9 "
int main() {
    int a[3];
    a[0] = 3;
    a[1] = 5;
    a[2] = 1;
    return a[0] + a[1] + a[2];
}"

assert 57 "
int a;
int main() {
    return 57;
}"

assert 99 "
int a;
int b;
int main() {
    a = 40;
    b = 59;
    return a + b;
}"

assert 57 "
int a;
int main() { 
    a = 19;
    return a * 3;
}"

assert 3 "
int main() { char x; return 3; }"

assert 3 "
int main() {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    return x[0] + y;  
}"

# string literals test
assert 25 '
int main() {
    char *s;
    char *c;
    c = " ignored ";
    s = "Hello";
    return 25;
}'

assert 0 'int main() { char *s; s = "Hello world!"; printf("%s\n", s); return 0; }'

echo "✅ all tests passed"