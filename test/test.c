int a = 2 * 5 - 30 + 80 / 4;
int b = - 5 + 10;
int c = -255;

char* s = "global string\n";

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

    if (0 != a) {
        return 1;
    }

    if (5 != b) {
        return 1;
    }

    if (-255 != c) {
        return 1;
    }

    printf("✅ All tests passed!\n");
    return 0;
}