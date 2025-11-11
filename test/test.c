int a = 2 * 5 - 30 + 80 / 4;
int b = - 5 + 10;
int c = -10;

char *str = "hello world!!";
char *str2 = "Hooray!";

int arr[3] = {1, 2 * 5, -3};

int hoge() {
    return 42;
}

int test_operator_prec() {
    return 2 + 3 * 4 == 14;
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

int test_ptr1() {
    int arr[3];
    arr[0] = 10; 
    arr[1] = 20; 
    arr[2] = 30;
    return arr[1];
}

int test_ptr2() {
    int arr[3];
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    int *p;
    p = arr;
    return *(p + 2);
}

int test_arr() {
    int arr[4];
    arr[0] = 10;
    arr[1] = 30;
    arr[2] = 20;
    arr[3] = 40;
    return arr[0] * arr[1] * arr[2] * arr[3];
}

int test_arr2() {
    int arr[5];
    arr[2] = 120;
    return arr[2];
}

int test_str() {
    char *s;
    s = "hello world";
    printf("%s\n",s);
    return 0;
}

int test_bool() {
    int true = 1;
    int false = 0;
    if (0) return 1;
    if (false) return 1;
    if (!true) return 1;
    if (!!false) return 1;
    if (!(!(!true))) return 1;
    if (true && false) return 1;
    if (!true && !false) return 1;
    if (false && true && true) return 1;
    if (true && false || !true) return 1;
    if (false || !true || false) return 1;
    if (false || (false && true)) return 1;
    if (!(true || false)) return 1;
    if (!(true && (false || true)) || !true) return 1;
    
    if (!1) return 1;
    if (!!!1) return 1;
    if (!(1 && (0 || 1)) || !1) return 1;
    if ((true * false) && (true - false)) return 1;
    
    return 0;
}

int main() {
    //一度にすべてのテストコードをコンパイルする
    if (0 != a) return 1;

    if (5 != b) return 1;

    if (-10 != c) return 1;

    if (42 != hoge()) return 1;

    if (1 != test_operator_prec()) return 1;

    if (5 != foo(a, b)) return 1;

    if (5 != test_pointer()) return 1;

    if (0 != test_str()) return 1;

    if (240000 != test_arr()) return 1;

    if (120 != test_arr2()) return 1;

    if (20 != test_ptr1()) return 1;
    if (30 != test_ptr2()) return 1;

    printf("%s\n", str);
    printf("%s\n", str2);
    str2 = "Changed string!";
    printf("%s\n", str2);

    printf("%d %d %d\n", arr[0], arr[1], arr[2]);

    int lvar_init = 512;
    int *p = &lvar_init;
    char *lvar_string_init = "Local string literal initialize test";
    printf("%s\n", lvar_string_init);
    printf("lvar_init: %d\n", lvar_init);
    printf("pointer deref: %d\n", *p);

    int larr_init = 255;
    int larr[3] = {7, -14, larr_init};
    printf("arr init: %d %d %d\n", larr[0], larr[1], larr[2]);
    larr[1] = 42;
    printf("arr after assign: %d %d %d\n", larr[0], larr[1], larr[2]);
    int larr_idx = 1;
    larr[larr_idx] = 100;
    printf("arr after assign with idx var: %d %d %d\n", larr[0], larr[1], larr[2]);

    if (test_bool() == 1) return 1;

    for (int i = 0; i < 10; i = i + 1) {
        printf("for loop :%d ", i);
    }

    for (int j = 1; j <= 100; j = j * 2) {
        printf("<pow2 :%d> ", j);
    }

    for (int i = 3; i > 0; i = i - 1) {
        printf("countdown %d\n", i);
    }

    for(int i = 0; i < 5; i = i + 1) {
        if (i == 3) {
            printf("break at %d\n", i);
            break;
        }
        printf("loop %d\n", i);
    }

    for(int i = 0; i < 5; i = i + 1) {
        if (i == 2) {
            printf("continue at %d\n", i);
            continue;
        }
        printf("loop %d\n", i);
    }

    printf("✅ All tests passed!\n");
    return 0;
}