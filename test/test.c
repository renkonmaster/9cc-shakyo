int hoge() {
    return 42;
}

int test_string() {
    char *s;
    s = "Hello world!";
    printf("%s\n", s);
    return 0;
}

int main() {
    //一度にすべてのテストコードをコンパイルする
    if (42 != hoge()) {
        return 1;
    }

    if (0 != test_string()) {
        return 1;
    }

    printf("✅ All tests passed!\n");
    return 0;
}