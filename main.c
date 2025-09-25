#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) 
        error("%s: invalid number of arguments", argv[0]);

    codegen(argv[1]);
    return 0;
}