int hoge() {
    return 42;
}

int test_string() {
    char *s;
    s = "Hello world!";
    printf("%s\n", s);
    return 0;
}

int foo(int a, int b) {
    return 2* a + b;
}

int main() {
    //一度にすべてのテストコードをコンパイルする
    if (42 != hoge()) {
        return 1;
    }

    if (0 != test_string()) {
        return 1;
    }

    if (foo(3, 4) != 10) {
        return 1;
    }

    char *z;
    z = "Hello 9cc!";

    printf("✅ All tests passed!\n");
    return 0;
}