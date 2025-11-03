#include "tokenize.h"
#include "parse.h"
#include "codegen.h"
#include "util.h"
#include "read.h"

Token *token;
Node *functions[100];

int main(int argc, char **argv) {
    if (argc != 2) {
        error("Usage: 9cc <filename>");
    }

    char *user_input = read_file(argv[1]);
    token = tokenize(user_input);

    program();

    printf(".intel_syntax noprefix\n");
    printf(".section .note.GNU-stack,\"\",@progbits\n");

    printf(".data\n");
    for (StringLiteral *str = string_literals; str; str = str->next) {
        static int str_count = 0;
        str->id = str_count;
        printf(".LC%d:\n", str_count++);
        printf("  .string \"%s\"\n", str->contents);
    }

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