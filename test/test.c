int a = 2 * 5 - 30 + 80 / 4;
int b = - 5 + 10;
int c = -10;

int hoge() {
    return 42;
}

int foo(int a, int b) {
    return 2* a + b;
}

int test_pointer() {
    int x;
    x = 3;
    int *p;
    p = &x;
    return *p + 2;
}

int test_arr() {
    int arr[3];
    arr[0] = 10;
    arr[1] = 30;
    arr[2] = 20;
    return arr[0] * arr[1] * arr[2];
}

int main() {
    //一度にすべてのテストコードをコンパイルする
    if (0 != a) {
        return 1;
    }

    if (5 != b) {
        return 1;
    }

    if (-10 != c) {
        return 1;
    }

    if (42 != hoge()) {
        return 1;
    }

    if (5 != foo(a, b)) {
        return 1;
    }

    if (5 != test_pointer()) {
        return 1;
    }


    printf("✅ All tests passed!\n");
    return 0;
}