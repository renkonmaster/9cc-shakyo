#include "9cc.h"

char *user_input;
Token *token;
Node *functions[100];
int functions_count = 0;

int main(int argc, char **argv) {
    if (argc != 2) {
        error("%s: invalid number of arguments", argv[0]);
        return 1;
    }

    user_input = argv[1];
    token = tokenize();

    program();

    printf(".intel_syntax noprefix\n");
    printf(".section .note.GNU-stack,\"\",@progbits\n");

    printf(".data\n");
    for (GVar *gvar = globals; gvar; gvar = gvar->next) {
        printf("%s:\n", gvar->name);
        printf("  .zero %d\n", 8);
    }

    printf(".text\n");
    printf(".global main\n");

    for (int i = 0; i < functions_count; i++) {
        gen(functions[i]);
    }

    return 0;
}