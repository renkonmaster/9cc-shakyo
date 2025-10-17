#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s foo.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'
assert 0 '0+2<1;'

assert 6 "a=6;"
assert 14 "a = 3;b = 5 * 6 - 8;a + b / 2;"
assert 1 "ab = 1;"
assert 80 "v1 = 100; v2 = v1 / 10; v3 = v1 - v2 * 2; v3;"

assert 5 "return 5; return 8;"
assert 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"
assert 12 "a = 4; b = 18; c = a * b / 2; return c / 3;"
assert 1 "return 1; a = 3;"

assert 6 "a = 2; if (1) b = a * 3; else b = a * 4; return b;"
assert 8 "a = 2; if (0) b = a * 3; else b = a * 4; return b;"
assert 8 "a = 2; if (a - 2) b = a * 3; else b = a * 4; return b;"

assert 10 "i=0; while (i<10) i = i + 1; return i;"
assert 16 "i=1; while (i<=8) i = i * 2; return i;"

assert 21 "sum=0; for (i=1;i<=6;i=i+1) sum=sum+i; return sum;"
assert 55 "sum=0; for (i=1;i<=10;i=i+1) sum=sum+i; return sum;"

assert 3 "{1;2; return 3;}"
assert 11 "if (1) {i = 1; i = i + 10;} return i;"
assert 15 "i = 1; j = 0; while (i <= 5) { j = j + i; i = i + 1; } return j;"

assert 13 "foo();"

echo OK