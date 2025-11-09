int test_ptr_add() {
    int arr[3];
    int *p;
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    p = arr;
    return *(p + 2);
}

int main() {
    if (30 != test_ptr_add()) {
        return 1;
    }

    return 0;
}